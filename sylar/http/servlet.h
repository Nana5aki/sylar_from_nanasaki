/*
 * @Author: Nana5aki
 * @Date: 2025-04-27 22:23:36
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-31 11:55:06
 * @FilePath: /sylar_from_nanasaki/sylar/http/servlet.h
 */

#pragma once

#include "http_request.h"
#include "http_response.h"
#include "http_session.h"
#include "sylar/mutex.h"
#include "sylar/util/util.h"
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>


namespace sylar {
namespace http {

/// @brief Servlet封装
class Servlet {
public:
  using ptr = std::shared_ptr<Servlet>;

  /**
   * @brief 构造函数
   * @param[in] name 名称
   */
  Servlet(const std::string& name)
    : m_name(name) {
  }

  /**
   * @brief 析构函数
   */
  virtual ~Servlet() = default;

  /**
   * @brief 处理请求
   * @param[in] request HTTP请求
   * @param[in] response HTTP响应
   * @param[in] session HTTP连接
   * @return 是否处理成功
   */
  virtual int32_t handle(HttpRequest::ptr request, HttpResponse::ptr response,
                         HttpSession::ptr session) = 0;

  /**
   * @brief 返回Servlet名称
   */
  const std::string& getName() const {
    return m_name;
  }

protected:
  /// 名称
  std::string m_name;
};


/// @brief 函数式Servlet
class FunctionServlet : public Servlet {
public:
  using ptr = std::shared_ptr<FunctionServlet>;
  using callback = std::function<int32_t(HttpRequest::ptr request, HttpResponse::ptr response,
                                         HttpSession::ptr session)>;

  /**
   * @brief 构造函数
   * @param[in] cb 回调函数
   */
  FunctionServlet(callback cb);

  virtual int32_t handle(HttpRequest::ptr request, HttpResponse::ptr response,
                         HttpSession::ptr session) override;

private:
  /// 回调函数
  callback m_cb;
};

class IServletCreator {
public:
  using ptr = std::shared_ptr<IServletCreator>;
  virtual ~IServletCreator() = default;
  virtual Servlet::ptr get() const = 0;
  virtual std::string getName() const = 0;
};

class HoldServletCreator : public IServletCreator {
public:
  using ptr = std::shared_ptr<HoldServletCreator>;

  HoldServletCreator(Servlet::ptr slt)
    : m_servlet(slt) {
  }

  Servlet::ptr get() const override {
    return m_servlet;
  }

  std::string getName() const override {
    return m_servlet->getName();
  }

private:
  Servlet::ptr m_servlet;
};

template <class T>
class ServletCreator : public IServletCreator {
public:
  using ptr = std::shared_ptr<ServletCreator>;

  ServletCreator() = default;

  Servlet::ptr get() const override {
    return Servlet::ptr(new T);
  }

  std::string getName() const override {
    return sylar::util::TypeToName<T>();
  }
};

/// @brief Servlet分发器
class ServletDispatch : public Servlet {
public:
  using ptr = std::shared_ptr<ServletDispatch>;
  using RWMutexType = RWMutex;

public:
  ServletDispatch();

  int32_t handle(HttpRequest::ptr request, HttpResponse::ptr response,
                 HttpSession::ptr session) override;

  /**
   * @brief 添加servlet
   * @param[in] uri uri
   * @param[in] slt serlvet
   */
  void addServlet(const std::string& uri, Servlet::ptr slt);

  /**
   * @brief 添加servlet
   * @param[in] uri uri
   * @param[in] cb FunctionServlet回调函数
   */
  void addServlet(const std::string& uri, FunctionServlet::callback cb);

  /**
   * @brief 添加模糊匹配servlet
   * @param[in] uri uri 模糊匹配 /sylar_*
   * @param[in] slt servlet
   */
  void addGlobServlet(const std::string& uri, Servlet::ptr slt);

  /**
   * @brief 添加模糊匹配servlet
   * @param[in] uri uri 模糊匹配 /sylar_*
   * @param[in] cb FunctionServlet回调函数
   */
  void addGlobServlet(const std::string& uri, FunctionServlet::callback cb);

  void addServletCreator(const std::string& uri, IServletCreator::ptr creator);
  void addGlobServletCreator(const std::string& uri, IServletCreator::ptr creator);

  template <class T>
  void addServletCreator(const std::string& uri) {
    addServletCreator(uri, std::make_shared<ServletCreator<T>>());
  }

  template <class T>
  void addGlobServletCreator(const std::string& uri) {
    addGlobServletCreator(uri, std::make_shared<ServletCreator<T>>());
  }

  /**
   * @brief 删除servlet
   * @param[in] uri uri
   */
  void delServlet(const std::string& uri);

  /**
   * @brief 删除模糊匹配servlet
   * @param[in] uri uri
   */
  void delGlobServlet(const std::string& uri);

  /**
   * @brief 返回默认servlet
   */
  Servlet::ptr getDefault() const {
    return m_default;
  }

  /**
   * @brief 设置默认servlet
   * @param[in] v servlet
   */
  void setDefault(Servlet::ptr v) {
    m_default = v;
  }

  /**
   * @brief 通过uri获取servlet
   * @param[in] uri uri
   * @return 返回对应的servlet
   */
  Servlet::ptr getServlet(const std::string& uri);

  /**
   * @brief 通过uri获取模糊匹配servlet
   * @param[in] uri uri
   * @return 返回对应的servlet
   */
  Servlet::ptr getGlobServlet(const std::string& uri);

  /**
   * @brief 通过uri获取servlet
   * @param[in] uri uri
   * @return 优先精准匹配,其次模糊匹配,最后返回默认
   */
  Servlet::ptr getMatchedServlet(const std::string& uri);

  void listAllServletCreator(std::map<std::string, IServletCreator::ptr>& infos);
  void listAllGlobServletCreator(std::map<std::string, IServletCreator::ptr>& infos);

private:
  /// 读写互斥量
  RWMutexType m_mutex;
  /// 精准匹配servlet MAP
  /// uri(/sylar/xxx) -> servlet
  std::unordered_map<std::string, IServletCreator::ptr> m_datas;
  /// 模糊匹配servlet 数组
  /// uri(/sylar/*) -> servlet
  std::vector<std::pair<std::string, IServletCreator::ptr>> m_globs;
  /// 默认servlet，所有路径都没匹配到时使用
  Servlet::ptr m_default;
};

/**
 * @brief NotFoundServlet(默认返回404)
 */
class NotFoundServlet : public Servlet {
public:
  using ptr = std::shared_ptr<NotFoundServlet>;
  /**
   * @brief 构造函数
   */
  NotFoundServlet(const std::string& name);

  virtual int32_t handle(HttpRequest::ptr request, HttpResponse::ptr response,
                         HttpSession::ptr session) override;

private:
  std::string m_name;
  std::string m_content;
};

}   // namespace http
}   // namespace sylar