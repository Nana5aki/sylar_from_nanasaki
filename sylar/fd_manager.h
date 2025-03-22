/*
 * @Author: Nana5aki
 * @Date: 2025-03-16 11:30:08
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-03-22 12:11:32
 * @FilePath: /MySylar/sylar/fd_manager.h
 */
#ifndef __SYLAR_FD_MANAGER_H__
#define __SYLAR_FD_MANAGER_H__

#include "mutex.h"
#include "singleton.h"
#include <memory>
#include <vector>

/**
 * @brief 文件句柄上下文类
 * @details 管理文件句柄类型(是否socket)
 *          是否阻塞,是否关闭,读/写超时时间
 */
namespace sylar {

class FdCtx : public std::enable_shared_from_this<FdCtx> {
public:
  using ptr = std::shared_ptr<FdCtx>;

public:
  /**
   * @brief 通过文件句柄构造FdCtx
   */
  FdCtx(int fd);
  /**
   * @brief 析构函数
   */
  ~FdCtx() = default;

  /**
   * @brief 是否初始化完成
   */
  bool isInit() const {
    return m_isInit;
  }

  /**
   * @brief 是否socket
   */
  bool isSocket() const {
    return m_isSocket;
  }

  /**
   * @brief 是否已关闭
   */
  bool isClose() const {
    return m_isClosed;
  }

  /**
   * @brief 设置用户主动设置非阻塞
   * @param[in] v 是否阻塞
   */
  void setUserNonblock(bool v) {
    m_userNonblock = v;
  }

  /**
   * @brief 获取是否用户主动设置的非阻塞
   */
  bool getUserNonblock() const {
    return m_userNonblock;
  }

  /**
   * @brief 设置系统非阻塞
   * @param[in] v 是否阻塞
   */
  void setSysNonblock(bool v) {
    m_sysNonblock = v;
  }

  /**
   * @brief 获取系统非阻塞
   */
  bool getSysNonblock() const {
    return m_sysNonblock;
  }

  /**
   * @brief 设置超时时间
   * @param[in] type 类型SO_RCVTIMEO(读超时), SO_SNDTIMEO(写超时)
   * @param[in] v 时间毫秒
   */
  void setTimeout(int type, uint64_t v);

  /**
   * @brief 获取超时时间
   * @param[in] type 类型SO_RCVTIMEO(读超时), SO_SNDTIMEO(写超时)
   * @return 超时时间毫秒
   */
  uint64_t getTimeout(int type);

private:
  /**
   * @brief 初始化
   */
  bool init();

private:
  /// 是否初始化
  bool m_isInit = false;
  /// 是否socket
  bool m_isSocket = false;
  /// 是否hook非阻塞
  bool m_sysNonblock = false;
  /// 是否用户主动设置非阻塞
  bool m_userNonblock = false;
  /// 是否关闭
  bool m_isClosed = false;
  /// 文件句柄
  int m_fd;
  /// 读超时时间毫秒
  uint64_t m_recvTimeout;
  /// 写超时时间毫秒
  uint64_t m_sendTimeout;
};

class FdManager {
public:
  using RWMutexType = RWMutex;

public:
  /**
   * @brief 无参构造函数
   */
  FdManager();

  /**
   * @brief 获取/创建文件句柄类FdCtx
   * @param[in] fd 文件句柄
   * @param[in] auto_create 是否自动创建
   * @return 返回对应文件句柄类FdCtx::ptr
   */
  FdCtx::ptr get(int fd, bool auto_create = false);

  /**
   * @brief 删除文件句柄类
   * @param[in] fd 文件句柄
   */
  void del(int fd);

private:
  /// 读写锁
  RWMutexType m_mutex;
  /// 文件句柄集合
  std::vector<FdCtx::ptr> m_datas;
};

using FdMgr = Singleton<FdManager>;

}   // namespace sylar


#endif