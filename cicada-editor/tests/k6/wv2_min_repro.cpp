// wv2_min_repro.cpp — minimal WebView2 environment/controller repro (no wx, no app code)
//
// Decisive experiment: does a bare Win32 console program that only does
//   CoInitializeEx -> CreateCoreWebView2EnvironmentWithOptions -> CreateCoreWebView2Controller -> Navigate
// also crash with 0xc0000005 (environment/runtime problem) or succeed (our integration problem)?
//
// Build (pattern from scripts/probe-build.bat):
//   call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
//   cl /nologo /EHsc /std:c++17 /I <repo>\third_party\webview2\include wv2_min_repro.cpp ^
//       <repo>\third_party\webview2\lib\WebView2LoaderStatic.lib ole32.lib user32.lib advapi32.lib
//
// Run: wv2_min_repro.exe [browserExecutableFolder]
//   optional arg = full path to Edge stable version dir, e.g.
//   "C:\Program Files (x86)\Microsoft\Edge\Application\152.0.4191.62"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>   // defines `interface` as a macro (MSVC 14.43 dropped the keyword)
#include <stdio.h>
#include <string>
#include "WebView2.h"

static volatile bool g_envDone = false;
static volatile bool g_ctrlDone = false;
static HRESULT g_envHr = E_FAIL;
static HRESULT g_ctrlHr = E_FAIL;
static ICoreWebView2Environment* g_env = nullptr;
static ICoreWebView2Controller* g_ctrl = nullptr;

static void PrintHr(const char* what, HRESULT hr) {
    printf("%s: HRESULT 0x%08lX\n", what, (unsigned long)hr);
}

// ---------------- environment completed handler ----------------
class EnvHandler : public ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler {
    LONG m_ref = 1;
public:
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override {
        if (riid == IID_IUnknown ||
            riid == IID_ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler) {
            *ppv = this; AddRef(); return S_OK;
        }
        *ppv = nullptr; return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() override { return (ULONG)InterlockedIncrement(&m_ref); }
    STDMETHODIMP_(ULONG) Release() override {
        LONG r = InterlockedDecrement(&m_ref);
        if (!r) delete this;
        return (ULONG)r;
    }
    STDMETHODIMP Invoke(HRESULT errorCode, ICoreWebView2Environment* result) override {
        g_envHr = errorCode;
        g_env = result;
        if (result) result->AddRef();
        printf("ENV-CALLBACK: errorCode=0x%08lX result=%p\n", (unsigned long)errorCode, (void*)result);
        g_envDone = true;
        return S_OK;
    }
};

// ---------------- controller completed handler ----------------
class CtrlHandler : public ICoreWebView2CreateCoreWebView2ControllerCompletedHandler {
    LONG m_ref = 1;
public:
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override {
        if (riid == IID_IUnknown ||
            riid == IID_ICoreWebView2CreateCoreWebView2ControllerCompletedHandler) {
            *ppv = this; AddRef(); return S_OK;
        }
        *ppv = nullptr; return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() override { return (ULONG)InterlockedIncrement(&m_ref); }
    STDMETHODIMP_(ULONG) Release() override {
        LONG r = InterlockedDecrement(&m_ref);
        if (!r) delete this;
        return (ULONG)r;
    }
    STDMETHODIMP Invoke(HRESULT errorCode, ICoreWebView2Controller* result) override {
        g_ctrlHr = errorCode;
        g_ctrl = result;
        if (result) result->AddRef();
        printf("CTRL-CALLBACK: errorCode=0x%08lX result=%p\n", (unsigned long)errorCode, (void*)result);
        g_ctrlDone = true;
        return S_OK;
    }
};

// Pump the COM apartment message loop until flag or timeout.
static void PumpUntil(volatile bool* done, DWORD timeoutMs, const char* what) {
    DWORD start = GetTickCount();
    while (!*done) {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (GetTickCount() - start > timeoutMs) {
            printf("%s: TIMEOUT after %lu ms\n", what, (unsigned long)timeoutMs);
            return;
        }
        Sleep(10);
    }
}

// ---------------- environment facts (simple) ----------------
static void PrintDirVersions(const wchar_t* base, const char* label) {
    std::wstring pattern = std::wstring(base) + L"*";
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW(pattern.c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) { printf("%s: <not found>\n", label); return; }
    printf("%s versions:", label);
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) printf(" %ls", fd.cFileName);
    } while (FindNextFileW(h, &fd));
    FindClose(h);
    printf("\n");
}

