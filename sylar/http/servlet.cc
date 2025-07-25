/*
 * @Author: Nana5aki
 * @Date: 2025-04-27 22:23:41
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-13 16:02:04
 * @FilePath: /sylar_from_nanasaki/sylar/http/servlet.cc
 */
#include "servlet.h"
#include <fnmatch.h>

namespace sylar {
namespace http {

FunctionServlet::FunctionServlet(callback cb)
  : Servlet("FunctionServlet")
  , m_cb(cb) {
}

int32_t FunctionServlet::handle(HttpRequest::ptr request, HttpResponse::ptr response,
                                IHttpSession::ptr session) {
  return m_cb(request, response, session);
}

ServletDispatch::ServletDispatch()
  : Servlet("ServletDispatch") {
  m_default = std::make_shared<NotFoundServlet>("sylar/1.0.0");
}

int32_t ServletDispatch::handle(HttpRequest::ptr request, HttpResponse::ptr response,
                                IHttpSession::ptr session) {
  auto slt = getMatchedServlet(request->getPath());
  if (slt) {
    slt->handle(request, response, session);
  }
  return 0;
}

void ServletDispatch::addServlet(const std::string& uri, Servlet::ptr slt) {
  RWMutexType::WriteLock lock(m_mutex);
  m_datas[uri] = std::make_shared<HoldServletCreator>(slt);
}

void ServletDispatch::addServlet(const std::string& uri, FunctionServlet::callback cb) {
  RWMutexType::WriteLock lock(m_mutex);
  m_datas[uri] = std::make_shared<HoldServletCreator>(std::make_shared<FunctionServlet>(cb));
}

void ServletDispatch::addGlobServlet(const std::string& uri, Servlet::ptr slt) {
  RWMutexType::WriteLock lock(m_mutex);
  for (auto it = m_globs.begin(); it != m_globs.end(); ++it) {
    if (it->first == uri) {
      m_globs.erase(it);
      break;
    }
  }
  m_globs.push_back(std::make_pair(uri, std::make_shared<HoldServletCreator>(slt)));
}

void ServletDispatch::addGlobServlet(const std::string& uri, FunctionServlet::callback cb) {
  return addGlobServlet(uri, std::make_shared<FunctionServlet>(cb));
}

void ServletDispatch::addServletCreator(const std::string& uri, IServletCreator::ptr creator) {
  RWMutexType::WriteLock lock(m_mutex);
  m_datas[uri] = creator;
}

void ServletDispatch::addGlobServletCreator(const std::string& uri, IServletCreator::ptr creator) {
  RWMutexType::WriteLock lock(m_mutex);
  for (auto it = m_globs.begin(); it != m_globs.end(); ++it) {
    if (it->first == uri) {
      m_globs.erase(it);
      break;
    }
  }
  m_globs.push_back(std::make_pair(uri, creator));
}

void ServletDispatch::delServlet(const std::string& uri) {
  RWMutexType::WriteLock lock(m_mutex);
  m_datas.erase(uri);
}

void ServletDispatch::delGlobServlet(const std::string& uri) {
  RWMutexType::WriteLock lock(m_mutex);
  for (auto it = m_globs.begin(); it != m_globs.end(); ++it) {
    if (it->first == uri) {
      m_globs.erase(it);
      break;
    }
  }
}

Servlet::ptr ServletDispatch::getMatchedServlet(const std::string& uri) {
  RWMutexType::ReadLock lock(m_mutex);

  auto it = m_datas.find(uri);
  if (it != m_datas.end()) {
    return it->second->get();
  }

  for (auto& i : m_globs) {
    if (!fnmatch(i.first.c_str(), uri.c_str(), 0)) {
      return i.second->get();
    }
  }

  return m_default;
}

void ServletDispatch::listAllServletCreator(std::map<std::string, IServletCreator::ptr>& infos) {
  RWMutexType::ReadLock lock(m_mutex);
  for (auto& i : m_datas) {
    infos[i.first] = i.second;
  }
}

void ServletDispatch::listAllGlobServletCreator(
  std::map<std::string, IServletCreator::ptr>& infos) {
  RWMutexType::ReadLock lock(m_mutex);
  for (auto& i : m_globs) {
    infos[i.first] = i.second;
  }
}

NotFoundServlet::NotFoundServlet(const std::string& name)
  : Servlet("NotFoundServlet")
  , m_name(name) {
  m_content = "<html><head><title>404 Not Found</title></head>"
              "<body><center><h1>404 Not Found</h1></center>"
              "<hr><center>" +
              name + "</center></body></html>";
}

int32_t NotFoundServlet::handle(HttpRequest::ptr request, HttpResponse::ptr response,
                                IHttpSession::ptr session) {
  response->setStatus(HttpStatus::NOT_FOUND);
  response->setHeader("Server", m_name);
  response->setHeader("Content-Type", "text/html");
  response->setBody(m_content);
  return 0;
}

}   // namespace http
}   // namespace sylar