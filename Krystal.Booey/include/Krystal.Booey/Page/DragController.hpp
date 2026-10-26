#pragma once

#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Page/DragActions.hpp"
#include "Krystal.Booey/Page/DragClient.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::dom
{
  class DataTransfer;
  class Document;
  class Element;
}

namespace krys::boo::html
{
  class HTMLInputElement;
}

namespace krys::boo
{
  class DragData;
  class FrameSelection;
  class HitTestResult;
  class LocalFrame;
  class Page;
  class PlatformMouseEvent;
  struct DragState;

  /// @brief A page's drags and drops: which element a press may drag, starting a drag through the host's
  /// drag client, and what a drag over the page and its drop do: the page's drag events, then by default
  /// editing (text moved or copied into editable content), files into file inputs, or a load of what's
  /// dropped.
  // FIXME: Dragging colour inputs and dropping colours, with colours on the pasteboard; the drag image
  // (doImageDrag, the drag image of selections, links and images), with drag images; dropped image
  // placeholders, attachments and models, which are Cocoa's; remote frames.
  class DragController
  {
    KRYS_NON_COPY_MOVABLE(DragController);

  private:
    Page &_page;
    UniquePtr<DragClient> _client;

    /// @brief The document the mouse was last dragged over.
    RefPtr<dom::Document> _documentUnderMouse;
    /// @brief The document (if any) that initiated the drag.
    RefPtr<dom::Document> _dragInitiator;
    RefPtr<html::HTMLInputElement> _fileInputElementUnderMouse;
    uint32 _numberOfItemsToBeAccepted {0};

    DragHandlingMethod _dragHandlingMethod {DragHandlingMethod::None};
    EnumSet<DragDestinationAction> _dragDestinationActionMask;
    EnumSet<DragSourceAction> _dragSourceAction;
    bool _didInitiateDrag {false};
    /// @brief Set in StartDrag when a drag starts from a mouse down within the page.
    EnumSet<DragOperation> _sourceDragOperationMask;
    IntPoint _dragOffset;
    url::URL _draggingImageURL;
    bool _isPerformingDrop {false};

  public:
    DragController(Page &page, UniquePtr<DragClient> &&client) noexcept;
    ~DragController() noexcept;

    KRYS_NODISCARD static DragOperation PlatformGenericDragOperation() noexcept;

    /// @brief A drag entered or moved over @p frame.
    /// @return The operation a drop there would do, if any.
    Maybe<DragOperation> DragEnteredOrUpdated(LocalFrame &frame, DragData &&dragData) noexcept;
    /// @brief A drag left @p frame.
    void DragExited(LocalFrame &frame, DragData &&dragData) noexcept;
    /// @brief Drops the drag where it is.
    /// @return Whether the drop was handled.
    bool PerformDragOperation(DragData &&dragData) noexcept;

    KRYS_NODISCARD bool MouseIsOverFileInput() const noexcept;
    KRYS_NODISCARD uint32 NumberOfItemsToBeAccepted() const noexcept
    {
      return _numberOfItemsToBeAccepted;
    }

    // FIXME: It should be possible to remove a number of these accessors once all
    // drag logic is in Booey.
    void SetDidInitiateDrag(bool initiated) noexcept
    {
      _didInitiateDrag = initiated;
    }
    KRYS_NODISCARD bool DidInitiateDrag() const noexcept
    {
      return _didInitiateDrag;
    }
    KRYS_NODISCARD EnumSet<DragOperation> SourceDragOperationMask() const noexcept
    {
      return _sourceDragOperationMask;
    }
    KRYS_NODISCARD const url::URL &DraggingImageURL() const noexcept
    {
      return _draggingImageURL;
    }
    void SetDragOffset(const IntPoint &offset) noexcept
    {
      _dragOffset = offset;
    }
    KRYS_NODISCARD const IntPoint &DragOffset() const noexcept
    {
      return _dragOffset;
    }
    KRYS_NODISCARD EnumSet<boo::DragSourceAction> GetDragSourceAction() const noexcept
    {
      return _dragSourceAction;
    }
    KRYS_NODISCARD DragHandlingMethod GetDragHandlingMethod() const noexcept
    {
      return _dragHandlingMethod;
    }
    KRYS_NODISCARD dom::Document *DocumentUnderMouse() const noexcept
    {
      return _documentUnderMouse.get();
    }
    KRYS_NODISCARD EnumSet<DragDestinationAction> DragDestinationActionMask() const noexcept
    {
      return _dragDestinationActionMask;
    }

