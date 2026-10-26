#include "Krystal.Booey/DOM/DataTransfer.hpp"
#include "Krystal.Booey/DOM/DataTransferItem.hpp"
#include "Krystal.Booey/DOM/DataTransferItemList.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Editing/Markup.hpp"
#include "Krystal.Booey/Editing/PasteboardFileListReader.hpp"
#include "Krystal.Booey/Editing/WebContentReader.hpp"
#include "Krystal.Booey/FileAPI/FileList.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImageClient.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Platform/Pasteboard.hpp"
#include "Krystal.Booey/Platform/StaticPasteboard.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include <algorithm>

namespace krys::boo
{
  /// @brief Updates a data transfer's drag image as the image the page set loads.
  class DragImageLoader final : public CachedImageClient, public RefCounted<DragImageLoader>
  {
    KRYS_NON_COPY_MOVABLE(DragImageLoader);

  private:
    WeakPtr<dom::DataTransfer> _dataTransfer;
    WeakPtr<const dom::Document> _document;

  public:
    KRYS_NODISCARD static Ref<DragImageLoader> Create(dom::DataTransfer &dataTransfer,
                                                      const dom::Document &document) noexcept
    {
      return AdoptRef(*new DragImageLoader(dataTransfer, document));
    }

    void StartLoading(CachedResourceHandle<CachedImage> &image) noexcept
    {
      // FIXME: Does this really trigger a load? Does it need to?
      image->AddClient(*this);
    }

    void StopLoading(CachedResourceHandle<CachedImage> &image) noexcept
    {
      image->RemoveClient(*this);
    }

    void MoveToDataTransfer(dom::DataTransfer &newDataTransfer) noexcept
    {
      _dataTransfer = CreateWeakPtr(&newDataTransfer);
    }

  private:
    DragImageLoader(dom::DataTransfer &dataTransfer, const dom::Document &document) noexcept
        : _dataTransfer(CreateWeakPtr(&dataTransfer)), _document(CreateWeakPtr(&document))
    {
    }

    void ImageChanged(CachedImage *, const IntRect *) noexcept final
    {
      if (RefPtr<dom::DataTransfer> dataTransfer = ShareRefPtr(_dataTransfer.get()))
      {
        dataTransfer->UpdateDragImage(_document.get());
      }
    }
  };
}

namespace krys::boo::dom
{
  namespace
  {
    KRYS_NODISCARD DOMString TrimmedLowercaseType(DOMStringView type) noexcept
    {
      while (!type.empty() && text::IsASCIIWhitespace(type.front()))
      {
        type.remove_prefix(1);
      }
      while (!type.empty() && text::IsASCIIWhitespace(type.back()))
      {
        type.remove_suffix(1);
      }
      return text::ToASCIILower(type);
    }

    KRYS_NODISCARD DOMString NormalizeType(DOMStringView type) noexcept
    {
      auto lowercaseType = TrimmedLowercaseType(type);
      if (lowercaseType == u8"text" || lowercaseType.starts_with(u8"text/plain"))
      {
        return u8"text/plain";
      }
      if (lowercaseType == u8"url" || lowercaseType.starts_with(u8"text/uri-list;"))
      {
        return u8"text/uri-list";
      }
      if (lowercaseType.starts_with(u8"text/html;"))
      {
        return u8"text/html";
      }

      return lowercaseType;
    }

    KRYS_NODISCARD bool Contains(const List<DOMString> &types, DOMStringView type) noexcept
    {
      return std::ranges::find(types, type) != types.end();
    }

    template <typename TPredicate>
    KRYS_NODISCARD DOMString ReadURLsFromPasteboardAsString(Pasteboard &pasteboard,
                                                            TPredicate &&shouldIncludeURL) noexcept
    {
      DOMString urlList;
      for (auto &urlString : pasteboard.ReadAllStrings(u8"text/uri-list"))
      {
        if (!shouldIncludeURL(urlString))
        {
          continue;
        }
        if (!urlList.empty())
        {
          urlList += u8'\n';
        }
        urlList += urlString;
      }
      return urlList;
    }

