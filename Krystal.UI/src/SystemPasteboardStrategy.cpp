#include "Krystal.UI/SystemPasteboardStrategy.hpp"
#include "Krystal.Booey/Platform/Pasteboard.hpp"
#include "Krystal.Booey/Platform/PasteboardCustomData.hpp"
#include "Krystal.Booey/Platform/SelectionData.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Types/ListHashSet.hpp"
#include "Krystal.PAL/Platform.hpp"

namespace krys::ui
{
  List<utf8_string> SystemPasteboardStrategy::Types(utf8_stringview) noexcept
  {
    List<utf8_string> types;
    if (pal::IsClipboardFormatAvailable(pal::ClipboardFormat::Text))
    {
      types.emplace_back(u8"text/plain");
    }
    if (pal::IsClipboardFormatAvailable(pal::ClipboardFormat::HTML))
    {
      types.emplace_back(u8"text/html");
    }
    if (pal::IsClipboardFormatAvailable(pal::ClipboardFormat::Files))
    {
      types.emplace_back(u8"text/uri-list");
    }
    if (pal::IsClipboardDataAvailable(boo::PasteboardCustomData::BooeyType()))
    {
      types.emplace_back(boo::PasteboardCustomData::BooeyType());
    }
    return types;
  }

  utf8_string SystemPasteboardStrategy::ReadTextFromClipboard(utf8_stringview,
                                                              utf8_stringview pasteboardType) noexcept
  {
    if (pasteboardType.starts_with(u8"text/plain"))
    {
      return pal::GetClipboardText().value_or(utf8_string());
    }
    if (pasteboardType == u8"text/html")
    {
      return pal::GetClipboardHTML().value_or(utf8_string());
    }
    if (pasteboardType == u8"text/uri-list")
    {
      utf8_string uriList;
      for (auto &path : pal::GetClipboardFiles())
      {
        if (!uriList.empty())
        {
          uriList += u8"\r\n";
        }
        uriList += boo::url::URL::FileURLWithFileSystemPath(path).String();
      }
      return uriList;
    }
    return {};
  }

  List<utf8_string> SystemPasteboardStrategy::ReadFilePathsFromClipboard(utf8_stringview) noexcept
  {
    return pal::GetClipboardFiles();
  }

  RefPtr<boo::SharedBuffer>
    SystemPasteboardStrategy::ReadBufferFromClipboard(utf8_stringview,
                                                      utf8_stringview pasteboardType) noexcept
  {
    if (pasteboardType.starts_with(u8"text/"))
    {
      return nullptr;
    }
    auto bytes = pal::GetClipboardData(pasteboardType);
    if (!bytes)
    {
      return nullptr;
    }
    return boo::SharedBuffer::Create(krys::move(*bytes));
  }

  void SystemPasteboardStrategy::WriteToClipboard(utf8_stringview,
                                                  boo::SelectionData &&selectionData) noexcept
  {
    pal::ClipboardContent content;
    if (selectionData.HasText())
    {
      content.Text = selectionData.Text();
    }
    else if (selectionData.HasURIList())
    {
      content.Text = selectionData.URIList();
    }
    if (selectionData.HasMarkup())
    {
      content.HTML = selectionData.Markup();
    }
    for (auto &[type, buffer] : selectionData.Buffers())
    {
      auto span = buffer->GetSpan();
      content.Data.emplace_back(type, List<uint8>(span.begin(), span.end()));
    }
    if (auto &customData = selectionData.CustomData())
    {
      auto span = customData->GetSpan();
      content.Data.emplace_back(utf8_string(boo::PasteboardCustomData::BooeyType()),
                                List<uint8>(span.begin(), span.end()));
    }
    (void)pal::SetClipboardContent(content);
  }

  void SystemPasteboardStrategy::ClearClipboard(utf8_stringview) noexcept
  {
    (void)pal::ClearClipboard();
  }

  int64 SystemPasteboardStrategy::ChangeCount(utf8_stringview) noexcept
  {
    return pal::GetClipboardChangeCount();
  }

  List<utf8_string> SystemPasteboardStrategy::TypesSafeForDOMToReadAndWrite(utf8_stringview pasteboardName,
                                                                            utf8_stringview origin) noexcept
  {
    ListHashSet<utf8_string> domTypes;
    if (auto buffer = ReadBufferFromClipboard(pasteboardName, boo::PasteboardCustomData::BooeyType()))
    {
      auto customData = boo::PasteboardCustomData::FromSharedBuffer(*buffer);
      if (customData.Origin() == origin)
      {
        for (auto &type : customData.OrderedTypes())
        {
          domTypes.add(type);
        }
      }
    }

    for (auto &format : Types(pasteboardName))
    {
      if (format == boo::PasteboardCustomData::BooeyType())
      {
        continue;
      }

      if (boo::Pasteboard::IsSafeTypeForDOMToReadAndWrite(format))
      {
        domTypes.add(format);
      }
    }

    return List<utf8_string>(domTypes.begin(), domTypes.end());
  }

  int64 SystemPasteboardStrategy::WriteCustomData(const List<boo::PasteboardCustomData> &data,
                                                  utf8_stringview pasteboardName) noexcept
  {
    if (data.empty() || data.size() > 1)
    {
      // We don't support more than one custom item in the clipboard.
      return ChangeCount(pasteboardName);
    }

    boo::SelectionData selectionData;
    const auto &customData = data[0];
    customData.ForEachPlatformStringOrBuffer(
      [&selectionData](auto &type, auto &stringOrBuffer)
      {
        if (auto *buffer = std::get_if<Ref<boo::SharedBuffer>>(&stringOrBuffer))
        {
          selectionData.AddBuffer(type, *buffer);
        }
        else if (auto *string = std::get_if<Maybe<utf8_string>>(&stringOrBuffer); string && *string)
        {
          if (type == u8"text/plain")
          {
            selectionData.SetText(**string);
          }
          else if (type == u8"text/html")
          {
            selectionData.SetMarkup(**string);
          }
          else if (type == u8"text/uri-list")
          {
            selectionData.SetURIList(**string);
          }
        }
      });

    if (customData.HasSameOriginCustomData() || !customData.Origin().empty())
    {
      selectionData.SetCustomData(customData.CreateSharedBuffer());
    }

    WriteToClipboard(pasteboardName, krys::move(selectionData));
    return ChangeCount(pasteboardName);
  }
}
