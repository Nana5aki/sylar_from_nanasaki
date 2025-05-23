/*
 * @Author: Nana5aki
 * @Date: 2025-04-20 15:05:37
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-04-25 22:47:14
 * @FilePath: /sylar_from_nanasaki/sylar/http/http_request_parser.cc
 */
#include "http_request_parser.h"
#include "sylar/config.h"
#include "sylar/log.h"

namespace sylar {
namespace http {

static Logger::ptr g_logger = SYLAR_LOG_NAME("http");

static ConfigVar<uint64_t>::ptr g_http_request_buffer_size =
  Config::Lookup("http.request.buffer_size", (uint64_t)(4 * 1024), "http request buffer size");

static ConfigVar<uint64_t>::ptr g_http_request_max_body_size = Config::Lookup(
  "http.request.max_body_size", (uint64_t)(64 * 1024 * 1024), "http request max body size");

static uint64_t s_http_request_buffer_size = 0;
static uint64_t s_http_request_max_body_size = 0;

uint64_t HttpRequestParser::GetHttpRequestBufferSize() {
  return s_http_request_buffer_size;
}

uint64_t HttpRequestParser::GetHttpRequestMaxBodySize() {
  return s_http_request_max_body_size;
}

namespace {
struct _RequestParseSizeIniter {
  _RequestParseSizeIniter() {
    s_http_request_buffer_size = g_http_request_buffer_size->getValue();
    s_http_request_max_body_size = g_http_request_max_body_size->getValue();

    g_http_request_buffer_size->addListener(
      [](const uint64_t& ov, const uint64_t& nv) { s_http_request_buffer_size = nv; });

    g_http_request_max_body_size->addListener(
      [](const uint64_t& ov, const uint64_t& nv) { s_http_request_max_body_size = nv; });
  }
};
static _RequestParseSizeIniter _init;
}   // namespace

/**
 * @brief http请求开始解析回调函数
 */
static int on_request_message_begin_cb(http_parser* p) {
  SYLAR_LOG_DEBUG(g_logger) << "on_request_message_begin_cb";
  return 0;
}

/**
 * @brief http请求头部字段解析结束，可获取头部信息字段，如method/version等
 * @note 返回0表示成功，返回1表示该HTTP消息无消息体，返回2表示无消息体并且该连接后续不会再有消息
 */
static int on_request_headers_complete_cb(http_parser* p) {
  SYLAR_LOG_DEBUG(g_logger) << "on_request_headers_complete_cb";
  HttpRequestParser* parser = static_cast<HttpRequestParser*>(p->data);
  parser->getData()->setVersion(((p->http_major) << 0x4) | (p->http_minor));
  parser->getData()->setMethod((HttpMethod)(p->method));
  return 0;
}

/**
 * @brief http解析结束回调
 */
static int on_request_message_complete_cb(http_parser* p) {
  SYLAR_LOG_DEBUG(g_logger) << "on_request_message_complete_cb";
  HttpRequestParser* parser = static_cast<HttpRequestParser*>(p->data);
  parser->setFinished(true);
  return 0;
}

/**
 * @brief http分段头部回调，可获取分段长度
 */
static int on_request_chunk_header_cb(http_parser* p) {
  SYLAR_LOG_DEBUG(g_logger) << "on_request_chunk_header_cb";
  return 0;
}

/**
 * @brief http分段结束回调，表示当前分段已解析完成
 */
static int on_request_chunk_complete_cb(http_parser* p) {
  SYLAR_LOG_DEBUG(g_logger) << "on_request_chunk_complete_cb";
  return 0;
}

/**
 * @brief http请求url解析完成回调
 */
static int on_request_url_cb(http_parser* p, const char* buf, size_t len) {
  SYLAR_LOG_DEBUG(g_logger) << "on_request_url_cb, url is:" << std::string(buf, len);

  int ret;
  struct http_parser_url url_parser;
  HttpRequestParser* parser = static_cast<HttpRequestParser*>(p->data);

  http_parser_url_init(&url_parser);
  ret = http_parser_parse_url(buf, len, 0, &url_parser);
  if (ret != 0) {
    SYLAR_LOG_DEBUG(g_logger) << "parse url fail";
    return 1;
  }
  if (url_parser.field_set & (1 << UF_PATH)) {
    parser->getData()->setPath(
      std::string(buf + url_parser.field_data[UF_PATH].off, url_parser.field_data[UF_PATH].len));
  }
  if (url_parser.field_set & (1 << UF_QUERY)) {
    parser->getData()->setQuery(
      std::string(buf + url_parser.field_data[UF_QUERY].off, url_parser.field_data[UF_QUERY].len));
  }
  if (url_parser.field_set & (1 << UF_FRAGMENT)) {
    parser->getData()->setFragment(std::string(buf + url_parser.field_data[UF_FRAGMENT].off,
                                               url_parser.field_data[UF_FRAGMENT].len));
  }
  return 0;
}

/**
 * @brief http请求首部字段名称解析完成回调
 */
static int on_request_header_field_cb(http_parser* p, const char* buf, size_t len) {
  std::string field(buf, len);
  SYLAR_LOG_DEBUG(g_logger) << "on_request_header_field_cb, field is:" << field;
  HttpRequestParser* parser = static_cast<HttpRequestParser*>(p->data);
  parser->setField(field);
  return 0;
}

/**
 * @brief http请求首部字段值解析完成回调
 */
static int on_request_header_value_cb(http_parser* p, const char* buf, size_t len) {
  std::string value(buf, len);
  SYLAR_LOG_DEBUG(g_logger) << "on_request_header_value_cb, value is:" << value;
  HttpRequestParser* parser = static_cast<HttpRequestParser*>(p->data);
  parser->getData()->setHeader(parser->getField(), value);
  return 0;
}

/**
 * @brief http请求响应状态回调，这个回调没有用，因为http请求不带状态
 */
static int on_request_status_cb(http_parser* p, const char* buf, size_t len) {
  SYLAR_LOG_DEBUG(g_logger) << "on_request_status_cb, should not happen";
  return 0;
}

/**
 * @brief http消息体回调
 * @note
 * 当传输编码是chunked时，每个chunked数据段都会触发一次当前回调，所以用append的方法将所有数据组合到一起
 */
static int on_request_body_cb(http_parser* p, const char* buf, size_t len) {
  std::string body(buf, len);
  SYLAR_LOG_DEBUG(g_logger) << "on_request_body_cb, body is:" << body;
  HttpRequestParser* parser = static_cast<HttpRequestParser*>(p->data);
  parser->getData()->appendBody(body);
  return 0;
}

static http_parser_settings s_request_settings = {
  .on_message_begin = on_request_message_begin_cb,
  .on_url = on_request_url_cb,
  .on_status = on_request_status_cb,
  .on_header_field = on_request_header_field_cb,
  .on_header_value = on_request_header_value_cb,
  .on_headers_complete = on_request_headers_complete_cb,
  .on_body = on_request_body_cb,
  .on_message_complete = on_request_message_complete_cb,
  .on_chunk_header = on_request_chunk_header_cb,
  .on_chunk_complete = on_request_chunk_complete_cb};

HttpRequestParser::HttpRequestParser() {
  http_parser_init(&m_parser, HTTP_REQUEST);
  m_data.reset(new HttpRequest);
  m_parser.data = this;
  m_error = 0;
  m_finished = false;
}

size_t HttpRequestParser::execute(char* data, size_t len) {
  size_t nparsed = http_parser_execute(&m_parser, &s_request_settings, data, len);
  if (m_parser.upgrade) {
    // 处理新协议，暂时不处理
    SYLAR_LOG_DEBUG(g_logger) << "found upgrade, ignore";
    setError(HPE_UNKNOWN);
  } else if (m_parser.http_errno != 0) {
    SYLAR_LOG_WARN(g_logger) << "parse request fail: "
                              << http_errno_name(HTTP_PARSER_ERRNO(&m_parser));
    setError((int8_t)m_parser.http_errno);
  } else {
    if (nparsed < len) { // 如果解析的数据小于总数据，则将剩余数据移动到数据头部
      memmove(data, data + nparsed, (len - nparsed));
    }
  }
  return nparsed;
}

}   // namespace http
}   // namespace sylar