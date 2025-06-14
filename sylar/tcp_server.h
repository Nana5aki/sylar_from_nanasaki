/*
 * @Author: Nana5aki
 * @Date: 2025-04-13 23:01:20
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-04-14 23:25:18
 * @FilePath: /sylar_from_nanasaki/sylar/tcp_server.h
 */
#ifndef __SYLAR_TCP_SERVER_H__
#define __SYLAR_TCP_SERVER_H__

#include "sylar/iomanager.h"
#include "sylar/noncopyable.h"
#include "sylar/socket.h"
#include <memory>
#include <vector>

namespace sylar {

/// @brief TCP服务封装
class TcpServer : public std::enable_shared_from_this<TcpServer>, Noncopyable {
public:
  using ptr = std::shared_ptr<TcpServer>;

  /**
   * @brief 构造函数
   * @param[in] name 服务器名称
   * @param[in] type 服务器类型
   * @param[in] io_worker socket客户端工作的协程调度器
   * @param[in] accept_worker 服务器socket执行接收socket连接的协程调度器
   */
  TcpServer(IOManager* io_woker = IOManager::GetThis(),
            IOManager* accept_worker = IOManager::GetThis());

  /**
   * @brief 析构函数
   */
  virtual ~TcpServer();

  /**
   * @brief 绑定地址
   * @return 返回是否绑定成功
   */
  virtual bool bind(Address::ptr addr);

  /**
   * @brief 绑定地址数组
   * @param[in] addrs 需要绑定的地址数组
   * @param[out] fails 绑定失败的地址
   * @return 是否绑定成功
   */
  virtual bool bind(const std::vector<Address::ptr>& addrs, std::vector<Address::ptr>& fails);

  /**
   * @brief 启动服务
   * @pre 需要bind成功后执行
   */
  virtual bool start();

  /**
   * @brief 停止服务
   */
  virtual void stop();

  /**
   * @brief 返回读取超时时间(毫秒)
   */
  uint64_t getRecvTimeout() const {
    return m_recvTimeout;
  }

  /**
   * @brief 返回服务器名称
   */
  std::string getName() const {
    return m_name;
  }

  /**
   * @brief 设置读取超时时间(毫秒)
   */
  void setRecvTimeout(uint64_t v) {
    m_recvTimeout = v;
  }

  /**
   * @brief 设置服务器名称
   */
  virtual void setName(const std::string& v) {
    m_name = v;
  }

  /**
   * @brief 是否停止
   */
  bool isStop() const {
    return m_isStop;
  }

  /**
   * @brief 以字符串形式dump server信息
   */
  virtual std::string toString(const std::string& prefix = "");

protected:
  /**
   * @brief 处理新连接的Socket类
   */
  virtual void handleClient(Socket::ptr client);

  /**
   * @brief 开始接受连接
   */
  virtual void startAccept(Socket::ptr sock);


protected:
  /// 监听Socket数组
  std::vector<Socket::ptr> m_socks;
  /// 新连接的Socket工作的调度器
  IOManager* m_ioWorker;
  /// 服务器Socket接收连接的调度器
  IOManager* m_acceptWorker;
  /// 接收超时时间(毫秒)
  uint64_t m_recvTimeout;
  /// 服务器名称
  std::string m_name;
  /// 服务器类型
  std::string m_type;
  /// 服务是否停止
  bool m_isStop;
};

}   // namespace sylar

#endif