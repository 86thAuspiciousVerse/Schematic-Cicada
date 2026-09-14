#include <cassert>
#include <cmath>
#include <string>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include "../../src/bridge/editor_http_client.h"
#include "../../src/bridge/editor_ws_client.h"
#include "../../src/app/stdout_parser.h"
#include "../../src/app/editor_controller.h"
#include "../../src/core/sexpr_model.h"
#include "../../src/core/kicad_sexpr_adapter.h"
#include "../../src/core/sexpr_writer.h"
#include "../../src/core/cicada_screen_adapter.h"
#include "../../src/core/connection_graph.h"
#include "../../src/core/document_editor.h"
#include "../../src/render/canvas_scene.h"
#include "../../src/app/host_process.h"
#include "../../src/core/tool_registry.h"
#include "../../src/bridge/winhttp_transport.h"
#include "../../src/bridge/winhttp_websocket.h"
#include <sexpr/sexpr_parser.h>
#include "../../src/bridge/event_dispatcher.h"
#include "../../src/render/canvas_document.h"

struct FakeTransport final : cicada::editor::IHttpTransport {
  std::string method, url, auth, body;
  cicada::editor::HttpResponse get(std::string_view u, std::string_view a) override { method = "GET"; url = u; auth = a; return {200, "{}"}; }
  cicada::editor::HttpResponse post(std::string_view u, std::string_view a, std::string_view b) override { method = "POST"; url = u; auth = a; body = b; return {200, "{}"}; }
};

struct FakeFileReader final : cicada::editor::ITextFileReader {
  std::string path;
  std::string read(std::string_view requested) override {
    path = requested;
    return "(kicad_sch (version 20250114) (symbol (lib_id \"cicada:R\") (property \"Reference\" \"R2\") (property \"Value\" \"1k\")))";
  }
};
struct FakeFileWriter final : cicada::editor::ITextFileWriter {
  std::string path, body;
  void write(std::string_view requested, std::string_view text) override { path = requested; body = text; }
};

