/*
 * @Author: Nana5aki
 * @Date: 2025-04-19 00:04:58
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-18 18:07:44
 * @FilePath: /sylar_from_nanasaki/sylar/http/http_response.cc
 */
#include "http_response.h"
#include "sylar/util/util.h"

namespace sylar {
namespace http {

HttpResponse::HttpResponse(uint8_t version, bool close)
  : m_status(HttpStatus::OK)
  , m_version(version)
  , m_close(close)
  , m_websocket(false) {
}

std::string HttpResponse::getHeader(const std::string& key, const std::string& def) const {
  auto it = m_headers.find(key);
  return it == m_headers.end() ? def : it->second;
}

void HttpResponse::setHeader(const std::string& key, const std::string& val) {
  m_headers.emplace(key, val);
}

void HttpResponse::delHeader(const std::string& key) {
  m_headers.erase(key);
}

void HttpResponse::setRedirect(const std::string& uri) {
  m_status = HttpStatus::FOUND;
  setHeader("Location", uri);
}

void HttpResponse::setCookie(const std::string& key, const std::string& val, time_t expired,
                             const std::string& path, const std::string& domain, bool secure) {
  std::stringstream ss;
  ss << key << "=" << val;
  if (expired > 0) {
    ss << ";expires=" << sylar::util::Time2Str(expired, "%a, %d %b %Y %H:%M:%S") << " GMT";
  }
  if (!domain.empty()) {
    ss << ";domain=" << domain;
  }
  if (!path.empty()) {
    ss << ";path=" << path;
  }
  if (secure) {
    ss << ";secure";
  }
  m_cookies.emplace_back(ss.str());
}

std::string HttpResponse::toString() const {
  std::stringstream ss;
  dump(ss);
  return ss.str();
}

std::ostream& HttpResponse::dump(std::ostream& os) const {
  os << "HTTP/" << ((uint32_t)(m_version >> 4)) << "." << ((uint32_t)(m_version & 0x0F)) << " "
     << (uint32_t)m_status << " " << (m_reason.empty() ? HttpStatusToString(m_status) : m_reason)
     << "\r\n";

  for (const auto& [key, value] : m_headers) {
    if (!m_websocket && strcasecmp(key.c_str(), "connection")) {
      continue;
    }
    os << key << ": " << value << "\r\n";
  }
  for (auto& i : m_cookies) {
    os << "Set-Cookie: " << i << "\r\n";
  }
  if (!m_websocket) {
    os << "connection: " << (m_close ? "close" : "keep-alive") << "\r\n";
  }
  if (!m_body.empty()) {
    os << "content-length: " << m_body.size() << "\r\n\r\n" << m_body;
  } else {
    os << "\r\n";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const HttpResponse& rsp) {
  return rsp.dump(os);
}

}   // namespace http
}   // namespace sylar