#include "webview2_host.h"

#ifdef _WIN32
#include <windows.h>
#include <objbase.h>
#ifndef interface
#define interface struct
#endif
#include <WebView2.h>

#include <atomic>
#include <fstream>
#include <mutex>

// K6 观测日志（与 app 日志同目录；直接追加，逐条 flush）
static void wv2log(const std::string& s) {
  std::ofstream f("C:/dsh/Schematic-Cicada/cicada-editor/logs/wv2-host.txt", std::ios::app);
  f << s << "\n";
  f.close();
}

struct cicada::editor::CicadaWebView2::Impl {
  // K6 架构定论（14号崩溃栈 + min-repro 对照实验）：
  //  - 专用 STA 线程 + 控制器父=wxPanel：Chromium in-proc 窗口挂跨线程父窗口，
  //    EmbeddedBrowserWebView.dll 内 wndproc 确定性 AV（0x...6A3C）。
  //  - 纯 Win32 最小复现（同 runtime/同 URL/同 SDK）：0 崩溃。
  //  - 结论：WebView2 必须用 主线程 + 纯 Win32 宿主窗（非 wx 窗口、非 HWND_MESSAGE），
  //    wx 的 PreProcessMessage 上溯后只对 chrome 窗口发出 IsDialogMessage 探测
  //    (WM_GETDLGCODE，Chromium 正常处理) —— 不再命中任何 loader 内部隐藏窗口。
  HWND parent = nullptr;       // wx holder（主线程；纯 pane 的视觉父）
  HWND pane = nullptr;         // 纯 Win32 child（控制器 parent；主线程创建）
  std::wstring url;
  std::mutex mu;
  ICoreWebView2Controller* controller = nullptr;
};

