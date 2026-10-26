#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceHandle.hpp"
#include "Krystal.Booey/Page/DragActions.hpp"
#include "Krystal.Booey/Platform/DragImage.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class CachedImage;
  class DragImageLoader;
  class File;
  class FileList;
  class Pasteboard;
  enum class WebContentReadingPolicy : bool;
}

namespace krys::boo::dom
{
  class DataTransferItemList;
  class Document;
  class Element;

  /// @brief The data of a drag, a clipboard event or an input event, and what a page may do with it.
  /// @see https://html.spec.whatwg.org/multipage/dnd.html#the-datatransfer-interface
  class DataTransfer final : public RefCounted<DataTransfer>, public CanMakeWeakPtr<DataTransfer>
  {
    KRYS_NON_COPY_MOVABLE(DataTransfer);

  public:
    /// @see https://html.spec.whatwg.org/multipage/dnd.html#drag-data-store-mode
    enum class StoreMode : uint8
    {
      Invalid,
      ReadWrite,
      Readonly,
      Protected
    };

  private:
    enum class Type : uint8
    {
      CopyAndPaste,
      DragAndDropData,
      DragAndDropFiles,
      InputEvent
    };

    DOMString _originIdentifier;
    StoreMode _storeMode;
    UniquePtr<Pasteboard> _pasteboard;
    UniquePtr<DataTransferItemList> _itemList;

    mutable RefPtr<FileList> _fileList;

    Type _type;
    DOMString _dropEffect;
    DOMString _effectAllowed;
    bool _shouldUpdateDragImage {false};
    IntPoint _dragLocation;
    CachedResourceHandle<CachedImage> _dragImage;
    RefPtr<Element> _dragImageElement;
    RefPtr<DragImageLoader> _dragImageLoader;

    DataTransfer(StoreMode mode, UniquePtr<Pasteboard> &&pasteboard, Type type = Type::CopyAndPaste,
                 DOMString &&effectAllowed = u8"uninitialized") noexcept;

  public:
    ~DataTransfer() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dnd.html#dom-datatransfer
    KRYS_NODISCARD static Ref<DataTransfer> Create() noexcept;
    KRYS_NODISCARD static Ref<DataTransfer>
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

    KRYS_NODISCARD bool DropEffectIsUninitialized() const noexcept
    {
      return _dropEffect == u8"uninitialized";
    }

    /// @brief The operations `effectAllowed` allows.
    KRYS_NODISCARD EnumSet<DragOperation> SourceOperationMask() const noexcept;
    /// @brief The operation `dropEffect` chose.
    KRYS_NODISCARD EnumSet<DragOperation> DestinationOperationMask() const noexcept;
    void SetSourceOperationMask(EnumSet<DragOperation> operationMask) noexcept;
    void SetDestinationOperationMask(EnumSet<DragOperation> operationMask) noexcept;

    /// @brief Sets the drag image to @p element (an image that isn't in a document, or a snapshot of the
    /// element), with the cursor at @p x, @p y in it.
    /// @see https://html.spec.whatwg.org/multipage/dnd.html#dom-datatransfer-setdragimage
    void SetDragImage(Ref<Element> &&element, int32 x, int32 y) noexcept;

    void SetDragHasStarted() noexcept
    {
      _shouldUpdateDragImage = true;
    }
    /// @brief Makes the drag image the page set, with where the cursor is in it in @p dragLocation; null if
    /// it set none.
    KRYS_NODISCARD DragImageRef CreateDragImage(const Document *document,
                                                IntPoint &dragLocation) const noexcept;
    /// @brief Gives the pasteboard the drag image the page set, once the drag has started.
    void UpdateDragImage(const Document *document) noexcept;
    /// @brief The element the page set as the drag image, if it isn't an image out of a document.
    KRYS_NODISCARD RefPtr<Element> DragImageElement() const noexcept;

    /// @brief Takes the data, effects, drag image and files @p other (`dragstart`'s data transfer) has.
    void MoveDragState(Ref<DataTransfer> &&other) noexcept;
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
    KRYS_NODISCARD List<DOMString> TypesForItemList(Document &document) const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dnd.html#dom-datatransfer-files
    KRYS_NODISCARD FileList &Files(Document *document) const noexcept;
    KRYS_NODISCARD FileList &Files(Document &document) const noexcept;

    /// @brief Clears the data of @p type, or all the data if none.
    /// @see https://html.spec.whatwg.org/multipage/dnd.html#dom-datatransfer-cleardata
    void ClearData(Maybe<DOMStringView> type = null) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dnd.html#dom-datatransfer-getdata
    KRYS_NODISCARD DOMString GetData(Document &document, DOMStringView type) const noexcept;
    KRYS_NODISCARD DOMString GetDataForItem(Document &document, DOMStringView type) const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dnd.html#dom-datatransfer-setdata
    void SetData(Document &document, DOMStringView type, DOMStringView data) noexcept;
    void SetDataFromItemList(Document &document, DOMStringView type, DOMStringView data) noexcept;

    void MakeInvalidForSecurity() noexcept
    {
      _storeMode = StoreMode::Invalid;
    }

    KRYS_NODISCARD bool CanReadTypes() const noexcept;
    KRYS_NODISCARD bool CanReadData() const noexcept;
    KRYS_NODISCARD bool CanWriteData() const noexcept;

    KRYS_NODISCARD bool HasFileOfType(DOMStringView type) noexcept;
    KRYS_NODISCARD bool HasStringOfType(Document &document, DOMStringView type) noexcept;

    KRYS_NODISCARD Pasteboard &GetPasteboard() noexcept
    {
      return *_pasteboard;
    }
    /// @brief Writes what the page put in this data transfer's static pasteboard to @p nativePasteboard.
    void CommitToPasteboard(Pasteboard &nativePasteboard) noexcept;

    void DidAddFileToItemList() noexcept;
    void UpdateFileList(Document *document) noexcept;

  private:
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
    KRYS_NODISCARD bool ShouldSuppressGetAndSetDataToAvoidExposingFilePaths() const noexcept;

    enum class AddFilesType : bool
    {
      No,
      Yes
    };
    KRYS_NODISCARD List<DOMString> Types(Document &document, AddFilesType addFilesType) const noexcept;
    KRYS_NODISCARD List<Ref<File>> FilesFromPasteboardAndItemList() const noexcept;
  };
}
