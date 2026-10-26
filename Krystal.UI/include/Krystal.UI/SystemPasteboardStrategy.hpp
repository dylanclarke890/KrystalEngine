#pragma once

#include "Krystal.Booey/Platform/PasteboardStrategy.hpp"

namespace krys::ui
{
  /// @brief Writes Booey pages' copies to the system clipboard, owned by the active window, and reads it for
  /// pasting.
  /// @note The clipboard carries text, HTML (Windows' HTML format), pages' custom data (in a private format)
  /// and files (read only, as their paths); a URI list is written as text when there's no text.
  class SystemPasteboardStrategy final : public boo::PasteboardStrategy
  {
  public:
    KRYS_NODISCARD List<utf8_string> Types(utf8_stringview pasteboardName) noexcept override;
    KRYS_NODISCARD utf8_string ReadTextFromClipboard(utf8_stringview pasteboardName,
                                                     utf8_stringview pasteboardType) noexcept override;
    KRYS_NODISCARD List<utf8_string>
      ReadFilePathsFromClipboard(utf8_stringview pasteboardName) noexcept override;
    KRYS_NODISCARD RefPtr<boo::SharedBuffer>
      ReadBufferFromClipboard(utf8_stringview pasteboardName,
                              utf8_stringview pasteboardType) noexcept override;
    void WriteToClipboard(utf8_stringview pasteboardName,
                          boo::SelectionData &&selectionData) noexcept override;
    void ClearClipboard(utf8_stringview pasteboardName) noexcept override;
    KRYS_NODISCARD int64 ChangeCount(utf8_stringview pasteboardName) noexcept override;
    KRYS_NODISCARD List<utf8_string> TypesSafeForDOMToReadAndWrite(utf8_stringview pasteboardName,
                                                                   utf8_stringview origin) noexcept override;
    int64 WriteCustomData(const List<boo::PasteboardCustomData> &data,
                          utf8_stringview pasteboardName) noexcept override;
  };
}
