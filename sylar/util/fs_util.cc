/*
 * @Author: Nana5aki
 * @Date: 2025-05-11 00:27:02
 * @LastEditors: Nana5aki
 * @LastEditTime: 2025-05-18 17:56:23
 * @FilePath: /sylar_from_nanasaki/sylar/util/fs_util.cc
 */
#include "fs_util.h"
#include <cstring>
#include <dirent.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

namespace sylar {
namespace fs_util {

void ListAllFile(std::vector<std::string>& files, const std::string& path,
                 const std::string& subfix) {
  if (access(path.c_str(), 0) != 0) {
    return;
  }
  DIR* dir = opendir(path.c_str());
  if (dir == nullptr) {
    return;
  }
  struct dirent* dp = nullptr;
  while ((dp = readdir(dir)) != nullptr) {
    if (dp->d_type == DT_DIR) {
      if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) {
        continue;
      }
      ListAllFile(files, path + "/" + dp->d_name, subfix);
    } else if (dp->d_type == DT_REG) {
      std::string filename(dp->d_name);
      if (subfix.empty()) {
        files.push_back(path + "/" + filename);
      } else {
        if (filename.size() < subfix.size()) {
          continue;
        }
        if (filename.substr(filename.length() - subfix.size()) == subfix) {
          files.push_back(path + "/" + filename);
        }
      }
    }
  }
  closedir(dir);
}

static int __lstat(const char* file, struct stat* st = nullptr) {
  struct stat lst;
  int ret = lstat(file, &lst);
  if (st) {
    *st = lst;
  }
  return ret;
}

bool Unlink(const std::string& filename, bool exist) {
  if (!exist && __lstat(filename.c_str())) {
    return true;
  }
  return ::unlink(filename.c_str()) == 0;
}

static int __mkdir(const char* dirname) {
  if (access(dirname, F_OK) == 0) {
    return 0;
  }
  return mkdir(dirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

bool Mkdir(const std::string& dirname) {
  if (__lstat(dirname.c_str()) == 0) {
    return true;
  }
  char* path = strdup(dirname.c_str());
  char* ptr = strchr(path + 1, '/');
  do {
    for (; ptr; *ptr = '/', ptr = strchr(ptr + 1, '/')) {
      *ptr = '\0';
      if (__mkdir(path) != 0) {
        break;
      }
    }
    if (ptr != nullptr) {
      break;
    } else if (__mkdir(path) != 0) {
      break;
    }
    free(path);
    return true;
  } while (0);
  free(path);
  return false;
}

bool Rm(const std::string& path) {
  struct stat st;
  if (lstat(path.c_str(), &st)) {
    return true;
  }
  if (!(st.st_mode & S_IFDIR)) {
    return Unlink(path);
  }

  DIR* dir = opendir(path.c_str());
  if (!dir) {
    return false;
  }

  bool ret = true;
  struct dirent* dp = nullptr;
  while ((dp = readdir(dir))) {
    if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) {
      continue;
    }
    std::string dirname = path + "/" + dp->d_name;
    ret = Rm(dirname);
  }
  closedir(dir);
  if (::rmdir(path.c_str())) {
    ret = false;
  }
  return ret;
}

bool Mv(const std::string& from, const std::string& to) {
  if (!Rm(to)) {
    return false;
  }
  return rename(from.c_str(), to.c_str()) == 0;
}

}   // namespace fs_util
}   // namespace sylar