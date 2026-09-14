#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/utils.h>
#if defined(CICADA_EDITOR_HAS_WEBVIEW2)
#include "../bridge/webview2_host.h"
#endif
#if defined(CICADA_EDITOR_HAS_WEBVIEW)
#include <wx/webview.h>
#endif
#include <memory>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#if defined(CICADA_EDITOR_HAS_CANVAS)
#include "../render/wx_canvas_panel.h"
#endif
#if defined(CICADA_EDITOR_HAS_CANVAS) && defined(CICADA_EDITOR_HAS_KICAD_MODEL)
#include "../kicad_bridge/kicad_scene_adapter.h"
#include "../kicad_bridge/sch_interaction_engine.h"
#include "../kicad_bridge/cicada_document_bridge.h"
#include "../bridge/editor_http_client.h"
#include "../bridge/winhttp_transport.h"
#include "../bridge/winhttp_websocket.h"
#include <sch_symbol.h>
#include <sch_label.h>
#include <sch_pin.h>
#include <sch_field.h>
#include <sch_junction.h>
#include <template_fieldnames.h>
#include <thread>
#include <mutex>
#include <deque>
#include <atomic>
#include <windows.h>
#include <dbghelp.h>
#pragma comment( lib, "dbghelp.lib" )
#endif
#include "../render/canvas_document.h"
#include "../core/document_editor.h"
#include "../core/tool_registry.h"
#include "editor_controller.h"
#include "host_process.h"

#if defined(CICADA_EDITOR_HAS_CANVAS) && defined(CICADA_EDITOR_HAS_KICAD_MODEL)
namespace {

// 极简 JSON 字符串字段提取（契约帧字段名固定；无外部 JSON 依赖）
std::string json_str_field(const std::string& json, const std::string_view key) {
  const std::string needle = "\"" + std::string(key) + "\"";
  const auto pos = json.find(needle);
  if (pos == std::string::npos) return {};
  const auto colon = json.find(':', pos + needle.size());
  if (colon == std::string::npos) return {};
  auto it = json.begin() + colon + 1;
  while (it != json.end() && (*it == ' ' || *it == '\t' || *it == '\n' || *it == '\r')) ++it;
  if (it == json.end() || *it != '"') return {};
  ++it;
  std::string out;
  while (it != json.end() && *it != '"') {
    if (*it == '\\' && it + 1 != json.end()) { ++it; }
    out.push_back(*it);
    ++it;
  }
  return out;
}

std::string json_esc(const wxString& s) {
  std::string out;
  for (const wxUniChar c : s) {
    switch (c.GetValue()) { case '"': out += "\\\""; break; case '\\': out += "\\\\"; break;
      case '\n': out += "\\n"; break; case '\r': out += "\\r"; break; case '\t': out += "\\t"; break;
      default: if (c.GetValue() < 0x20) { char b[8]; std::snprintf(b, 8, "\\u%04x", c.GetValue()); out += b; }
               else out += wxString(c).ToUTF8().data(); }
  }
  return out;
}

}  // namespace
#endif

namespace {

class WxTextFileReader final : public cicada::editor::ITextFileReader {
 public:
  explicit WxTextFileReader(wxString path) : path_(std::move(path)) {}

  std::string read(std::string_view) override {
    std::ifstream input(std::filesystem::path(path_.ToStdWstring()), std::ios::binary);
    if (!input) throw std::runtime_error("unable to open schematic file");
    std::ostringstream contents;
    contents << input.rdbuf();
    if (!input.good() && !input.eof()) throw std::runtime_error("unable to read schematic file");
    return contents.str();
  }

 private:
  wxString path_;
};

class WxTextFileWriter final : public cicada::editor::ITextFileWriter {
 public:
  void write(std::string_view path, std::string_view text) override {
    const std::string path_copy(path);
    std::ofstream output(std::filesystem::path(wxString::FromUTF8(path_copy.c_str()).ToStdWstring()),
                         std::ios::binary | std::ios::trunc);
    if (!output) throw std::runtime_error("unable to open schematic file for writing");
    output.write(text.data(), static_cast<std::streamsize>(text.size()));
    if (!output) throw std::runtime_error("unable to write schematic file");
  }
};

}  // namespace

