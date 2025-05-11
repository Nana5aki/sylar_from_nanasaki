/*
 * @Author: Nana5aki
 * @Date: 2025-05-11 00:26:56
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-11 00:42:01
 * @FilePath: /MySylar/sylar/util/fs_util.h
 */
#pragma once

#include <string>
#include <vector>

namespace sylar {
namespace FSUtil {

/**
 * @brief
 * 递归列举指定目录下所有指定后缀的常规文件，如果不指定后缀，则遍历所有文件，返回的文件名带路径
 * @param[out] files 文件列表
 * @param[in] path 路径
 * @param[in] subfix 后缀名，比如 ".yml"
 */
void ListAllFile(std::vector<std::string>& files, const std::string& path,
                 const std::string& subfix);

/**
 * @brief 删除文件，参考unlink(2)
 * @param[in] filename 文件名
 * @param[in] exist 是否存在
 * @return  是否成功
 * @note 内部会判断一次是否真的不存在该文件
 */
bool Unlink(const std::string& filename, bool exist = false);

bool Mkdir(const std::string& dirname);
bool Rm(const std::string& path);
bool Mv(const std::string& from, const std::string& to);

}   // namespace FSUtil
}   // namespace sylar