int main(int argc, char** argv) {
  if (argc == 3 && std::string(argv[1]) == "--write-kicad") {
    std::ifstream fixture("../../cicada-harness/packages/cicada/cicada-format/tests/fixtures/minimal.cicada_sch", std::ios::binary);
    if (!fixture.good()) return 2;
    std::ostringstream source;
    source << fixture.rdbuf();
    try {
      const auto model = cicada::editor::SexprModelReader().read(source.str());
      std::ofstream output(argv[2], std::ios::binary | std::ios::trunc);
      if (!output.good()) return 3;
      output << cicada::editor::SexprModelWriter().write(model);
      return output.good() ? 0 : 4;
    } catch (...) {
      return 5;
    }
  }
  cicada::editor::EditorHttpClient http("http://127.0.0.1:3123");
  cicada::editor::EditorWsClient ws("ws://127.0.0.1:3123");
  assert(http.state_url().rfind("/cicada/editor/state") == http.state_url().size() - std::string("/cicada/editor/state").size());
  assert(http.selection_url().rfind("/cicada/editor/selection") == http.selection_url().size() - std::string("/cicada/editor/selection").size());
  assert(ws.endpoint().rfind("/cicada/editor/ws") == ws.endpoint().size() - std::string("/cicada/editor/ws").size());
  assert(http.authorization_header("abc") == "Bearer abc");
  assert(http.selection_body("[]", "main") == "{\"selection\":[],\"sessionId\":\"main\"}");
  assert(cicada::editor::EditorWsClient::is_known_frame_type("canvas.refresh"));
  assert(!cicada::editor::EditorWsClient::is_known_frame_type("unknown"));
  FakeTransport transport;
  assert(http.get_state(transport, "abc").status == 200 && transport.method == "GET");
  assert(http.post_selection(transport, "abc", "[]", "main").status == 200 && transport.method == "POST");
  assert(transport.auth == "Bearer abc" && transport.body.find("sessionId") != std::string::npos);
  const auto frame = cicada::editor::EditorWsClient::parse_frame("{\"type\":\"canvas.refresh\",\"file\":\"x\"}");
  assert(frame.has_value() && frame->type == "canvas.refresh");
  assert(!cicada::editor::EditorWsClient::parse_frame("{\"type\":\"evil\"}").has_value());
  cicada::editor::StdoutParser parser;
  parser.feed("noise\ndsh web: http://127.0.0.1:3123/?token=redacted\ncicada-ed");
  parser.feed("itor: 3123 abc_token\r\n");
  assert(parser.state().web_ui_url.value() == "http://127.0.0.1:3123/?token=redacted");
  assert(parser.state().editor_port.value() == 3123);
  assert(parser.state().editor_token.value() == "abc_token");
  cicada::editor::EditorController controller({"node", "dsh", "cicada-home", "tsx/esm", 3123});
  controller.start_requested();
  assert(controller.state() == cicada::editor::ControllerState::starting);
  controller.consume_stdout("dsh web: http://127.0.0.1:3123/?token=x\ncicada-editor: 3123 y\n");
  assert(controller.state() == cicada::editor::ControllerState::ready);
  assert(controller.spawner().argv().size() == 11);
  assert(controller.spawner().environment().second == "cicada-home");
  controller.process_exited(1);
  assert(controller.state() == cicada::editor::ControllerState::failed);
  cicada::editor::HostProcess process;
  std::string process_error;
#ifndef _WIN32
  assert(!process.start(controller.spawner(), &process_error) && !process_error.empty());
#endif
  assert(controller.consume_frame("{\"type\":\"canvas.refresh\",\"file\":\"schematic.cicada_sch\"}"));
  assert(controller.events().reload_requested);
  const auto model = cicada::editor::SexprModelReader().read("(kicad_sch (version 20250114) (symbol (lib_id \"cicada:R\") (property \"Reference\" \"R1\") (property \"Value\" \"10k\")))");
  assert(model.symbols.size() == 1 && model.symbols[0].refdes == "R1" && model.symbols[0].value == "10k");
  {
    std::ifstream fixture("../../cicada-harness/packages/cicada/cicada-format/tests/fixtures/minimal.cicada_sch", std::ios::binary);
    assert(fixture.good());
    std::ostringstream text;
    text << fixture.rdbuf();
    const auto minimal = cicada::editor::SexprModelReader().read(text.str());
    assert(minimal.lib_symbols.size() == 2 && minimal.symbols.size() == 2);
    assert(minimal.wires.size() == 1 && minimal.labels.size() == 1 && minimal.no_connects.size() == 1);
    assert(minimal.symbols[0].pins.size() == 2 && minimal.symbols[1].pins.size() == 2);
    cicada::editor::CicadaScreenAdapter minimal_screen;
    std::string minimal_screen_error;
    assert(minimal_screen.load(text.str(), &minimal_screen_error));
    const auto indexed = minimal_screen.items();
    assert(indexed.size() == 5);
    assert(indexed[0].kind == cicada::editor::ScreenItemKind::symbol);
    assert(indexed[1].kind == cicada::editor::ScreenItemKind::symbol);
    assert(indexed[2].kind == cicada::editor::ScreenItemKind::wire);
    assert(indexed[3].kind == cicada::editor::ScreenItemKind::label);
    assert(indexed[4].kind == cicada::editor::ScreenItemKind::no_connect);
    assert(minimal_screen.selection_json().find("\"kind\":\"junction\"") == std::string::npos);
  }
  const auto markers = cicada::editor::SexprModelReader().read(R"((kicad_sch
    (version 20250114)
    (junction (at 1.00 2.00 0))
    (no_connect (at 3.00 4.00))))");
  assert(markers.junctions.size() == 1 && markers.junctions[0].x_g == 100 && markers.junctions[0].y_g == 200);
  assert(markers.no_connects.size() == 1 && markers.no_connects[0].x_g == 300 && markers.no_connects[0].y_g == 400);
  bool unsupported = false;
  try {
    (void) cicada::editor::SexprModelReader().read("(kicad_sch (version 20250114) (bus (pts (xy 0 0) (xy 1 1))))");
  } catch (const std::runtime_error& error) {
    unsupported = std::string(error.what()).find("unsupported top-level token bus") != std::string::npos;
  }
  assert(unsupported);
  bool missing_version = false;
  try {
    (void)cicada::editor::SexprModelReader().read("(kicad_sch (generator \"cicada\"))");
  } catch (const std::runtime_error& error) {
    missing_version = std::string(error.what()).find("missing version") != std::string::npos;
  }
  assert(missing_version);
  bool malformed_kicad = false;
  try {
    (void)cicada::editor::KiCadSexprAdapter().parse(")");
  } catch (const std::runtime_error& error) {
    malformed_kicad = std::string(error.what()).find("KiCad parser") != std::string::npos;
  }
  assert(malformed_kicad);
  const auto rich = cicada::editor::SexprModelReader().read(R"((kicad_sch
    (version 20250114)
    (symbol (lib_id "cicada:C") (at 12.34 5.67 0)
      (property "Reference" "C1") (property "Value" "100nF")
      (pin "1") (pin "2"))
    (wire (pts (xy 1.00 2.00) (xy 3.00 4.00)))
    (label "NET1" (at 3.00 4.00 0))))");
  assert(rich.symbols.size() == 1 && rich.symbols[0].pins.size() == 2);
  assert(rich.symbols[0].x_g == 1234 && rich.symbols[0].y_g == 567);
  assert(rich.wires.size() == 1 && rich.wires[0].points.size() == 2);
  assert(rich.labels.size() == 1 && rich.labels[0].text == "NET1" && rich.labels[0].x_g == 300);
  const auto canonical = cicada::editor::SexprModelWriter().write(cicada::editor::SexprModelReader().read(R"((kicad_sch
    (version 20250114) (generator "cicada") (generator_version "0.1") (uuid root)
    (symbol (lib_id "cicada:R") (at 1.23 4.56 90) (unit 1) (uuid sym)
      (property "Reference" "R1") (property "Value" "10k") (pin "1" (uuid pin)))
    (wire (pts (xy 0 0) (xy 1.00 2.00)) (uuid wire))
    (label "NET1" (at 1 2 0) (uuid label))
    (junction (at 1 2) (uuid j)) (no_connect (at 3 4) (uuid n))))"));
  assert(canonical.find("(version 20250114)") != std::string::npos);
  assert(canonical.find("(symbol (lib_id \"cicada:R\") (at 1.23 4.56 90)") != std::string::npos);
  assert(canonical.find("(label \"NET1\"") != std::string::npos);
  const auto roundtrip = cicada::editor::SexprModelReader().read(canonical);
  assert(roundtrip.symbols.size() == 1 && roundtrip.symbols[0].uuid == "sym" && roundtrip.symbols[0].rotation == 90);
  assert(roundtrip.wires.size() == 1 && roundtrip.wires[0].uuid == "wire");
  const auto graph = cicada::editor::ConnectionGraph::build(roundtrip);
  assert(graph.connected({0, 0}, {100, 200}));
  assert(graph.label_at({100, 200}).value() == "NET1");
  assert(graph.component_count() == 1);
  const auto interior = cicada::editor::SexprModelReader().read(R"((kicad_sch (version 20250114)
    (wire (pts (xy 0 0) (xy 10 0))) (junction (at 5 0)) (label "MID" (at 5 0 0))))");
  const auto interior_graph = cicada::editor::ConnectionGraph::build(interior);
  assert(interior_graph.connected({0, 0}, {500, 0}) && interior_graph.label_at({500, 0}).value() == "MID");
  const auto with_lib = cicada::editor::SexprModelReader().read(R"((kicad_sch
    (version 20250114)
    (lib_symbols (symbol "cicada:R" (symbol "R_1_1"
      (pin passive line (at 0 3.81 270) (length 1.27) (name "") (number "1")))))
    (symbol (lib_id "cicada:R") (at 10 20 0) (unit 1)
      (property "Reference" "R1") (property "Value" "10k") (pin "1"))))");
  assert(with_lib.lib_symbols.size() == 1 && with_lib.lib_symbols[0].pins.size() == 1);
  assert(with_lib.lib_symbols[0].pins[0].x_g == 0 && with_lib.lib_symbols[0].pins[0].y_g == 381);
  const auto lib_roundtrip = cicada::editor::SexprModelWriter().write(with_lib);
  assert(lib_roundtrip.find("(lib_symbols") != std::string::npos);
  assert(lib_roundtrip.find("(symbol \"cicada:R\"") != std::string::npos);
  const auto sheet_model = cicada::editor::SexprModelReader().read("(kicad_sch (version 20250114) (sheet_instances (path \"/\" (page \"1\"))))");
  assert(sheet_model.has_sheet_instances && !sheet_model.sheet_instances_body.empty());
  const auto sheet_roundtrip = cicada::editor::SexprModelWriter().write(sheet_model);
  assert(sheet_roundtrip.find(sheet_model.sheet_instances_body) != std::string::npos);
  const auto pin_graph = cicada::editor::ConnectionGraph::build(with_lib);
  const auto pin_world = pin_graph.pin_at("R1", "1");
  assert(pin_world.has_value() && pin_world->first == 1000 && pin_world->second == 1619);
  cicada::editor::DocumentEditor editor(with_lib);
  std::string edit_error;
  assert(editor.apply({cicada::editor::EditCommandKind::MoveSymbol, "R1", {}, {}, {}, 1200, 2200}, &edit_error));
  assert(editor.model().symbols[0].x_g == 1200 && editor.undo_depth() == 1);
  assert(editor.undo(&edit_error) && editor.model().symbols[0].x_g == 1000);
  assert(editor.redo(&edit_error) && editor.model().symbols[0].x_g == 1200);
  assert(editor.apply({cicada::editor::EditCommandKind::AddWire, {}, {}, {}, "new-wire", 1200, 1819, 0, 1500, 1819}, &edit_error));
  assert(editor.model().wires.size() == 1 && editor.redo_depth() == 0);
  assert(editor.apply({cicada::editor::EditCommandKind::MoveSymbol, "R1", {}, {}, {}, 1300, 2200}, &edit_error));
  assert(editor.model().wires[0].points[0].first == 1300 && editor.model().wires[0].points[0].second == 1819);
  cicada::editor::ToolRegistry tools(editor);
  cicada::editor::EditCommand place;
  place.kind = cicada::editor::EditCommandKind::AddSymbol;
  place.refdes = "R2";
  place.value = "10k";
  place.lib_id = "cicada:R";
  place.uuid = "placed-r2";
  place.x_g = 2000;
  place.y_g = 2200;
  assert(tools.invoke("place_symbol", place, &edit_error));
  assert(editor.model().symbols.size() == 2 && editor.model().symbols.back().refdes == "R2");
  assert(editor.model().symbols.back().pins.size() == editor.model().lib_symbols[0].pins.size());
  cicada::editor::EditCommand power;
  power.kind = cicada::editor::EditCommandKind::AddPowerSymbol;
  power.refdes = "#PWR01";
  power.value = "GND";
  power.lib_id = "cicada:GND";
  power.x_g = 2400;
  power.y_g = 2200;
  assert(!tools.invoke("place_power_symbol", power, &edit_error));
  assert(edit_error.find("library definition") != std::string::npos);
  cicada::editor::EditCommand disconnect;
  disconnect.kind = cicada::editor::EditCommandKind::Disconnect;
  disconnect.refdes = "R1";
  disconnect.pin_number = "1";
  assert(tools.invoke("disconnect", disconnect, &edit_error));
  assert(editor.model().wires.empty());
  cicada::editor::EditCommand remove;
  remove.kind = cicada::editor::EditCommandKind::RemoveComponent;
  remove.refdes = "R2";
  assert(tools.invoke("remove_component", remove, &edit_error));
  assert(editor.model().symbols.size() == 1 && editor.model().symbols[0].refdes == "R1");
  cicada::editor::EditCommand replacement_wire;
  replacement_wire.kind = cicada::editor::EditCommandKind::AddWire;
  replacement_wire.uuid = "delete-wire";
  replacement_wire.x_g = 1500;
  replacement_wire.y_g = 1900;
  replacement_wire.x2_g = 1700;
  replacement_wire.y2_g = 1900;
  assert(tools.invoke("add_wire", replacement_wire, &edit_error));
  cicada::editor::EditCommand delete_wire;
  delete_wire.kind = cicada::editor::EditCommandKind::DeleteWire;
  delete_wire.uuid = "delete-wire";
  assert(tools.invoke("delete_wire", delete_wire, &edit_error));
  assert(editor.model().wires.empty());
  assert(cicada::editor::ToolRegistry::is_allowed("move_symbol"));
  assert(cicada::editor::ToolRegistry::is_allowed("place_label"));
  assert(cicada::editor::ToolRegistry::is_allowed("place_no_connect"));
  assert(cicada::editor::ToolRegistry::is_allowed("remove_component"));
  assert(cicada::editor::ToolRegistry::is_allowed("disconnect"));
  assert(!cicada::editor::ToolRegistry::is_allowed("emit_bus"));
  assert(!tools.invoke("emit_bus", {cicada::editor::EditCommandKind::AddWire}, &edit_error));
  const auto scene = cicada::editor::CanvasScene::from_model(editor.model(), 2.0, {10.0, 20.0});
  assert(!scene.primitives().empty());
  const auto body_hit = scene.hit_test(scene.world_to_view({1300 - 254, 2200 - 381}));
  assert(body_hit.has_value() && body_hit->kind == cicada::editor::CanvasPrimitive::Kind::SymbolBody && body_hit->id == "R1");
  const auto pin_hit = scene.hit_test(scene.world_to_view({1300, 1819}), 3.0);
  assert(pin_hit.has_value() && pin_hit->kind == cicada::editor::CanvasPrimitive::Kind::Pin && pin_hit->id == "R1.1");
  const auto view_point = scene.world_to_view({100, 200});
  assert(view_point.x == 210.0 && view_point.y == 420.0);
  const auto world_point = scene.view_to_world(view_point);
  assert(std::abs(world_point.x - 100.0) < 1e-9 && std::abs(world_point.y - 200.0) < 1e-9);
  bool dropped_sections = false;
  try {
    (void)cicada::editor::SexprModelWriter().write(cicada::editor::SexprModelReader().read(
      "(kicad_sch (version 20250114) (title_block (title \"x\")))"));
  } catch (const std::runtime_error& error) {
    dropped_sections = std::string(error.what()).find("unrepresented sections") != std::string::npos;
  }
  assert(dropped_sections);
  bool malformed_vendor = false;
  try { (void)SEXPR::PARSER().Parse(")"); } catch (...) { malformed_vendor = true; }
  assert(malformed_vendor);
  cicada::editor::CanvasDocument document;
  std::string error;
  assert(document.reload("(kicad_sch (version 20250114) (symbol (lib_id \"cicada:R\") (property \"Reference\" \"R1\") (property \"Value\" \"10k\")))", &error));
  assert(document.loaded() && document.model().symbols.size() == 1 && error.empty());
  assert(!document.reload("(bad)", &error) && !document.loaded() && !error.empty());
  FakeFileReader file_reader;
  assert(document.reload_file(file_reader, "schematic.cicada_sch", &error));
  assert(file_reader.path == "schematic.cicada_sch" && document.model().symbols[0].refdes == "R2");
  FakeFileWriter file_writer;
  assert(document.save_file(file_writer, "out.cicada_sch", &error));
  assert(file_writer.path == "out.cicada_sch" && file_writer.body.find("(version 20250114)") != std::string::npos);
  cicada::editor::CicadaScreenAdapter screen;
  std::string screen_text;
  assert(!screen.save(&screen_text, &error) && error == "screen is not loaded");
  assert(screen.load("(kicad_sch (version 20250114) (symbol (lib_id \"cicada:R\") (property \"Reference\" \"R3\") (property \"Value\" \"1k\")))", &error));
  assert(screen.loaded() && screen.model().symbols.size() == 1 && error.empty());
  assert(screen.save(&screen_text, &error) && screen_text.find("(property \"Reference\" \"R3\"") != std::string::npos);
  const auto screen_items = screen.items();
  assert(screen_items.size() == 1 && screen_items[0].kind == cicada::editor::ScreenItemKind::symbol &&
         screen_items[0].id == "R3#0");
  const auto screen_selection = screen.selection_json();
  assert(screen_selection == "[{\"kind\":\"symbol\",\"refdes\":\"R3\",\"uuid\":\"R3#0\",\"value\":\"1k\"}]");
  cicada::editor::CicadaScreenAdapter selection_edges;
  assert(selection_edges.load(
    "(kicad_sch (version 20250114) "
    "(symbol (lib_id \"cicada:R\") (at 0 0 0) "
    "(property \"Reference\" \"R" "\n" "1\") (property \"Value\" \"1k\")) "
    "(wire (pts (xy 0 0) (xy 1 0))) "
    "(wire (pts (xy 2 0) (xy 3 0))))", &error));
  const auto edge_selection = selection_edges.selection_json();
  assert(edge_selection.find("R\\n1") != std::string::npos);
  assert(edge_selection.find("wire@0:0#0") != std::string::npos);
  assert(edge_selection.find("wire@200:0#1") != std::string::npos);
  assert(!screen.load("(kicad_sch (version 20250114) (wire (pts (xy 0 0) (xy 1 1)) (bus (x 1))) )", &error));
  assert(!screen.loaded() && !error.empty());
  assert(!screen.load("(kicad_sch (version 20250114) (wire junk (pts (xy 0 0) (xy 1 1))))", &error));
  assert(!screen.loaded() && error.find("unsupported atom") != std::string::npos);
  bool duplicate_property = false;
  try {
    (void)cicada::editor::SexprModelReader().read(
      "(kicad_sch (version 20250114) (symbol (lib_id \"cicada:R\") "
      "(property \"Reference\" \"R1\") (property \"Reference\" \"R2\") "
      "(property \"Value\" \"1k\")))");
  } catch (const std::runtime_error& ex) {
    duplicate_property = std::string(ex.what()).find("duplicate symbol property") != std::string::npos;
  }
  assert(duplicate_property);
  bool malformed_wire = false;
  try {
    (void)cicada::editor::SexprModelReader().read(
      "(kicad_sch (version 20250114) (wire (pts (xy 0 0) (xy 1 1) (xy 2 2))))");
  } catch (const std::runtime_error& ex) {
    malformed_wire = std::string(ex.what()).find("exactly two points") != std::string::npos;
  }
  assert(malformed_wire);
  bool duplicate_uuid = false;
  try {
    (void)cicada::editor::SexprModelReader().read(
      "(kicad_sch (version 20250114) (uuid root) "
      "(symbol (lib_id \"cicada:R\") (uuid same) (property \"Reference\" \"R1\") "
      "(property \"Value\" \"1k\") (pin \"1\" (uuid same))))");
  } catch (const std::runtime_error& ex) {
    duplicate_uuid = std::string(ex.what()).find("duplicate ") != std::string::npos &&
                     std::string(ex.what()).find("uuid") != std::string::npos;
  }
  assert(duplicate_uuid);
  bool duplicate_version = false;
  try {
    (void)cicada::editor::SexprModelReader().read(
      "(kicad_sch (version 20250114) (version 20250114))");
  } catch (const std::runtime_error& ex) {
    duplicate_version = std::string(ex.what()).find("duplicate top-level token version") != std::string::npos;
  }
  assert(duplicate_version);
  cicada::editor::EventDispatcher dispatcher;
  assert(dispatcher.consume("{\"type\":\"canvas.refresh\",\"file\":\"schematic.cicada_sch\"}"));
  assert(dispatcher.state().reload_requested && dispatcher.state().file.value() == "schematic.cicada_sch");
  assert(dispatcher.consume("{\"type\":\"changelog\",\"seq\":7,\"baselineHash\":\"abc\"}"));
  assert(dispatcher.state().changelog_seq.value() == 7 && dispatcher.state().baseline_hash.value() == "abc");
  assert(dispatcher.consume("{\"type\":\"selection.confirm\",\"sessionId\":\"main\",\"selection\":[]}"));
  assert(dispatcher.state().selection_confirmed && dispatcher.state().session_id.value() == "main");
  assert(dispatcher.consume("{\"type\":\"ping\",\"t\":123}"));
  assert(dispatcher.state().last_ping.value() == 123);
  assert(!dispatcher.consume("{\"type\":\"changelog\",\"seq\":-1}"));
  cicada::editor::WinHttpTransport real_http;
#ifndef _WIN32
  assert(real_http.get("http://127.0.0.1:1", "Bearer x").status == 0);
  cicada::editor::WinHttpWebSocket real_ws;
  assert(!real_ws.connect("ws://127.0.0.1:1", "x", &edit_error) && !real_ws.connected());
#endif
}
