/*
 * Copyright (C) 2007-2020 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Core/Geometry/Point.hpp"
#include "Krystal.Core/Geometry/Size.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Net/Url.hpp"
#include "Krystal.Web/Page/Clients/DragClient.hpp"
#include "Krystal.Web/Page/Drag/DragActions.hpp"

namespace krys::web
{
  class DataTransfer;
  class Document;
  class DragData;
  class Element;
  class Frame;
  class FrameSelection;
  class HTMLInputElement;
  class HitTestResult;
  class Image;
  class PlatformMouseEvent;

  struct DragState;

  /// @brief A page's drags and drops: which element a press may drag, starting a drag through the host's
  /// drag client, and what a drag over the page and its drop do: the page's drag events, then by default
  /// editing (text moved or copied into editable content) or files into file inputs.
  // FIXME: Dragging colour inputs and dropping colours, with colours on the pasteboard; dropped image
  // placeholders. (No attachments or models, which are Cocoa's.)
  class DragController
  {
    KRYS_NON_COPY_MOVABLE(DragController);

  private:
    Frame &_frame;
    UniquePtr<DragClient> _client;

    /// @brief The document the mouse was last dragged over.
    RefPtr<Document> _documentUnderMouse;
    /// @brief The document (if any) that initiated the drag.
    RefPtr<Document> _dragInitiator;
    RefPtr<HTMLInputElement> _fileInputElementUnderMouse;
    uint32 _numberOfItemsToBeAccepted {0};

    DragHandlingMethod _dragHandlingMethod {DragHandlingMethod::None};
    EnumSet<DragDestinationAction> _dragDestinationActionMask;
    EnumSet<DragSourceAction> _dragSourceAction;
    bool _didInitiateDrag {false};
    /// @brief Set in StartDrag when a drag starts from a mouse down within the page.
    EnumSet<DragOperation> _sourceDragOperationMask;
    PointI _dragOffset;
    net::Url _draggingImageUrl;
    bool _isPerformingDrop {false};

  public:
    DragController(Frame &page, UniquePtr<DragClient> &&client) noexcept;

    ~DragController() noexcept;

    KRYS_NODISCARD static DragOperation PlatformGenericDragOperation() noexcept;

    /// @brief A drag entered or moved over @p frame.
    /// @return The operation a drop there would do, if any.
    Maybe<DragOperation> DragEnteredOrUpdated(Frame &frame, DragData dragData) noexcept;

    /// @brief A drag left @p frame.
    void DragExited(Frame &frame, DragData dragData) noexcept;

    /// @brief Drops the drag where it is.
    /// @return Whether the drop was handled.
    bool PerformDragOperation(DragData dragData) noexcept;

    KRYS_NODISCARD bool MouseIsOverFileInput() const noexcept;

    KRYS_NODISCARD uint32 NumberOfItemsToBeAccepted() const noexcept
    {
      return _numberOfItemsToBeAccepted;
    }

    // FIXME: It should be possible to remove a number of these accessors once all
    // drag logic is in Krystal.Web.
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

    KRYS_NODISCARD const net::Url &DraggingImageUrl() const noexcept
    {
      return _draggingImageUrl;
    }

    void SetDragOffset(const PointI &offset) noexcept
    {
      _dragOffset = offset;
    }

    KRYS_NODISCARD const PointI &DragOffset() const noexcept
    {
      return _dragOffset;
    }

    KRYS_NODISCARD EnumSet<web::DragSourceAction> GetDragSourceAction() const noexcept
    {
      return _dragSourceAction;
    }

    KRYS_NODISCARD DragHandlingMethod GetDragHandlingMethod() const noexcept
    {
      return _dragHandlingMethod;
    }

    KRYS_NODISCARD Document *DocumentUnderMouse() const noexcept
    {
      return _documentUnderMouse.get();
    }

    KRYS_NODISCARD EnumSet<DragDestinationAction> DragDestinationActionMask() const noexcept
    {
      return _dragDestinationActionMask;
    }

    /// @brief Asks the client which drags may start at @p rootViewPoint, and remembers it.
    EnumSet<DragSourceAction> DelegateDragSourceAction(const PointI &rootViewPoint) noexcept;

    /// @brief The element a press at @p dragOrigin on @p start would drag, noting in @p state what it drags.
    KRYS_NODISCARD RefPtr<Element> DraggableElement(const Frame *sourceFrame, Element *start,
                                                    const PointI &dragOrigin,
                                                    DragState &state) const noexcept;

    void DragEnded() noexcept;

    /// @brief Puts the drag caret at @p windowPoint, as a host controlling the drag itself does.
    void PlaceDragCaret(const PointI &windowPoint) noexcept;

    /// @brief Writes what a drag of @p element (the selection, an image or a link, as @p actionMask says)
    /// carries by default to @p dataTransfer, before `dragstart` lets the page change it.
    void PrepareForDragStart(Frame &sourceFrame, EnumSet<DragSourceAction> actionMask, Element &sourceElement,
                             DataTransfer &dataTransfer, const PointI &dragOrigin) const noexcept;

    /// @brief Starts the drag @p state describes, handing it to the client.
    /// @return Whether it started.
    bool StartDrag(Frame &src, const DragState &state, EnumSet<DragOperation> sourceOperationMask,
                   const PlatformMouseEvent &dragEvent, const PointI &dragOrigin,
                   HasNonDefaultPasteboardData hasData) noexcept;

    /// @brief The largest an image's drag image is, in CSS pixels.
    KRYS_NODISCARD static const SizeI &MaxDragImageSize() noexcept;

    // FIXME: These values are straight out of DragControllerMac, so probably have
    // little correlation with Gdk standards...
    constexpr static int32 MaxOriginalImageArea = 1'500 * 1'500;
    constexpr static int32 DragIconRightInset = 7;
    constexpr static int32 DragIconBottomInset = 3;
    constexpr static float DragImageAlpha = 0.75f;

  private:
    bool DispatchTextInputEventFor(Frame *innerFrame, const DragData &dragData) noexcept;

    bool CanProcessDrag(const DragData &dragData) noexcept;

    bool ConcludeEditDrag(const DragData &dragData) noexcept;

    DragHandlingMethod TryDocumentDrag(Frame &frame, const DragData &dragData,
                                       EnumSet<DragDestinationAction> destinationActionMask,
                                       Maybe<DragOperation> &dragOperation) noexcept;

    bool TryDHTMLDrag(Frame &frame, const DragData &dragData, Maybe<DragOperation> &operation) noexcept;

    void ClearDragCaret() noexcept;

    bool DragIsMove(FrameSelection &selection, const DragData &dragData) noexcept;

    static bool IsCopyKeyDown(const DragData &dragData) noexcept;

    void MouseMovedIntoDocument(RefPtr<Document> &&newDocument) noexcept;

    void DisallowFileAccessIfNeeded(DragData &dragData) noexcept;

    KRYS_NODISCARD static Maybe<HitTestResult> HitTestResultForDragStart(Frame &source, Element &element,
                                                                         const PointI &location) noexcept;

    KRYS_NODISCARD static bool ShouldUseCachedImageForDragImage(const Image &image) noexcept;

    void DoImageDrag(Element &element, const PointI &dragOrigin, const RectI &layoutRect, Frame &frame,
                     PointI &dragImageOffset, const DragState &state) noexcept;

    void DoSystemDrag(DragImage image, const PointI &dragLoc, const PointI &eventPos, Frame &frame,
                      const DragState &state) noexcept;

    KRYS_NODISCARD DragClient &Client() const noexcept
    {
      return *_client;
    }

    static void DeclareAndWriteDragImage(DataTransfer &dataTransfer, Element &element, const net::Url &url,
                                         utf8_stringview label) noexcept;
  };

  /// @brief Whether @p element is a link a press may drag: an `a` with an `href` that isn't editable, or an
  /// SVG `a`.
  KRYS_NODISCARD bool IsDraggableLink(const Element &element) noexcept;
}