// K6：崩溃落盘（AV/断言→ logs/app-crash.txt，含符号栈；PDB 与 exe 同目录）
static LONG CALLBACK cicadaSehHandler( EXCEPTION_POINTERS* aInfo )
{
    std::ofstream f( "C:/dsh/Schematic-Cicada/cicada-editor/logs/app-crash.txt", std::ios::app );
    f << "==== CRASH code=0x" << std::hex << aInfo->ExceptionRecord->ExceptionCode
      << " addr=" << (void*) aInfo->ExceptionRecord->ExceptionAddress << std::dec << " ====\n";
    {  // K6：模块名（谁崩的——loader/msedge.dll/wx 一锤定音）
      HMODULE mod = nullptr;
      if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                             (LPCWSTR) aInfo->ExceptionRecord->ExceptionAddress, &mod) && mod) {
        wchar_t path[MAX_PATH] = {0};
        if (GetModuleFileNameW(mod, path, MAX_PATH) > 0)
          f << "  module=" << (const char*) wxString(path).ToUTF8().data() << "\n";
      }
    }
    HANDLE   proc = GetCurrentProcess();
    SymInitialize( proc, nullptr, TRUE );
    SymSetOptions( SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME | SYMOPT_LOAD_LINES );
    void* frames[ 32 ];
    int   n = CaptureStackBackTrace( 0, 32, frames, nullptr );
    for( int i = 0; i < n; ++i )
    {
        DWORD64 dish = 0;
        SYMBOL_INFO* si = (SYMBOL_INFO*) calloc( 1, sizeof( SYMBOL_INFO ) + 512 );
        si->SizeOfStruct = sizeof( SYMBOL_INFO );
        si->MaxNameLen = 512;
        if( SymFromAddr( proc, (DWORD64) frames[ i ], &dish, si ) )
        {
            IMAGEHLP_LINE64 line = { sizeof( IMAGEHLP_LINE64 ) };
            DWORD d = 0;
            if( SymGetLineFromAddr64( proc, (DWORD64) frames[ i ], &d, &line ) )
                f << "  #" << i << " " << si->Name << "+0x" << std::hex << dish << std::dec
                  << " (" << line.FileName << ":" << line.LineNumber << ")\n";
            else
                f << "  #" << i << " " << si->Name << "+0x" << std::hex << dish << std::dec << "\n";
        }
        else
            f << "  #" << i << " 0x" << std::hex << (unsigned long long) frames[ i ] << std::dec << "\n";
        free( si );
    }
    f.flush();
    f.close();
    return EXCEPTION_EXECUTE_HANDLER;
}

static void cicadaAssertHandler( const wxString& aFile, int aLine, const wxString& aFunc,
                                 const wxString& aCond, const wxString& aMsg )
{
    std::ofstream f( "C:/dsh/Schematic-Cicada/cicada-editor/logs/app-crash.txt", std::ios::app );
    f << "[ASSERT] " << aFile.ToUTF8().data() << ":" << aLine << " in " << aFunc.ToUTF8().data()
      << " | cond=" << aCond.ToUTF8().data() << " | msg=" << aMsg.ToUTF8().data() << "\n";
    f.close();
}

