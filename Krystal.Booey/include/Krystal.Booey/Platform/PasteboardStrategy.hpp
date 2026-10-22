#pragma once

#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief How the host reads and writes the system clipboard.
  // FIXME: Reading the pasteboard, its types, buffers, URLs, images and custom data, and clearing it, with
  // pasting and DataTransfer.
  class PasteboardStrategy
  {
  public:
    virtual ~PasteboardStrategy() noexcept = default;

    /// @brief Puts @p text on the clipboard as @p pasteboardType (a MIME type, such as
    /// `text/plain;charset=utf-8`), replacing what's there.
    virtual void WriteToPasteboard(utf8_stringview pasteboardType, utf8_stringview text) noexcept = 0;
  };
}
