/*
 * @Author: Nana5aki
 * @Date: 2025-01-12 15:10:46
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-02-22 14:20:21
 * @FilePath: /MySylar/sylar/time.cc
 */
#include "time.h"
#include "macro.h"
#include "util.h"

namespace sylar {

bool Timer::Comparator::operator()(const Timer::ptr& lhs, const Timer::ptr& rhs) const {
  SYLAR_ASSERT(lhs && rhs);
  if (lhs->m_next != rhs->m_next) return lhs->m_next < rhs->m_next;
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
  if (!m_cb) return false;
  if (auto manager = m_manager.lock()) {
    TimerManager::RWMutexType::WriteLock lock(manager->m_mutex);
    m_cb = nullptr;
    auto it = manager->m_timers.find(shared_from_this());
    if (it != manager->m_timers.end()) {
      manager->m_timers.erase(it);
      return true;
    }
  }
  return false;
}

bool Timer::refresh() {
  if (!m_cb) return false;
  if (auto manager = m_manager.lock()) {
    TimerManager::RWMutexType::WriteLock lock(manager->m_mutex);
    auto it = manager->m_timers.find(shared_from_this());
    if (it == manager->m_timers.end()) return false;
    manager->m_timers.erase(it);
    m_next = sylar::GetElapsedMS() + m_ms;
    manager->m_timers.insert(shared_from_this());
    return true;
  }
  return false;
}

bool Timer::reset(uint64_t ms, bool from_now) {
  if (ms == m_ms && !from_now) return true;
  if (!m_cb) return false;
  if (auto manager = m_manager.lock()) {
    TimerManager::RWMutexType::WriteLock lock(manager->m_mutex);
    auto it = manager->m_timers.find(shared_from_this());
    if (it == manager->m_timers.end()) return false;
    manager->m_timers.erase(it);
    uint64_t start = from_now ? sylar::GetElapsedMS() : (m_next - m_ms);
    m_ms = ms;
    m_next = start + m_ms;
    manager->addTimer(shared_from_this(), lock);
    return true;
  }
  return false;
}

TimerManager::TimerManager() {
}

Timer::ptr TimerManager::addTimer(uint64_t ms, std::function<void()> cb, bool recurring) {
  Timer::ptr timer(new Timer(ms, std::move(cb), recurring, shared_from_this()));
  RWMutexType::WriteLock lock(m_mutex);
  addTimer(timer, lock);
  return timer;
}

static void OnTimer(std::weak_ptr<void> weak_cond, std::function<void()> cb) {
  if (auto ptr = weak_cond.lock()) {
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
  if (m_timers.empty()) return ~0ull;   // 对0ull取反

  const Timer::ptr& next = *m_timers.begin();
  uint64_t now_ms = sylar::GetElapsedMS();
  return now_ms >= next->m_next ? 0 : next->m_next - now_ms;
}

void TimerManager::listExpiredCb(std::vector<std::function<void()>>& cbs) {
  uint64_t now_ms = sylar::GetElapsedMS();
  std::vector<Timer::ptr> expired;

  RWMutexType::WriteLock lock(m_mutex);
  if (m_timers.empty()) return;

  Timer::ptr now_timer(new Timer(now_ms));
  auto it = m_timers.lower_bound(now_timer);
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
  if (at_front) m_tickled = true;

  lock.unlock();

  if (at_front) {
    onTimerInsertedAtFront();
  }
}

bool TimerManager::hasTimer() {
  RWMutexType::ReadLock lock(m_mutex);
  return !m_timers.empty();
}

}   // namespace sylar
