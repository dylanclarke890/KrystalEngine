#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief How the host reads and writes the system clipboard.
  // FIXME: The pasteboard's types, buffers, URLs, images and custom data, and clearing it, with DataTransfer.
  class PasteboardStrategy
  {
  public:
    virtual ~PasteboardStrategy() noexcept = default;

    /// @brief Puts @p text on the clipboard as @p pasteboardType (a MIME type, such as
    /// `text/plain;charset=utf-8`), replacing what's there.
    virtual void WriteToPasteboard(utf8_stringview pasteboardType, utf8_stringview text) noexcept = 0;

    /// @brief Reads the clipboard's @p pasteboardType (a MIME type, such as `text/plain`) as a string.
    /// @return The string, or empty if the clipboard doesn't hold that type.
    KRYS_NODISCARD virtual utf8_string ReadStringFromPasteboard(utf8_stringview pasteboardType) noexcept = 0;
  };
}
