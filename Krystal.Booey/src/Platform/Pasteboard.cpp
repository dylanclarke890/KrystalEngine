#include "Krystal.Booey/Platform/Pasteboard.hpp"
#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/Editing/Markup.hpp"
#include "Krystal.Booey/Platform/PasteboardStrategy.hpp"
#include "Krystal.Booey/Platform/PlatformStrategies.hpp"

namespace krys::boo
{
  UniquePtr<Pasteboard> Pasteboard::CreateForCopyAndPaste() noexcept
  {
    return CreateUnique<Pasteboard>();
  }

  void Pasteboard::Read(PasteboardPlainText &text) noexcept
  {
    text.Text = ReadString(u8"text/plain");
    text.IsURL = false;
  }

  utf8_string Pasteboard::ReadString(utf8_stringview type) noexcept
  {
    if (auto *strategy = PlatformStrategies::Singleton().GetPasteboardStrategy())
    {
      return strategy->ReadStringFromPasteboard(type);
    }
    return {};
  }

  // FIXME: Reading web content (the HTML format), files and images, with rich pasting.
  RefPtr<dom::DocumentFragment> Pasteboard::ReadDocumentFragment(const dom::SimpleRange &context,
                                                                 bool allowPlainText,
                                                                 bool &chosePlainText) noexcept
  {
    chosePlainText = false;
    if (!allowPlainText)
    {
      return nullptr;
    }
    PasteboardPlainText text;
    Read(text);
    if (text.Text.empty())
    {
      return nullptr;
    }
    chosePlainText = true;
    return CreateFragmentFromText(context, text.Text);
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
