/*
 * @Author: Nana5aki
 * @Date: 2025-04-27 22:23:36
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-07-13 16:01:09
 * @FilePath: /sylar_from_nanasaki/sylar/http/servlet.h
 */

#pragma once

#include "core/http_session_interface.h"
#include "core/http_request.h"
#include "core/http_response.h"
#include "sylar/mutex.h"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace sylar {
namespace http {

/**
 * @brief Servlet基类
 */
class Servlet {
public:
  using ptr = std::shared_ptr<Servlet>;

  /**
   * @brief 构造函数
   * @param name Servlet名称
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
   * @param request HTTP请求
   * @param response HTTP响应
   * @param session HTTP会话
   * @return 处理结果，>0成功，<=0失败
   */
  virtual int32_t handle(HttpRequest::ptr request, HttpResponse::ptr response,
                         IHttpSession::ptr session) = 0;

  /**
   * @brief 获取Servlet名称
   * @return const std::string& Servlet名称
   */
  const std::string& getName() const {
    return m_name;
  }

protected:
  /// Servlet名称
  std::string m_name;
};

/**
 * @brief 函数式Servlet
 */
class FunctionServlet : public Servlet {
public:
  using ptr = std::shared_ptr<FunctionServlet>;
  using callback = std::function<int32_t(HttpRequest::ptr request, HttpResponse::ptr response,
                                         IHttpSession::ptr session)>;

  /**
   * @brief 构造函数
   * @param cb 回调函数
   */
  FunctionServlet(callback cb);

  /**
   * @brief 处理请求
   * @param request HTTP请求
   * @param response HTTP响应
   * @param session HTTP会话
   * @return 处理结果，>0成功，<=0失败
   */
  virtual int32_t handle(HttpRequest::ptr request, HttpResponse::ptr response,
                         IHttpSession::ptr session) override;

private:
  /// 回调函数
  callback m_cb;
};

/**
 * @brief Servlet创建器接口
 */
class IServletCreator {
public:
  using ptr = std::shared_ptr<IServletCreator>;

  virtual ~IServletCreator() = default;

  /**
   * @brief 获取Servlet名称
   * @return std::string Servlet名称
   */
  virtual std::string getName() const = 0;

  /**
   * @brief 创建Servlet对象
   * @return Servlet::ptr Servlet对象
   */
  virtual Servlet::ptr get() const = 0;
};

/**
 * @brief 保持Servlet对象的创建器
 */
class HoldServletCreator : public IServletCreator {
public:
  using ptr = std::shared_ptr<HoldServletCreator>;

  /**
   * @brief 构造函数
   * @param slt Servlet对象
   */
  HoldServletCreator(Servlet::ptr slt)
    : m_servlet(slt) {
  }

  /**
   * @brief 获取Servlet名称
   * @return std::string Servlet名称
   */
  virtual std::string getName() const override {
    return m_servlet->getName();
  }

  /**
   * @brief 创建Servlet对象
   * @return Servlet::ptr Servlet对象
   */
  virtual Servlet::ptr get() const override {
    return m_servlet;
  }

private:
  /// Servlet对象
  Servlet::ptr m_servlet;
};

/**
 * @brief Servlet分发器
 */
class ServletDispatch : public Servlet {
public:
  using ptr = std::shared_ptr<ServletDispatch>;
  using RWMutexType = RWMutex;

  /**
   * @brief 构造函数
   */
  ServletDispatch();

  /**
   * @brief 处理请求
   * @param request HTTP请求
   * @param response HTTP响应
   * @param session HTTP会话
   * @return 处理结果，>0成功，<=0失败
   */
  virtual int32_t handle(HttpRequest::ptr request, HttpResponse::ptr response,
                         IHttpSession::ptr session) override;

  /**
   * @brief 添加Servlet
   * @param uri URI路径
   * @param slt Servlet对象
   */
  void addServlet(const std::string& uri, Servlet::ptr slt);

  /**
   * @brief 添加Servlet
   * @param uri URI路径
   * @param cb 回调函数
   */
  void addServlet(const std::string& uri, FunctionServlet::callback cb);

  /**
   * @brief 添加模糊匹配Servlet
   * @param uri URI路径模式
   * @param slt Servlet对象
   */
  void addGlobServlet(const std::string& uri, Servlet::ptr slt);

  /**
   * @brief 添加模糊匹配Servlet
   * @param uri URI路径模式
   * @param cb 回调函数
   */
  void addGlobServlet(const std::string& uri, FunctionServlet::callback cb);

  /**
   * @brief 添加Servlet创建器
   * @param uri URI路径
   * @param creator Servlet创建器
   */
  void addServletCreator(const std::string& uri, IServletCreator::ptr creator);

  /**
   * @brief 添加模糊匹配Servlet创建器
   * @param uri URI路径模式
   * @param creator Servlet创建器
   */
  void addGlobServletCreator(const std::string& uri, IServletCreator::ptr creator);

  /**
   * @brief 删除Servlet
   * @param uri URI路径
   */
  void delServlet(const std::string& uri);

  /**
   * @brief 删除模糊匹配Servlet
   * @param uri URI路径模式
   */
  void delGlobServlet(const std::string& uri);

  /**
   * @brief 获取默认Servlet
   * @return Servlet::ptr 默认Servlet
   */
  Servlet::ptr getDefault() const {
    return m_default;
  }

  /**
   * @brief 设置默认Servlet
   * @param v 默认Servlet
   */
  void setDefault(Servlet::ptr v) {
    m_default = v;
  }

  /**
   * @brief 获取匹配的Servlet
   * @param uri URI路径
   * @return Servlet::ptr 匹配的Servlet
   */
  Servlet::ptr getMatchedServlet(const std::string& uri);

  /**
   * @brief 列出所有Servlet
   * @param info 输出信息
   * @param prefix 前缀
   */
  void listAllServletCreator(std::map<std::string, IServletCreator::ptr>& infos);

  /**
   * @brief 列出所有模糊匹配Servlet
   * @param info 输出信息
   * @param prefix 前缀
   */
  void listAllGlobServletCreator(std::map<std::string, IServletCreator::ptr>& infos);

private:
  /// 读写锁
  RWMutexType m_mutex;
  /// 精确匹配servlet MAP
  std::unordered_map<std::string, IServletCreator::ptr> m_datas;
  /// 模糊匹配servlet 数组
  std::vector<std::pair<std::string, IServletCreator::ptr>> m_globs;
  /// 默认servlet，所有路径都没匹配到时使用
  Servlet::ptr m_default;
};

/**
 * @brief 404 NotFound Servlet
 */
class NotFoundServlet : public Servlet {
public:
  using ptr = std::shared_ptr<NotFoundServlet>;

  /**
   * @brief 构造函数
   * @param name Servlet名称
   */
  NotFoundServlet(const std::string& name = "NotFoundServlet");

  /**
   * @brief 处理请求
   * @param request HTTP请求
   * @param response HTTP响应
   * @param session HTTP会话
   * @return 处理结果，>0成功，<=0失败
   */
  virtual int32_t handle(HttpRequest::ptr request, HttpResponse::ptr response,
                         IHttpSession::ptr session) override;

private:
  /// 名称
  std::string m_name;
  /// 内容
  std::string m_content;
};

}   // namespace http
}   // namespace sylar