class CicadaApp final : public wxApp {
 public:
  bool OnInit() override {
    SetUnhandledExceptionFilter( cicadaSehHandler );
    wxSetAssertHandler( cicadaAssertHandler );
    auto* frame = new wxFrame(nullptr, wxID_ANY, "Schematic-Cicada");
    frame->SetClientSize(1100, 720);
#if defined(CICADA_EDITOR_HAS_CANVAS)
    auto* splitter = new wxSplitterWindow(frame, wxID_ANY);
    auto* canvas = new cicada::editor::WxCanvasPanel(splitter);
    canvas_ = canvas;
#if defined(CICADA_EDITOR_HAS_KICAD_MODEL)
    // B3a-2：持久文档（引擎自建活文档）→ 种子 demo → 画布；编辑走引擎原子操作
    engine_ = std::make_unique<cicada::kicad_geometry::SchInteractionEngine>();
    cicada::editor::kicad_adapter::SeedDemoScreen(*engine_);
    canvas->set_engine(
        engine_.get(),
        [](cicada::kicad_geometry::SchInteractionEngine& e) {
          return cicada::editor::kicad_adapter::BuildScene(e, &e.Selection());
        });
    // K6：右键 → "加入到上下文"（延迟注入：随下一条用户消息送达当前回合；不再自动上抛——用户裁定）
    canvas->set_on_context_menu([this](const wxPoint& pos) { show_context_menu(pos); });
    // K6 写回：画布编辑提交 → 保存 .cicada_sch → runtime watcher 感知（user_edit 链）
    canvas->set_on_commit([this] { saveback_edit(); });
#endif
#endif
    wxWindow* right = nullptr;
#if defined(CICADA_EDITOR_HAS_WEBVIEW)
    wxString enableWebView;
    const bool requested = wxGetEnv("CICADA_EDITOR_ENABLE_WEBVIEW", &enableWebView)
      && (enableWebView == "1" || enableWebView.CmpNoCase("true") == 0);
    if (requested) {
      auto* view = wxWebView::New(
#if defined(CICADA_EDITOR_HAS_CANVAS)
        splitter,
#else
        frame,
#endif
        wxID_ANY);
      if (view) {
        view->SetPage("<html><body><h1>Schematic-Cicada</h1><p>DSH webview bridge pending.</p></body></html>", "");
        view_ = view;
        right = view;
      }
    }
#endif
    if (!right) {
#if defined(CICADA_EDITOR_HAS_CANVAS)
      wxWindow* panel_parent = splitter;
#else
      wxWindow* panel_parent = frame;
#endif
      auto* panel = new wxPanel(panel_parent);
      auto* sizer = new wxBoxSizer(wxVERTICAL);
      // K6：完整 DSH 面板入口（浏览器打开不带 layout 参数的整页；右栏 webview 已是对话子页）
      // 注意：wxButton 窄字符串字面量按本地代码页（GBK）解释，中文必须 FromUTF8。
      auto* chat_btn = new wxButton(panel, wxID_ANY, wxString::FromUTF8("打开完整 DSH 面板"));
      sizer->Add(chat_btn, 0, wxEXPAND | wxALL, 4);
      auto* fallback = new wxTextCtrl(panel, wxID_ANY,
        "Schematic-Cicada\n\n"
#if defined(CICADA_EDITOR_HAS_WEBVIEW)
        "wxWebView is disabled or unavailable.\nSet CICADA_EDITOR_ENABLE_WEBVIEW=1 only after a WebView2 backend is installed."
#else
        "WebView module is disabled in this build.\nThe native schematic canvas is available on the left."
#endif
        , wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
      fallback->SetBackgroundColour(wxColour(245, 245, 245));
#if defined(CICADA_EDITOR_HAS_WEBVIEW2)
      // K6：右栏 = DSH 对话子页（WebView2 直挂；?layout=chat → cicada-layout 单列模式）
            wxString wv;
      // K6：WebView2 为代码默认开启（修复后稳定）；仅显式 CICADA_EDITOR_NO_WEBVIEW2=1 回退。
      const bool webview2_on = !wxGetEnv("CICADA_EDITOR_NO_WEBVIEW2", &wv)
        || (wv != "1" && wv.CmpNoCase("true") != 0);
      if (webview2_on) {
      wv2_holder_ = new wxPanel(panel);
      wv2_holder_->SetMinSize(wxSize(-1, 400));
      wv2_holder_->Bind(wxEVT_SIZE, [this](wxSizeEvent& ev) {
        if (wv2_holder_ && wv2_) wv2_->SyncBounds(wv2_holder_->GetHWND());
        ev.Skip();
      });
      sizer->Add(wv2_holder_, 3, wxEXPAND | wxALL, 4);
      sizer->Add(fallback, 0, wxEXPAND | wxALL, 4);   // 状态栏保留（底部，4 行内）
      } else {
        sizer->Add(fallback, 1, wxEXPAND | wxALL, 4);
      }
#else
      sizer->Add(fallback, 1, wxEXPAND | wxALL, 4);
#endif
      panel->SetSizer(sizer);
      status_ = fallback;
      right = panel;
      chat_btn->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
        if (!controller_) { update_status("DSH host 未启动"); return; }
        const auto& h = controller_->host();
        if (!h.web_ui_url || h.web_ui_url->empty()) {
          update_status("尚未收到 dsh web: 行（host 还在启动），稍候再点");
          return;
        }
        const wxString url = wxString::FromUTF8(h.web_ui_url->c_str());
        if (!wxLaunchDefaultBrowser(url))
          update_status("打开浏览器失败，请手动复制状态栏里的 cicada 对话链接");
      });
    }
#if defined(CICADA_EDITOR_HAS_CANVAS)
    // 实时 sash（wxSP_LIVE_UPDATE）：无此样式时 wxMSW 会用黑色轨迹笔在画布上
    // 画一条跟手竖线（splitter.cpp m_sashTrackerPen），松手才真正 resize——codex 时代的黑线伪影
    splitter->SplitVertically(canvas, right, 650);
    splitter->SetSashGravity( 0.5 );
    splitter->SetWindowStyleFlag( splitter->GetWindowStyleFlag() | wxSP_LIVE_UPDATE );
#endif
    load_requested_schematic();
    wxString node, entry, home, preload, cwd;
    {
      std::ofstream debg("C:/dsh/Schematic-Cicada/cicada-editor/logs/app-env-dump.txt");
      wxString v;
      const char* names[] = { "CICADA_DSH_NODE", "CICADA_DSH_ENTRY", "CICADA_DSH_HOME", "CICADA_DSH_PRELOAD", "CICADA_DSH_CWD" };
      for (const char* n : names)
        debg << n << "=" << (wxGetEnv(n, &v) ? std::string(v.ToUTF8()) : std::string("<unset>")) << "\n";
      debg << "argc=" << argc << "\n";
    }
    if (wxGetEnv("CICADA_DSH_NODE", &node) && wxGetEnv("CICADA_DSH_ENTRY", &entry)
        && wxGetEnv("CICADA_DSH_HOME", &home)) {
      // K6：preload 通道（源码 entry 需 --import tsx/esm；built entry 留空）
      wxGetEnv("CICADA_DSH_PRELOAD", &preload);
      // K6：子进程 cwd（模块解析须与 launcher spawnHost 一致——harness 根；缺省=home）
      wxGetEnv("CICADA_DSH_CWD", &cwd);
      controller_ = std::make_unique<cicada::editor::EditorController>(
        cicada::editor::HostLaunchSpec{node.ToUTF8().data(), entry.ToUTF8().data(),
                                       home.ToUTF8().data(), preload.ToUTF8().data(), 3123,
                                       cwd.ToUTF8().data()});
      controller_->start_requested();
      if (process_.start(controller_->spawner(), &startup_error_)) {
        std::ofstream debg("C:/dsh/Schematic-Cicada/cicada-editor/logs/app-start.txt", std::ios::app);
        debg << "spawn OK, timer starting\n";
        timer_.SetOwner(this); timer_.Start(50); Bind(wxEVT_TIMER, &CicadaApp::poll_host, this);
      } else {
        std::ofstream debg("C:/dsh/Schematic-Cicada/cicada-editor/logs/app-start.txt", std::ios::app);
        debg << "spawn FAILED: " << startup_error_ << "\n";
        controller_->process_exited(1);
        update_status("DSH spawn 失败: " + startup_error_ + "\n请核对 CICADA_DSH_NODE/ENTRY/HOME/PRELOAD 四个环境变量");
      }
    }
    frame->Show(true);
    return true;
  }

