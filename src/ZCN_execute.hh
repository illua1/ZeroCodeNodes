#pragma once

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>

#include "ZCN_node.hh"
#include "ZCN_runtime_type.hh"

namespace zcn {

struct ExecuteLog {
  std::unordered_map<std::string, RData> socket_value;
};

class SideEffectReciver {
 public:
  virtual ~SideEffectReciver() = default;

  virtual RData get_from_path(std::string path) = 0;
  virtual void set_for_path(std::string path, RData data) = 0;
};

class FileSystemProvider : public SideEffectReciver {
 public:
  ~FileSystemProvider() override = default;

  RData get_from_path(std::string path) override;
  void set_for_path(std::string path, RData data) override;
};

class VirtualFileSystemProvider : public SideEffectReciver {
 public:
  std::unordered_map<std::string, std::string> file_overide;

  ~VirtualFileSystemProvider() override = default;

  RData get_from_path(std::string path) override;
  void set_for_path(std::string path, RData data) override;
};

void execute(const TreePtr &tree, ExecuteLog &log, SideEffectReciver &reciver);

}