    KRYS_NODISCARD EnumSet<DragOperation> DragOpFromIEOp(DOMStringView operation) noexcept
    {
      if (operation == u8"uninitialized")
      {
        return AnyDragOperation();
      }
      if (operation == u8"none")
      {
        return {};
      }
      if (operation == u8"copy")
      {
        return {DragOperation::Copy};
      }
      if (operation == u8"link")
      {
        return {DragOperation::Link};
      }
      if (operation == u8"move")
      {
        return {DragOperation::Generic, DragOperation::Move};
      }
      if (operation == u8"copyLink")
      {
        return {DragOperation::Copy, DragOperation::Link};
      }
      if (operation == u8"copyMove")
      {
        return {DragOperation::Copy, DragOperation::Generic, DragOperation::Move};
      }
      if (operation == u8"linkMove")
      {
        return {DragOperation::Link, DragOperation::Generic, DragOperation::Move};
      }
      if (operation == u8"all")
      {
        return AnyDragOperation();
      }
      return {DragOperation::Private}; // Really a marker for "no conversion".
    }

    KRYS_NODISCARD DOMStringView IEOpFromDragOp(EnumSet<DragOperation> operationMask) noexcept
    {
      bool isGenericMove = operationMask.ContainsAny({DragOperation::Generic, DragOperation::Move});

      if ((isGenericMove && operationMask.ContainsAll({DragOperation::Copy, DragOperation::Link}))
          || operationMask.ContainsAll(AnyDragOperation()))
      {
        return u8"all";
      }
      if (isGenericMove && operationMask.Contains(DragOperation::Copy))
      {
        return u8"copyMove";
      }
      if (isGenericMove && operationMask.Contains(DragOperation::Link))
      {
        return u8"linkMove";
      }
      if (operationMask.ContainsAll({DragOperation::Copy, DragOperation::Link}))
      {
        return u8"copyLink";
      }
      if (isGenericMove)
      {
        return u8"move";
      }
      if (operationMask.Contains(DragOperation::Copy))
      {
        return u8"copy";
      }
      if (operationMask.Contains(DragOperation::Link))
      {
        return u8"link";
      }
      return u8"none";
    }

    /// @brief Reads the types of the files on a pasteboard.
    struct PasteboardFileTypeReader final : PasteboardFileReader
    {
      HashSet<DOMString> Types;

      void ReadFilename(utf8_stringview filename) noexcept final
      {
        Types.emplace(text::ToASCIILower(File::ContentTypeForFile(filename)));
      }

      void ReadBuffer(utf8_stringview, utf8_stringview type, Ref<SharedBuffer> &&) noexcept final
      {
        Types.emplace(text::ToASCIILower(type));
      }
    };
  }

  DataTransfer::DataTransfer(StoreMode mode, UniquePtr<Pasteboard> &&pasteboard, Type type,
                             DOMString &&effectAllowed) noexcept
      : _storeMode(mode), _pasteboard(krys::move(pasteboard)), _type(type), _dropEffect(u8"uninitialized"),
        _effectAllowed(krys::move(effectAllowed))
  {
  }

  DataTransfer::~DataTransfer() noexcept
  {
    if (RefPtr<DragImageLoader> dragImageLoader = _dragImageLoader; dragImageLoader && _dragImage)
    {
      dragImageLoader->StopLoading(_dragImage);
    }
  }

  Ref<DataTransfer> DataTransfer::CreateForCopyAndPaste(const Document &document, StoreMode storeMode,
                                                        UniquePtr<Pasteboard> &&pasteboard) noexcept
  {
    auto dataTransfer = AdoptRef(*new DataTransfer(storeMode, krys::move(pasteboard)));
    dataTransfer->_originIdentifier = document.OriginIdentifierForPasteboard();
    return dataTransfer;
  }

