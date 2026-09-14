#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show) {
  const wchar_t class_name[] = L"CicadaWin32Probe";
  WNDCLASSW wc{};
  wc.hInstance = instance;
  wc.lpfnWndProc = DefWindowProcW;
  wc.lpszClassName = class_name;
  if (!RegisterClassW(&wc)) return static_cast<int>(GetLastError());
  HWND window = CreateWindowExW(0, class_name, L"Cicada Win32 probe",
                                WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                                480, 240, nullptr, nullptr, instance, nullptr);
  if (!window) return static_cast<int>(GetLastError());
  ShowWindow(window, show);
  UpdateWindow(window);
  // Keep the probe alive briefly so a desktop session can create/paint it;
  // this is not the product launcher and never starts DSH.
  Sleep(500);
  DestroyWindow(window);
  UnregisterClassW(class_name, instance);
  return 0;
}