#if defined(CICADA_EDITOR_HAS_CANVAS) && defined(CICADA_EDITOR_HAS_KICAD_MODEL)
  int OnExit() override {
    ws_stop_ = true;
    if (ws_thread_.joinable()) ws_thread_.join();
    return wxApp::OnExit();
  }
#endif
 private:
  void load_requested_schematic() {
    wxString path;
    if (argc > 1) {
      path = argv[1];
    } else {
      wxGetEnv("CICADA_SCHEMATIC_FILE", &path);
    }
    if (path.empty()) {
      update_status("No schematic loaded.\nPass a .cicada_sch path as the first argument or set CICADA_SCHEMATIC_FILE.");
      return;
    }

    WxTextFileReader reader(path);
    std::string error;
    if (!document_.reload_file(reader, path.ToUTF8().data(), &error)) {
      update_status("Failed to load schematic:\n" + path.ToStdString() + "\n\n" + error);
      return;
    }
    editor_ = std::make_unique<cicada::editor::DocumentEditor>(document_.model());
    schematic_path_ = path;
#if defined(CICADA_EDITOR_HAS_CANVAS)
    // Model coordinates are stored in centi-millimetres.  This initial view
    // keeps the standard 25--50 mm fixtures in the first viewport.
    canvas_->set_scene(cicada::editor::CanvasScene::from_model(document_.model(), 0.10, {120.0, 80.0}));
#endif
    const auto& model = document_.model();
    std::ostringstream summary;
    summary << "Loaded schematic:\n" << path.ToStdString() << "\n\n"
            << "symbols: " << model.symbols.size() << "\n"
            << "wires: " << model.wires.size() << "\n"
            << "labels: " << model.labels.size() << "\n"
            << "junctions: " << model.junctions.size() << "\n"
            << "no-connects: " << model.no_connects.size();
    update_status(summary.str());
  }

  void update_status(const std::string& text) {
    if (status_ != nullptr) status_->SetValue(wxString::FromUTF8(text.c_str()));
  }

  // K6：DSH 通道诊断落盘（CICADA_DSH_DEBUG=1 时；WS/帧/reload 全链路可见）
  void log_dsh(const std::string& s) {
    if (!getenv("CICADA_DSH_DEBUG")) return;
    std::ofstream f("C:/dsh/Schematic-Cicada/cicada-editor/logs/dsh-channel.txt", std::ios::app);
    f << s << "\n";
  }

  // K6 写回：引擎编辑提交 → 存回会话工作区 .cicada_sch（DSH watcher → user_edit）
  void saveback_edit() {
#if defined(CICADA_EDITOR_HAS_CANVAS) && defined(CICADA_EDITOR_HAS_KICAD_MODEL)
    if (!engine_) return;
    if (state_cwd_.empty() || state_file_.empty()) {
      log_dsh("saveback skipped: no cwd/file (无会话或未装载)");
      return;
    }
    const std::string full = state_cwd_ + "/.cicada/" + state_file_;
    wxString err;
    log_dsh("saveback: " + full);
    if (cicada::editor::cicada_doc::SaveCicadaSchIntoFile(*engine_, base_model_,
                                                          wxString::FromUTF8(full.c_str()), &err)) {
      log_dsh("saveback OK");
      update_status("已同步到 " + full + "\n(runtime watcher 记录 user_edit)");
    } else {
      log_dsh("saveback FAILED: " + err.ToStdString());
      update_status("保存失败: " + err.ToStdString());
    }
#endif
  }

  // K6：右键菜单——"加入到上下文"（延迟注入）/ "清除选择"
  void show_context_menu(const wxPoint& pos) {
#if defined(CICADA_EDITOR_HAS_CANVAS) && defined(CICADA_EDITOR_HAS_KICAD_MODEL)
    if (!engine_ || !canvas_) return;
    wxMenu menu;
    // 显式 id：wxID_ANY 动态分配给负值（-31988 级），PopupMenu 返回的却是另一个
    // 空间的 id（实测恒=1）→ 恒不匹配 = 菜单无任何效果（K6 实测三个版本翻车）。
    const int kAdd = 20001, kClr = 20002;
    const size_t n = engine_->Selection().size();
    wxMenuItem* add = menu.Append(kAdd, wxString::FromUTF8("加入到上下文") +
      (n > 0 ? wxString(" (") + wxString::FromUTF8(std::to_string(n).c_str()) + wxString::FromUTF8(" 项)") : wxString()));
    wxMenuItem* clr = menu.Append(kClr, wxString::FromUTF8("清除选择"));
    if (n == 0) add->Enable(false);
    // wxEVT_MENU 事件携带真实选中 id（PopupMenu 返回值在本环境恒为 1，不可信——K6 实测）
    canvas_->Bind(wxEVT_MENU, [this](wxCommandEvent& e) {
      const int picked = e.GetId();
      log_dsh("menu event: picked=" + std::to_string(picked));
      if (picked == kAdd) { log_dsh("menu: add event"); maybe_post_selection(true); }
      else if (picked == kClr) {
        engine_->ClearSelection();
        canvas_->refresh_from_engine();
        update_status("已清除选择");
      }
    }, kAdd, kClr);
    const int id = canvas_->PopupMenu(&menu, pos);
    log_dsh("context menu: items=" + std::to_string(n) + " popup-returned=" + std::to_string(id));
#endif
  }

  void persist_editor(const std::string& message) {
    if (!editor_ || schematic_path_.empty()) { update_status(message); return; }
    WxTextFileWriter writer;
    std::string error;
    if (!document_.save_model(editor_->model(), writer, schematic_path_.ToUTF8().data(), &error)) {
      update_status("Edit applied in memory, save failed: " + error);
      return;
    }
    update_status(message + " (saved)");
  }

  void poll_host(wxTimerEvent&) {
    if (!controller_) return;
    if (!process_.running()) {
      // K6：失败可见（此前 host 起不来时静默，只留"无文件"文案）
      if (controller_->state() != cicada::editor::ControllerState::ready) {
        update_status("DSH host 未就绪: " + startup_error_ + "\n" + last_host_out_);
        if (getenv("CICADA_DSH_DEBUG")) {
          std::ofstream dbg("C:/dsh/Schematic-Cicada/cicada-editor/logs/app-host-status.txt");
          dbg << "error=" << startup_error_ << "\ntail=\n" << last_host_out_ << "\n";
        }
      }
      timer_.Stop();
      return;
    }
    const auto output = process_.read_stdout();
    if (!output.empty()) {
      controller_->consume_stdout(output);
      last_host_out_ = output.size() > 400 ? output.substr(output.size() - 400) : output;
    }
#if defined(CICADA_EDITOR_HAS_WEBVIEW2)
    // K6：host ready → 右栏 WebView2 装载对话子页（?layout=chat 单列模式）
    if (!wv2_started_ && wv2_holder_ && controller_->state() == cicada::editor::ControllerState::ready
        && controller_->host().web_ui_url) {
      wxString wv;
      // K6：代码默认开启（修复后稳定）；仅显式 CICADA_EDITOR_NO_WEBVIEW2=1 回退。
      const bool wvOff = wxGetEnv("CICADA_EDITOR_NO_WEBVIEW2", &wv)
        && (wv == "1" || wv.CmpNoCase("true") == 0);
      log_dsh("poll: wv2 candidate wvOff=" + std::to_string(wvOff));
      if (wvOff) {
        wv2_started_ = true;   // 显式回退（日志框右栏）
        return;
      }
      wv2_started_ = true;
      std::string url = *controller_->host().web_ui_url;
      url += (url.find('?') == std::string::npos ? '?' : '&');
      url += "layout=chat";
      log_dsh("wv2: starting with url head=" + url.substr(0, 40));
      wv2_ = std::make_unique<cicada::editor::CicadaWebView2>();
      const std::wstring wurl(url.begin(), url.end());
      if (wv2_->Start(wv2_holder_->GetHWND(), wurl))
        update_status("DSH 对话已嵌入右栏（单列模式）\n" + url.substr(0, 60) + "…");
      else
        update_status("WebView2 启动失败（检查 WebView2 Runtime）");
    }
#endif
#if defined(CICADA_EDITOR_HAS_CANVAS) && defined(CICADA_EDITOR_HAS_KICAD_MODEL)
    // K6：ready 一次性建立 DSH 通道（GET /state + WS 收帧线程）
    if (controller_->state() == cicada::editor::ControllerState::ready && !dsh_channels_up_)
      setup_dsh_channels();
    drain_ws_frames();
#endif
  }

