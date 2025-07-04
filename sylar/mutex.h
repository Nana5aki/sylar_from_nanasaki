/*
 * @Author: Nana5aki
 * @Date: 2024-11-27 09:05:56
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-23 21:36:46
 * @FilePath: /sylar_from_nanasaki/sylar/mutex.h
 */
#ifndef __SYLAR_MUTEX_H__
#define __SYLAR_MUTEX_H__

#include "noncopyable.h"

#include <atomic>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <thread>

namespace sylar {

class Semaphore : public Noncopyable {
public:
  /**
   * @brief 构造函数
   * @param[in] count 信号量值的大小
   */
  Semaphore(uint32_t count = 0);

  /**
   * @brief 析构函数
   */
  ~Semaphore();

  /**
   * @brief 获取信号量
   */
  void wait();

  /**
   * @brief 释放信号量
   */
  void notify();

private:
  sem_t m_semaphore;
};

/**
 * @brief 局部锁的模板实现
 */
template <class T>
class ScopedLockImpl {
public:
  /**
   * @brief 构造函数
   * @param[in] mutex Mutex
   */
  ScopedLockImpl(T& mutex)
    : m_mutex(mutex) {
    m_mutex.lock();
    m_locked = true;
  }

  /**
   * @brief 析构函数,自动释放锁
   */
  ~ScopedLockImpl() {
    unlock();
  }

  /**
   * @brief 加锁
   */
  void lock() {
    if (!m_locked) {
      m_mutex.lock();
      m_locked = true;
    }
  }

  /**
   * @brief 解锁
   */
  void unlock() {
    if (m_locked) {
      m_mutex.unlock();
      m_locked = false;
    }
  }

private:
  /// mutex
  T& m_mutex;
  /// 是否已上锁
  bool m_locked;
};

/**
 * @brief 局部读锁模板实现
 */
template <class T>
class ReadScopedLockImpl {
public:
  /**
   * @brief 构造函数
   * @param[in] mutex 读写锁
   */
  ReadScopedLockImpl(T& mutex)
    : m_mutex(mutex) {
    m_mutex.rdlock();
    m_locked = true;
  }

  /**
   * @brief 析构函数,自动释放锁
   */
  ~ReadScopedLockImpl() {
    unlock();
  }

  /**
   * @brief 上读锁
   */
  void lock() {
    if (!m_locked) {
      m_mutex.rdlock();
      m_locked = true;
    }
  }

  /**
   * @brief 释放锁
   */
  void unlock() {
    if (m_locked) {
      m_mutex.unlock();
      m_locked = false;
    }
  }

private:
  /// mutex
  T& m_mutex;
  /// 是否已上锁
  bool m_locked;
};

/**
 * @brief 局部写锁模板实现
 */
template <class T>
class WriteScopedLockImpl {
public:
  /**
   * @brief 构造函数
   * @param[in] mutex 读写锁
   */
  WriteScopedLockImpl(T& mutex)
    : m_mutex(mutex) {
    m_mutex.wrlock();
    m_locked = true;
  }

  /**
   * @brief 析构函数
   */
  ~WriteScopedLockImpl() {
    unlock();
  }

  /**
   * @brief 上写锁
   */
  void lock() {
    if (!m_locked) {
      m_mutex.wrlock();
      m_locked = true;
    }
  }

  /**
   * @brief 解锁
   */
  void unlock() {
    if (m_locked) {
      m_mutex.unlock();
      m_locked = false;
    }
  }

private:
  /// Mutex
  T& m_mutex;
  /// 是否已上锁
  bool m_locked;
};

/**
 * @brief 互斥量
 */
class Mutex : public Noncopyable {
public:
  /// 局部锁
  typedef ScopedLockImpl<Mutex> Lock;

  /**
   * @brief 构造函数
   */
  Mutex() {
    pthread_mutex_init(&m_mutex, nullptr);
  }

  /**
   * @brief 析构函数
   */
  ~Mutex() {
    pthread_mutex_destroy(&m_mutex);
  }