  Ref<DataTransfer> DataTransfer::Create() noexcept
  {
    return AdoptRef(*new DataTransfer(StoreMode::ReadWrite, CreateUnique<StaticPasteboard>(),
                                      Type::CopyAndPaste, u8"none"));
  }

  bool DataTransfer::CanReadTypes() const noexcept
  {
    return _storeMode == StoreMode::Readonly || _storeMode == StoreMode::Protected
           || _storeMode == StoreMode::ReadWrite;
  }

  bool DataTransfer::CanReadData() const noexcept
  {
    return _storeMode == StoreMode::Readonly || _storeMode == StoreMode::ReadWrite;
  }

  bool DataTransfer::CanWriteData() const noexcept
  {
    return _storeMode == StoreMode::ReadWrite;
  }

  void DataTransfer::ClearData(Maybe<DOMStringView> type) noexcept
  {
    if (!CanWriteData())
    {
      return;
    }

    Maybe<DOMString> normalizedType;
    if (type)
    {
      normalizedType = NormalizeType(*type);
    }
    if (!normalizedType)
    {
      _pasteboard->Clear();
    }
    else
    {
      _pasteboard->Clear(*normalizedType);
    }
    if (_itemList)
    {
      _itemList->DidClearStringData(normalizedType);
    }
  }

  DOMString DataTransfer::GetDataForItem(Document &document, DOMStringView type) const noexcept
  {
    if (!CanReadData())
    {
      return {};
    }

    auto lowercaseType = TrimmedLowercaseType(type);
    if (ShouldSuppressGetAndSetDataToAvoidExposingFilePaths())
    {
      if (lowercaseType == u8"text/uri-list")
      {
        return ReadURLsFromPasteboardAsString(
          *_pasteboard, [](auto &urlString)
          { return Pasteboard::CanExposeURLToDOMWhenPasteboardContainsFiles(urlString); });
      }

      if (lowercaseType == u8"text/html")
      {
        // If the pasteboard contains files and the page requests 'text/html', we only read from rich text
        // types to prevent file paths from leaking (e.g. from plain text data on the pasteboard) since we
        // sanitize cross-origin markup.
        return ReadStringFromPasteboard(document, lowercaseType, WebContentReadingPolicy::OnlyRichTextTypes);
      }

      return {};
    }

    return ReadStringFromPasteboard(document, lowercaseType, WebContentReadingPolicy::AnyType);
  }

  DOMString DataTransfer::ReadStringFromPasteboard(Document &document, DOMStringView lowercaseType,
                                                   WebContentReadingPolicy policy) const noexcept
  {
    // StaticPasteboard is only used to stage data written by websites before being committed to the system
    // pasteboard.
    bool isStatic = Is<StaticPasteboard>(*_pasteboard);
    bool isSameOrigin =
      isStatic || (!_originIdentifier.empty() && _originIdentifier == _pasteboard->ReadOrigin());
    if (isSameOrigin)
    {
      if (auto value = _pasteboard->ReadStringInCustomData(lowercaseType))
      {
        return krys::move(*value);
      }
    }
    if (!Pasteboard::IsSafeTypeForDOMToReadAndWrite(lowercaseType))
    {
      return {};
    }

    if (!isStatic && lowercaseType == u8"text/html")
    {
      auto *frame = document.Frame();
      if (!frame)
      {
        return {};
      }
      WebContentMarkupReader reader {*frame};
      _pasteboard->Read(reader, policy);
      return reader.TakeMarkup();
    }

    if (!isStatic && lowercaseType == u8"text/uri-list")
    {
      return ReadURLsFromPasteboardAsString(*_pasteboard, [](auto &) { return true; });
    }

    return _pasteboard->ReadString(lowercaseType);
  }

  DOMString DataTransfer::GetData(Document &document, DOMStringView type) const noexcept
  {
    return GetDataForItem(document, NormalizeType(type));
  }

  bool DataTransfer::ShouldSuppressGetAndSetDataToAvoidExposingFilePaths() const noexcept
  {
    return _pasteboard->GetFileContentState() == Pasteboard::FileContentState::MayContainFilePaths;
  }

