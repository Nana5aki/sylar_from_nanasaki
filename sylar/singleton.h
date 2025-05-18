/*
 * @Author: Nana5aki
 * @Date: 2024-11-30 16:36:24
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-18 17:54:27
 * @FilePath: /MySylar/sylar/singleton.h
 */
#ifndef __SYLAR_SINGLETON_H__
#define __SYLAR_SINGLETON_H__

#include <memory>
namespace sylar {

/**
 * @brief 单例模式封装类
 * @details T 类型
 */
template <class T>
class Singleton {
public:
  /**
   * @brief 返回单例裸指针
   */
  static T* GetInstance() {
    static T v;
    return &v;
  }
};

/**
 * @brief 单例模式智能指针封装类
 * @details T 类型
 */
template <class T>
class SingletonPtr {
public:
  /**
   * @brief 返回单例智能指针
   */
  static std::shared_ptr<T> GetInstance() {
    static std::shared_ptr<T> v(new T);
    return v;
  }
};

}   // namespace sylar

#endif