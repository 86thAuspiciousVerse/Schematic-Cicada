#include "host_process.h"
#include <sstream>
#include <fstream>
#ifdef _WIN32
#include <windows.h>
#endif
namespace cicada::editor {
#ifdef _WIN32
static std::wstring wide(const std::string& text) {
  if (text.empty()) return {};
  const int size = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
  if (size <= 0) return {};
  std::wstring out(static_cast<std::size_t>(size), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), out.data(), size); return out;
}

// K6：环境块 = 继承环境 + spawner 注入项（DSH_HOME/CICADA_HOME）。之前 lpEnvironment=nullptr
// 导致 launcher home 解析落 ~/.cicada/home（错配），且原始 env 完全不透传。
static std::wstring build_env_block(const std::vector<std::pair<std::string, std::string>>& extra) {
  std::wstring block;
  auto append = [&](const std::wstring& line) { block.append(line); block.push_back(L'\0'); };
  LPWCH raw = ::GetEnvironmentStringsW();
  if (raw) {
    for (LPWCH p = raw; *p; p += wcslen(p) + 1) {
      // K6 修复：剔除 =X:=... per-drive 当前目录伪变量（cmd 进程 env 自带；Copy 进新块且与
      // lpCurrentDirectory 不一致时 CreateProcessW 报 ERROR_INVALID_PARAMETER(87)——探针实证）
      if (*p == L'=')
        continue;
      // 被注入覆盖的键：跳过原值（避免重复）
      std::wstring key(p);
      const size_t eq = key.find(L'=');
      std::wstring k = eq == std::wstring::npos ? key : key.substr(0, eq);
      bool skip = false;
      for (const auto& [ek, ev] : extra)
        if (k == wide(ek)) { skip = true; break; }
      if (!skip) append(key);
    }
    ::FreeEnvironmentStringsW(raw);
  }
  for (const auto& [ek, ev] : extra) append(wide(ek + "=" + ev));
  block.push_back(L'\0');   // 双空结尾
  return block;
}
#endif
HostProcess::~HostProcess() { terminate(); }
bool HostProcess::start(const CicadaHostSpawner& spawner, std::string* error) {
#ifdef _WIN32
  terminate(); SECURITY_ATTRIBUTES security{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE}; HANDLE read_handle = nullptr, write_handle = nullptr;
  if (!CreatePipe(&read_handle, &write_handle, &security, 0)) { if (error) *error = "CreatePipe failed"; return false; }
  SetHandleInformation(read_handle, HANDLE_FLAG_INHERIT, 0);
  std::ostringstream command; for (const auto& arg : spawner.argv()) { if (command.tellp() > 0) command << ' '; command << '"' << arg << '"'; }
  std::wstring command_line = wide(command.str());
  std::wstring cwd = wide(spawner.spec().cwd.empty() ? spawner.spec().dsh_home : spawner.spec().cwd);
  // K6：不再构建 env 块（块内的 =X:= 伪变量/条目类型触发 CreateProcessW ERROR_INVALID_PARAMETER(87)）。
  // 改为：spawn 前在父进程注入变量（子进程自然继承）+ 创建后还原。
  std::vector<std::wstring> injected;
  for (const auto& [k, v] : spawner.env()) {
    const std::wstring kw = wide(k), vw = wide(v);
    ::SetEnvironmentVariableW(kw.c_str(), vw.c_str());
    injected.push_back(kw);
  }
  // K6：分离启动（start ""）时无有效 STD_INPUT → STARTF_USESTDHANDLES + 无效句柄 = CreateProcessW 失败
  HANDLE nul_in = INVALID_HANDLE_VALUE;
  HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
  if (!in || in == INVALID_HANDLE_VALUE || GetFileType(in) == FILE_TYPE_UNKNOWN)
  { nul_in = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr); in = nul_in; }
  STARTUPINFOW startup{}; startup.cb = sizeof(startup); startup.dwFlags = STARTF_USESTDHANDLES; startup.hStdOutput = write_handle; startup.hStdError = write_handle; startup.hStdInput = in;
  PROCESS_INFORMATION info{};
  if (!CreateProcessW(nullptr, command_line.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, cwd.empty() ? nullptr : cwd.c_str(), &startup, &info)) {
    for (const auto& kw : injected) ::SetEnvironmentVariableW(kw.c_str(), nullptr);
    if (nul_in != INVALID_HANDLE_VALUE) CloseHandle(nul_in);
    CloseHandle(read_handle); CloseHandle(write_handle);
    if (error) *error = "CreateProcessW failed (" + std::to_string(static_cast<unsigned long>(GetLastError())) + ")";
    // K6 调试：落盘命令行与错误码（CICADA_DSH_DEBUG=1 时）
    if (getenv("CICADA_DSH_DEBUG")) {
      std::ofstream dbg("C:/dsh/Schematic-Cicada/cicada-editor/logs/spawn-debug.txt");
      dbg << "cmdline=" << command.str() << "\ncwd=" << (spawner.spec().cwd.empty() ? spawner.spec().dsh_home : spawner.spec().cwd)
          << "\nlastError=" << GetLastError() << "\n";
    }
    return false;
  }
  for (const auto& kw : injected) ::SetEnvironmentVariableW(kw.c_str(), nullptr);
  if (nul_in != INVALID_HANDLE_VALUE) CloseHandle(nul_in);
  CloseHandle(write_handle); CloseHandle(info.hThread); process_ = info.hProcess; stdout_read_ = read_handle; if (error) error->clear(); return true;
#else
  (void)spawner; if (error) *error = "host process is only implemented on Windows"; return false;
#endif
}
bool HostProcess::running() const {
#ifdef _WIN32
  if (!process_) return false; DWORD code = 0; return GetExitCodeProcess(static_cast<HANDLE>(process_), &code) && code == STILL_ACTIVE;
#else
  return false;
#endif
}
void HostProcess::terminate() {
#ifdef _WIN32
  if (process_) { TerminateProcess(static_cast<HANDLE>(process_), 1); CloseHandle(static_cast<HANDLE>(process_)); process_ = nullptr; }
  if (stdout_read_) { CloseHandle(static_cast<HANDLE>(stdout_read_)); stdout_read_ = nullptr; }
#endif
}
std::string HostProcess::read_stdout() const {
#ifdef _WIN32
  if (!stdout_read_) return {}; DWORD available = 0; if (!PeekNamedPipe(static_cast<HANDLE>(stdout_read_), nullptr, 0, nullptr, &available, nullptr) || available == 0) return {};
  std::string out(available, '\0'); DWORD read = 0; if (!ReadFile(static_cast<HANDLE>(stdout_read_), out.data(), available, &read, nullptr)) return {}; out.resize(read); return out;
#else
  return {};
#endif
}
}