#if defined(CICADA_EDITOR_HAS_CANVAS) && defined(CICADA_EDITOR_HAS_KICAD_MODEL)
  void setup_dsh_channels() {
    dsh_channels_up_ = true;
    const auto& h = controller_->host();
    if (!h.editor_port || !h.editor_token) {
      update_status("host ready but no editor token (cicada-editor line missing)");
      return;
    }
    base_ = "127.0.0.1:" + std::to_string(*h.editor_port);
    // K6 修复：EditorHttpClient 默认构造为「空 base」→ 相对 URL → WinHttpCrackUrl 失败
    // → 一切请求 status 0（GET /state、POST /selection 都踩过）。必须在此填入真实 base。
    http_ = cicada::editor::EditorHttpClient("http://" + base_);

    // GET /state → 文件路径 + 会话 cwd + 基线（DSH 侧唯一坐标权威）
    cicada::editor::WinHttpTransport t;
    const auto r = http_.get_state(t, *h.editor_token);
    log_dsh("GET /state status=" + std::to_string(r.status) + " body=" + r.body);
    if (r.status == 200) {
      state_file_ = json_str_field(r.body, "file");
      state_cwd_ = json_str_field(r.body, "cwd");
      state_session_id_ = json_str_field(r.body, "sessionId");
      // K6：把 dsh web 的完整鉴权 URL（含 ?token=）露出来——直接访问 3123 会被
      // “authentication required” 挡掉；浏览器必须打开这个 URL 才能开 cicada 会话。
      const std::string chat = h.web_ui_url && !h.web_ui_url->empty() ? *h.web_ui_url : "(未见 dsh web: 行)";
      update_status("DSH ready @ " + base_ + "\nfile=" + state_file_ +
                    "\nsession=" + (state_session_id_.empty() ? "(无会话)" : state_session_id_) +
                    "\ncicada 对话: " + chat);
    } else {
      update_status("DSH ready @ " + base_ + "  GET /state=" + std::to_string(r.status) + "\n" + r.body);
    }

    // WS 收帧线程（WinHTTP receive 阻塞 → 队列 + 50ms 主循环 drain）
    ws_stop_ = false;
    ws_thread_ = std::thread([this] {
      cicada::editor::WinHttpWebSocket ws;
      std::string err;
      if (!ws.connect("ws://" + base_ + "/cicada/editor/ws", *controller_->host().editor_token, &err)) {
        log_dsh("WS connect FAILED: " + err);
        push_ws_frame(std::string("{\"type\":\"ws_error\",\"err\":\"") + err + "\"}");
        return;
      }
      log_dsh("WS connected");
      for (;;) {
        auto f = ws.receive_text(&err);
        if (!err.empty() || ws_stop_.load()) { log_dsh("WS closed: " + err); break; }
        push_ws_frame(f);
      }
    });
  }

  void push_ws_frame(std::string f) {
    std::lock_guard<std::mutex> lk(ws_mx_);
    ws_frames_.push_back(std::move(f));
  }

  void drain_ws_frames() {
    for (;;) {
      std::string f;
      { std::lock_guard<std::mutex> lk(ws_mx_);
        if (ws_frames_.empty()) break;
        f = std::move(ws_frames_.front()); ws_frames_.pop_front(); }

      if (!controller_->consume_frame(f)) { log_dsh("frame UNKNOWN: " + f); continue; }
      log_dsh("frame: " + f);
      if (controller_->events().reload_requested) {
        controller_->clear_reload_request();
        do_file_reload();
      }
      // 帧日志（状态栏尾行，便于冒烟确认收帧）
      const auto trimmed = f.size() > 180 ? f.substr(0, 180) + "…" : f;
      update_status("WS 帧: " + trimmed + "\n" + (state_file_.empty() ? "" : "file=" + state_file_));
    }
  }

  void do_file_reload() {
#if defined(CICADA_EDITOR_HAS_CANVAS) && defined(CICADA_EDITOR_HAS_KICAD_MODEL)
    // 会话可能晚于 app 启动才创建：启动时的 /state 快照 cwd 为空 → 相对路径解析失败
    // → 画布不刷新（K6 实测 canvas 停在 demo）。每次 canvas.refresh 前重拉 /state。
    refresh_state();
    // /state 只给相对文件名（schematic.cicada_sch）；真路径 = 会话 cwd/.cicada/<file>
    //（P6 G-P6-7 工作区布局）。cwd 缺失（旧 host）才退回用相对路径直试。
    std::string full = state_file_;
    if (!state_cwd_.empty() && !state_file_.empty() && std::filesystem::path(state_file_).is_relative())
      full = state_cwd_ + "/.cicada/" + state_file_;
    wxString path = wxString::FromUTF8(full.c_str());
    if (path.IsEmpty()) { log_dsh("reload: no file path"); update_status("canvas.refresh 但 /state 无文件路径"); return; }
    wxString err;
    log_dsh("reload: loading " + path.ToStdString());
    if (cicada::editor::cicada_doc::LoadCicadaSchIntoEngine(*engine_, path, &err, &base_model_)) {
      canvas_->refresh_from_engine();
      log_dsh("reload OK");
      update_status("已重载 " + path.ToStdString() + "\n(canvas.refresh → 引擎重建)");
    } else {
      log_dsh("reload FAILED: " + err.ToStdString());
      update_status("reload 失败: " + err.ToStdString() + "\npath=" + path.ToStdString());
    }
#endif
  }

  // K6：重拉 /state（file/cwd/sessionId 权威刷新；启动快照可能无会话）
  void refresh_state() {
    const auto& h = controller_->host();
    if (!h.editor_token) return;
    cicada::editor::WinHttpTransport t;
    const auto r = http_.get_state(t, *h.editor_token);
    log_dsh("refresh_state status=" + std::to_string(r.status) + " body=" + r.body);
    if (r.status == 200) {
      state_file_ = json_str_field(r.body, "file");
      state_cwd_ = json_str_field(r.body, "cwd");
      state_session_id_ = json_str_field(r.body, "sessionId");
    }
  }

  // K6：引擎选择集 → SelectionItem JSON（契约 contract.ts；junction 无 kind 跳过）
  void maybe_post_selection(bool attach_next_turn = false) {
#if defined(CICADA_EDITOR_HAS_CANVAS) && defined(CICADA_EDITOR_HAS_KICAD_MODEL)
    log_dsh("maybe_post_selection enter engine=" + std::to_string(engine_ != nullptr)
            + " controller=" + std::to_string(controller_ != nullptr)
            + " state=" + std::to_string(controller_ ? static_cast<int>(controller_->state()) : -1));
    if (!engine_ || !controller_ || controller_->state() != cicada::editor::ControllerState::ready) return;
    const auto& h = controller_->host();
    log_dsh("  token=" + std::to_string(h.editor_token.has_value())
            + " port=" + std::to_string(h.editor_port ? *h.editor_port : 0));
    if (!h.editor_token) return;

    const std::string json = selection_json();
    log_dsh("  json=" + json + " (len=" + std::to_string(json.size()) + ")");
    if (json == last_selection_json_) return;
    last_selection_json_ = json;
    if (json == "[]") return;   // 空选/仅 junction：无注入内容，跳过 POST

    cicada::editor::WinHttpTransport t;
    const auto r = http_.post_selection(t, *h.editor_token, json, state_session_id_, attach_next_turn);
    log_dsh("selection POST attach=" + std::string(attach_next_turn ? "true" : "false") +
            " status=" + std::to_string(r.status) + " body=" + r.body);
    if (r.status == 200) {
      // 契约 SelectionResponse {ok, sessionId, injected};失败也把 r.body 亮出来
      const auto sid = json_str_field(r.body, "sessionId");
      update_status(attach_next_turn
        ? "已加入上下文 (" + std::to_string(engine_->Selection().size()) + " 项)\n将随下一条消息发送"
        : ("selection POST=200 ✓ injected into " + (sid.empty() ? "?" : sid)) +
          "\nfile=" + state_file_);
    } else {
      update_status("selection POST=" + std::to_string(r.status) + "\n" +
                    (r.body.empty() ? "(无响应体)" : r.body));
    }
  }