  /**
   * @brief 加锁
   */
  void lock() {
    pthread_mutex_lock(&m_mutex);
  }

  /**
   * @brief 解锁
   */
  void unlock() {
    pthread_mutex_unlock(&m_mutex);
  }

private:
  /// mutex
  pthread_mutex_t m_mutex;
};

/**
 * @brief 读写互斥量
 */
class RWMutex : public Noncopyable {
public:
  /// 局部读锁
  using ReadLock = ReadScopedLockImpl<RWMutex>;

  /// 局部写锁
  using WriteLock = WriteScopedLockImpl<RWMutex>;

  /**
   * @brief 构造函数
   */
  RWMutex() {
    pthread_rwlock_init(&m_lock, nullptr);
  }

  /**
   * @brief 析构函数
   */
  ~RWMutex() {
    pthread_rwlock_destroy(&m_lock);
  }

  /**
   * @brief 上读锁
   */
  void rdlock() {
    pthread_rwlock_rdlock(&m_lock);
  }

  /**
   * @brief 上写锁
   */
  void wrlock() {
    pthread_rwlock_wrlock(&m_lock);
  }

  /**
   * @brief 解锁
   */
  void unlock() {
    pthread_rwlock_unlock(&m_lock);
  }

private:
  /// 读写锁
  pthread_rwlock_t m_lock;
};

/**
 * @brief 自旋锁
 */
class Spinlock : public Noncopyable {
public:
  /// 局部锁
  using Lock = ScopedLockImpl<Spinlock>;

  /**
   * @brief 构造函数
   */
  Spinlock() {
    pthread_spin_init(&m_mutex, 0);
  }

  /**
   * @brief 析构函数
   */
  ~Spinlock() {
    pthread_spin_destroy(&m_mutex);
  }

  /**
   * @brief 上锁
   */
  void lock() {
    pthread_spin_lock(&m_mutex);
  }

  /**
   * @brief 解锁
   */
  void unlock() {
    pthread_spin_unlock(&m_mutex);
  }

private:
  /// 自旋锁
  pthread_spinlock_t m_mutex;
};

/**
 * @brief 原子锁
 */
class CASLock : public Noncopyable {
public:
  /// 局部锁
  using Lock = ScopedLockImpl<CASLock>;

  /**
   * @brief 构造函数
   */
  CASLock() {
    m_mutex.clear();
  }

  /**
   * @brief 析构函数
   */
  ~CASLock() = default;

  /**
   * @brief 上锁
   */
  void lock() {
    while (std::atomic_flag_test_and_set_explicit(&m_mutex, std::memory_order_acquire))
      ;
  }

  /**
   * @brief 解锁
   */
  void unlock() {
    std::atomic_flag_clear_explicit(&m_mutex, std::memory_order_release);
  }

private:
  /// 原子状态
  volatile std::atomic_flag m_mutex;
};

/**
 * @brief 空锁(用于调试)
 */
class NullMutex : Noncopyable {
public:
  /// 局部锁
  using Lock = ScopedLockImpl<NullMutex>;

  /**
   * @brief 构造函数
   */
  NullMutex() = default;

  /**
   * @brief 析构函数
   */
  ~NullMutex() = default;

  /**
   * @brief 加锁
   */
  void lock() {
  }

  /**
   * @brief 解锁
   */
  void unlock() {
  }
};

/**
 * @brief 空读写锁(用于调试)
 */
class NullRWMutex : Noncopyable {
public:
  /// 局部读锁
  using ReadLock = ReadScopedLockImpl<RWMutex>;

  /// 局部写锁
  using WriteLock = WriteScopedLockImpl<RWMutex>;

  /**
   * @brief 构造函数
   */
  NullRWMutex() {
  }
  /**
   * @brief 析构函数
   */
  ~NullRWMutex() {
  }

  /**
   * @brief 上读锁
   */
  void rdlock() {
  }

  /**
   * @brief 上写锁
   */
  void wrlock() {
  }
  /**
   * @brief 解锁
   */
  void unlock() {
  }
};

};   // namespace sylar

#endif