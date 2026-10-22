#include "Krystal.UI/SystemPasteboardStrategy.hpp"
#include "Krystal.PAL/Platform.hpp"

namespace krys::ui
{
  void SystemPasteboardStrategy::WriteToPasteboard(utf8_stringview pasteboardType,
                                                   utf8_stringview text) noexcept
  {
    if (!pasteboardType.starts_with(u8"text/plain"))
    {
      return;
    }
    (void)pal::SetClipboardText(text);
  }

  utf8_string SystemPasteboardStrategy::ReadStringFromPasteboard(utf8_stringview pasteboardType) noexcept
  {
    if (!pasteboardType.starts_with(u8"text/plain"))
    {
      return {};
    }
    return pal::GetClipboardText().value_or(utf8_string());
  }
}