    /// @brief Asks the client which drags may start at @p rootViewPoint, and remembers it.
    EnumSet<DragSourceAction> DelegateDragSourceAction(const IntPoint &rootViewPoint) noexcept;

    /// @brief The element a press at @p dragOrigin on @p start would drag, noting in @p state what it drags.
    KRYS_NODISCARD RefPtr<dom::Element> DraggableElement(const LocalFrame *sourceFrame, dom::Element *start,
                                                         const IntPoint &dragOrigin,
                                                         DragState &state) const noexcept;

    void DragEnded() noexcept;
    /// @brief Puts the drag caret at @p windowPoint, as a host controlling the drag itself does.
    void PlaceDragCaret(const IntPoint &windowPoint) noexcept;

    /// @brief Writes what a drag of @p element (the selection, an image or a link, as @p actionMask says)
    /// carries by default to @p dataTransfer, before `dragstart` lets the page change it.
    void PrepareForDragStart(LocalFrame &sourceFrame, EnumSet<DragSourceAction> actionMask,
                             dom::Element &sourceElement, dom::DataTransfer &dataTransfer,
                             const IntPoint &dragOrigin) const noexcept;
    /// @brief Starts the drag @p state describes, handing it to the client.
    /// @return Whether it started.
    bool StartDrag(LocalFrame &src, const DragState &state, EnumSet<DragOperation> sourceOperationMask,
                   const PlatformMouseEvent &dragEvent, const IntPoint &dragOrigin,
                   HasNonDefaultPasteboardData hasData) noexcept;

  private:
    bool DispatchTextInputEventFor(LocalFrame *innerFrame, const DragData &dragData) noexcept;
    bool CanProcessDrag(const DragData &dragData) noexcept;
    bool ConcludeEditDrag(const DragData &dragData) noexcept;
    Maybe<DragOperation> OperationForLoad(const DragData &dragData) noexcept;
    DragHandlingMethod TryDocumentDrag(LocalFrame &frame, const DragData &dragData,
                                       EnumSet<DragDestinationAction> destinationActionMask,
                                       Maybe<DragOperation> &dragOperation) noexcept;
    bool TryDHTMLDrag(LocalFrame &frame, const DragData &dragData, Maybe<DragOperation> &operation) noexcept;
    Maybe<DragOperation> DragOperationFor(const DragData &dragData) noexcept;
    void ClearDragCaret() noexcept;
    bool DragIsMove(FrameSelection &selection, const DragData &dragData) noexcept;
    bool IsCopyKeyDown(const DragData &dragData) noexcept;

    void MouseMovedIntoDocument(RefPtr<dom::Document> &&newDocument) noexcept;

    void DisallowFileAccessIfNeeded(DragData &dragData) noexcept;

    KRYS_NODISCARD Maybe<HitTestResult> HitTestResultForDragStart(LocalFrame &source, dom::Element &element,
                                                                  const IntPoint &location) const noexcept;

    void DoSystemDrag(const IntPoint &dragLoc, const IntPoint &eventPos, LocalFrame &frame,
                      const DragState &state) noexcept;

    KRYS_NODISCARD DragClient &Client() const noexcept
    {
      return *_client;
    }

    void DeclareAndWriteDragImage(dom::DataTransfer &dataTransfer, dom::Element &element, const url::URL &url,
                                  utf8_stringview label) noexcept;
  };

  /// @brief Whether @p element is a link a press may drag: an `a` with an `href` that isn't editable, or an
  /// SVG `a`.
  KRYS_NODISCARD bool IsDraggableLink(const dom::Element &element) noexcept;
}