  void DataTransfer::SetData(Document &document, DOMStringView type, DOMStringView data) noexcept
  {
    if (!CanWriteData())
    {
      return;
    }

    if (ShouldSuppressGetAndSetDataToAvoidExposingFilePaths())
    {
      return;
    }

    auto normalizedType = NormalizeType(type);
    SetDataFromItemList(document, normalizedType, data);
    if (_itemList)
    {
      _itemList->DidSetStringData(normalizedType);
    }
  }

  void DataTransfer::SetDataFromItemList(Document &document, DOMStringView type, DOMStringView data) noexcept
  {
    krys_debug_assert(CanWriteData());

    auto &pasteboard = Downcast<StaticPasteboard>(*_pasteboard);

    Maybe<DOMString> sanitizedData;
    if (type == u8"text/html")
    {
      sanitizedData = SanitizeMarkup(data, &document);
    }
    else if (type == u8"text/uri-list")
    {
      url::URL parsedURL({}, data);
      if (parsedURL.IsValid())
      {
        sanitizedData = parsedURL.String();
      }
    }
    else if (type == u8"text/plain")
    {
      sanitizedData = DOMString(data); // Nothing to sanitize.
    }

    if (sanitizedData != data)
    {
      pasteboard.WriteStringInCustomData(type, data);
    }

    if (Pasteboard::IsSafeTypeForDOMToReadAndWrite(type) && sanitizedData)
    {
      pasteboard.WriteString(type, *sanitizedData);
    }
  }

  void DataTransfer::UpdateFileList(Document *) noexcept
  {
    krys_debug_assert(CanWriteData());

    _fileList->_files = FilesFromPasteboardAndItemList();
  }

  void DataTransfer::DidAddFileToItemList() noexcept
  {
    krys_debug_assert(CanWriteData());
    if (!_fileList)
    {
      return;
    }

    auto &newItem = _itemList->Items().back();
    krys_debug_assert(newItem->IsFile());
    _fileList->Append(newItem->GetFile());
  }

  DataTransferItemList &DataTransfer::Items(Document &document) noexcept
  {
    if (!_itemList)
    {
      _itemList = CreateUnique<DataTransferItemList>(document, *this);
    }
    return *_itemList;
  }

  List<DOMString> DataTransfer::Types(Document &document) const noexcept
  {
    return Types(document, AddFilesType::Yes);
  }

  List<DOMString> DataTransfer::TypesForItemList(Document &document) const noexcept
  {
    return Types(document, AddFilesType::No);
  }

  List<DOMString> DataTransfer::Types(Document &, AddFilesType addFilesType) const noexcept
  {
    if (!CanReadTypes())
    {
      return {};
    }

    bool hideFilesType = !CanWriteData() && !AllowsFileAccess();

    auto safeTypes = _pasteboard->TypesSafeForBindings(_originIdentifier);
    bool hasFileBackedItem =
      _itemList && _itemList->HasItems()
      && std::ranges::any_of(_itemList->Items(), [](auto &item) { return item->IsFile(); });

    auto fileContentState = _pasteboard->GetFileContentState();
    if (hasFileBackedItem || fileContentState != Pasteboard::FileContentState::NoFileOrImageData)
    {
      List<DOMString> types;
      if (!hideFilesType && addFilesType == AddFilesType::Yes)
      {
        types.emplace_back(u8"Files");
      }

      if (fileContentState != Pasteboard::FileContentState::MayContainFilePaths)
      {
        types.insert(types.end(), std::make_move_iterator(safeTypes.begin()),
                     std::make_move_iterator(safeTypes.end()));
        return types;
      }

      if (Contains(safeTypes, u8"text/uri-list"))
      {
        types.emplace_back(u8"text/uri-list");
      }
      if (Contains(safeTypes, u8"text/html"))
      {
        types.emplace_back(u8"text/html");
      }
      return types;
    }

    krys_debug_assert(!Contains(safeTypes, u8"Files"));
    return safeTypes;
  }

