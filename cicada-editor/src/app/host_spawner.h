#pragma once

#include <string>
#include <vector>
#include <utility>

namespace cicada::editor {
struct HostLaunchSpec {
  std::string node_binary;
  std::string dsh_entry;
  std::string dsh_home;
  std::string preload;
  unsigned port = 3123;
  std::string cwd;   // K6：子进程工作目录（模块解析；与 launcher spawnHost"继承调用方 cwd"语义对齐）
};

/** Platform-neutral argv/env description consumed by the wx process layer. */
class CicadaHostSpawner {
 public:
  explicit CicadaHostSpawner(HostLaunchSpec spec);
  std::vector<std::string> argv() const;
  // 注入子进程的环境变量（叠加在继承环境之上；K6：launcher 两个 home 解析入口）
  std::vector<std::pair<std::string, std::string>> env() const;
  const HostLaunchSpec& spec() const { return spec_; }
 private:
  HostLaunchSpec spec_;
};
}
