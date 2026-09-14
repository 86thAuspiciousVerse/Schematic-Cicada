# Cicada-owned KiCad S-expression seed

This directory contains the first selective copy of KiCad's `libs/sexpr`
implementation. The source origin is `kicad/libs/sexpr` in this workspace.
It is copied here so future changes are owned and reviewable by Cicada; the
upstream `kicad/` tree remains read-only.

The copy keeps the upstream GPL copyright headers. Cicada-specific changes in
this seed remove the file API and replace wxWidgets/fmt-only formatting code
with standard-library equivalents, leaving text input to `CanvasDocument`'s
injected reader. It is not yet the public `.cicada_sch` parser: the adapter in
`src/core/sexpr_model.*` remains responsible for whitelist enforcement,
coordinate quantization, and fail-closed validation. Integration into the
production target is deliberately deferred until adapter negative tests cover
the upstream parser's edge cases.
