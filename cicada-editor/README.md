# cicada-editor

Windows-first wxWidgets host for the Schematic-Cicada desktop shape. The
planned window owns the KiCad-derived schematic canvas and embeds the DSH web
UI in a wxWebView panel. This first slice freezes the bridge contract and keeps
the KiCad/M2 rendering implementation replaceable.

The project deliberately builds its contract/core library without wxWidgets;
when wxWidgets (and WebView2 on Windows) is available, CMake also emits the
desktop executable. No local SDK or KiCad checkout path is assumed.

Next M2 work: copy the smallest KiCad eeschema model/IO/render/tool closure,
then connect `EditorHttpClient` and `EditorWsClient` to the editor-bridge
selection, state, and refresh frames.

The current core slice now includes `KiCadSexprAdapter`, a Cicada-owned seam
around the copied KiCad `libs/sexpr` parser. It validates the top-level
allow-list and projects the supported subset into the Cicada model; the full
eeschema SCH/GAL/TOOL closure remains a later M2 slice.

The current core slice also includes `StdoutParser` (chunk-safe launcher line
protocol), `CicadaHostSpawner` (argv/environment description), and
`EditorController` (stopped/starting/ready/failed lifecycle state). Process
creation and socket I/O remain intentionally injected/platform-specific.

M2's logical bridge loop is now present as `EventDispatcher`: it accepts the
editor-bridge downlink union and records refresh, baseline, changelog,
selection confirmation, datasheet update, and ping state. `CanvasDocument`
reloads a schematic through an injected text-file reader and replaces its model
only after a successful parse.

K1 now exposes `CicadaScreenAdapter` as the headless load/save seam.  The
adapter preserves unsupported-but-allowed metadata such as `sheet_instances`
without inventing replacement content, and the KiCad-origin S-expression
boundary rejects duplicate references/UUIDs, malformed wires, and unknown
nested tokens.  A test-only writer mode can emit a temporary `.kicad_sch` for
`kicad-cli` netlist comparison; this does not change the `.cicada_sch` source
of truth or claim that the full KiCad `SCH_SCREEN`/`sch_io` closure is already
embedded.

The wx canvas now has a direct minimal interaction path over the whitelist model:
dragging a symbol previews connected wire endpoints together, dragging a wire endpoint
performs a POINT_EDITOR-style disconnect, and committed edits are serialized back to the
source `.cicada_sch`. The core whitelist editor also accepts `place_symbol` and
`place_power_symbol` commands when their embedded `lib_symbols` definition is present;
instances inherit the definition's pin table and are saved through the same writer boundary.
The same whitelist now exposes `disconnect` (by `refdes.pin_number`) and
`remove_component`, including cleanup of anchored labels/no-connect markers and wires.
This remains the pre-KiCad seam; native KiCad GAL/SCH_VIEW/TOOL_MANAGER integration is
still the next K2 implementation step, and the wx UI has not yet exposed a symbol chooser.
