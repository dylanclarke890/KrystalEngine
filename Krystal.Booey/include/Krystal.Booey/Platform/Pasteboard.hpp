#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::dom
{
  class DocumentFragment;
  struct SimpleRange;
}

namespace krys::boo
{
  /// @brief Plain text read from a pasteboard.
  struct PasteboardPlainText
  {
    utf8_string Text;
    bool IsURL {false};
  };

  /// @brief The system clipboard, read and written through the host's pasteboard strategy.
  // FIXME: Reading web content, files and URLs, writing web content, images, URLs and custom data, drag
  // pasteboards and the pasteboard context, with rich pasting, drag and drop and DataTransfer.
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

    /// @brief Reads the pasteboard's plain text into @p text.
    void Read(PasteboardPlainText &text) noexcept;
    KRYS_NODISCARD utf8_string ReadString(utf8_stringview type) noexcept;
    /// @brief Reads the pasteboard as a fragment to paste into @p context.
    /// @param allowPlainText Whether plain text may be read when there's no web content.
    /// @param chosePlainText Set to whether the fragment was made from plain text.
    /// @return The fragment, or null if there's nothing to paste.
    KRYS_NODISCARD RefPtr<dom::DocumentFragment> ReadDocumentFragment(const dom::SimpleRange &context,
                                                                      bool allowPlainText,
                                                                      bool &chosePlainText) noexcept;
    /// @brief Whether the pasteboard's contents were copied as whole words, so pasting them adds spaces
    /// around them.
    // FIXME: The smart paste type (WebSmartPastePboardType), with writing it.
    KRYS_NODISCARD bool CanSmartReplaceContents() noexcept
    {
      return false;
    }

    void WritePlainText(utf8_stringview text, SmartReplaceOption smartReplaceOption) noexcept;
    void WriteString(utf8_stringview type, utf8_stringview text) noexcept;
  };
}