namespace cicada::editor {
namespace {

class ScriptResultHandler final : public ICoreWebView2ExecuteScriptCompletedHandler {
 public:
  STDMETHOD(QueryInterface)(REFIID iid, void** out) override {
    if (iid == IID_IUnknown || iid == __uuidof(ICoreWebView2ExecuteScriptCompletedHandler)) {
      *out = this; AddRef(); return S_OK;
    }
    *out = nullptr; return E_NOINTERFACE;
  }
  STDMETHOD_(ULONG, AddRef)() override { return ++ref_; }
  STDMETHOD_(ULONG, Release)() override { ULONG r = --ref_; if (r == 0) delete this; return r; }
  STDMETHOD(Invoke)(HRESULT result, PCWSTR script_result) override {
    std::string s = "(null)";
    if (script_result) {
      const int n = WideCharToMultiByte(CP_UTF8, 0, script_result, -1, nullptr, 0, nullptr, nullptr);
      if (n > 1) { s.assign(n - 1, L'\0'); WideCharToMultiByte(CP_UTF8, 0, script_result, -1, s.data(), n, nullptr, nullptr); }
      if (s.size() > 300) s = s.substr(0, 300);
    }
    char b[32]; snprintf(b, sizeof(b), "0x%08lX", (unsigned long)result);
    wv2log(std::string("probe result hr=") + b + " body=" + s);
    return S_OK;
  }
 private:
  ULONG ref_ = 1;
};

class NavCompletedHandler final : public ICoreWebView2NavigationCompletedEventHandler {
 public:
  explicit NavCompletedHandler(ICoreWebView2Controller* c) : controller_(c) {}
  STDMETHOD(QueryInterface)(REFIID iid, void** out) override {
    if (iid == IID_IUnknown || iid == __uuidof(ICoreWebView2NavigationCompletedEventHandler)) {
      *out = this; AddRef(); return S_OK;
    }
    *out = nullptr; return E_NOINTERFACE;
  }
  STDMETHOD_(ULONG, AddRef)() override { return ++ref_; }
  STDMETHOD_(ULONG, Release)() override { ULONG r = --ref_; if (r == 0) delete this; return r; }
  STDMETHOD(Invoke)(ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) override {
    BOOL ok = FALSE;
    COREWEBVIEW2_WEB_ERROR_STATUS err = COREWEBVIEW2_WEB_ERROR_STATUS_UNKNOWN;
    if (args) { args->get_IsSuccess(&ok); args->get_WebErrorStatus(&err); }
    char b[128];
    snprintf(b, sizeof(b), "navigation completed success=%d webError=%d", (int)ok, (int)err);
    wv2log(b);
    if (ok && controller_) {
      ICoreWebView2* webview = nullptr;
      if (SUCCEEDED(controller_->get_CoreWebView2(&webview)) && webview) {
        wv2log("probe -> executeScript");
        webview->ExecuteScript(
          L"(()=>{const b=document.body;return JSON.stringify({t:document.title,"
          L"url:location.href,body:!!b,nodes:b?b.children.length:-1,"
          L"text:b?b.innerText.slice(0,200):''})})()",
          new ScriptResultHandler());
        webview->Release();
      }
    }
    return S_OK;
  }
 private:
  ICoreWebView2Controller* controller_;
  ULONG ref_ = 1;
};

class NavStartingHandler final : public ICoreWebView2NavigationStartingEventHandler {
 public:
  STDMETHOD(QueryInterface)(REFIID iid, void** out) override {
    if (iid == IID_IUnknown || iid == __uuidof(ICoreWebView2NavigationStartingEventHandler)) {
      *out = this; AddRef(); return S_OK;
    }
    *out = nullptr; return E_NOINTERFACE;
  }
  STDMETHOD_(ULONG, AddRef)() override { return ++ref_; }
  STDMETHOD_(ULONG, Release)() override { ULONG r = --ref_; if (r == 0) delete this; return r; }
  STDMETHOD(Invoke)(ICoreWebView2* sender, ICoreWebView2NavigationStartingEventArgs* args) override {
    PWSTR uri = nullptr;
    if (args) args->get_Uri(&uri);
    std::string s = "(null)";
    if (uri) {
      const int n = WideCharToMultiByte(CP_UTF8, 0, uri, -1, nullptr, 0, nullptr, nullptr);
      if (n > 1) { s.assign(n - 1, L'\0'); WideCharToMultiByte(CP_UTF8, 0, uri, -1, s.data(), n, nullptr, nullptr); }
      CoTaskMemFree(uri);
    }
    wv2log("navigation starting: " + s);
    return S_OK;
  }
 private:
  ULONG ref_ = 1;
};

class ControllerHandler final : public ICoreWebView2CreateCoreWebView2ControllerCompletedHandler {
 public:
  explicit ControllerHandler(CicadaWebView2::Impl* impl) : impl_(impl) {}
  STDMETHOD(QueryInterface)(REFIID iid, void** out) override {
    if (iid == IID_IUnknown || iid == __uuidof(ICoreWebView2CreateCoreWebView2ControllerCompletedHandler)) {
      *out = this; AddRef(); return S_OK;
    }
    *out = nullptr; return E_NOINTERFACE;
  }
  STDMETHOD_(ULONG, AddRef)() override { return ++ref_; }
  STDMETHOD_(ULONG, Release)() override { ULONG r = --ref_; if (r == 0) delete this; return r; }
  STDMETHOD(Invoke)(HRESULT result, ICoreWebView2Controller* controller) override {
    CicadaWebView2::Impl* impl = impl_;
    if (FAILED(result) || controller == nullptr || impl == nullptr) {
      if (controller) controller->Release();
      char b[96]; snprintf(b, sizeof(b), "controller FAILED hr=0x%08lX", (unsigned long)result);
      wv2log(b);
      return result == S_OK ? E_FAIL : result;
    }
    {
      std::lock_guard<std::mutex> g(impl->mu);
      impl->controller = controller;   // 主线程单泵；仍加锁防 ready() 竞态
    }
    wv2log("controller ready");
    ICoreWebView2* webview = nullptr;
    if (SUCCEEDED(controller->get_CoreWebView2(&webview))) {
      controller->put_IsVisible(TRUE);
      HWND pane = impl->pane;
      RECT rc{};
      if (pane && GetClientRect(pane, &rc)) {
        const RECT bounds{0, 0, rc.right, rc.bottom};
        controller->put_Bounds(bounds);
      }
      if (webview) {
        NavStartingHandler* ns = new NavStartingHandler();
        NavCompletedHandler* nc = new NavCompletedHandler(controller);
        EventRegistrationToken t1{}, t2{};
        webview->add_NavigationStarting(ns, &t1);
        webview->add_NavigationCompleted(nc, &t2);
        if (!impl->url.empty()) {
          const std::wstring w = impl->url.size() > 120 ? impl->url.substr(0, 120) + L"..." : impl->url;
          char buf[260] = {0};
          const int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
          if (n > 0) WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, buf, sizeof(buf), nullptr, nullptr);
          wv2log(std::string("navigate -> ") + buf);
          webview->Navigate(impl->url.c_str());
        }
      }
    }
    return S_OK;
  }
 private:
  CicadaWebView2::Impl* impl_;
  ULONG ref_ = 1;
};

class EnvHandler final : public ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler {
 public:
  explicit EnvHandler(CicadaWebView2::Impl* impl) : impl_(impl) {}
  STDMETHOD(QueryInterface)(REFIID iid, void** out) override {
    if (iid == IID_IUnknown || iid == __uuidof(ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler)) {
      *out = this; AddRef(); return S_OK;
    }
    *out = nullptr; return E_NOINTERFACE;
  }
  STDMETHOD_(ULONG, AddRef)() override { return ++ref_; }
  STDMETHOD_(ULONG, Release)() override { ULONG r = --ref_; if (r == 0) delete this; return r; }
  STDMETHOD(Invoke)(HRESULT result, ICoreWebView2Environment* env) override {
    CicadaWebView2::Impl* impl = impl_;
    if (FAILED(result) || env == nullptr || impl == nullptr) {
      if (env) env->Release();
      char b[96]; snprintf(b, sizeof(b), "environment FAILED hr=0x%08lX", (unsigned long)result);
      wv2log(b);
      return result == S_OK ? E_FAIL : result;
    }
    wv2log("environment ready");
    ICoreWebView2CreateCoreWebView2ControllerCompletedHandler* cb = new ControllerHandler(impl);
    env->CreateCoreWebView2Controller(impl->pane, cb);
    env->Release();
    return S_OK;
  }
 private:
  CicadaWebView2::Impl* impl_;
  ULONG ref_ = 1;
};

} // namespace

