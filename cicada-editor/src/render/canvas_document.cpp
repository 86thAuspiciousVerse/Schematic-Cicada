#include "canvas_document.h"
#include "../core/sexpr_writer.h"
#include <exception>

namespace cicada::editor {
bool CanvasDocument::reload(std::string_view schematic_text, std::string* error) {
  return screen_.load(schematic_text, error);
}

bool CanvasDocument::reload_file(ITextFileReader& reader, std::string_view path, std::string* error) {
  try {
    return reload(reader.read(path), error);
  } catch (const std::exception& ex) {
    if (error != nullptr) *error = ex.what();
    return false;
  }
}

bool CanvasDocument::save_file(ITextFileWriter& writer, std::string_view path, std::string* error) const {
  std::string text;
  if (!screen_.save(&text, error)) return false;
  try {
    writer.write(path, text);
    if (error) error->clear();
    return true;
  } catch (const std::exception& ex) {
    if (error) *error = ex.what();
    return false;
  }
}

bool CanvasDocument::save_model(const SchematicModel& model, ITextFileWriter& writer, std::string_view path, std::string* error) const {
  try {
    const std::string text = SexprModelWriter().write(model);
    writer.write(path, text);
    if (error) error->clear();
    return true;
  } catch (const std::exception& ex) {
    if (error) *error = ex.what();
    return false;
  }
}
}
