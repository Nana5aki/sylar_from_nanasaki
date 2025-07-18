/*
 * @Author: Nana5aki
 * @Date: 2025-04-13 12:07:26
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-04-13 12:56:44
 * @FilePath: /sylar_from_nanasaki/sylar/stream.h
 */

#ifndef __SYLAR_STREAM_H__
#define __SYLAR_STREAM_H__

#include <memory>
#include "bytearray.h"
namespace sylar {

/// @brief 流式结构
class Stream {
public:
  using ptr = std::shared_ptr<Stream>;

  virtual ~Stream() = default;

  /**
   * @brief 读数据
   * @param[out] buffer 接收数据的内存
   * @param[in] length 接收数据的内存大小
   * @return
   *      @retval >0 返回接收到的数据的实际大小
   *      @retval =0 被关闭
   *      @retval <0 出现流错误
   */
  virtual int read(void* buffer, size_t length) = 0;

  /**
   * @brief 读数据
   * @param[out] ba 接收数据的ByteArray
   * @param[in] length 接收数据的内存大小
   * @return
   *      @retval >0 返回接收到的数据的实际大小
   *      @retval =0 被关闭
   *      @retval <0 出现流错误
   */
  virtual int read(ByteArray::ptr ba, size_t length) = 0;

  /**
   * @brief 读固定长度的数据
   * @param[out] buffer 接收数据的内存
   * @param[in] length 接收数据的内存大小
   * @return
   *      @retval >0 返回接收到的数据的实际大小
   *      @retval =0 被关闭
   *      @retval <0 出现流错误
   */
  virtual int readFixSize(void* buffer, size_t length);

  /**
   * @brief 读固定长度的数据
   * @param[out] ba 接收数据的ByteArray
   * @param[in] length 接收数据的内存大小
   * @return
   *      @retval >0 返回接收到的数据的实际大小
   *      @retval =0 被关闭
   *      @retval <0 出现流错误
   */
  virtual int readFixSize(ByteArray::ptr ba, size_t length);

  /**
   * @brief 写数据
   * @param[in] buffer 写数据的内存
   * @param[in] length 写入数据的内存大小
   * @return
   *      @retval >0 返回写入到的数据的实际大小
   *      @retval =0 被关闭
   *      @retval <0 出现流错误
   */
  virtual int write(const void* buffer, size_t length) = 0;

  /**
   * @brief 写数据
   * @param[in] ba 写数据的ByteArray
   * @param[in] length 写入数据的内存大小
   * @return
   *      @retval >0 返回写入到的数据的实际大小
   *      @retval =0 被关闭
   *      @retval <0 出现流错误
   */
  virtual int write(ByteArray::ptr ba, size_t length) = 0;

  /**
   * @brief 写固定长度的数据
   * @param[in] buffer 写数据的内存
   * @param[in] length 写入数据的内存大小
   * @return
   *      @retval >0 返回写入到的数据的实际大小
   *      @retval =0 被关闭
   *      @retval <0 出现流错误
   */
  virtual int writeFixSize(const void* buffer, size_t length);

  /**
   * @brief 写固定长度的数据
   * @param[in] ba 写数据的ByteArray
   * @param[in] length 写入数据的内存大小
   * @return
   *      @retval >0 返回写入到的数据的实际大小
   *      @retval =0 被关闭
   *      @retval <0 出现流错误
   */
  virtual int writeFixSize(ByteArray::ptr ba, size_t length);

  /**
   * @brief 关闭流
   */
  virtual void close() = 0;
};

}   // namespace sylar

#endif
