#include "cicada_screen_adapter.h"

#include "sexpr_writer.h"

#include <exception>
#include <algorithm>
#include <string>
#include <sstream>

namespace cicada::editor {

namespace {

std::string json_escape(std::string_view value) {
  std::string escaped;
  escaped.reserve(value.size());
  for (const unsigned char c : value) {
    switch (c) {
      case '\\': escaped += "\\\\"; break;
      case '"': escaped += "\\\""; break;
      case '\b': escaped += "\\b"; break;
      case '\f': escaped += "\\f"; break;
      case '\n': escaped += "\\n"; break;
      case '\r': escaped += "\\r"; break;
      case '\t': escaped += "\\t"; break;
      default:
        if (c < 0x20) {
          static constexpr char hex[] = "0123456789abcdef";
          escaped += "\\u00";
          escaped += hex[(c >> 4) & 0x0f];
          escaped += hex[c & 0x0f];
        } else {
          escaped.push_back(static_cast<char>(c));
        }
    }
  }
  return escaped;
}

}  // namespace

bool CicadaScreenAdapter::load(std::string_view text, std::string* error) {
  try {
    const auto parsed = SexprModelReader().read(text);
    model_ = parsed;
    loaded_ = true;
    if (error != nullptr) error->clear();
    return true;
  } catch (const std::exception& ex) {
    loaded_ = false;
    if (error != nullptr) *error = ex.what();
    return false;
  }
}

bool CicadaScreenAdapter::save(std::string* text, std::string* error) const {
  if (text == nullptr) {
    if (error != nullptr) *error = "output text is null";
    return false;
  }
  if (!loaded_) {
    if (error != nullptr) *error = "screen is not loaded";
    return false;
  }
  try {
    *text = SexprModelWriter().write(model_);
    if (error != nullptr) error->clear();
    return true;
  } catch (const std::exception& ex) {
    // Never replace a caller's output with a partial/invalid serialization.
    text->clear();
    if (error != nullptr) *error = ex.what();
    return false;
  }
}

std::vector<ScreenItem> CicadaScreenAdapter::items() const {
  std::vector<ScreenItem> result;
  if (!loaded_) return result;
  result.reserve(model_.symbols.size() + model_.wires.size() + model_.labels.size() +
                 model_.junctions.size() + model_.no_connects.size());
  std::size_t ordinal = 0;
  for (const auto& symbol : model_.symbols) {
    result.push_back({ScreenItemKind::symbol,
                      symbol.uuid.empty() ? symbol.refdes + "#" + std::to_string(ordinal) : symbol.uuid,
                      {symbol.x_g, symbol.y_g}});
    ++ordinal;
  }
  ordinal = 0;
  for (const auto& wire : model_.wires) {
    if (wire.points.size() != 2) continue;
    result.push_back({ScreenItemKind::wire,
                      wire.uuid.empty() ? "wire@" + std::to_string(wire.points[0].first) + ":" +
                                             std::to_string(wire.points[0].second) + "#" + std::to_string(ordinal)
                                         : wire.uuid,
                      wire.points[0]});
    ++ordinal;
  }
  ordinal = 0;
  for (const auto& label : model_.labels) {
    result.push_back({ScreenItemKind::label,
                      label.uuid.empty() ? "label@" + std::to_string(label.x_g) + ":" +
                                              std::to_string(label.y_g) + "#" + std::to_string(ordinal)
                                          : label.uuid,
                      {label.x_g, label.y_g}});
    ++ordinal;
  }
  ordinal = 0;
  for (const auto& junction : model_.junctions) {
    result.push_back({ScreenItemKind::junction,
                      junction.uuid.empty() ? "junction@" + std::to_string(junction.x_g) + ":" +
                                                 std::to_string(junction.y_g) + "#" + std::to_string(ordinal)
                                             : junction.uuid,
                      {junction.x_g, junction.y_g}});
    ++ordinal;
  }
  ordinal = 0;
  for (const auto& no_connect : model_.no_connects) {
    result.push_back({ScreenItemKind::no_connect,
                      no_connect.uuid.empty() ? "no_connect@" + std::to_string(no_connect.x_g) + ":" +
                                                   std::to_string(no_connect.y_g) + "#" + std::to_string(ordinal)
                                               : no_connect.uuid,
                      {no_connect.x_g, no_connect.y_g}});
    ++ordinal;
  }
  std::stable_sort(result.begin(), result.end(), [](const ScreenItem& a, const ScreenItem& b) {
    if (a.kind != b.kind) return static_cast<int>(a.kind) < static_cast<int>(b.kind);
    return a.id < b.id;
  });
  return result;
}

std::string CicadaScreenAdapter::selection_json() const {
  const auto indexed = items();
  std::ostringstream out;
  out << "[";
  bool first = true;
  for (const auto& item : indexed) {
    // The frozen editor-bridge SelectionKind contract has no junction entry;
    // junctions remain in the screen index but are not injected as selection
    // items until that bridge contract is explicitly extended.
    if (item.kind == ScreenItemKind::junction) continue;
    if (!first) out << ",";
    first = false;
    const char* kind = "";
    switch (item.kind) {
      case ScreenItemKind::symbol: kind = "symbol"; break;
      case ScreenItemKind::wire: kind = "wire"; break;
      case ScreenItemKind::label: kind = "label"; break;
      case ScreenItemKind::junction: kind = "junction"; break;
      case ScreenItemKind::no_connect: kind = "no_connect"; break;
    }
    out << "{\"kind\":\"" << kind << "\"";
    auto field = [&](const char* key, const std::string& value) {
      out << ",\"" << key << "\":\"" << json_escape(value) << "\"";
    };
    if (item.kind == ScreenItemKind::symbol) {
      const auto it = std::find_if(model_.symbols.begin(), model_.symbols.end(), [&](const auto& symbol) {
        return (symbol.uuid.empty() ? symbol.refdes + "#" + std::to_string(&symbol - model_.symbols.data()) : symbol.uuid) == item.id;
      });
      if (it != model_.symbols.end()) {
        field("refdes", it->refdes);
        // The bridge's stable identity is the file UUID.  Legacy/headless
        // fixtures may omit it; expose the deterministic fallback id so a
        // selection remains distinguishable until the format validator fixes
        // the source document.
        field("uuid", it->uuid.empty() ? item.id : it->uuid);
        if (!it->value.empty()) field("value", it->value);
      }
    } else if (item.kind == ScreenItemKind::wire) {
      const auto it = std::find_if(model_.wires.begin(), model_.wires.end(), [&](const auto& wire) {
        return (wire.uuid.empty() ? "wire@" + std::to_string(wire.points[0].first) + ":" + std::to_string(wire.points[0].second) + "#" + std::to_string(&wire - model_.wires.data()) : wire.uuid) == item.id;
      });
      if (it != model_.wires.end()) field("uuid", it->uuid.empty() ? item.id : it->uuid);
    } else if (item.kind == ScreenItemKind::label) {
      const auto it = std::find_if(model_.labels.begin(), model_.labels.end(), [&](const auto& label) {
        return (label.uuid.empty() ? "label@" + std::to_string(label.x_g) + ":" + std::to_string(label.y_g) + "#" + std::to_string(&label - model_.labels.data()) : label.uuid) == item.id;
      });
      if (it != model_.labels.end()) field("uuid", it->uuid.empty() ? item.id : it->uuid);
    } else if (item.kind == ScreenItemKind::no_connect) {
      const auto it = std::find_if(model_.no_connects.begin(), model_.no_connects.end(), [&](const auto& marker) {
        return (marker.uuid.empty() ? "no_connect@" + std::to_string(marker.x_g) + ":" + std::to_string(marker.y_g) + "#" + std::to_string(&marker - model_.no_connects.data()) : marker.uuid) == item.id;
      });
      if (it != model_.no_connects.end()) field("uuid", it->uuid.empty() ? item.id : it->uuid);
    }
    out << "}";
  }
  out << "]";
  return out.str();
}

}  // namespace cicada::editor
