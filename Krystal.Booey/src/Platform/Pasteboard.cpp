#include "Krystal.Booey/Platform/Pasteboard.hpp"
#include "Krystal.Booey/Platform/DragData.hpp"
#include "Krystal.Booey/Platform/MIMETypeRegistry.hpp"
#include "Krystal.Booey/Platform/PasteboardStrategy.hpp"
#include "Krystal.Booey/Platform/PlatformStrategies.hpp"
#include "Krystal.Core/Types/ListHashSet.hpp"
#include <algorithm>
#include <array>

namespace krys::boo
{
  namespace
  {
    enum ClipboardDataType
    {
      ClipboardDataTypeText,
      ClipboardDataTypeMarkup,
      ClipboardDataTypeURIList,
      ClipboardDataTypeURL,
      ClipboardDataTypeImage,
      ClipboardDataTypeUnknown
    };

    KRYS_NODISCARD PasteboardStrategy *Strategy() noexcept
    {
      return PlatformStrategies::Singleton().GetPasteboardStrategy();
    }

    KRYS_NODISCARD bool Contains(const List<utf8_string> &types, utf8_stringview type) noexcept
    {
      return std::ranges::find(types, type) != types.end();
    }

    KRYS_NODISCARD ClipboardDataType SelectionDataTypeFromHTMLClipboardType(utf8_stringview type) noexcept
    {
      // From the Mac port: Ignore any trailing charset - JS strings are
      // Unicode, which encapsulates the charset issue.
      if (type == u8"text/plain")
      {
        return ClipboardDataTypeText;
      }
      if (type == u8"text/html")
      {
        return ClipboardDataTypeMarkup;
      }
      if (type == u8"Files" || type == u8"text/uri-list")
      {
        return ClipboardDataTypeURIList;
      }

      // Not a known type, so just default to using the text portion.
      return ClipboardDataTypeUnknown;
    }

    constexpr std::array<utf8_stringview, 6> ImageTypes {
      u8"image/png",   u8"image/jpeg", u8"image/gif", u8"image/bmp", u8"image/vnd.microsoft.icon",
      u8"image/x-icon"};

    KRYS_NODISCARD utf8_stringview ImageTypeToFakeFilename(utf8_stringview type) noexcept
    {
      if (type == u8"image/png")
      {
        return u8"image.png";
      }
      if (type == u8"image/jpeg")
      {
        return u8"image.jpeg";
      }
      if (type == u8"image/gif")
      {
        return u8"image.gif";
      }
      if (type == u8"image/bmp")
      {
        return u8"image.bmp";
      }
      if (type == u8"image/vnd.microsoft.icon" || type == u8"image/x-icon")
      {
        return u8"image.ico";
      }

      krys_debug_assert(false);
      return {};
    }
  }

  UniquePtr<Pasteboard> Pasteboard::CreateForCopyAndPaste() noexcept
  {
    return CreateUnique<Pasteboard>(u8"CLIPBOARD");
  }

  UniquePtr<Pasteboard> Pasteboard::CreateForDragAndDrop() noexcept
  {
    return CreateUnique<Pasteboard>(SelectionData());
  }

  UniquePtr<Pasteboard> Pasteboard::Create(const DragData &dragData) noexcept
  {
    krys_debug_assert(dragData.PlatformData());
    return CreateUnique<Pasteboard>(*dragData.PlatformData());
  }

  Pasteboard::Pasteboard(utf8_stringview name) noexcept : _name(name)
  {
    if (auto *strategy = Strategy())
    {
      _changeCount = strategy->ChangeCount(_name);
    }
  }

  Pasteboard::Pasteboard(SelectionData &&selectionData) noexcept : _selectionData(krys::move(selectionData))
  {
  }

  Pasteboard::Pasteboard(const SelectionData &selectionData) noexcept : _selectionData(selectionData)
  {
  }

  bool Pasteboard::IsSafeTypeForDOMToReadAndWrite(utf8_stringview type) noexcept
  {
    return type == u8"text/plain" || type == u8"text/html" || type == u8"text/uri-list";
  }

  bool Pasteboard::CanExposeURLToDOMWhenPasteboardContainsFiles(utf8_stringview urlString) noexcept
  {
    url::URL url({}, urlString);
    return url.ProtocolIsInHTTPFamily() || url.ProtocolIsBlob() || url.ProtocolIsData();
  }

  List<utf8_string> Pasteboard::ReadAllStrings(utf8_stringview type) noexcept
  {
    auto result = ReadString(type);
    if (result.empty())
    {
      return {};
    }

    return {krys::move(result)};
  }

