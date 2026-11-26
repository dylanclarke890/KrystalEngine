#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Geometry/Point.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Web/Dom/Types/DOMString.hpp"
#include "Krystal.Web/Html/DragAndDrop/DragActions.hpp"
#include "Krystal.Web/Loader/Cache/CachedResourceHandle.hpp"
#include "Krystal.Web/Platform/Pasteboard/DragImage.hpp"

namespace krys::web
{
  class CachedImage;
  class Document;
  class DragImageLoader;
  class Element;
  class File;
  class FileList;
  class Pasteboard;
  enum class WebContentReadingPolicy : bool;

  class DataTransferItemList;

  /// @brief The data of a drag, a clipboard event or an input event, and what a page may do with it.
  /// @see https://html.spec.whatwg.org/multipage/dnd.html#the-datatransfer-interface
  class DataTransfer final : public RefCounted<DataTransfer>, public CanMakeWeakPtr<DataTransfer>
  {
    KRYS_NON_COPY_MOVABLE(DataTransfer);

  public:
    /// @see https://html.spec.whatwg.org/multipage/dnd.html#drag-data-store-mode
    enum class StoreMode : uint8
    {
      /// @brief Cut off: its event is over (`MakeInvalidForSecurity`).
      Invalid,
      /// @brief The page may read and write the data, in `dragstart` and `copy`/`cut`.
      ReadWrite,
      /// @brief The page may read the data, in `drop` and `paste`.
      Readonly,
      /// @brief The page may see the types but not the data, as during a drag.
      Protected,
    };

  private:
    /// @brief What the data is for, which decides whether files can be read.
    enum class Type : uint8
    {
      CopyAndPaste,
      DragAndDropData,
      DragAndDropFiles,
      InputEvent,
    };

    /// @brief Identifies the document on the pasteboard, so its own custom data can be read back.
    DOMString _originIdentifier;
    StoreMode _storeMode;
    UniquePtr<Pasteboard> _pasteboard;
    UniquePtr<DataTransferItemList> _itemList;

    /// @brief `files`, made when first asked for.
    mutable RefPtr<FileList> _fileList;

    Type _type;
    DOMString _dropEffect;
    DOMString _effectAllowed;
    /// @brief Whether the drag has started, after which a new drag image goes to the pasteboard at once.
    bool _shouldUpdateDragImage {false};
    /// @brief The drag image and where the cursor is in it.
    PointI _dragLocation;
    CachedResourceHandle<CachedImage> _dragImage;
    RefPtr<Element> _dragImageElement;
    RefPtr<DragImageLoader> _dragImageLoader;

    DataTransfer(StoreMode mode, UniquePtr<Pasteboard> &&pasteboard, Type type = Type::CopyAndPaste,
                 DOMString effectAllowed = u8"uninitialized") noexcept;

  public:
    ~DataTransfer() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dnd.html#dom-datatransfer
    KRYS_NODISCARD static Ref<DataTransfer> Create() noexcept;

    KRYS_NODISCARD static Ref<DataTransfer>
      /// @brief The data of a clipboard event in @p document, over @p pasteboard.
      CreateForCopyAndPaste(const Document &document, StoreMode storeMode,
                            UniquePtr<Pasteboard> &&pasteboard) noexcept;

    /// @brief The read-only data of an input event: @p plainText and @p htmlText.
    KRYS_NODISCARD static Ref<DataTransfer> CreateForInputEvent(DOMStringView plainText,
                                                                DOMStringView htmlText) noexcept;

    /// @brief The data of a drag that starts in @p document, which its pasteboard carries to the drop.
    KRYS_NODISCARD static Ref<DataTransfer> CreateForDrag(const Document &document) noexcept;

    /// @brief What `dragstart`'s listeners may write, moved into the drag's data transfer after it.
    KRYS_NODISCARD static Ref<DataTransfer> CreateForDragStartEvent(const Document &document) noexcept;

    /// @brief The read-only data of `drop`.
    KRYS_NODISCARD static Ref<DataTransfer> CreateForDrop(const Document &document,
                                                          UniquePtr<Pasteboard> &&pasteboard,
                                                          EnumSet<DragOperation> sourceOperationMask,
                                                          bool draggingFiles) noexcept;

    /// @brief The protected data of `dragenter`, `dragover` and `dragleave`: its types, not its data.
    KRYS_NODISCARD static Ref<DataTransfer>
      CreateForUpdatingDropTarget(const Document &document, UniquePtr<Pasteboard> &&pasteboard,
                                  EnumSet<DragOperation> sourceOperationMask, bool draggingFiles) noexcept;

    /// @brief Whether no listener has set `dropEffect`.
    KRYS_NODISCARD bool DropEffectIsUninitialized() const noexcept
    {
      return _dropEffect == u8"uninitialized";
    }

    /// @brief The operations `effectAllowed` allows.
    KRYS_NODISCARD EnumSet<DragOperation> SourceOperationMask() const noexcept;

    /// @brief The operation `dropEffect` chose.
    KRYS_NODISCARD EnumSet<DragOperation> DestinationOperationMask() const noexcept;

    /// @brief Sets `effectAllowed` and `dropEffect` from the operations the host allows and chose.
    void SetSourceOperationMask(EnumSet<DragOperation> operationMask) noexcept;

    void SetDestinationOperationMask(EnumSet<DragOperation> operationMask) noexcept;

