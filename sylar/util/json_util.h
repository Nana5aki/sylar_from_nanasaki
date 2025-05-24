/*
 * @Author: Nana5aki
 * @Date: 2025-05-24 19:41:41
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-24 19:46:25
 * @FilePath: /sylar_from_nanasaki/sylar/util/json_util.h
 */
#pragma once

#include <json/json.h>
#include <string>

namespace sylar {
namespace JsonUtil {

std::string ToString(const Json::Value& json);

}   // namespace JsonUtil
}   // namespace sylar