  void Pasteboard::WriteString(utf8_stringview type, utf8_stringview data) noexcept
  {
    krys_debug_assert(_selectionData);
    switch (SelectionDataTypeFromHTMLClipboardType(type))
    {
      case ClipboardDataTypeURIList:
      case ClipboardDataTypeURL:     _selectionData->SetURIList(data); break;
      case ClipboardDataTypeMarkup:  _selectionData->SetMarkup(data); break;
      case ClipboardDataTypeText:    _selectionData->SetText(data); break;
      case ClipboardDataTypeUnknown:
      case ClipboardDataTypeImage:   break;
    }
  }

  void Pasteboard::WritePlainText(utf8_stringview text, SmartReplaceOption smartReplaceOption) noexcept
  {
    if (_selectionData)
    {
      _selectionData->ClearAll();
      _selectionData->SetText(text);
      _selectionData->SetCanSmartReplace(smartReplaceOption == CanSmartReplace);
    }
    else
    {
      SelectionData data;
      data.SetText(text);
      data.SetCanSmartReplace(smartReplaceOption == CanSmartReplace);
      if (auto *strategy = Strategy())
      {
        strategy->WriteToClipboard(_name, krys::move(data));
      }
    }
  }

  void Pasteboard::Write(const PasteboardURL &pasteboardURL) noexcept
  {
    krys_debug_assert(!pasteboardURL.Url.IsEmpty());
    if (_selectionData)
    {
      _selectionData->ClearAll();
      _selectionData->SetURL(pasteboardURL.Url, pasteboardURL.Title);
    }
    else
    {
      SelectionData data;
      data.SetURL(pasteboardURL.Url, pasteboardURL.Title);
      if (auto *strategy = Strategy())
      {
        strategy->WriteToClipboard(_name, krys::move(data));
      }
    }
  }

  void Pasteboard::Write(const PasteboardImage &pasteboardImage) noexcept
  {
    if (_selectionData)
    {
      _selectionData->ClearAll();
      if (!pasteboardImage.Url.Url.IsEmpty())
      {
        _selectionData->SetURL(pasteboardImage.Url.Url, pasteboardImage.Url.Title);
        _selectionData->SetMarkup(pasteboardImage.Url.Markup);
      }
    }
    else
    {
      SelectionData data;
      if (!pasteboardImage.Url.Url.IsEmpty())
      {
        data.SetURL(pasteboardImage.Url.Url, pasteboardImage.Url.Title);
        data.SetMarkup(pasteboardImage.Url.Markup);
      }
      if (auto *strategy = Strategy())
      {
        strategy->WriteToClipboard(_name, krys::move(data));
      }
    }
  }

  void Pasteboard::Write(const PasteboardWebContent &pasteboardContent) noexcept
  {
    PasteboardCustomData customData;
    customData.SetOrigin(pasteboardContent.ContentOrigin);
    if (_selectionData)
    {
      _selectionData->ClearAll();
      _selectionData->SetText(pasteboardContent.Text);
      _selectionData->SetMarkup(pasteboardContent.Markup);
      _selectionData->SetCanSmartReplace(pasteboardContent.CanSmartCopyOrDelete);
      _selectionData->SetCustomData(customData.CreateSharedBuffer());
    }
    else
    {
      SelectionData data;
      data.SetText(pasteboardContent.Text);
      data.SetMarkup(pasteboardContent.Markup);
      data.SetCanSmartReplace(pasteboardContent.CanSmartCopyOrDelete);
      data.SetCustomData(customData.CreateSharedBuffer());
      if (auto *strategy = Strategy())
      {
        strategy->WriteToClipboard(_name, krys::move(data));
      }
    }
  }

  void Pasteboard::Clear() noexcept
  {
    if (!_selectionData)
    {
      if (auto *strategy = Strategy())
      {
        strategy->ClearClipboard(_name);
      }
      return;
    }

    // We do not clear filenames. According to the spec: "The clearData() method
    // does not affect whether any files were included in the drag, so the types
    // attribute's list might still not be empty after calling clearData() (it would
    // still contain the "Files" string if any files were included in the drag)."
    _selectionData->ClearAllExceptFilenames();
  }

  void Pasteboard::Clear(utf8_stringview type) noexcept
  {
    krys_debug_assert(_selectionData);
    switch (SelectionDataTypeFromHTMLClipboardType(type))
    {
      case ClipboardDataTypeURIList:
      case ClipboardDataTypeURL:     _selectionData->ClearURIList(); break;
      case ClipboardDataTypeMarkup:  _selectionData->ClearMarkup(); break;
      case ClipboardDataTypeText:    _selectionData->ClearText(); break;
      case ClipboardDataTypeImage:   break;
      case ClipboardDataTypeUnknown: _selectionData->ClearAll(); break;
    }
  }

