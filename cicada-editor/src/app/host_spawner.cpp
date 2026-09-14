#include "host_spawner.h"
#include <utility>

namespace cicada::editor {
CicadaHostSpawner::CicadaHostSpawner(HostLaunchSpec spec) : spec_(std::move(spec)) {}

std::vector<std::string> CicadaHostSpawner::argv() const {
  std::vector<std::string> out;
  out.reserve(10);
  out.push_back(spec_.node_binary);
  if (!spec_.preload.empty()) { out.push_back("--import"); out.push_back(spec_.preload); }
  out.push_back(spec_.dsh_entry);
  out.insert(out.end(), {"--profile", "cicada", "--no-open", "--port", std::to_string(spec_.port), "--host", "127.0.0.1"});
  return out;
}

std::vector<std::pair<std::string, std::string>> CicadaHostSpawner::env() const {
  return { { "DSH_HOME", spec_.dsh_home }, { "CICADA_HOME", spec_.dsh_home } };
}
}
