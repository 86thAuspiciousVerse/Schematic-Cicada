// webview2_host.h — Win32 WebView2 直挂（wx 壳右栏嵌入）
// pimpl：重 COM 头（WebView2.h，引入 win32 VARIANT 等与 KiCad 头冲突）只在 .cpp 出现。
#pragma once

#include <memory>
#include <string>

namespace cicada::editor {

/** Minimal WebView2 host: environment (async) → controller → navigable page. */
class CicadaWebView2 {
 public:
  CicadaWebView2();
  ~CicadaWebView2();
  CicadaWebView2(const CicadaWebView2&) = delete;
  CicadaWebView2& operator=(const CicadaWebView2&) = delete;

  /** Start environment creation; the page is navigated once the controller is ready. */
  bool Start(void* hwnd_parent, const std::wstring& initial_url);
  void Navigate(const std::wstring& url);
  /** Sync the browser bounds to the parent's client size. */
  void SyncBounds(void* hwnd_parent);
  bool ready() const;

  struct Impl;   // pimpl（回调助手需访问；不完全类型，外部不可构造）
  std::unique_ptr<Impl> impl_;
};

} // namespace cicada::editor