  void Pasteboard::Read(PasteboardPlainText &text, PlainTextURLReadingPolicy) noexcept
  {
    if (auto *strategy = Strategy())
    {
      text.Text = strategy->ReadTextFromClipboard(_name, u8"text/plain");
    }
  }

  // FIXME: Images on the system clipboard, with readImage.
  void Pasteboard::Read(PasteboardWebContentReader &reader, WebContentReadingPolicy policy) noexcept
  {
    reader.SetContentOrigin(ReadOrigin());

    if (_selectionData)
    {
      if (_selectionData->HasMarkup() && reader.ReadHTML(_selectionData->Markup()))
      {
        return;
      }

      if (policy == WebContentReadingPolicy::OnlyRichTextTypes)
      {
        return;
      }

      if (_selectionData->HasFilenames() && reader.ReadFilePaths(_selectionData->Filenames()))
      {
        return;
      }

      if (_selectionData->HasText() && reader.ReadPlainText(_selectionData->Text()))
      {
        return;
      }

      return;
    }

    auto *strategy = Strategy();
    if (!strategy)
    {
      return;
    }

    auto types = strategy->Types(_name);
    if (Contains(types, u8"text/html"))
    {
      auto text = strategy->ReadTextFromClipboard(_name, u8"text/html");
      if (!text.empty() && reader.ReadHTML(text))
      {
        return;
      }
    }

    if (policy == WebContentReadingPolicy::OnlyRichTextTypes)
    {
      return;
    }

    if (Contains(types, u8"text/uri-list"))
    {
      auto filePaths = strategy->ReadFilePathsFromClipboard(_name);
      if (reader.ReadFilePaths(filePaths))
      {
        return;
      }
    }

    if (Contains(types, u8"text/plain"))
    {
      auto text = strategy->ReadTextFromClipboard(_name, u8"text/plain");
      if (!text.empty() && reader.ReadPlainText(text))
      {
        return;
      }
    }

    if (Contains(types, u8"text/plain;charset=utf-8"))
    {
      auto text = strategy->ReadTextFromClipboard(_name, u8"text/plain;charset=utf-8");
      if (!text.empty() && reader.ReadPlainText(text))
      {
        return;
      }
    }
  }

  void Pasteboard::Read(PasteboardFileReader &reader) noexcept
  {
    if (_selectionData)
    {
      for (const auto &filePath : _selectionData->Filenames())
      {
        reader.ReadFilename(filePath);
      }
      return;
    }

    auto *strategy = Strategy();
    if (!strategy)
    {
      return;
    }

    auto filePaths = strategy->ReadFilePathsFromClipboard(_name);
    if (!filePaths.empty())
    {
      for (const auto &filePath : filePaths)
      {
        reader.ReadFilename(filePath);
      }
      return;
    }

    for (auto imageType : ImageTypes)
    {
      if (!reader.ShouldReadBuffer(imageType))
      {
        continue;
      }
      if (auto buffer = ReadBuffer(imageType))
      {
        reader.ReadBuffer(ImageTypeToFakeFilename(imageType), imageType, buffer);
        return;
      }
    }
  }

  bool Pasteboard::HasData() noexcept
  {
    if (_selectionData)
    {
      return _selectionData->HasText() || _selectionData->HasMarkup() || _selectionData->HasURIList()
             || _selectionData->HasCustomData();
    }
    auto *strategy = Strategy();
    return strategy && !strategy->Types(_name).empty();
  }

  List<utf8_string> Pasteboard::TypesSafeForBindings(utf8_stringview origin) noexcept
  {
    if (_selectionData)
    {
      ListHashSet<utf8_string> types;
      if (auto &buffer = _selectionData->CustomData())
      {
        auto customData = PasteboardCustomData::FromSharedBuffer(*buffer);
        if (customData.Origin() == origin)
        {
          for (auto &type : customData.OrderedTypes())
          {
            types.add(type);
          }
        }
      }

      if (_selectionData->HasText())
      {
        types.add(u8"text/plain");
      }
      if (_selectionData->HasMarkup())
      {
        types.add(u8"text/html");
      }
      if (_selectionData->HasURIList())
      {
        types.add(u8"text/uri-list");
      }

      return List<utf8_string>(types.begin(), types.end());
    }

    if (auto *strategy = Strategy())
    {
      return strategy->TypesSafeForDOMToReadAndWrite(_name, origin);
    }
    return {};
  }

