#pragma once

#include "Krystal.Booey/Platform/SharedBuffer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  class PasteboardCustomData;
  class SelectionData;

  /// @brief How the host reads and writes the system clipboard.
  /// @note Types are MIME types (`text/plain`, `text/html`, `text/uri-list` when the clipboard holds files)
  /// or the private type of pages' custom data (@ref PasteboardCustomData::BooeyType).
  // FIXME: Reading items by index (readStringFromPasteboard, readBufferFromPasteboard, readURLFromPasteboard,
  // informationForItemAtIndex, allPasteboardItemInfo, getPasteboardItemsCount) and
  // containsStringSafeForDOMToReadForType, with the async clipboard API.
  class PasteboardStrategy
  {
  public:
    virtual ~PasteboardStrategy() noexcept = default;

    /// @brief The types the clipboard @p pasteboardName holds.
    KRYS_NODISCARD virtual List<utf8_string> Types(utf8_stringview pasteboardName) noexcept = 0;
    /// @brief The clipboard's @p pasteboardType as a string, or empty if it doesn't hold that type.
    KRYS_NODISCARD virtual utf8_string ReadTextFromClipboard(utf8_stringview pasteboardName,
                                                             utf8_stringview pasteboardType) noexcept = 0;
    /// @brief The paths of the files on the clipboard.
    KRYS_NODISCARD virtual List<utf8_string>
      ReadFilePathsFromClipboard(utf8_stringview pasteboardName) noexcept = 0;
    /// @brief The clipboard's @p pasteboardType as bytes, or null if it doesn't hold that type.
    KRYS_NODISCARD virtual RefPtr<SharedBuffer>
      ReadBufferFromClipboard(utf8_stringview pasteboardName, utf8_stringview pasteboardType) noexcept = 0;
    /// @brief Replaces what's on the clipboard with @p selectionData's text, markup, URI list, buffers and
    /// custom data.
    virtual void WriteToClipboard(utf8_stringview pasteboardName, SelectionData &&selectionData) noexcept = 0;
    virtual void ClearClipboard(utf8_stringview pasteboardName) noexcept = 0;
    /// @brief A count that changes whenever the clipboard's contents do.
    KRYS_NODISCARD virtual int64 ChangeCount(utf8_stringview pasteboardName) noexcept = 0;

    /// @brief The types a page of @p origin may read: the custom data's types if that page wrote it, and the
    /// types safe for any page.
    KRYS_NODISCARD virtual List<utf8_string>
      TypesSafeForDOMToReadAndWrite(utf8_stringview pasteboardName, utf8_stringview origin) noexcept = 0;
    /// @brief Replaces what's on the clipboard with @p data's platform strings and buffers, and its
    /// serialised custom data.
    /// @return The clipboard's change count afterwards.
    virtual int64 WriteCustomData(const List<PasteboardCustomData> &data,
                                  utf8_stringview pasteboardName) noexcept = 0;
  };
}
