/*
 * @Author: Nana5aki
 * @Date: 2025-04-29 00:00:44
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-31 11:54:49
 * @FilePath: /sylar_from_nanasaki/sylar/http/http_connection.cc
 */
#include "http_connection.h"
#include "http_request_parser.h"
#include "http_response_parser.h"
#include "sylar/log.h"
#include "sylar/util/util.h"
#include <sstream>
#include <functional>

namespace sylar {
namespace http {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

std::string HttpResult::toString() const {
  std::stringstream ss;
  ss << "[HttpResult result=" << result << " error=" << error
     << " response=" << (response ? response->toString() : "nullptr") << "]";
  return ss.str();
}

HttpConnection::HttpConnection(Socket::ptr sock, bool owner)
  : SocketStream(sock, owner) {
}

HttpConnection::~HttpConnection() {
  SYLAR_LOG_DEBUG(g_logger) << "HttpConnection::~HttpConnection";
}

HttpResponse::ptr HttpConnection::recvResponse() {
  HttpResponseParser::ptr parser = std::make_shared<HttpResponseParser>();
  uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();
  std::shared_ptr<char> buffer(new char[buff_size + 1], [](char* ptr) { delete[] ptr; });
  char* data = buffer.get();
  int offset = 0;
  do {
    int len = read(data + offset, buff_size - offset);
    if (len <= 0) {
      close();
      return nullptr;
    }
    len += offset;
    data[len] = '\0';
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

int HttpConnection::sendRequest(HttpRequest::ptr rsp) {
  std::stringstream ss;
  ss << *rsp;
  std::string data = ss.str();
  return writeFixSize(data.c_str(), data.size());
}

HttpResult::ptr HttpConnection::DoGet(const std::string& url, uint64_t timeout_ms,
                                      const std::map<std::string, std::string>& headers,
                                      const std::string& body) {
  Uri::ptr uri = Uri::Create(url);
  if (!uri) {
    return std::make_shared<HttpResult>(
      (int)HttpResult::Error::INVALID_URL, nullptr, "invalid url: " + url);
  }
  return DoGet(uri, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnection::DoGet(Uri::ptr uri, uint64_t timeout_ms,
                                      const std::map<std::string, std::string>& headers,
                                      const std::string& body) {
  return DoRequest(HttpMethod::GET, uri, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnection::DoPost(const std::string& url, uint64_t timeout_ms,
                                       const std::map<std::string, std::string>& headers,
                                       const std::string& body) {
  Uri::ptr uri = Uri::Create(url);
  if (!uri) {
    return std::make_shared<HttpResult>(
      (int)HttpResult::Error::INVALID_URL, nullptr, "invalid url: " + url);
  }
  return DoPost(uri, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnection::DoPost(Uri::ptr uri, uint64_t timeout_ms,
                                       const std::map<std::string, std::string>& headers,
                                       const std::string& body) {
  return DoRequest(HttpMethod::POST, uri, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnection::DoRequest(HttpMethod method, const std::string& url,
                                          uint64_t timeout_ms,
                                          const std::map<std::string, std::string>& headers,
                                          const std::string& body) {
  Uri::ptr uri = Uri::Create(url);
  if (!uri) {
    return std::make_shared<HttpResult>(
      (int)HttpResult::Error::INVALID_URL, nullptr, "invalid url: " + url);
  }
  return DoRequest(method, uri, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnection::DoRequest(HttpMethod method, Uri::ptr uri, uint64_t timeout_ms,
                                          const std::map<std::string, std::string>& headers,
                                          const std::string& body) {
  HttpRequest::ptr req = std::make_shared<HttpRequest>();
  req->setPath(uri->getPath());
  req->setQuery(uri->getQuery());
  req->setFragment(uri->getFragment());
  req->setMethod(method);
  bool has_host = false;
  for (auto& i : headers) {
    if (strcasecmp(i.first.c_str(), "connection") == 0) {
      if (strcasecmp(i.second.c_str(), "keep-alive") == 0) {
        req->setClose(false);
      }
      continue;
    }

    if (!has_host && strcasecmp(i.first.c_str(), "host") == 0) {
      has_host = !i.second.empty();
    }

    req->setHeader(i.first, i.second);
  }
  if (!has_host) {
    req->setHeader("Host", uri->getHost());
  }
  req->setBody(body);
  return DoRequest(req, uri, timeout_ms);
}

HttpResult::ptr HttpConnection::DoRequest(HttpRequest::ptr req, Uri::ptr uri, uint64_t timeout_ms) {
  Address::ptr addr = uri->createAddress();
  if (!addr) {
    return std::make_shared<HttpResult>(
      (int)HttpResult::Error::INVALID_HOST, nullptr, "invalid host: " + uri->getHost());
  }
  Socket::ptr sock = Socket::CreateTCP(addr);
  if (!sock) {
    return std::make_shared<HttpResult>((int)HttpResult::Error::CREATE_SOCKET_ERROR,
                                        nullptr,
                                        "create socket fail: " + addr->toString() +
                                          " errno=" + std::to_string(errno) +
                                          " errstr=" + std::string(strerror(errno)));
  }
  if (!sock->connect(addr)) {
    return std::make_shared<HttpResult>(
      (int)HttpResult::Error::CONNECT_FAIL, nullptr, "connect fail: " + addr->toString());
  }
  sock->setRecvTimeout(timeout_ms);
  HttpConnection::ptr conn = std::make_shared<HttpConnection>(sock);
  int rt = conn->sendRequest(req);
  if (rt == 0) {
    return std::make_shared<HttpResult>((int)HttpResult::Error::SEND_CLOSE_BY_PEER,
                                        nullptr,
                                        "send request closed by peer: " + addr->toString());
  }
  if (rt < 0) {
    return std::make_shared<HttpResult>((int)HttpResult::Error::SEND_SOCKET_ERROR,
                                        nullptr,
                                        "send request socket error errno=" + std::to_string(errno) +
                                          " errstr=" + std::string(strerror(errno)));
  }
  auto rsp = conn->recvResponse();
  if (!rsp) {
    return std::make_shared<HttpResult>(
      (int)HttpResult::Error::TIMEOUT,
      nullptr,
      "recv response timeout: " + addr->toString() + " timeout_ms:" + std::to_string(timeout_ms));
  }
  return std::make_shared<HttpResult>((int)HttpResult::Error::OK, rsp, "ok");
}

HttpConnectionPool::ptr HttpConnectionPool::Create(const std::string& uri, const std::string& vhost,
                                                   uint32_t max_size, uint32_t max_alive_time,
                                                   uint32_t max_request) {
  Uri::ptr turi = Uri::Create(uri);
  if (!turi) {
    SYLAR_LOG_ERROR(g_logger) << "invalid uri=" << uri;
  }
  return std::make_shared<HttpConnectionPool>(
    turi->getHost(), vhost, turi->getPort(), max_size, max_alive_time, max_request);
}

HttpConnectionPool::HttpConnectionPool(const std::string& host, const std::string& vhost,
                                       uint32_t port, uint32_t max_size, uint32_t max_alive_time,
                                       uint32_t max_request)
  : m_host(host)
  , m_vhost(vhost)
  , m_port(port)
  , m_maxSize(max_size)
  , m_maxAliveTime(max_alive_time)
  , m_maxRequest(max_request) {
}

HttpConnection::ptr HttpConnectionPool::getConnection() {
  uint64_t now_ms = sylar::util::GetCurrentMS();
  std::vector<HttpConnection*> invalid_conns;
  HttpConnection* ptr = nullptr;
  MutexType::Lock lock(m_mutex);
  while (!m_conns.empty()) {
    auto conn = *m_conns.begin();
    m_conns.pop_front();
    if (!conn->isConnected()) {
      invalid_conns.push_back(conn);
      continue;
    }
    if ((conn->m_createTime + m_maxAliveTime) > now_ms) {
      invalid_conns.push_back(conn);
      continue;
    }
    ptr = conn;
    break;
  }
  lock.unlock();
  for (auto i : invalid_conns) {
    delete i;
  }
  m_total -= invalid_conns.size();

  if (!ptr) {
    IPAddress::ptr addr = Address::LookupAnyIPAddress(m_host);
    if (!addr) {
      SYLAR_LOG_ERROR(g_logger) << "get addr fail: " << m_host;
      return nullptr;
    }
    addr->setPort(m_port);
    Socket::ptr sock = Socket::CreateTCP(addr);
    if (!sock) {
      SYLAR_LOG_ERROR(g_logger) << "create sock fail: " << *addr;
      return nullptr;
    }
    if (!sock->connect(addr)) {
      SYLAR_LOG_ERROR(g_logger) << "sock connect fail: " << *addr;
      return nullptr;
    }

    if ((uint32_t)m_total >= m_maxSize) {
      SYLAR_LOG_ERROR(g_logger) << "ip=" << addr->toString() << " total=" << m_total
                                << " max_size=" << m_maxSize << " over max_size";
      return nullptr;
    }

    ptr = new HttpConnection(sock);
    ++m_total;
  }
  return HttpConnection::ptr(
    ptr, std::bind(&HttpConnectionPool::ReleasePtr, std::placeholders::_1, this));
}

void HttpConnectionPool::ReleasePtr(HttpConnection* ptr, HttpConnectionPool* pool) {
  ++ptr->m_request;
  if (!ptr->isConnected() ||
      ((ptr->m_createTime + pool->m_maxAliveTime) >= sylar::util::GetCurrentMS()) ||
      (ptr->m_request >= pool->m_maxRequest)) {
    delete ptr;
    --pool->m_total;
    return;
  }
  MutexType::Lock lock(pool->m_mutex);
  pool->m_conns.push_back(ptr);
}

HttpResult::ptr HttpConnectionPool::doGet(const std::string& url, uint64_t timeout_ms,
                                          const std::map<std::string, std::string>& headers,
                                          const std::string& body) {
  return doRequest(HttpMethod::GET, url, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnectionPool::doGet(Uri::ptr uri, uint64_t timeout_ms,
                                          const std::map<std::string, std::string>& headers,
                                          const std::string& body) {
  std::stringstream ss;
  ss << uri->getPath() << (uri->getQuery().empty() ? "" : "?") << uri->getQuery()
     << (uri->getFragment().empty() ? "" : "#") << uri->getFragment();
  return doGet(ss.str(), timeout_ms, headers, body);
}

HttpResult::ptr HttpConnectionPool::doPost(const std::string& url, uint64_t timeout_ms,
                                           const std::map<std::string, std::string>& headers,
                                           const std::string& body) {
  return doRequest(HttpMethod::POST, url, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnectionPool::doPost(Uri::ptr uri, uint64_t timeout_ms,
                                           const std::map<std::string, std::string>& headers,
                                           const std::string& body) {
  std::stringstream ss;
  ss << uri->getPath() << (uri->getQuery().empty() ? "" : "?") << uri->getQuery()
     << (uri->getFragment().empty() ? "" : "#") << uri->getFragment();
  return doPost(ss.str(), timeout_ms, headers, body);
}

HttpResult::ptr HttpConnectionPool::doRequest(HttpMethod method, const std::string& url,
                                              uint64_t timeout_ms,
                                              const std::map<std::string, std::string>& headers,
                                              const std::string& body) {
  HttpRequest::ptr req = std::make_shared<HttpRequest>();
  req->setPath(url);
  req->setMethod(method);
  req->setClose(false);
  bool has_host = false;
  for (auto& i : headers) {
    if (strcasecmp(i.first.c_str(), "connection") == 0) {
      if (strcasecmp(i.second.c_str(), "keep-alive") == 0) {
        req->setClose(false);
      }
      continue;
    }

    if (!has_host && strcasecmp(i.first.c_str(), "host") == 0) {
      has_host = !i.second.empty();
    }

    req->setHeader(i.first, i.second);
  }
  if (!has_host) {
    if (m_vhost.empty()) {
      req->setHeader("Host", m_host);
    } else {
      req->setHeader("Host", m_vhost);
    }
  }
  req->setBody(body);
  return doRequest(req, timeout_ms);
}

HttpResult::ptr HttpConnectionPool::doRequest(HttpMethod method, Uri::ptr uri, uint64_t timeout_ms,
                                              const std::map<std::string, std::string>& headers,
                                              const std::string& body) {
  std::stringstream ss;
  ss << uri->getPath() << (uri->getQuery().empty() ? "" : "?") << uri->getQuery()
     << (uri->getFragment().empty() ? "" : "#") << uri->getFragment();
  return doRequest(method, ss.str(), timeout_ms, headers, body);
}

HttpResult::ptr HttpConnectionPool::doRequest(HttpRequest::ptr req, uint64_t timeout_ms) {
  auto conn = getConnection();
  if (!conn) {
    return std::make_shared<HttpResult>((int)HttpResult::Error::POOL_GET_CONNECTION,
                                        nullptr,
                                        "pool host:" + m_host + " port:" + std::to_string(m_port));
  }
  auto sock = conn->getSocket();
  if (!sock) {
    return std::make_shared<HttpResult>((int)HttpResult::Error::POOL_INVALID_CONNECTION,
                                        nullptr,
                                        "pool host:" + m_host + " port:" + std::to_string(m_port));
  }
  sock->setRecvTimeout(timeout_ms);
  int rt = conn->sendRequest(req);
  if (rt == 0) {
    return std::make_shared<HttpResult>(
      (int)HttpResult::Error::SEND_CLOSE_BY_PEER,
      nullptr,
      "send request closed by peer: " + sock->getRemoteAddress()->toString());
  }
  if (rt < 0) {
    return std::make_shared<HttpResult>((int)HttpResult::Error::SEND_SOCKET_ERROR,
                                        nullptr,
                                        "send request socket error errno=" + std::to_string(errno) +
                                          " errstr=" + std::string(strerror(errno)));
  }
  auto rsp = conn->recvResponse();
  if (!rsp) {
    return std::make_shared<HttpResult>(
      (int)HttpResult::Error::TIMEOUT,
      nullptr,
      "recv response timeout: " + sock->getRemoteAddress()->toString() +
        " timeout_ms:" + std::to_string(timeout_ms));
  }
  return std::make_shared<HttpResult>((int)HttpResult::Error::OK, rsp, "ok");
}

}   // namespace http
}   // namespace sylar