static void PrintGpuName() {
    wchar_t buf[512] = {0};
    DWORD sz = (DWORD)sizeof(buf);
    LSTATUS st = RegGetValueW(HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e968-e325-11ce-bfc1-08002be10318}\\0000",
        L"DriverDesc", RRF_RT_REG_SZ, NULL, buf, &sz);
    if (st == ERROR_SUCCESS) printf("GPU: %ls\n", buf);
    else printf("GPU: n/a (reg 0x%lx)\n", (unsigned long)st);
}

static HWND FindChromeRecursive(HWND parent, const wchar_t* cls, int depth) {
    if (depth <= 0) return NULL;
    HWND child = NULL;
    while ((child = FindWindowExW(parent, child, cls, NULL)) != NULL) {
        wprintf(L"FOUND %ls child=%p parent=%p style=0x%08lX exstyle=0x%08lX\n", cls,
                (void*)child, (void*)parent,
                GetWindowLongW(child, GWL_STYLE), GetWindowLongW(child, GWL_EXSTYLE));
        return child;
    }
    // recurse into known container classes
    const wchar_t* containers[] = { L"Chrome_WidgetWin_1", L"Chrome_RenderWidgetHostHWND",
                                    L"Chrome_AuraWindow", L"Chrome_MainWindow", L"CicadaWv2Pane" };
    child = NULL;
    while ((child = FindWindowExW(parent, child, NULL, NULL)) != NULL) {
        wchar_t name[64] = {0};
        GetClassNameW(child, name, 63);
        HWND r = FindChromeRecursive(child, cls, depth - 1);
        if (r) return r;
    }
    return NULL;
}

// ---------------- main ----------------
static HWND CreateHostWindow() {
    const wchar_t* cls = L"Wv2MinReproWnd";
    WNDCLASSW wc = {};
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = cls;
    RegisterClassW(&wc);
    HWND hwnd = CreateWindowExW(0, cls, L"wv2_min_repro", WS_OVERLAPPEDWINDOW,
                                100, 100, 640, 480, nullptr, nullptr, wc.hInstance, nullptr);
    return hwnd;
}