#endif

  std::string selection_json() const {
    // 只产数组；外层 {"selection":…} 由 EditorHttpClient::selection_body 加
    //（契约 SelectionRequest；此前这里也套了 {"selection":…} → 双层 → 400 invalid selection request）
    std::ostringstream out; out << "[";
    bool first = true;
    for (SCH_ITEM* it : engine_->Selection()) {
      if (it->Type() == SCH_JUNCTION_T) continue;   // 契约无 junction kind（v1 跳过）
      if (!first) out << ","; first = false;
      out << selection_item_json(it);
    }
    out << "]";
    return out.str();
  }

  std::string selection_item_json(const SCH_ITEM* it) const {
    std::ostringstream o;
    switch (it->Type()) {
      case SCH_SYMBOL_T: {
        const SCH_SYMBOL* s = static_cast<const SCH_SYMBOL*>(it);
        o << "{\"kind\":\"symbol\"";
        o << ",\"refdes\":\"" << json_esc(s->GetRef(&engine_->sheetPath(), false)) << "\"";
        o << ",\"uuid\":\"" << json_esc(s->m_Uuid.AsString()) << "\"";
        if (const SCH_FIELD* v = s->GetField(FIELD_T::VALUE))
          o << ",\"value\":\"" << json_esc(v->GetText()) << "\"";
        o << ",\"pins\":[";
        bool pf = true;
        for (const SCH_PIN* p : s->GetLibPins()) {
          if (!pf) o << ","; pf = false;
          o << "\"" << json_esc(p->GetNumber()) << "\"";
        }
        o << "]}";
        break;
      }
      case SCH_LINE_T:
        o << "{\"kind\":\"wire\",\"uuid\":\"" << json_esc(it->m_Uuid.AsString()) << "\"}";
        break;
      case SCH_LABEL_T:
      case SCH_GLOBAL_LABEL_T:
      case SCH_HIER_LABEL_T: {
        const SCH_LABEL_BASE* l = static_cast<const SCH_LABEL_BASE*>(it);
        o << "{\"kind\":\"label\",\"uuid\":\"" << json_esc(it->m_Uuid.AsString())
          << "\",\"value\":\"" << json_esc(l->GetText()) << "\"}";
        break;
      }
      case SCH_NO_CONNECT_T:
        o << "{\"kind\":\"no_connect\",\"uuid\":\"" << json_esc(it->m_Uuid.AsString()) << "\"}";
        break;
      default:
        o << "{\"kind\":\"wire\"}";   // 兜底（不可达：junction 已过滤）
        break;
    }
    return o.str();
  }
