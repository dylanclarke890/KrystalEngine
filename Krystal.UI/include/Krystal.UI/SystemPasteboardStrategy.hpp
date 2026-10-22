#pragma once

#include "Krystal.Booey/Platform/PasteboardStrategy.hpp"

namespace krys::ui
{
  /// @brief Writes Booey pages' copies to the system clipboard, owned by the active window.
  /// @note Only plain text reaches the clipboard; other types are dropped.
  class SystemPasteboardStrategy final : public boo::PasteboardStrategy
  {
  public:
    void WriteToPasteboard(utf8_stringview pasteboardType, utf8_stringview text) noexcept override;
  };
}