  List<Ref<File>> DataTransfer::FilesFromPasteboardAndItemList() const noexcept
  {
    bool addedFilesFromPasteboard = false;
    List<Ref<File>> files;
    if (AllowsFileAccess()
        && _pasteboard->GetFileContentState() != Pasteboard::FileContentState::NoFileOrImageData)
    {
      PasteboardFileListReader reader;
      _pasteboard->Read(reader);
      files = krys::move(reader.Files);
      addedFilesFromPasteboard = !files.empty();
    }

    bool itemListContainsItems = false;
    if (_itemList && _itemList->HasItems())
    {
      for (auto &item : _itemList->Items())
      {
        if (auto file = item->GetFile())
        {
          files.push_back(krys::move(file));
        }
      }
      itemListContainsItems = true;
    }

    KRYS_MAYBE_UNUSED bool containsItemsAndFiles = itemListContainsItems && addedFilesFromPasteboard;
    krys_debug_assert(!containsItemsAndFiles);
    return files;
  }

  FileList &DataTransfer::Files(Document *) const noexcept
  {
    if (!CanReadData())
    {
      if (_fileList)
      {
        _fileList->Clear();
      }
      else
      {
        _fileList = FileList::Create();
      }
      return *_fileList;
    }

    if (!_fileList)
    {
      _fileList = FileList::Create(FilesFromPasteboardAndItemList());
    }

    return *_fileList;
  }

  FileList &DataTransfer::Files(Document &document) const noexcept
  {
    return Files(&document);
  }

  bool DataTransfer::HasFileOfType(DOMStringView type) noexcept
  {
    krys_debug_assert(CanReadTypes());
    PasteboardFileTypeReader reader;
    _pasteboard->Read(reader);
    return reader.Types.contains(text::ToASCIILower(type));
  }

  bool DataTransfer::HasStringOfType(Document &document, DOMStringView type) noexcept
  {
    krys_debug_assert(CanReadTypes());

    return Contains(Types(document), type);
  }

  Ref<DataTransfer> DataTransfer::CreateForInputEvent(DOMStringView plainText,
                                                      DOMStringView htmlText) noexcept
  {
    auto pasteboard = CreateUnique<StaticPasteboard>();
    pasteboard->WriteString(u8"text/plain", plainText);
    pasteboard->WriteString(u8"text/html", htmlText);
    return AdoptRef(*new DataTransfer(StoreMode::Readonly, krys::move(pasteboard), Type::InputEvent));
  }

  void DataTransfer::CommitToPasteboard(Pasteboard &nativePasteboard) noexcept
  {
    krys_debug_assert(!Is<StaticPasteboard>(nativePasteboard));
    auto &staticPasteboard = Downcast<StaticPasteboard>(*_pasteboard);
    if (!staticPasteboard.HasNonDefaultData())
    {
      // We clear the platform pasteboard here to ensure that the pasteboard doesn't contain any data
      // that may have been written before starting the drag or copying, and after ending the last
      // drag session or paste. After pushing the static pasteboard's contents to the platform, the
      // pasteboard should only contain data that was in the static pasteboard.
      nativePasteboard.Clear();
      return;
    }

    auto customData = staticPasteboard.TakeCustomData();
    customData.SetOrigin(_originIdentifier);
    nativePasteboard.WriteCustomData({customData});
  }

  DOMString DataTransfer::DropEffect() const noexcept
  {
    return _dropEffect == u8"uninitialized" ? DOMString(u8"none") : _dropEffect;
  }

  void DataTransfer::SetDropEffect(DOMStringView effect) noexcept
  {
    if (!ForDrag())
    {
      return;
    }

    if (effect != u8"none" && effect != u8"copy" && effect != u8"link" && effect != u8"move")
    {
      return;
    }

    // FIXME: The spec allows this in all circumstances. There is probably no value
    // in ignoring attempts to change it.
    if (!CanReadTypes())
    {
      return;
    }

    _dropEffect = DOMString(effect);
  }

