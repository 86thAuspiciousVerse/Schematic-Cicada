#pragma once

#include "../core/cicada_screen_adapter.h"
#include <string>

namespace cicada::editor {
class ITextFileReader {
 public:
  virtual ~ITextFileReader() = default;
  virtual std::string read(std::string_view path) = 0;
};
class ITextFileWriter {
 public:
  virtual ~ITextFileWriter() = default;
  virtual void write(std::string_view path, std::string_view text) = 0;
};

class CanvasDocument {
 public:
  bool reload(std::string_view schematic_text, std::string* error = nullptr);
  bool reload_file(ITextFileReader& reader, std::string_view path, std::string* error = nullptr);
  bool save_file(ITextFileWriter& writer, std::string_view path, std::string* error = nullptr) const;
  bool save_model(const SchematicModel& model, ITextFileWriter& writer, std::string_view path, std::string* error = nullptr) const;
  const SchematicModel& model() const { return screen_.model(); }
  bool loaded() const { return screen_.loaded(); }
 private:
  CicadaScreenAdapter screen_;
};
}