#endif
#if defined(CICADA_EDITOR_HAS_WEBVIEW)
  wxWebView* view_ = nullptr;
#endif
  wxTimer timer_;
  cicada::editor::HostProcess process_;
  std::unique_ptr<cicada::editor::EditorController> controller_;
  std::string startup_error_;
  std::string last_host_out_;   // K6：host stdout 尾部（失败诊断可见性）
#if defined(CICADA_EDITOR_HAS_CANVAS)
  cicada::editor::WxCanvasPanel* canvas_ = nullptr;
#if defined(CICADA_EDITOR_HAS_KICAD_MODEL)
  std::unique_ptr<cicada::kicad_geometry::SchInteractionEngine> engine_;
  // K6 DSH 通道
  cicada::editor::EditorHttpClient http_;
  std::string base_;
  std::string state_file_;
  std::string state_cwd_;         // K6：/state 会话工作区（绝对）→ 解析 .cicada_sch 真路径
  std::string state_session_id_;  // K6：/state 当前主会话 id（POST 注入目标）
  cicada::editor::SchematicModel base_model_;   // K6 写回基底（最近一次文件装载的模型：lib/version/uuid）
  std::string last_selection_json_;
  bool dsh_channels_up_ = false;
  std::thread ws_thread_;
  std::mutex ws_mx_;
  std::deque<std::string> ws_frames_;
  std::atomic<bool> ws_stop_{false};
#endif
#endif
#if defined(CICADA_EDITOR_HAS_WEBVIEW2)
  wxPanel* wv2_holder_ = nullptr;
  std::unique_ptr<cicada::editor::CicadaWebView2> wv2_;
  bool wv2_started_ = false;
#endif
  wxTextCtrl* status_ = nullptr;
  cicada::editor::CanvasDocument document_;
  std::unique_ptr<cicada::editor::DocumentEditor> editor_;
  wxString schematic_path_;
};

wxIMPLEMENT_APP(CicadaApp);
