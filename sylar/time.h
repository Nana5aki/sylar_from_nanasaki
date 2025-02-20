/*
 * @Author: Nana5aki
 * @Date: 2025-01-12 15:10:42
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-02-15 18:03:21
 * @FilePath: /MySylar/sylar/time.h
 */
#ifndef __SYLAR_TIMER_H__
#define __SYLAR_TIMER_H__

#include "mutex.h"
#include <functional>
#include <memory>
#include <set>
#include <vector>

namespace sylar {

class TimerManager;

class Timer : public std::enable_shared_from_this<Timer> {
  friend TimerManager;

public:
  using ptr = std::shared_ptr<Timer>;

public:
  /**
   * @brief 取消定时器
   */
  bool cancel();

  /**
   * @brief 刷新设置定时器的执行时间
   */
  bool refresh();

  /**
   * @brief 重置定时器时间
   * @param[in] ms 定时器执行间隔时间(毫秒)
   * @param[in] from_now 是否从当前时间开始计算
   */
  bool reset(uint64_t ms, bool from_now);

private:
  /**
   * @brief 构造函数
   * @param[in] ms 定时器执行间隔时间
   * @param[in] cb 回调函数
   * @param[in] recurring 是否循环
   * @param[in] manager 定时器管理器
   */
  Timer(uint64_t ms, std::function<void()> cb, bool recurring,
        std::shared_ptr<TimerManager> manager);

  /**
   * @brief 构造函数
   * @param[in] next 执行的时间戳(毫秒)
   */
  Timer(uint64_t next);

private:
  /// 是否循环定时器
  bool m_recurring = false;
  /// 执行周期
  uint64_t m_ms = 0;
  /// 精确的执行时间
  uint64_t m_next = 0;
  /// 回调函数
  std::function<void()> m_cb;
  /// 定时器管理器
  std::weak_ptr<TimerManager> m_manager;

private:
  /**
   * @brief 定时器比较仿函数
   */
  struct Comparator {
    /**
     * @brief 比较定时器的智能指针的大小(按执行时间排序)
     * @param[in] lhs 定时器智能指针
     * @param[in] rhs 定时器智能指针
     */
    bool operator()(const Timer::ptr& lhs, const Timer::ptr& rhs) const;
  };
};

/**
 * @brief 定时器管理器
 */
class TimerManager : public std::enable_shared_from_this<TimerManager>{
  friend Timer;
  using RWMutexType = RWMutex;

public:
  /**
   * @brief 构造函数
   */
  TimerManager();

  /**
   * @brief 析构函数
   */
  virtual ~TimerManager() = default;

  /**
   * @brief 添加定时器
   * @param[in] ms 定时器执行间隔时间
   * @param[in] cb 定时器回调函数
   * @param[in] recurring 是否循环定时器
   */
  Timer::ptr addTimer(uint64_t ms, std::function<void()> cb, bool recurring = false);

  /**
   * @brief 添加条件定时器
   * @param[in] ms 定时器执行间隔时间
   * @param[in] cb 定时器回调函数
   * @param[in] weak_cond 条件
   * @param[in] recurring 是否循环
   */
  Timer::ptr addConditionTimer(uint64_t ms, std::function<void()> cb, std::weak_ptr<void> weak_cond,
                               bool recurring = false);

  /**
   * @brief 到最近一个定时器执行的时间间隔(毫秒)
   */
  uint64_t getNextTimer();

  /**
   * @brief 获取需要执行的定时器的回调函数列表
   * @param[out] cbs 回调函数数组
   */
  void listExpiredCb(std::vector<std::function<void()>>& cbs);

  /**
   * @brief 是否有定时器
   */
  bool hasTimer();

protected:
  /**
   * @brief 当有新的定时器插入到定时器的首部,执行该函数
   */
  virtual void onTimerInsertedAtFront() = 0;

  /**
   * @brief 将定时器添加到管理器中
   */
  void addTimer(Timer::ptr val, RWMutexType::WriteLock& lock);

private:
  /**
   * @brief 检测服务器时间是否被调后了
   */
  bool detectClockRollover(uint64_t now_ms);

private:
  /// Mutex
  RWMutexType m_mutex;
  /// 定时器集合
  std::set<Timer::ptr, Timer::Comparator> m_timers;
  /// 是否触发onTimerInsertedAtFront
  bool m_tickled = false;
  /// 上次执行时间
  uint64_t m_previouseTime = 0;
};

}   // namespace sylar

#endif