CicadaWebView2::CicadaWebView2() : impl_(std::make_unique<Impl>()) {}
CicadaWebView2::~CicadaWebView2() {
  std::lock_guard<std::mutex> g(impl_->mu);
  if (impl_->controller) { impl_->controller->Close(); impl_->controller->Release(); impl_->controller = nullptr; }
  if (impl_->pane) { DestroyWindow(impl_->pane); impl_->pane = nullptr; }
}

bool CicadaWebView2::Start(void* hwnd_parent, const std::wstring& initial_url) {
  impl_->parent = static_cast<HWND>(hwnd_parent);
  impl_->url = initial_url;
  // 主线程（wx 事件循环）驱动：单泵 + 纯 Win32 宿主窗（见 Impl 注释的架构定论）
  HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
    wv2log("CoInitializeEx failed");
    return false;
  }
  const wchar_t* paneCls = L"CicadaWv2Pane";
  WNDCLASSW pw{};
  pw.lpfnWndProc = DefWindowProcW;
  pw.hInstance = GetModuleHandleW(nullptr);
  pw.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  pw.lpszClassName = paneCls;
  RegisterClassW(&pw);
  impl_->pane = CreateWindowExW(0, paneCls, L"", WS_CHILD | WS_VISIBLE,
                                0, 0, 10, 10, impl_->parent,
                                nullptr, GetModuleHandleW(nullptr), nullptr);
  if (!impl_->pane) {
    wv2log("pane creation failed");
    return false;
  }
  wv2log("pane created; env creation starting");
  CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr, new EnvHandler(impl_.get()));
  return true;
}

void CicadaWebView2::Navigate(const std::wstring& url) {
  impl_->url = url;
}

void CicadaWebView2::SyncBounds(void* hwnd_parent) {
  HWND p = static_cast<HWND>(hwnd_parent);
  RECT rc{};
  if (p == nullptr || !GetClientRect(p, &rc)) return;
  if (impl_->pane)
    SetWindowPos(impl_->pane, nullptr, 0, 0, rc.right, rc.bottom,
                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
  std::lock_guard<std::mutex> g(impl_->mu);
  if (impl_->controller) {
    const RECT bounds{0, 0, rc.right, rc.bottom};
    impl_->controller->put_Bounds(bounds);
  }
}

bool CicadaWebView2::ready() const {
  std::lock_guard<std::mutex> g(impl_->mu);
  return impl_->controller != nullptr;
}
} // namespace cicada::editor

#pragma comment(lib, "WebView2LoaderStatic.lib")
#endif
