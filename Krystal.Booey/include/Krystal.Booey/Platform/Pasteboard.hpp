#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  /// @brief The system clipboard, written through the host's pasteboard strategy.
  // FIXME: Reading (plain text, web content, files, URLs), writing web content, images, URLs and custom data,
  // drag pasteboards and the pasteboard context, with pasting, drag and drop and DataTransfer.
  class Pasteboard
  {
    KRYS_NON_COPY_MOVABLE(Pasteboard);

  public:
    enum SmartReplaceOption : bool
    {
      CanSmartReplace,
      CannotSmartReplace
    };

    Pasteboard() noexcept = default;

    KRYS_NODISCARD static UniquePtr<Pasteboard> CreateForCopyAndPaste() noexcept;

    void WritePlainText(utf8_stringview text, SmartReplaceOption smartReplaceOption) noexcept;
    void WriteString(utf8_stringview type, utf8_stringview text) noexcept;
  };
}