    /// @brief Sets the drag image to @p element (an image that isn't in a document, or a snapshot of the
    /// element), with the cursor at @p x, @p y in it.
    /// @see https://html.spec.whatwg.org/multipage/dnd.html#dom-datatransfer-setdragimage
    void SetDragImage(Ref<Element> element, int32 x, int32 y) noexcept;

    /// @brief The drag has started, so drag images the page sets from now on go to the pasteboard at once.
    void SetDragHasStarted() noexcept
    {
      _shouldUpdateDragImage = true;
    }

    /// @brief Makes the drag image the page set, with where the cursor is in it in @p dragLocation; null if
    /// it set none.
    KRYS_NODISCARD DragImageRef CreateDragImage(const Document *document,
                                                PointI &dragLocation) const noexcept;

    /// @brief Gives the pasteboard the drag image the page set, once the drag has started.
    void UpdateDragImage(const Document *document) noexcept;

    /// @brief The element the page set as the drag image, if it isn't an image out of a document.
    KRYS_NODISCARD RefPtr<Element> DragImageElement() const noexcept;

    /// @brief Takes the data, effects, drag image and files @p other (`dragstart`'s data transfer) has.
    void MoveDragState(const Ref<DataTransfer> &other) noexcept;

    /// @brief Whether the page set a drag image.
    KRYS_NODISCARD bool HasDragImage() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dnd.html#dom-datatransfer-dropeffect
    KRYS_NODISCARD DOMString DropEffect() const noexcept;

    void SetDropEffect(DOMStringView effect) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dnd.html#dom-datatransfer-effectallowed
    KRYS_NODISCARD DOMString EffectAllowed() const noexcept;

    void SetEffectAllowed(DOMStringView effect) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dnd.html#dom-datatransfer-items
    KRYS_NODISCARD DataTransferItemList &Items(Document &document) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dnd.html#dom-datatransfer-types
    KRYS_NODISCARD List<DOMString> Types(Document &document) const noexcept;

    /// @brief The types without `Files`, which the item list lists as items of its own.
    KRYS_NODISCARD List<DOMString> TypesForItemList(Document &document) const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dnd.html#dom-datatransfer-files
    KRYS_NODISCARD FileList &Files(Document *document) const noexcept;

    KRYS_NODISCARD FileList &Files(Document &document) const noexcept;

    /// @brief Clears the data of @p type, or all the data if none.
    /// @see https://html.spec.whatwg.org/multipage/dnd.html#dom-datatransfer-cleardata
    void ClearData(Maybe<DOMStringView> type = null) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dnd.html#dom-datatransfer-getdata
    KRYS_NODISCARD DOMString GetData(Document &document, DOMStringView type) const noexcept;

    /// @brief `getData` and `setData` for a type that's already normalized, as the item list's are.
    KRYS_NODISCARD DOMString GetDataForItem(Document &document, DOMStringView type) const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dnd.html#dom-datatransfer-setdata
    void SetData(Document &document, DOMStringView type, DOMStringView data) noexcept;

    void SetDataFromItemList(Document &document, DOMStringView type, DOMStringView data) noexcept;

    /// @brief Cuts the page off from the data once its event is over.
    void MakeInvalidForSecurity() noexcept
    {
      _storeMode = StoreMode::Invalid;
    }

    /// @brief What the store mode lets the page do: see `StoreMode`.
    KRYS_NODISCARD bool CanReadTypes() const noexcept;

    KRYS_NODISCARD bool CanReadData() const noexcept;

    KRYS_NODISCARD bool CanWriteData() const noexcept;

    /// @brief Whether the pasteboard has a file of MIME type @p type.
    KRYS_NODISCARD bool HasFileOfType(DOMStringView type) noexcept;

    /// @brief Where the data is kept: the host's pasteboard, or a static one for data the page writes.
    KRYS_NODISCARD Pasteboard &GetPasteboard() noexcept
    {
      return *_pasteboard;
    }

    /// @brief Writes what the page put in this data transfer's static pasteboard to @p nativePasteboard.
    void CommitToPasteboard(Pasteboard &nativePasteboard) noexcept;

    /// @brief Keeps `files` in step with the item list as the page adds or removes files.
    void DidAddFileToItemList() noexcept;

    void UpdateFileList(Document *document) noexcept;

  private:
    /// @brief Files can be read except in a drag of something other than files.
    KRYS_NODISCARD bool AllowsFileAccess() const noexcept
    {
      return !ForDrag() || ForFileDrag();
    }

    KRYS_NODISCARD bool ForDrag() const noexcept
    {
      return _type == Type::DragAndDropData || _type == Type::DragAndDropFiles;
    }

    KRYS_NODISCARD bool ForFileDrag() const noexcept
    {
      return _type == Type::DragAndDropFiles;
    }

    KRYS_NODISCARD DOMString ReadStringFromPasteboard(Document &document, DOMStringView lowercaseType,
                                                      WebContentReadingPolicy policy) const noexcept;

    /// @brief Whether the pasteboard may hold file paths, so plain data is kept from the page.
    KRYS_NODISCARD bool ShouldSuppressGetAndSetDataToAvoidExposingFilePaths() const noexcept;

    enum class AddFilesType : bool
    {
      No,
      Yes,
    };

    KRYS_NODISCARD List<DOMString> Types(Document &document, AddFilesType addFilesType) const noexcept;

    KRYS_NODISCARD List<Ref<File>> FilesFromPasteboardAndItemList() const noexcept;
  };
}
