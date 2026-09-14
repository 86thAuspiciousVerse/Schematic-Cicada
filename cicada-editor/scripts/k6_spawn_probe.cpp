// k6_spawn_probe.cpp — MINIMAL CreateProcessW 复现（复刻 host_process.cpp 各因素，逐项开关）
#include <cstdlib>
#include <vector>
// 编译: cl /nologo /EHsc /Fe:spawn_probe.exe k6_spawn_probe.cpp
#include <windows.h>
#include <cstdio>
#include <string>

static std::wstring wide(const std::string& s) {
  int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
  std::wstring o((size_t)n, L'\0');
  MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), &o[0], n);
  return o;
}

int wmain(int argc, wchar_t** argv) {
  if (argc >= 2 && _wcsicmp(argv[1], L"probeinc") == 0) {
    std::vector<std::wstring> rawLines;
    LPWCH rr = GetEnvironmentStringsW();
    if (rr) { for (LPWCH p = rr; *p; p += wcslen(p) + 1) rawLines.push_back(p); FreeEnvironmentStringsW(rr); }
    for (size_t i = 0; i < rawLines.size(); ++i) {
      if (rawLines[i].find(L'=') == std::wstring::npos) continue;
      if (rawLines[i][0] == L'=') continue;   // skip per-drive pseudo vars
      std::wstring testBlock;
      auto tadd = [&](const std::wstring& l) { testBlock += l; testBlock.push_back(L'\0'); };
      tadd(L"DSH_HOME=C:\\dsh\\Schematic-Cicada\\cicada-dev-home");
      tadd(L"CICADA_HOME=C:\\dsh\\Schematic-Cicada\\cicada-dev-home");
      tadd(rawLines[i]);
      testBlock.push_back(L'\0');
      SECURITY_ATTRIBUTES tsa{ sizeof(tsa), nullptr, TRUE };
      HANDLE tr = nullptr, tw = nullptr; CreatePipe(&tr, &tw, &tsa, 0); SetHandleInformation(tr, HANDLE_FLAG_INHERIT, 0);
      STARTUPINFOW tsi{}; tsi.cb = sizeof(tsi); tsi.dwFlags = STARTF_USESTDHANDLES; tsi.hStdOutput = tw; tsi.hStdError = tw; tsi.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
      PROCESS_INFORMATION tpi{};
      wchar_t* tcmd = _wcsdup(L"\"C:\\Program Files\\nodejs\\node.exe\" -v");
      BOOL tok = CreateProcessW(nullptr, tcmd, nullptr, nullptr, TRUE, CREATE_NO_WINDOW, &testBlock[0], L"C:\\dsh\\Schematic-Cicada\\cicada-harness", &tsi, &tpi);
      if (!tok) { wprintf(L"BAD: '%s'\n", rawLines[i].c_str()); return 0; }
      if (tok) { TerminateProcess(tpi.hProcess, 0); CloseHandle(tpi.hProcess); CloseHandle(tpi.hThread); }
      CloseHandle(tr); CloseHandle(tw);
    }
    wprintf(L"ALL-OK\n"); return 0;
  }
  const bool useEnvBlock = (argc < 2 || (_wcsicmp(argv[1], L"noenv") != 0 && _wcsicmp(argv[1], L"onlytwo") != 0));
  const bool onlyTwo = argc >= 2 && _wcsicmp(argv[1], L"onlytwo") == 0;
  const bool useNulIn    = argc < 3 || _wcsicmp(argv[2], L"nonul") != 0;
  const bool useNoWindow = argc < 4 || _wcsicmp(argv[3], L"console") != 0;

  std::wstring cmd = L"\"C:\\Program Files\\nodejs\\node.exe\" --import tsx/esm \"C:\\dsh\\Schematic-Cicada\\cicada-harness\\apps\\cli\\src\\bin.ts\" --profile cicada --no-open --port 3123 --host 127.0.0.1";

  // env block：完整复刻 build_env_block
  std::wstring block;
  auto add = [&](const std::wstring& l) { block += l; block.push_back(L'\0'); };
  LPWCH raw = GetEnvironmentStringsW();
  if (raw && !onlyTwo) {
    for (LPWCH p = raw; *p; p += wcslen(p) + 1) {
      std::wstring key(p); size_t eq = key.find(L'=');
      std::wstring k = eq == std::wstring::npos ? key : key.substr(0, eq);
      if (k == L"DSH_HOME" || k == L"CICADA_HOME") continue;
      add(key);
    }
    FreeEnvironmentStringsW(raw);
  }
  add(L"DSH_HOME=C:\\dsh\\Schematic-Cicada\\cicada-dev-home");
  add(L"CICADA_HOME=C:\\dsh\\Schematic-Cicada\\cicada-dev-home");
  block.push_back(L'\0');

  HANDLE nulIn = INVALID_HANDLE_VALUE;
  HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
  if (useNulIn && (!in || in == INVALID_HANDLE_VALUE || GetFileType(in) == FILE_TYPE_UNKNOWN)) {
    nulIn = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
    in = nulIn;
  }

  SECURITY_ATTRIBUTES sa{ sizeof(sa), nullptr, TRUE };
  HANDLE r = nullptr, w = nullptr;
  CreatePipe(&r, &w, &sa, 0);
  SetHandleInformation(r, HANDLE_FLAG_INHERIT, 0);

  STARTUPINFOW si{}; si.cb = sizeof(si); si.dwFlags = STARTF_USESTDHANDLES;
  si.hStdOutput = w; si.hStdError = w; si.hStdInput = in;
  PROCESS_INFORMATION pi{};
  wchar_t* mutableCmd = _wcsdup(cmd.c_str());
  DWORD flags = useNoWindow ? CREATE_NO_WINDOW : 0;
  BOOL ok = CreateProcessW(nullptr, mutableCmd, nullptr, nullptr, TRUE, flags,
                           (useEnvBlock && block.size() >= 4) ? (LPVOID)&block[0] : nullptr,
                           L"C:\\dsh\\Schematic-Cicada\\cicada-harness",
                           &si, &pi);
  printf("ok=%d err=%lu  envBlock=%d nulIn=%d noWindow=%d\n", ok, GetLastError(), useEnvBlock, useNulIn, useNoWindow);
  if (ok) { TerminateProcess(pi.hProcess, 0); CloseHandle(pi.hProcess); CloseHandle(pi.hThread); }
  return 0;
}
// probeinc: 逐条继承项试（bisect 毒条目）
