/*
 * @Author: Nana5aki
 * @Date: 2025-04-20 18:29:05
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-04-26 18:52:04
 * @FilePath: /MySylar/sylar/http/http_session.cc
 */
#include "http_session.h"
#include "http_request_parser.h"

namespace sylar {
namespace http {

HttpSession::HttpSession(Socket::ptr sock, bool owner)
  : SocketStream(sock, owner) {
}

HttpRequest::ptr HttpSession::recvRequest() {
  HttpRequestParser::ptr parser(new HttpRequestParser);
  uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();
  std::shared_ptr<char> buffer(new char[buff_size], [](char* ptr) { delete[] ptr; });
  char* data = buffer.get();
  int offset = 0;
  do {
    int len = read(data + offset, buff_size - offset);
    if (len <= 0) {
      close();
      return nullptr;
    }
    len += offset;
    size_t nparse = parser->execute(data, len);
    if (parser->hasError()) {
      close();
      return nullptr;
    }
    offset = len - nparse;
    if (offset == (int)buff_size) {
      close();
      return nullptr;
    }
    if (parser->isFinished()) {
      break;
    }
  } while (true);

  return parser->getData();
}

int HttpSession::sendResponse(HttpResponse::ptr rsp) {
  std::stringstream ss;
  ss << *rsp;
  std::string data = ss.str();
  return writeFixSize(data.c_str(), data.size());
}

}   // namespace http
}   // namespace sylar