  utf8_string Pasteboard::ReadOrigin() noexcept
  {
    if (_selectionData)
    {
      if (auto &buffer = _selectionData->CustomData())
      {
        return PasteboardCustomData::FromSharedBuffer(*buffer).Origin();
      }

      return {};
    }

    // FIXME: cache custom data?
    if (auto buffer = ReadBuffer(PasteboardCustomData::BooeyType()))
    {
      return PasteboardCustomData::FromSharedBuffer(*buffer).Origin();
    }

    return {};
  }

  utf8_string Pasteboard::ReadString(utf8_stringview type) noexcept
  {
    if (!_selectionData)
    {
      auto *strategy = Strategy();
      return strategy ? strategy->ReadTextFromClipboard(_name, type) : utf8_string();
    }

    switch (SelectionDataTypeFromHTMLClipboardType(type))
    {
      case ClipboardDataTypeURIList: return _selectionData->URIList();
      case ClipboardDataTypeURL:     return _selectionData->GetURL().String();
      case ClipboardDataTypeMarkup:  return _selectionData->Markup();
      case ClipboardDataTypeText:    return _selectionData->Text();
      case ClipboardDataTypeUnknown:
      case ClipboardDataTypeImage:   break;
    }

    return {};
  }

  Maybe<utf8_string> Pasteboard::ReadStringInCustomData(utf8_stringview type) noexcept
  {
    if (_selectionData)
    {
      if (auto &buffer = _selectionData->CustomData())
      {
        return PasteboardCustomData::FromSharedBuffer(*buffer).ReadStringInCustomData(type);
      }

      return null;
    }

    // FIXME: cache custom data?
    if (auto buffer = ReadBuffer(PasteboardCustomData::BooeyType()))
    {
      return PasteboardCustomData::FromSharedBuffer(*buffer).ReadStringInCustomData(type);
    }

    return null;
  }

  Pasteboard::FileContentState Pasteboard::GetFileContentState() noexcept
  {
    if (_selectionData)
    {
      return _selectionData->Filenames().empty() ? FileContentState::NoFileOrImageData
                                                 : FileContentState::MayContainFilePaths;
    }

    auto *strategy = Strategy();
    if (!strategy)
    {
      return FileContentState::NoFileOrImageData;
    }

    auto types = strategy->Types(_name);
    if (Contains(types, u8"text/uri-list"))
    {
      auto filePaths = strategy->ReadFilePathsFromClipboard(_name);
      if (!filePaths.empty())
      {
        return FileContentState::MayContainFilePaths;
      }
    }

    bool hasImage = std::ranges::any_of(types, [](const utf8_string &type)
                                        { return MIMETypeRegistry::IsSupportedImageMIMEType(type); });
    return hasImage ? FileContentState::MayContainFilePaths : FileContentState::NoFileOrImageData;
  }

  // FIXME: The smart paste type on the system clipboard, with writing it.
  bool Pasteboard::CanSmartReplaceContents() noexcept
  {
    if (_selectionData)
    {
      return _selectionData->CanSmartReplace();
    }
    return false;
  }

  void Pasteboard::WriteMarkup(utf8_stringview) noexcept
  {
  }

  void Pasteboard::WriteCustomData(const List<PasteboardCustomData> &data) noexcept
  {
    if (_selectionData)
    {
      if (!data.empty())
      {
        const auto &customData = data[0];
        customData.ForEachPlatformString([this](auto &type, auto &string) { WriteString(type, string); });
        if (customData.HasSameOriginCustomData() || !customData.Origin().empty())
        {
          _selectionData->SetCustomData(customData.CreateSharedBuffer());
        }
      }
      return;
    }

    if (auto *strategy = Strategy())
    {
      _changeCount = strategy->WriteCustomData(data, _name);
    }
  }

  int64 Pasteboard::ChangeCount() const noexcept
  {
    if (_selectionData)
    {
      return 0;
    }
    auto *strategy = Strategy();
    return strategy ? strategy->ChangeCount(_name) : 0;
  }

  RefPtr<SharedBuffer> Pasteboard::ReadBuffer(utf8_stringview type) noexcept
  {
    if (_selectionData)
    {
      return ShareRefPtr(_selectionData->Buffer(type));
    }
    auto *strategy = Strategy();
    return strategy ? strategy->ReadBufferFromClipboard(_name, type) : nullptr;
  }
}
