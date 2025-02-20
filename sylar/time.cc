/*
 * @Author: Nana5aki
 * @Date: 2025-01-12 15:10:46
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-02-21 00:03:01
 * @FilePath: /MySylar/sylar/time.cc
 */
#include "time.h"
#include "macro.h"
#include "util.h"

namespace sylar {

bool Timer::Comparator::operator()(const Timer::ptr& lhs, const Timer::ptr& rhs) const {
  if (!rhs) {
    return false;
  }
  if (!lhs) {
    return true;
  }
  if (lhs->m_next != rhs->m_next) {
    return lhs->m_next < rhs->m_next;
  }
  return lhs.get() < rhs.get();
}

Timer::Timer(uint64_t ms, std::function<void()> cb, bool recurring,
             std::shared_ptr<TimerManager> manager)
  : m_recurring(recurring)
  , m_ms(ms)
  , m_cb(cb)
  , m_manager(manager) {
  m_next = sylar::GetElapsedMS() + m_ms;
}

Timer::Timer(uint64_t next)
  : m_next(next) {
}

bool Timer::cancel() {
  return true;
}

TimerManager::TimerManager() {
  m_previouseTime = sylar::GetElapsedMS();
}

Timer::ptr TimerManager::addTimer(uint64_t ms, std::function<void()> cb, bool recurring) {
  Timer::ptr timer(new Timer(ms, std::move(cb), recurring, shared_from_this()));
  RWMutexType::WriteLock lock(m_mutex);
  addTimer(timer, lock);
  return timer;
}

static void OnTimer(std::weak_ptr<void> weak_cond, std::function<void()> cb) {
  std::shared_ptr<void> tmp = weak_cond.lock();
  if (tmp) {
    cb();
  }
}

Timer::ptr TimerManager::addConditionTimer(uint64_t ms, std::function<void()> cb,
                                           std::weak_ptr<void> weak_cond, bool recurring) {
  return addTimer(ms, std::bind(&OnTimer, weak_cond, cb), recurring);
}

uint64_t TimerManager::getNextTimer() {
  RWMutexType::ReadLock lock(m_mutex);
  m_tickled = false;
  if (m_timers.empty()) {
    return ~0ull;   // 对0ull取反
  }

  const Timer::ptr& next = *m_timers.begin();
  uint64_t now_ms = sylar::GetElapsedMS();
  return now_ms >= next->m_next ? 0 : next->m_next - now_ms;
}

void TimerManager::listExpiredCb(std::vector<std::function<void()>>& cbs) {
  uint64_t now_ms = sylar::GetElapsedMS();
  std::vector<Timer::ptr> expired;
  ///@todo：感觉不需要读锁这一部分
  {
    RWMutexType::ReadLock lock(m_mutex);
    if (m_timers.empty()) {
      return;
    }
  }

  RWMutexType::WriteLock lock(m_mutex);
  if (m_timers.empty()) {
    return;
  }

  bool rollover = false;

  if (SYLAR_UNLIKELY(detectClockRollover(now_ms))) {
    // 使用clock_gettime(CLOCK_MONOTONIC_RAW)，应该不可能出现时间回退的问题
    rollover = true;
  }

  if (!rollover && ((*m_timers.begin())->m_next > now_ms)) {
    return;
  }

  Timer::ptr now_timer(new Timer(now_ms));
  auto it = rollover ? m_timers.end() : m_timers.lower_bound(now_timer);
  while (it != m_timers.end() && (*it)->m_next == now_ms) {
    ++it;
  }
  expired.insert(expired.begin(), m_timers.begin(), it);
  m_timers.erase(m_timers.begin(), it);
  cbs.reserve(expired.size());

  for (auto& timer : expired) {
    cbs.push_back(timer->m_cb);
    if (timer->m_recurring) {
      timer->m_next = now_ms + timer->m_ms;
      m_timers.insert(timer);
    } else {
      timer->m_cb = nullptr;
    }
  }
}

void TimerManager::addTimer(Timer::ptr val, RWMutexType::WriteLock& lock) {
  const auto it = m_timers.insert(val).first;
  const bool at_front = (it == m_timers.begin()) && !m_tickled;
  if (at_front) {
    m_tickled = true;
  }

  lock.unlock();

  if (at_front) {
    onTimerInsertedAtFront();
  }
}

bool TimerManager::detectClockRollover(uint64_t now_ms) {
  m_previouseTime = now_ms;
  return (now_ms < m_previouseTime && now_ms < (m_previouseTime - 60 * 60 * 1000));
}

bool TimerManager::hasTimer() {
  RWMutexType::ReadLock lock(m_mutex);
  return !m_timers.empty();
}

}   // namespace sylar