int wmain(int argc, wchar_t** argv) {
    printf("=== wv2_min_repro ===\n");
    fflush(stdout);

    PrintDirVersions(L"C:\\Program Files (x86)\\Microsoft\\EdgeWebView\\Application\\", "msedgewebview2");
    PrintDirVersions(L"C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\", "Edge");
    PrintGpuName();
    SYSTEM_INFO si; GetSystemInfo(&si);
    printf("processor arch: %lu\n", (unsigned long)si.wProcessorArchitecture);
    fflush(stdout);

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    printf("CoInitializeEx: HRESULT 0x%08lX %s\n", (unsigned long)hr,
           hr == S_OK ? "(S_OK)" : "(NOT S_OK)");
    fflush(stdout);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        printf("FATAL: CoInitializeEx failed\n");
        return 2;
    }

    HWND hwnd = CreateHostWindow();
    printf("host HWND: %p\n", (void*)hwnd);
    fflush(stdout);
    if (!hwnd) { printf("FATAL: CreateWindow failed\n"); return 2; }

    PCWSTR browserFolder = nullptr;
    std::wstring folderArg;
    if (argc > 2) {
        folderArg = argv[2];
        browserFolder = folderArg.c_str();
        printf("browserExecutableFolder: %ls\n", browserFolder);
        fflush(stdout);
    }

    EnvHandler* envHandler = new EnvHandler();
    hr = CreateCoreWebView2EnvironmentWithOptions(browserFolder, nullptr, nullptr, envHandler);
    PrintHr("CreateCoreWebView2EnvironmentWithOptions", hr);
    fflush(stdout);
    if (FAILED(hr)) {
        printf("RESULT-C: env creation returned failure HRESULT\n");
        return 3;
    }

    PumpUntil(&g_envDone, 15000, "ENV");
    if (!g_envDone) { printf("RESULT-C: env callback TIMEOUT\n"); return 4; }
    if (FAILED(g_envHr) || !g_env) {
        printf("RESULT-C: env callback errorCode=0x%08lX\n", (unsigned long)g_envHr);
        return 5;
    }
    printf("ENV OK\n");
    fflush(stdout);

    CtrlHandler* ctrlHandler = new CtrlHandler();
    hr = g_env->CreateCoreWebView2Controller(hwnd, ctrlHandler);
    PrintHr("CreateCoreWebView2Controller", hr);
    fflush(stdout);
    if (FAILED(hr)) {
        printf("RESULT-C: controller creation returned failure HRESULT\n");
        return 6;
    }

    PumpUntil(&g_ctrlDone, 15000, "CTRL");
    if (!g_ctrlDone) { printf("RESULT-C: controller callback TIMEOUT\n"); return 7; }
    if (FAILED(g_ctrlHr) || !g_ctrl) {
        printf("RESULT-C: controller callback errorCode=0x%08lX\n", (unsigned long)g_ctrlHr);
        return 8;
    }
    printf("CTRL OK\n");
    fflush(stdout);
    {   // K6: enumerate Chrome_* windows + styles (wx IsDialogMessage walk analysis)
        HWND r = FindChromeRecursive(hwnd, L"Chrome_WidgetWin_0", 4);
        if (!r) printf("chrome windows: none found (top-level?)\n");
        r = FindChromeRecursive(hwnd, L"Chrome_WidgetWin_1", 4);
        if (!r) printf("chrome windows1: none found\n");
        // 顶层查找（也许 chrome 是 popup，父为 NULL/桌面）
        r = FindWindowExW(NULL, NULL, L"Chrome_WidgetWin_0", NULL);
        if (r) wprintf(L"TOP-LEVEL Chrome_WidgetWin_0=%p style=0x%08lX\n", (void*)r, GetWindowLongW(r, GWL_STYLE));
        fflush(stdout);
    }

    ICoreWebView2* wv = nullptr;
    hr = g_ctrl->get_CoreWebView2(&wv);
    PrintHr("get_CoreWebView2", hr);
    fflush(stdout);
    if (SUCCEEDED(hr) && wv) {
        // K6: URL from argv[1] (default data:) — decisive: HTTP page load vs data: URL
        const wchar_t* url = argc > 1 ? argv[1] : L"data:text/html,<h1>ok</h1>";
        printf("URL: %ls\n", url);
        fflush(stdout);
        hr = wv->Navigate(url);
        PrintHr("Navigate", hr);
        fflush(stdout);
        // pump 10s so a real HTTP page can load (crash would kill the process)
        DWORD t0 = GetTickCount();
        while (GetTickCount() - t0 < 10000) {
            MSG m;
            while (PeekMessage(&m, NULL, 0, 0, PM_REMOVE)) {
                TranslateMessage(&m);
                DispatchMessage(&m);
            }
            Sleep(10);
        }
        printf("PUMP-10S-DONE\n");
        fflush(stdout);
    }

    Sleep(3000);
    printf("NAVDONE\n");
    printf("RESULT-B: minimal program OK (a crash only in the wx path would be OUR integration issue)\n");
    fflush(stdout);
    return 0;
}
