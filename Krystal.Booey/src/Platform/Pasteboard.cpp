#include "Krystal.Booey/Platform/Pasteboard.hpp"
#include "Krystal.Booey/Platform/PasteboardStrategy.hpp"
#include "Krystal.Booey/Platform/PlatformStrategies.hpp"

namespace krys::boo
{
  UniquePtr<Pasteboard> Pasteboard::CreateForCopyAndPaste() noexcept
  {
    return CreateUnique<Pasteboard>();
  }

  void Pasteboard::WritePlainText(utf8_stringview text, SmartReplaceOption) noexcept
  {
    WriteString(u8"text/plain;charset=utf-8", text);
  }

  void Pasteboard::WriteString(utf8_stringview type, utf8_stringview text) noexcept
  {
    if (auto *strategy = PlatformStrategies::Singleton().GetPasteboardStrategy())
    {
      strategy->WriteToPasteboard(type, text);
    }
  }
}