  DOMString DataTransfer::EffectAllowed() const noexcept
  {
    return _effectAllowed;
  }

  void DataTransfer::SetEffectAllowed(DOMStringView effect) noexcept
  {
    if (!ForDrag())
    {
      return;
    }

    // Ignore any attempts to set it to an unknown value.
    if (DragOpFromIEOp(effect) == EnumSet<DragOperation> {DragOperation::Private})
    {
      return;
    }

    if (!CanWriteData())
    {
      return;
    }

    _effectAllowed = DOMString(effect);
  }

  Ref<DataTransfer> DataTransfer::CreateForDrag(const Document &) noexcept
  {
    return AdoptRef(
      *new DataTransfer(StoreMode::ReadWrite, Pasteboard::CreateForDragAndDrop(), Type::DragAndDropData));
  }

  Ref<DataTransfer> DataTransfer::CreateForDragStartEvent(const Document &document) noexcept
  {
    auto dataTransfer = AdoptRef(
      *new DataTransfer(StoreMode::ReadWrite, CreateUnique<StaticPasteboard>(), Type::DragAndDropData));
    dataTransfer->_originIdentifier = document.OriginIdentifierForPasteboard();
    return dataTransfer;
  }

  Ref<DataTransfer> DataTransfer::CreateForDrop(const Document &document, UniquePtr<Pasteboard> &&pasteboard,
                                                EnumSet<DragOperation> sourceOperationMask,
                                                bool draggingFiles) noexcept
  {
    auto dataTransfer =
      AdoptRef(*new DataTransfer(StoreMode::Readonly, krys::move(pasteboard),
                                 draggingFiles ? Type::DragAndDropFiles : Type::DragAndDropData));
    dataTransfer->SetSourceOperationMask(sourceOperationMask);
    dataTransfer->_originIdentifier = document.OriginIdentifierForPasteboard();
    return dataTransfer;
  }

  Ref<DataTransfer> DataTransfer::CreateForUpdatingDropTarget(const Document &document,
                                                              UniquePtr<Pasteboard> &&pasteboard,
                                                              EnumSet<DragOperation> sourceOperationMask,
                                                              bool draggingFiles) noexcept
  {
    auto dataTransfer =
      AdoptRef(*new DataTransfer(StoreMode::Protected, krys::move(pasteboard),
                                 draggingFiles ? Type::DragAndDropFiles : Type::DragAndDropData));
    dataTransfer->SetSourceOperationMask(sourceOperationMask);
    dataTransfer->_originIdentifier = document.OriginIdentifierForPasteboard();
    return dataTransfer;
  }

  EnumSet<DragOperation> DataTransfer::SourceOperationMask() const noexcept
  {
    auto operationMask = DragOpFromIEOp(_effectAllowed);
    krys_debug_assert((operationMask != EnumSet<DragOperation> {DragOperation::Private}));
    return operationMask;
  }

  EnumSet<DragOperation> DataTransfer::DestinationOperationMask() const noexcept
  {
    auto operationMask = DragOpFromIEOp(_dropEffect);
    krys_debug_assert(
      (operationMask == EnumSet<DragOperation> {DragOperation::Copy} || operationMask.IsEmpty()
       || operationMask == EnumSet<DragOperation> {DragOperation::Link}
       || operationMask == EnumSet<DragOperation> {DragOperation::Generic, DragOperation::Move}
       || operationMask.ContainsAll(AnyDragOperation())));
    return operationMask;
  }

  void DataTransfer::SetSourceOperationMask(EnumSet<DragOperation> operationMask) noexcept
  {
    krys_debug_assert((operationMask != EnumSet<DragOperation> {DragOperation::Private}));
    _effectAllowed = DOMString(IEOpFromDragOp(operationMask));
  }

