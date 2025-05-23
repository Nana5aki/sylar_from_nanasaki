/*
 * @Author: Nana5aki
 * @Date: 2025-04-02 20:20:06
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-04-02 22:21:24
 * @FilePath: /sylar_from_nanasaki/sylar/endian.h
 */
#ifndef __SYLAR_ENDIAN_H__
#define __SYLAR_ENDIAN_H__

#define SYLAR_LITTLE_ENDIAN 1
#define SYLAR_BIG_ENDIAN 2

#include <byteswap.h>
#include <stdint.h>
#include <type_traits>

namespace sylar {

/**
 * @brief 8字节类型的字节序反转
 */
template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type byteswap(T value) {
  return (T)bswap_64((uint64_t)value);
}

/**
 * @brief 4字节类型的字节序反转
 */
template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type byteswap(T value) {
  return (T)bswap_32((uint32_t)value);
}

/**
 * @brief 2字节类型的字节序反转
 */
template <class T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type byteswap(T value) {
  return (T)bswap_16((uint16_t)value);
}

#if BYTE_ORDER == BIG_ENDIAN
#  define SYLAR_BYTE_ORDER SYLAR_BIG_ENDIAN
#else
#  define SYLAR_BYTE_ORDER SYLAR_LITTLE_ENDIAN
#endif

#if SYLAR_BYTE_ORDER == SYLAR_BIG_ENDIAN

/**
 * @brief 只在小端机器上执行byteswap, 在大端机器上什么都不做
 */
template <class T>
T byteswapOnLittleEndian(T t) {
  return t;
}

/**
 * @brief 只在大端机器上执行byteswap, 在小端机器上什么都不做
 */
template <class T>
T byteswapOnBigEndian(T t) {
  return byteswap(t);
}
#else

/**
 * @brief 只在小端机器上执行byteswap, 在大端机器上什么都不做
 */
template <class T>
T byteswapOnLittleEndian(T t) {
  return byteswap(t);
}

/**
 * @brief 只在大端机器上执行byteswap, 在小端机器上什么都不做
 */
template <class T>
T byteswapOnBigEndian(T t) {
  return t;
}
#endif

}   // namespace sylar

#endif