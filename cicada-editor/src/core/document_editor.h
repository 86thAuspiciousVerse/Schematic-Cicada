#pragma once

#include "sexpr_model.h"
#include "sexpr_writer.h"
#include <cstddef>
#include <string>
#include <vector>

namespace cicada::editor {

enum class EditCommandKind { MoveSymbol, AddWire, AddLabel, AddJunction, AddNoConnect, SetProperty, DeleteSymbol, DeleteWire, AddSymbol, AddPowerSymbol, Disconnect, RemoveComponent };

struct EditCommand {
  EditCommandKind kind;
  std::string refdes;
  std::string property;
  std::string value;
  std::string uuid;
  int x_g = 0;
  int y_g = 0;
  int endpoint = 0;
  int x2_g = 0;
  int y2_g = 0;
  // Library id used by AddSymbol/AddPowerSymbol.  Placement is only allowed
  // for definitions already present in the document's embedded lib_symbols.
  std::string lib_id;
  std::string pin_number;
};

/** Transactional whitelist editor for the lightweight Cicada model. */
class DocumentEditor {
 public:
  explicit DocumentEditor(SchematicModel model);
  const SchematicModel& model() const { return model_; }
  bool apply(const EditCommand& command, std::string* error = nullptr);
  bool undo(std::string* error = nullptr);
  bool redo(std::string* error = nullptr);
  std::string serialize(std::string* error = nullptr) const;
  std::size_t undo_depth() const { return undo_.size(); }
  std::size_t redo_depth() const { return redo_.size(); }

 private:
  struct Snapshot { SchematicModel model; };
  bool restore(SchematicModel model, std::string* error);
  SchematicModel model_;
  std::vector<Snapshot> undo_;
  std::vector<Snapshot> redo_;
};
}
