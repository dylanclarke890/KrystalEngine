#pragma once

#include "Krystal.Booey/Platform/PasteboardStrategy.hpp"

namespace krys::ui
{
  /// @brief Writes Booey pages' copies to the system clipboard, owned by the active window, and reads it for
  /// pasting.
  /// @note Only plain text reaches and leaves the clipboard; other types are dropped or read as empty.
  class SystemPasteboardStrategy final : public boo::PasteboardStrategy
  {
  public:
    void WriteToPasteboard(utf8_stringview pasteboardType, utf8_stringview text) noexcept override;
    KRYS_NODISCARD utf8_string ReadStringFromPasteboard(utf8_stringview pasteboardType) noexcept override;
  };
}
