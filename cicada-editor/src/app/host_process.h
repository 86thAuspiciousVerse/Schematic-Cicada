#pragma once

#include "host_spawner.h"
#include <string>

namespace cicada::editor {
class HostProcess {
 public:
  HostProcess() = default;
  ~HostProcess();
  HostProcess(const HostProcess&) = delete;
  HostProcess& operator=(const HostProcess&) = delete;
  bool start(const CicadaHostSpawner& spawner, std::string* error = nullptr);
  bool running() const;
  void terminate();
  std::string read_stdout() const;
 private:
  void* process_ = nullptr;
  void* stdout_read_ = nullptr;
};
}
