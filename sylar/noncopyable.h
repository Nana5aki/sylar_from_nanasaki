/*
 * @Author: Nana5aki
 * @Date: 2024-11-27 09:07:13
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-23 21:36:52
 * @FilePath: /MySylar/sylar/noncopyable.h
 */
#ifndef __SYLAR_NONCOPYABLE_H__
#define __SYLAR_NONCOPYABLE_H__

namespace sylar {

class Noncopyable {
public:
  /**
   * @brief 默认构造函数
   */
  Noncopyable() = default;

  /**
   * @brief 默认析构函数
   */
  ~Noncopyable() = default;

  /**
   * @brief 拷贝构造函数(禁用)
   */
  Noncopyable(const Noncopyable&) = delete;

  /**
   * @brief 赋值函数(禁用)
   */
  Noncopyable& operator=(const Noncopyable&) = delete;
};

};   // namespace sylar

#endif