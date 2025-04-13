/*
 * @Author: Nana5aki
 * @Date: 2025-04-13 12:07:31
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-04-13 13:50:07
 * @FilePath: /MySylar/sylar/stream.cc
 */
#include "stream.h"

namespace sylar {

int Stream::readFixSize(void* buffer, size_t length) {
  size_t offset = 0;
  int64_t left = length;
  while (left > 0) {
    int64_t len = read((char*)buffer + offset, left);
    if (len <= 0) {
      return len;
    }
    offset += len;
    left -= len;
  }
  return length;
}

int Stream::readFixSize(ByteArray::ptr ba, size_t length) {
  int64_t left = length;
  while (left > 0) {
    int64_t len = read(ba, left);
    if (len <= 0) {
      return len;
    }
    left -= len;
  }
  return length;
}

int Stream::writeFixSize(const void* buffer, size_t length) {
  size_t offset = 0;
  int64_t left = length;
  while (left > 0) {
    int64_t len = write((const char*)buffer + offset, left);
    if (len <= 0) {
      return len;
    }
    offset += len;
    left -= len;
  }
  return length;
}

int Stream::writeFixSize(ByteArray::ptr ba, size_t length) {
  int64_t left = length;
  while (left > 0) {
    int64_t len = write(ba, left);
    if (len <= 0) {
      return len;
    }
    left -= len;
  }
  return length;
}

}   // namespace sylar
