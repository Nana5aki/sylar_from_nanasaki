/*
 * @Author: Nana5aki
 * @Date: 2025-05-24 19:41:46
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-24 20:31:32
 * @FilePath: /sylar_from_nanasaki/sylar/util/json_util.cc
 */
#include "json_util.h"
#include "json/writer.h"

namespace sylar {
namespace JsonUtil {

std::string ToString(const Json::Value& json) {
  Json::FastWriter w;
  return w.write(json);
}

}   // namespace JsonUtil
}   // namespace sylar