  void DataTransfer::SetDestinationOperationMask(EnumSet<DragOperation> operationMask) noexcept
  {
    krys_debug_assert(
      (operationMask == EnumSet<DragOperation> {DragOperation::Copy} || operationMask.IsEmpty()
       || operationMask == EnumSet<DragOperation> {DragOperation::Link}
       || operationMask == EnumSet<DragOperation> {DragOperation::Generic}
       || operationMask == EnumSet<DragOperation> {DragOperation::Move}
       || operationMask == EnumSet<DragOperation> {DragOperation::Generic, DragOperation::Move}));
    _dropEffect = DOMString(IEOpFromDragOp(operationMask));
  }

  void DataTransfer::MoveDragState(Ref<DataTransfer> &&other) noexcept
  {
    krys_assert(Is<StaticPasteboard>(other->GetPasteboard()));
    other->CommitToPasteboard(*_pasteboard);

    _dropEffect = other->_dropEffect;
    _effectAllowed = other->_effectAllowed;
    _dragLocation = other->_dragLocation;
    _dragImage = other->_dragImage;
    _dragImageElement = krys::move(other->_dragImageElement);
    _dragImageLoader = krys::move(other->_dragImageLoader);
    if (RefPtr<DragImageLoader> dragImageLoader = _dragImageLoader)
    {
      dragImageLoader->MoveToDataTransfer(*this);
    }
    _fileList = krys::move(other->_fileList);
  }

  bool DataTransfer::HasDragImage() const noexcept
  {
    return _dragImage || _dragImageElement;
  }

  void DataTransfer::SetDragImage(Ref<Element> &&element, int32 x, int32 y) noexcept
  {
    if (!ForDrag() || !CanWriteData())
    {
      return;
    }

    CachedResourceHandle<CachedImage> image;
    if (auto *imageElement = DynamicDowncast<html::HTMLImageElement>(*element);
        imageElement && !imageElement->IsConnected())
    {
      image = imageElement->GetCachedImage();
    }

    _dragLocation = IntPoint(x, y);

    Ref<const Document> document = ShareRef(element->NodeDocument());
    if (RefPtr<DragImageLoader> dragImageLoader = _dragImageLoader; dragImageLoader && _dragImage)
    {
      dragImageLoader->StopLoading(_dragImage);
    }
    _dragImage = image;
    if (_dragImage)
    {
      if (!_dragImageLoader)
      {
        _dragImageLoader = DragImageLoader::Create(*this, *document);
      }
      Ref<DragImageLoader> dragImageLoader = ShareRef(*_dragImageLoader);
      dragImageLoader->StartLoading(_dragImage);
    }

    if (image)
    {
      _dragImageElement = nullptr;
    }
    else
    {
      _dragImageElement = krys::move(element);
    }

    UpdateDragImage(document.get());
  }

  void DataTransfer::UpdateDragImage(const Document *document) noexcept
  {
    // Don't allow setting the image if we haven't started dragging yet; we'll rely on the dragging code
    // to install this drag image as part of getting the drag kicked off.
    if (!_shouldUpdateDragImage)
    {
      return;
    }

    IntPoint computedHotSpot;
    auto computedImage = DragImage {CreateDragImage(document, computedHotSpot)};
    if (!computedImage)
    {
      return;
    }

    _pasteboard->SetDragImage(krys::move(computedImage), computedHotSpot);
  }

  RefPtr<Element> DataTransfer::DragImageElement() const noexcept
  {
    return _dragImageElement;
  }

  DragImageRef DataTransfer::CreateDragImage(const Document *, IntPoint &location) const noexcept
  {
    location = _dragLocation;

    if (_dragImage)
    {
      return CreateDragImageFromImage(_dragImage->GetImage(), ImageOrientation::Orientation::None);
    }

    if (_dragImageElement)
    {
      if (RefPtr<LocalFrame> frame = ShareRefPtr(_dragImageElement->NodeDocument().Frame()))
      {
        return CreateDragImageForNode(*frame, *DragImageElement());
      }
    }

    // We do not have enough information to create a drag image, use the default icon.
    return nullptr;
  }
}
