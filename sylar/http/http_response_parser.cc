/*
 * @Author: Nana5aki
 * @Date: 2025-04-20 15:06:14
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-04-25 22:44:36
 * @FilePath: /MySylar/sylar/http/http_response_parser.cc
 */
#include "http_response_parser.h"
#include "sylar/config.h"
#include "sylar/log.h"

namespace sylar {
namespace http {

static Logger::ptr g_logger = SYLAR_LOG_NAME("http");

static ConfigVar<uint64_t>::ptr g_http_response_buffer_size =
  Config::Lookup("http.response.buffer_size", (uint64_t)(4 * 1024), "http response buffer size");

static ConfigVar<uint64_t>::ptr g_http_response_max_body_size = Config::Lookup(
  "http.response.max_body_size", (uint64_t)(64 * 1024 * 1024), "http response max body size");

static uint64_t s_http_response_buffer_size = 0;
static uint64_t s_http_response_max_body_size = 0;

uint64_t HttpResponseParser::GetHttpResponseBufferSize() {
  return s_http_response_buffer_size;
}

uint64_t HttpResponseParser::GetHttpResponseMaxBodySize() {
  return s_http_response_max_body_size;
}

namespace {
struct _ResponseParseSizeIniter {
  _ResponseParseSizeIniter() {
    s_http_response_buffer_size = g_http_response_buffer_size->getValue();
    s_http_response_max_body_size = g_http_response_max_body_size->getValue();

    g_http_response_buffer_size->addListener(
      [](const uint64_t& ov, const uint64_t& nv) { s_http_response_buffer_size = nv; });

    g_http_response_max_body_size->addListener(
      [](const uint64_t& ov, const uint64_t& nv) { s_http_response_max_body_size = nv; });
  }
};
static _ResponseParseSizeIniter _init;
}   // namespace

/**
 * @brief http响应开始解析回调函数
 */
static int on_response_message_begin_cb(http_parser* p) {
  SYLAR_LOG_DEBUG(g_logger) << "on_response_message_begin_cb";
  return 0;
}

/**
 * @brief http响应头部字段解析结束，可获取头部信息字段，如status_code/version等
 * @note 返回0表示成功，返回1表示该HTTP消息无消息体，返回2表示无消息体并且该连接后续不会再有消息
 */
static int on_response_headers_complete_cb(http_parser* p) {
  SYLAR_LOG_DEBUG(g_logger) << "on_response_headers_complete_cb";
  HttpResponseParser* parser = static_cast<HttpResponseParser*>(p->data);
  parser->getData()->setVersion(((p->http_major) << 0x4) | (p->http_minor));
  parser->getData()->setStatus((HttpStatus)(p->status_code));
  return 0;
}

/**
 * @brief http响应解析结束回调
 */
static int on_response_message_complete_cb(http_parser* p) {
  SYLAR_LOG_DEBUG(g_logger) << "on_response_message_complete_cb";
  HttpResponseParser* parser = static_cast<HttpResponseParser*>(p->data);
  parser->setFinished(true);
  return 0;
}

/**
 * @brief http分段头部回调，可获取分段长度
 */
static int on_response_chunk_header_cb(http_parser* p) {
  SYLAR_LOG_DEBUG(g_logger) << "on_response_chunk_header_cb";
  return 0;
}

/**
 * @brief http分段结束回调，表示全部分段已解析完成
 */
static int on_response_chunk_complete_cb(http_parser* p) {
  SYLAR_LOG_DEBUG(g_logger) << "on_response_chunk_complete_cb";
  return 0;
}

/**
 * @brief http响应url解析完成回调，这个回调没有意义，因为响应不会携带url
 */
static int on_response_url_cb(http_parser* p, const char* buf, size_t len) {
  SYLAR_LOG_DEBUG(g_logger) << "on_response_url_cb, should not happen";
  return 0;
}

/**
 * @brief http响应首部字段名称解析完成回调
 */
static int on_response_header_field_cb(http_parser* p, const char* buf, size_t len) {
  std::string field(buf, len);
  SYLAR_LOG_DEBUG(g_logger) << "on_response_header_field_cb, field is:" << field;
  HttpResponseParser* parser = static_cast<HttpResponseParser*>(p->data);
  parser->setField(field);
  return 0;
}

/**
 * @brief http响应首部字段值解析完成回调
 */
static int on_response_header_value_cb(http_parser* p, const char* buf, size_t len) {
  std::string value(buf, len);
  SYLAR_LOG_DEBUG(g_logger) << "on_response_header_value_cb, value is:" << value;
  HttpResponseParser* parser = static_cast<HttpResponseParser*>(p->data);
  parser->getData()->setHeader(parser->getField(), value);
  return 0;
}

/**
 * @brief http响应状态回调
 */
static int on_response_status_cb(http_parser* p, const char* buf, size_t len) {
  SYLAR_LOG_DEBUG(g_logger) << "on_response_status_cb, status code is: " << p->status_code
                            << ", status msg is: " << std::string(buf, len);
  HttpResponseParser* parser = static_cast<HttpResponseParser*>(p->data);
  parser->getData()->setStatus(HttpStatus(p->status_code));
  return 0;
}

/**
 * @brief http响应消息体回调
 */
static int on_response_body_cb(http_parser* p, const char* buf, size_t len) {
  std::string body(buf, len);
  SYLAR_LOG_DEBUG(g_logger) << "on_response_body_cb, body is:" << body;
  HttpResponseParser* parser = static_cast<HttpResponseParser*>(p->data);
  parser->getData()->appendBody(body);
  return 0;
}

static http_parser_settings s_response_settings = {
  .on_message_begin = on_response_message_begin_cb,
  .on_url = on_response_url_cb,
  .on_status = on_response_status_cb,
  .on_header_field = on_response_header_field_cb,
  .on_header_value = on_response_header_value_cb,
  .on_headers_complete = on_response_headers_complete_cb,
  .on_body = on_response_body_cb,
  .on_message_complete = on_response_message_complete_cb,
  .on_chunk_header = on_response_chunk_header_cb,
  .on_chunk_complete = on_response_chunk_complete_cb};

HttpResponseParser::HttpResponseParser() {
  http_parser_init(&m_parser, HTTP_RESPONSE);
  m_data.reset(new HttpResponse);
  m_parser.data = this;
  m_error = 0;
  m_finished = false;
}

size_t HttpResponseParser::execute(char* data, size_t len) {
  size_t nparsed = http_parser_execute(&m_parser, &s_response_settings, data, len);
  if (m_parser.http_errno != 0) {
    SYLAR_LOG_WARN(g_logger) << "parse response fail: "
                              << http_errno_name(HTTP_PARSER_ERRNO(&m_parser));
    setError((int8_t)m_parser.http_errno);
  } else {
    if (nparsed < len) {
      memmove(data, data + nparsed, (len - nparsed));
    }
  }
  return nparsed;
}

}   // namespace http
}   // namespace sylar