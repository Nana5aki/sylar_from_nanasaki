/*
 * @Author: Nana5aki
 * @Date: 2025-07-13 15:30:00
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-13 15:07:25
 * @FilePath: /sylar_from_nanasaki/sylar/http/protocol/http11_protocol.cc
 */

#include "http11_protocol.h"
#include "sylar/http/core/http_request_parser.h"
#include "sylar/config.h"
#include <sstream>

namespace sylar {
namespace http {

static sylar::ConfigVar<uint64_t>::ptr g_http_request_buffer_size = 
    sylar::Config::Lookup("http.request.buffer_size", (uint64_t)(4 * 1024), "http request buffer size");

Http11Protocol::Http11Protocol() {
}

std::string Http11Protocol::getVersion() {
  return "1.1";
}

HttpRequest::ptr Http11Protocol::recvRequest(IHttpTransport::ptr transport) {
  HttpRequestParser::ptr parser = std::make_shared<HttpRequestParser>();
  
  uint64_t buff_size = getHttpRequestBufferSize();
  std::shared_ptr<char> buffer(new char[buff_size], [](char* ptr) { delete[] ptr; });
  char* data = buffer.get();
  int offset = 0;
  
  do {
    int len = transport->read(data + offset, buff_size - offset);
    if (len <= 0) {
      transport->close();
      return nullptr;
    }
    len += offset;
    size_t nparse = parser->execute(data, len);
    if (parser->hasError()) {
      transport->close();
      return nullptr;
    }
    offset = len - nparse;
    if (offset == (int)buff_size) {
      transport->close();
      return nullptr;
    }
    if (parser->isFinished()) {
      break;
    }
  } while (true);

  return parser->getData();
}

int Http11Protocol::sendResponse(IHttpTransport::ptr transport, HttpResponse::ptr response) {
  std::stringstream ss;
  ss << *response;
  std::string data = ss.str();
  return transport->writeFixSize(data.c_str(), data.size());
}

bool Http11Protocol::supportsKeepAlive() {
  return true;
}

uint64_t Http11Protocol::getHttpRequestBufferSize() {
  return g_http_request_buffer_size->getValue();
}

}  // namespace http
}  // namespace sylar 