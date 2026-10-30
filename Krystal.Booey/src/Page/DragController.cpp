#include "Krystal.Booey/Page/DragController.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/DataTransfer.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/DOM/TextEvent.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Editing/Markup.hpp"
#include "Krystal.Booey/Editing/MoveSelectionCommand.hpp"
#include "Krystal.Booey/Editing/ReplaceSelectionCommand.hpp"
#include "Krystal.Booey/Editing/VisiblePosition.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLAnchorElement.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/HTMLTextFormControlElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Loader/FrameLoadRequest.hpp"
#include "Krystal.Booey/Page/DragState.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Platform/DragData.hpp"
#include "Krystal.Booey/Platform/Pasteboard.hpp"
#include "Krystal.Booey/Platform/PlatformMouseEvent.hpp"
#include "Krystal.Booey/Rendering/HitTestRequest.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/RenderImage.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo
{
  // FIXME: Editable links, which are live only as the editable link behavior setting says, with that
  // setting; data detector links, which are Cocoa's.
  bool IsDraggableLink(const dom::Element &element) noexcept
  {
    if (auto *anchorElement = DynamicDowncast<html::HTMLAnchorElement>(element))
    {
      return anchorElement->HasAttribute(u8"href") && !anchorElement->HasEditableStyle();
    }
    if (element.NamespaceURI().View() == infra::Namespaces::SVG && element.LocalName().View() == u8"a")
    {
      return css::IsLink(element);
    }
    return false;
  }

  namespace
  {
    // FIXME: The keys held (currentStateOfModifierKeys), once the host reports them outside its events.
    KRYS_NODISCARD PlatformMouseEvent CreateMouseEvent(const DragData &dragData) noexcept
    {
      return PlatformMouseEvent(DoublePoint(dragData.ClientPosition()),
                                DoublePoint(dragData.GlobalPosition()), MouseButton::Left,
                                PlatformEvent::Type::MouseMoved, 0, {}, html::EventLoop::Current().Now(), 0,
                                {}, ForceAtClick, SyntheticClickType::NoTap);
    }

    KRYS_NODISCARD RefPtr<dom::DocumentFragment> DocumentFragmentFromDragData(const DragData &dragData,
                                                                              LocalFrame &frame,
                                                                              const dom::SimpleRange &context,
                                                                              bool allowPlainText,
                                                                              bool &chosePlainText) noexcept
    {
      chosePlainText = false;

      if (dragData.ContainsCompatibleContent())
      {
        if (RefPtr<dom::DocumentFragment> fragment = frame.Document()->GetEditor().WebContentFromPasteboard(
              *Pasteboard::Create(dragData), context, allowPlainText, chosePlainText))
        {
          return fragment;
        }

        if (dragData.ContainsURL(DragData::DoNotConvertFilenames))
        {
          utf8_string title;
          utf8_string url = dragData.AsURL(DragData::DoNotConvertFilenames, &title);
          if (!url.empty())
          {
            Ref<dom::Document> document = ShareRef(context.Start.Container->NodeDocument());
            auto anchor = CreateHTMLElement(*document, u8"a");
            dom::ElementAlgorithms::SetAttributeValue(*anchor, u8"href", dom::DOMString(url));
            if (title.empty())
            {
              // Try the plain text first because the URL might be normalized or escaped.
              if (dragData.ContainsPlainText())
              {
                title = dragData.AsPlainText();
              }
              if (title.empty())
              {
                title = url;
              }
            }
            (void)anchor->AppendChild(*document->CreateTextNode(krys::move(title)));
            Ref<dom::DocumentFragment> fragment = document->CreateDocumentFragment();
            (void)fragment->AppendChild(*anchor);
            return fragment;
          }
        }
      }
      if (allowPlainText && dragData.ContainsPlainText())
      {
        chosePlainText = true;
        return CreateFragmentFromText(context, dragData.AsPlainText());
      }

      return nullptr;
    }

    KRYS_NODISCARD bool DragIsHandledByDocument(DragHandlingMethod dragHandlingMethod) noexcept
    {
      return dragHandlingMethod != DragHandlingMethod::None
             && dragHandlingMethod != DragHandlingMethod::PageLoad;
    }

    KRYS_NODISCARD RefPtr<html::HTMLInputElement> AsFileInput(dom::Node &node) noexcept
    {
      auto *inputElement = DynamicDowncast<html::HTMLInputElement>(node);
      if (!inputElement)
      {
        return nullptr;
      }

      // If this is a button inside of the a file input, move up to the file input.
      if (inputElement->IsTextButton())
      {
        inputElement = DynamicDowncast<html::HTMLInputElement>(dom::TreeQueries::ShadowHost(*inputElement));
      }

      return inputElement && inputElement->IsFileUpload() ? ShareRefPtr(inputElement) : nullptr;
    }

    // This can return null if an empty document is loaded.
    KRYS_NODISCARD RefPtr<dom::Element> ElementUnderMouse(dom::Document &documentUnderMouse,
                                                          const IntPoint &p) noexcept
    {
      auto *frame = documentUnderMouse.Frame();
      float zoomFactor = frame ? frame->PageZoomFactor() : 1;
      LayoutPoint point(LayoutUnit(p.X() * zoomFactor), LayoutUnit(p.Y() * zoomFactor));

      HitTestResult result(point);
      documentUnderMouse.HitTest(HitTestRequest(), result);

      auto *node = result.InnerNode();
      if (!node)
      {
        return nullptr;
      }

      // FIXME: Use parentElementInComposedTree here.
      auto *element = DynamicDowncast<dom::Element>(*node);
      if (!element)
      {
        element = node->ParentElement();
      }
      auto *host = dom::TreeQueries::ShadowHost(*element);
      return ShareRefPtr(host ? host : element);
    }

    KRYS_NODISCARD bool SetSelectionToDragCaret(LocalFrame *frame, VisibleSelection &dragCaret,
                                                const IntPoint &point) noexcept
    {
      Ref protectedFrame = ShareRef(*frame);
      auto &selection = frame->Document()->Selection();
      selection.SetSelection(dragCaret);
      if (selection.Selection().IsNone())
      {
        dragCaret = VisibleSelection(frame->VisiblePositionForPoint(point));
        selection.SetSelection(dragCaret);
      }
      return !selection.IsNone() && selection.Selection().IsContentEditable();
    }

    KRYS_NODISCARD Maybe<DragOperation>
      DefaultOperationForDrag(EnumSet<DragOperation> sourceOperationMask) noexcept
    {
      // This is designed to match IE's operation fallback for the case where
      // the page calls preventDefault() in a drag event but doesn't set dropEffect.
      if (sourceOperationMask.ContainsAll(AnyDragOperation()))
      {
        return DragOperation::Copy;
      }
      if (sourceOperationMask.IsEmpty())
      {
        return null;
      }
      if (sourceOperationMask.Contains(DragOperation::Move))
      {
        return DragOperation::Move;
      }
      if (sourceOperationMask.Contains(DragOperation::Generic))
      {
        return DragController::PlatformGenericDragOperation();
      }
      if (sourceOperationMask.Contains(DragOperation::Copy))
      {
        return DragOperation::Copy;
      }
      if (sourceOperationMask.Contains(DragOperation::Link))
      {
        return DragOperation::Link;
      }

      // FIXME: Does IE really return "generic" even if no operations were allowed by the source?
      return DragOperation::Generic;
    }

    // FIXME: Multi-representation images, which are Cocoa's; the setting to load images automatically
    // (loadsImagesAutomatically), whose default (on) this takes, once there's one.
    KRYS_NODISCARD bool ImageElementIsDraggable(const html::HTMLImageElement &, const LocalFrame &) noexcept
    {
      return true;
    }

    KRYS_NODISCARD CachedImage *GetCachedImage(dom::Element &element) noexcept
    {
      auto *renderImage = DynamicDowncast<RenderImage>(element.Renderer());
      return renderImage ? renderImage->GetCachedImage() : nullptr;
    }

    KRYS_NODISCARD Image *GetImage(dom::Element &element) noexcept
    {
      auto *cachedImage = GetCachedImage(element);
      return (cachedImage && !cachedImage->ErrorOccurred()) ? cachedImage->GetImage() : nullptr;
    }

    void SelectElement(dom::Element &element) noexcept
    {
      if (element.NodeDocument().Frame())
      {
        if (auto range = dom::MakeRangeSelectingNode(element))
        {
          element.NodeDocument().Selection().SetSelection(VisibleSelection(*range));
        }
      }
    }

    KRYS_NODISCARD IntPoint DragLocForDHTMLDrag(const IntPoint &mouseDraggedPoint, const IntPoint &dragOrigin,
                                                const IntPoint &dragImageOffset, bool isLinkImage) noexcept
    {
      // dragImageOffset is the cursor position relative to the lower-left corner of the image.
      const int32 yOffset = -dragImageOffset.Y();

      if (isLinkImage)
      {
        return IntPoint(mouseDraggedPoint.X() - dragImageOffset.X(), mouseDraggedPoint.Y() + yOffset);
      }

      return IntPoint(dragOrigin.X() - dragImageOffset.X(), dragOrigin.Y() + yOffset);
    }

    /// @brief Where @p element is in the root view: its renderer's bounding box, in window coordinates.
    KRYS_NODISCARD IntRect BoundsInRootViewSpace(dom::Element &element) noexcept
    {
      auto *view = element.NodeDocument().View();
      auto *renderer = element.Renderer();
      if (!view || !renderer)
      {
        return {};
      }
      return view->ContentsToWindow(renderer->AbsoluteBoundingBoxRect());
    }

    KRYS_NODISCARD IntPoint DragLocForSelectionDrag(LocalFrame &src) noexcept
    {
      IntRect draggingRect = EnclosingIntRect(src.Document()->Selection().SelectionBounds());
      int32 xpos = draggingRect.MaxX();
      xpos = draggingRect.X() < xpos ? draggingRect.X() : xpos;
      int32 ypos = draggingRect.MaxY();
      ypos = draggingRect.Y() < ypos ? draggingRect.Y() : ypos;
      return IntPoint(xpos, ypos);
    }

    KRYS_NODISCARD RefPtr<dom::Element> ContainingLinkElement(dom::Element &element) noexcept
    {
      for (auto *currentElement = &element; currentElement; currentElement = currentElement->ParentElement())
      {
        if (css::IsLink(*currentElement))
        {
          return ShareRefPtr(currentElement);
        }
      }
      return nullptr;
    }

    /// @brief @p text with its runs of spaces and newlines made single spaces, and trimmed.
    KRYS_NODISCARD utf8_string SimplifyWhiteSpace(utf8_stringview text) noexcept
    {
      utf8_string result;
      bool pendingSpace = false;
      for (auto c : text)
      {
        if (c == u8' ' || c == u8'\t' || c == u8'\n' || c == u8'\r')
        {
          pendingSpace = !result.empty();
          continue;
        }
        if (pendingSpace)
        {
          result += u8' ';
          pendingSpace = false;
        }
        result += c;
      }
      return result;
    }
  }

  DragController::DragController(Page &page, UniquePtr<DragClient> &&client) noexcept
      : _page(page), _client(krys::move(client))
  {
  }

  DragController::~DragController() noexcept = default;

  bool DragController::MouseIsOverFileInput() const noexcept
  {
    return _fileInputElementUnderMouse != nullptr;
  }

  DragOperation DragController::PlatformGenericDragOperation() noexcept
  {
    return DragOperation::Move;
  }

  bool DragController::IsCopyKeyDown(const DragData &dragData) noexcept
  {
    return dragData.Flags().Contains(DragApplicationFlags::IsCopyKeyDown);
  }

  // FIXME: This logic is incomplete
  Maybe<DragOperation> DragController::DragOperationFor(const DragData &dragData) noexcept
  {
    if (dragData.ContainsURL())
    {
      return DragOperation::Copy;
    }

    return null;
  }

  bool DragController::DragIsMove(FrameSelection &selection, const DragData &dragData) noexcept
  {
    const VisibleSelection &visibleSelection = selection.Selection();
    return _documentUnderMouse == _dragInitiator && visibleSelection.IsContentEditable()
           && visibleSelection.IsRange() && !IsCopyKeyDown(dragData);
  }

  void DragController::ClearDragCaret() noexcept
  {
    _page.GetDragCaretController().Clear();
  }

  void DragController::DragEnded() noexcept
  {
    _dragInitiator = nullptr;
    _didInitiateDrag = false;
    _documentUnderMouse = nullptr;
    ClearDragCaret();

    Client().DragEnded();
  }

  void DragController::DragExited(LocalFrame &frame, DragData &&dragData) noexcept
  {
    DisallowFileAccessIfNeeded(dragData);
    if (frame.View())
    {
      frame.GetEventHandler().CancelDragAndDrop(CreateMouseEvent(dragData), Pasteboard::Create(dragData),
                                                dragData.DraggingSourceOperationMask(),
                                                dragData.ContainsFiles());
    }
    MouseMovedIntoDocument(nullptr);
    if (RefPtr<html::HTMLInputElement> fileInput = std::exchange(_fileInputElementUnderMouse, nullptr))
    {
      fileInput->SetCanReceiveDroppedFiles(false);
    }
  }

  // FIXME: Opening external URLs (shouldOpenExternalURLsPolicyToPropagate), with external URLs.
  bool DragController::PerformDragOperation(DragData &&dragData) noexcept
  {
    SetForScope isPerformingDrop(_isPerformingDrop, true);
    Ref<LocalFrame> mainFrame = ShareRef(_page.MainFrame());
    if (!mainFrame->Document())
    {
      return false;
    }

    IgnoreSelectionChangeForScope ignoreSelectionChanges {*mainFrame};

    _documentUnderMouse = ShareRefPtr(mainFrame->DocumentAtPoint(dragData.ClientPosition()));

    DisallowFileAccessIfNeeded(dragData);

    if (_dragDestinationActionMask.Contains(DragDestinationAction::DHTML)
        && DragIsHandledByDocument(_dragHandlingMethod))
    {
      Client().WillPerformDragDestinationAction(DragDestinationAction::DHTML, dragData);
      bool preventedDefault = false;
      if (mainFrame->View())
      {
        preventedDefault = mainFrame->GetEventHandler().PerformDragAndDrop(
          CreateMouseEvent(dragData), Pasteboard::Create(dragData), dragData.DraggingSourceOperationMask(),
          dragData.ContainsFiles());
      }
      if (preventedDefault)
      {
        ClearDragCaret();
        _documentUnderMouse = nullptr;
        return true;
      }
    }

    if (_dragDestinationActionMask.Contains(DragDestinationAction::Edit) && ConcludeEditDrag(dragData))
    {
      Client().DidConcludeEditDrag();
      _documentUnderMouse = nullptr;
      ClearDragCaret();
      return true;
    }

    _documentUnderMouse = nullptr;
    ClearDragCaret();

    if (!OperationForLoad(dragData))
    {
      return false;
    }

    auto urlString = dragData.AsURL();
    if (urlString.empty())
    {
      return false;
    }

    Client().WillPerformDragDestinationAction(DragDestinationAction::Load, dragData);
    FrameLoadRequest frameLoadRequest(ShareRef(*mainFrame->Document()), ResourceRequest(url::URL(urlString)),
                                      dom::DOMStringAtom::Null());
    frameLoadRequest.SetIsRequestFromClientOrUserInput(true);
    mainFrame->Loader().Load(krys::move(frameLoadRequest));
    return true;
  }

  void DragController::MouseMovedIntoDocument(RefPtr<dom::Document> &&newDocument) noexcept
  {
    if (_documentUnderMouse == newDocument)
    {
      return;
    }

    // If we were over another document clear the selection
    if (_documentUnderMouse)
    {
      ClearDragCaret();
    }
    _documentUnderMouse = krys::move(newDocument);
  }

  Maybe<DragOperation> DragController::DragEnteredOrUpdated(LocalFrame &frame, DragData &&dragData) noexcept
  {
    auto point = frame.View()->WindowToContents(dragData.ClientPosition());
    auto hitTestResult = HitTestResult(LayoutPoint(point));
    if (frame.Document() && frame.Document()->GetRenderView())
    {
      hitTestResult = frame.GetEventHandler().HitTestResultAtPoint(
        LayoutPoint(point),
        {HitTestRequest::Type::ReadOnly, HitTestRequest::Type::Active,
         HitTestRequest::Type::DisallowUserAgentShadowContent, HitTestRequest::Type::AllowChildFrameContent});
    }

    MouseMovedIntoDocument(hitTestResult.InnerNode() ? ShareRefPtr(&hitTestResult.InnerNode()->NodeDocument())
                                                     : nullptr);

    _dragDestinationActionMask = dragData.DragDestinationActionMask();
    if (_dragDestinationActionMask.IsEmpty())
    {
      ClearDragCaret(); // FIXME: Why not call MouseMovedIntoDocument(nullptr)?
      return null;
    }

    DisallowFileAccessIfNeeded(dragData);

    Maybe<DragOperation> dragOperation;
    _dragHandlingMethod = TryDocumentDrag(frame, dragData, _dragDestinationActionMask, dragOperation);
    if (_dragHandlingMethod == DragHandlingMethod::None
        && _dragDestinationActionMask.Contains(DragDestinationAction::Load))
    {
      dragOperation = OperationForLoad(dragData);
      if (dragOperation)
      {
        _dragHandlingMethod = DragHandlingMethod::PageLoad;
      }
    }
    else if (_dragHandlingMethod == DragHandlingMethod::SetColor)
    {
      dragOperation = DragOperation::Copy;
    }

    return dragOperation;
  }

  void DragController::DisallowFileAccessIfNeeded(DragData &dragData) noexcept
  {
    auto *frame = _documentUnderMouse ? _documentUnderMouse->Frame() : nullptr;
    if (frame && !frame->GetEventHandler().CanDropCurrentlyDraggedImageAsFile())
    {
      dragData.DisallowFileAccess();
    }
  }

  // FIXME: Whether the document under the mouse can receive the initiator's data (canReceiveDragData), once
  // there are security origins.
  DragHandlingMethod DragController::TryDocumentDrag(LocalFrame &frame, const DragData &dragData,
                                                     EnumSet<DragDestinationAction> destinationActionMask,
                                                     Maybe<DragOperation> &dragOperation) noexcept
  {
    if (!_documentUnderMouse)
    {
      return DragHandlingMethod::None;
    }

    bool isHandlingDrag = false;
    if (destinationActionMask.Contains(DragDestinationAction::DHTML))
    {
      isHandlingDrag = TryDHTMLDrag(frame, dragData, dragOperation);
      // Do not continue if _documentUnderMouse has been reset by TryDHTMLDrag.
      // TryDHTMLDrag fires dragenter event. The event listener that listens
      // to this event may create a nested message loop (open a modal dialog),
      // which could process dragleave event and reset _documentUnderMouse in
      // DragExited.
      if (!_documentUnderMouse)
      {
        return DragHandlingMethod::None;
      }
    }

    // It's unclear why this check is after TryDHTMLDrag.
    // We send drag events in TryDHTMLDrag and that may be the reason.
    RefPtr<LocalFrameView> frameView = ShareRefPtr(_documentUnderMouse->View());
    if (!frameView)
    {
      return DragHandlingMethod::None;
    }

    if (isHandlingDrag)
    {
      ClearDragCaret();
      _numberOfItemsToBeAccepted = dragData.NumberOfFiles();
      return DragHandlingMethod::NonDefault;
    }

    if (destinationActionMask.Contains(DragDestinationAction::Edit) && CanProcessDrag(dragData))
    {
      if (dragData.ContainsColor())
      {
        dragOperation = DragOperation::Generic;
        return DragHandlingMethod::SetColor;
      }

      IntPoint point = frameView->WindowToContents(dragData.ClientPosition());
      RefPtr<dom::Element> element = ElementUnderMouse(*_documentUnderMouse, point);
      if (!element)
      {
        return DragHandlingMethod::None;
      }

      RefPtr<html::HTMLInputElement> elementAsFileInput = AsFileInput(*element);
      if (_fileInputElementUnderMouse != elementAsFileInput)
      {
        if (_fileInputElementUnderMouse)
        {
          _fileInputElementUnderMouse->SetCanReceiveDroppedFiles(false);
        }
        _fileInputElementUnderMouse = elementAsFileInput;
      }

      if (!_fileInputElementUnderMouse)
      {
        _page.GetDragCaretController().SetCaretPosition(
          _documentUnderMouse->Frame()->VisiblePositionForPoint(point));
      }
      else
      {
        ClearDragCaret();
      }

      auto &innerDocument = element->NodeDocument();
      dragOperation =
        DragIsMove(innerDocument.Selection(), dragData) ? DragOperation::Move : DragOperation::Copy;

      uint32 numberOfFiles = dragData.NumberOfFiles();
      if (RefPtr<html::HTMLInputElement> fileInput = _fileInputElementUnderMouse)
      {
        if (fileInput->IsDisabledFormControl())
        {
          _numberOfItemsToBeAccepted = 0;
        }
        else if (fileInput->Multiple())
        {
          _numberOfItemsToBeAccepted = numberOfFiles;
        }
        else if (numberOfFiles > 1)
        {
          _numberOfItemsToBeAccepted = 0;
        }
        else
        {
          _numberOfItemsToBeAccepted = 1;
        }

        if (!_numberOfItemsToBeAccepted)
        {
          dragOperation = null;
        }
        fileInput->SetCanReceiveDroppedFiles(_numberOfItemsToBeAccepted);
      }
      else
      {
        // We are not over a file input element. The dragged item(s) will loaded into the view,
        // dropped as text paths on other input elements, or handled by script on the page.
        _numberOfItemsToBeAccepted = numberOfFiles;
      }

      if (_fileInputElementUnderMouse)
      {
        return DragHandlingMethod::UploadFile;
      }

      if (_page.GetDragCaretController().IsContentRichlyEditable())
      {
        return DragHandlingMethod::EditRichText;
      }

      return DragHandlingMethod::EditPlainText;
    }

    // We are not over an editable region. Make sure we're clearing any prior drag cursor.
    ClearDragCaret();
    if (RefPtr<html::HTMLInputElement> fileInput = std::exchange(_fileInputElementUnderMouse, nullptr))
    {
      fileInput->SetCanReceiveDroppedFiles(false);
    }
    return DragHandlingMethod::None;
  }

  EnumSet<DragSourceAction> DragController::DelegateDragSourceAction(const IntPoint &rootViewPoint) noexcept
  {
    _dragSourceAction = Client().DragSourceActionMaskForPoint(rootViewPoint);
    return _dragSourceAction;
  }

  // FIXME: Plugin documents, once there are plugins.
  Maybe<DragOperation> DragController::OperationForLoad(const DragData &dragData) noexcept
  {
    auto *document = _page.MainFrame().DocumentAtPoint(dragData.ClientPosition());

    if (document && (_didInitiateDrag || document->HasEditableStyle()))
    {
      return null;
    }
    return DragOperationFor(dragData);
  }

  bool DragController::DispatchTextInputEventFor(LocalFrame *innerFrame, const DragData &dragData) noexcept
  {
    auto &dragCaretController = _page.GetDragCaretController();
    krys_debug_assert(dragCaretController.HasCaret());
    utf8_string text = dragCaretController.IsContentRichlyEditable() ? utf8_string() : dragData.AsPlainText();
    auto target = innerFrame->Document()->GetEditor().FindEventTargetFrom(
      VisibleSelection(dragCaretController.CaretPosition()));
    // FIXME: What guarantees target is not null?
    auto event = dom::TextEvent::CreateForDrop(text);
    (void)target->DispatchEvent(*event);
    return !event->DefaultPrevented();
  }

  // FIXME: Two-step drops (the editor client's performTwoStepDrop), which are iOS's.
  bool DragController::ConcludeEditDrag(const DragData &dragData) noexcept
  {
    RefPtr<html::HTMLInputElement> fileInput = _fileInputElementUnderMouse;
    if (RefPtr<html::HTMLInputElement> fileInputUnderMouse =
          std::exchange(_fileInputElementUnderMouse, nullptr))
    {
      fileInputUnderMouse->SetCanReceiveDroppedFiles(false);
    }

    if (!_documentUnderMouse)
    {
      return false;
    }

    IntPoint point = _documentUnderMouse->View()->WindowToContents(dragData.ClientPosition());
    RefPtr<dom::Element> element = ElementUnderMouse(*_documentUnderMouse, point);
    if (!element)
    {
      return false;
    }
    RefPtr<LocalFrame> innerFrame = ShareRefPtr(element->NodeDocument().Frame());
    krys_debug_assert(innerFrame);

    auto &dragCaretController = _page.GetDragCaretController();
    if (dragCaretController.HasCaret() && !DispatchTextInputEventFor(innerFrame.get(), dragData))
    {
      return true;
    }

    // FIXME: Dropping colours, with colours on the pasteboard.
    if (dragData.ContainsColor())
    {
      return false;
    }

    if (dragData.ContainsFiles() && fileInput)
    {
      // fileInput should be the element we hit tested for, unless it was made
      // display:none in a drop event handler.
      krys_debug_assert(fileInput.get() == element.get() || !fileInput->Renderer());
      if (fileInput->IsDisabledFormControl())
      {
        return false;
      }

      return fileInput->ReceiveDroppedFiles(dragData);
    }

    if (!CanProcessDrag(dragData))
    {
      return false;
    }

    VisibleSelection dragCaret(dragCaretController.CaretPosition());
    auto range = dragCaret.ToNormalizedRange();
    auto &innerSelection = innerFrame->Document()->Selection();
    RefPtr<dom::Element> rootEditableElement = ShareRefPtr(innerSelection.Selection().RootEditableElement());

    // For range to be null a client must have done something bad while
    // manually controlling drag behaviour
    if (!range)
    {
      return false;
    }

    auto &editor = innerFrame->Document()->GetEditor();
    bool isMove = DragIsMove(innerSelection, dragData);
    if (isMove || dragCaret.IsContentRichlyEditable())
    {
      bool chosePlainText = false;
      RefPtr<dom::DocumentFragment> fragment =
        DocumentFragmentFromDragData(dragData, *innerFrame, *range, true, chosePlainText);
      if (!fragment || !editor.ShouldInsertFragment(*fragment, range))
      {
        return false;
      }

      Client().WillPerformDragDestinationAction(DragDestinationAction::Edit, dragData);

      if (isMove)
      {
        // NSTextView behavior is to always smart delete on moving a selection,
        // but only to smart insert if the selection granularity is word granularity.
        bool smartDelete = editor.SmartInsertDeleteEnabled();
        bool smartInsert = smartDelete && innerSelection.Granularity() == TextGranularity::WordGranularity
                           && dragData.CanSmartReplace();
        MoveSelectionCommand::Create(ShareRef(*fragment), dragCaret.Base(), smartInsert, smartDelete)
          ->Apply();
      }
      else
      {
        if (SetSelectionToDragCaret(innerFrame.get(), dragCaret, point))
        {
          EnumSet<ReplaceSelectionCommand::CommandOption> options {
            ReplaceSelectionCommand::CommandOption::SelectReplacement,
            ReplaceSelectionCommand::CommandOption::PreventNesting};
          if (dragData.CanSmartReplace())
          {
            options.Add(ReplaceSelectionCommand::CommandOption::SmartReplace);
          }
          if (chosePlainText || dragData.ShouldMatchStyleOnDrop())
          {
            options.Add(ReplaceSelectionCommand::CommandOption::MatchStyle);
          }
          ReplaceSelectionCommand::Create(*_documentUnderMouse, krys::move(fragment), options,
                                          EditAction::InsertFromDrop)
            ->Apply();
        }
      }
    }
    else
    {
      utf8_string text = dragData.AsPlainText();
      if (text.empty() || !editor.ShouldInsertText(text, range))
      {
        return false;
      }

      Client().WillPerformDragDestinationAction(DragDestinationAction::Edit, dragData);
      RefPtr<dom::DocumentFragment> fragment = CreateFragmentFromText(*range, text);

      if (SetSelectionToDragCaret(innerFrame.get(), dragCaret, point))
      {
        ReplaceSelectionCommand::Create(*_documentUnderMouse, krys::move(fragment),
                                        {ReplaceSelectionCommand::CommandOption::SelectReplacement,
                                         ReplaceSelectionCommand::CommandOption::MatchStyle,
                                         ReplaceSelectionCommand::CommandOption::PreventNesting},
                                        EditAction::InsertFromDrop)
          ->Apply();
      }
    }

    if (rootEditableElement)
    {
      if (auto *frame = rootEditableElement->NodeDocument().Frame())
      {
        frame->GetEventHandler().UpdateDragStateAfterEditDragIfNeeded(*rootEditableElement);
      }
    }

    return true;
  }

  // FIXME: Colour inputs (ForColorControl), with dragging colours.
  bool DragController::CanProcessDrag(const DragData &dragData) noexcept
  {
    auto &mainFrame = _page.MainFrame();
    if (!mainFrame.View() || !mainFrame.Document() || !mainFrame.Document()->GetRenderView())
    {
      return false;
    }
    IntPoint point = mainFrame.View()->WindowToContents(dragData.ClientPosition());

    HitTestResult result = mainFrame.GetEventHandler().HitTestResultAtPoint(
      LayoutPoint(point), {HitTestRequest::Type::ReadOnly, HitTestRequest::Type::Active,
                           HitTestRequest::Type::AllowChildFrameContent});

    RefPtr<dom::Node> dragNode = ShareRefPtr(result.InnerNonSharedNode());
    if (!dragNode)
    {
      return false;
    }

    DragData::DraggingPurpose dragPurpose = DragData::DraggingPurpose::ForEditing;
    if (AsFileInput(*dragNode))
    {
      dragPurpose = DragData::DraggingPurpose::ForFileUpload;
    }

    if (!dragData.ContainsCompatibleContent(dragPurpose))
    {
      return false;
    }

    if (dragPurpose == DragData::DraggingPurpose::ForFileUpload)
    {
      return true;
    }

    if (!dragNode->HasEditableStyle())
    {
      return false;
    }

    if (_didInitiateDrag && _documentUnderMouse == _dragInitiator && result.IsSelected())
    {
      return false;
    }

    return true;
  }

  bool DragController::TryDHTMLDrag(LocalFrame &frame, const DragData &dragData,
                                    Maybe<DragOperation> &operation) noexcept
  {
    krys_debug_assert(_documentUnderMouse);
    Ref protectedFrame = ShareRef(frame);
    if (!protectedFrame->View())
    {
      return false;
    }

    auto sourceOperationMask = dragData.DraggingSourceOperationMask();
    auto targetResponse = protectedFrame->GetEventHandler().UpdateDragAndDrop(
      CreateMouseEvent(dragData), [&dragData] { return Pasteboard::Create(dragData); }, sourceOperationMask,
      dragData.ContainsFiles());
    if (!targetResponse.Accept)
    {
      return false;
    }

    if (!targetResponse.OperationMask)
    {
      operation = DefaultOperationForDrag(sourceOperationMask);
    }
    else if (!sourceOperationMask.ContainsAny(*targetResponse.OperationMask))
    {
      // The element picked an operation which is not supported by the source.
      operation = null;
    }
    else
    {
      operation = DefaultOperationForDrag(*targetResponse.OperationMask);
    }

    return true;
  }

  // FIXME: Attachments and models, which are Cocoa's; image overlays' text, once there are image overlays;
  // colour inputs, with dragging colours.
  RefPtr<dom::Element> DragController::DraggableElement(const LocalFrame *sourceFrame,
                                                        dom::Element *startElement,
                                                        const IntPoint &dragOrigin,
                                                        DragState &state) const noexcept
  {
    state.Type = sourceFrame->Document()->Selection().Contains(LayoutPoint(dragOrigin))
                   ? EnumSet<boo::DragSourceAction> {DragSourceAction::Selection}
                   : EnumSet<boo::DragSourceAction> {};
    if (!startElement)
    {
      return nullptr;
    }

    RefPtr<dom::Element> selectionDragElement = state.Type.Contains(DragSourceAction::Selection)
                                                    && _dragSourceAction.Contains(DragSourceAction::Selection)
                                                  ? ShareRefPtr(startElement)
                                                  : nullptr;

    for (auto *element = startElement; element;
         element = dom::TreeQueries::ParentOrShadowHostElement(*element))
    {
      auto *renderer = element->Renderer();
      if (!renderer)
      {
        continue;
      }

      UserDrag dragMode = renderer->Style().GetUserDrag();
      if (_dragSourceAction.Contains(DragSourceAction::DHTML) && dragMode == UserDrag::Element)
      {
        state.Type = DragSourceAction::DHTML;
        return ShareRefPtr(element);
      }
      if (dragMode == UserDrag::Auto)
      {
        if (auto *image = DynamicDowncast<html::HTMLImageElement>(*element);
            image && _dragSourceAction.Contains(DragSourceAction::Image)
            && ImageElementIsDraggable(*image, *sourceFrame))
        {
          state.Type.Add(DragSourceAction::Image);
          return ShareRefPtr(element);
        }
        if (_dragSourceAction.Contains(DragSourceAction::Link) && IsDraggableLink(*element))
        {
          state.Type.Add(DragSourceAction::Link);
          return ShareRefPtr(element);
        }
      }
    }

    // We either have nothing to drag or we have a selection and we're not over a draggable element.
    return selectionDragElement;
  }

  // FIXME: Whether the source's document may display the link (canDisplay), once there are security origins.
  void DragController::PrepareForDragStart(LocalFrame &source, EnumSet<boo::DragSourceAction> actionMask,
                                           dom::Element &element, dom::DataTransfer &dataTransfer,
                                           const IntPoint &dragOrigin) const noexcept
  {
    Ref protectedSource = ShareRef(source);
    auto hitTestResult = HitTestResultForDragStart(source, element, dragOrigin);
    if (!hitTestResult)
    {
      return;
    }

    auto &pasteboard = dataTransfer.GetPasteboard();
    auto &editor = source.Document()->GetEditor();
    if (actionMask == EnumSet<boo::DragSourceAction> {DragSourceAction::Selection})
    {
      if (html::EnclosingTextFormControl(source.Document()->Selection().Selection().Start()))
      {
        pasteboard.WritePlainText(editor.SelectedTextForDataTransfer(), Pasteboard::CannotSmartReplace);
      }
      else
      {
        editor.WriteSelectionToPasteboard(pasteboard);
      }
      return;
    }

    auto *image = GetImage(element);
    auto imageURL = hitTestResult->AbsoluteImageURL();
    if (actionMask.Contains(DragSourceAction::Image) && !imageURL.IsEmpty() && image && !image->IsNull())
    {
      editor.WriteImageToPasteboard(pasteboard, element, imageURL, {});
      return;
    }

    auto linkURL = hitTestResult->AbsoluteLinkURL();
    if (actionMask.Contains(DragSourceAction::Link) && !linkURL.IsEmpty())
    {
      editor.CopyURL(linkURL, SimplifyWhiteSpace(hitTestResult->TextContent()), pasteboard);
    }
  }

  Maybe<HitTestResult> DragController::HitTestResultForDragStart(LocalFrame &source, dom::Element &element,
                                                                 const IntPoint &location) const noexcept
  {
    if (!source.View() || !source.Document() || !source.Document()->GetRenderView())
    {
      return null;
    }

    auto hitTestResult = source.GetEventHandler().HitTestResultAtPoint(
      LayoutPoint(location), {HitTestRequest::Type::ReadOnly, HitTestRequest::Type::Active,
                              HitTestRequest::Type::AllowChildFrameContent});

    bool sourceContainsHitNode =
      hitTestResult.InnerNode()
      && dom::TreeQueries::IsShadowIncludingInclusiveAncestor(element, *hitTestResult.InnerNode());
    if (!sourceContainsHitNode)
    {
      // The original node being dragged isn't under the drag origin anymore... maybe it was
      // hidden or moved out from under the cursor. Regardless, we don't want to start a drag on
      // something that's not actually under the drag origin.
      return null;
    }

    return hitTestResult;
  }

  // FIXME: Touch drags, which upstream refuses on GTK, with touch drags; whether the source's document may
  // display the link (canDisplay), once there are security origins. The page has no device scale factor of
  // its own: the document's is used.
  bool DragController::StartDrag(LocalFrame &src, const DragState &state,
                                 EnumSet<DragOperation> sourceOperationMask,
                                 const PlatformMouseEvent &dragEvent, const IntPoint &dragOrigin,
                                 HasNonDefaultPasteboardData hasData) noexcept
  {
    if (!state.Source)
    {
      return false;
    }

    Ref protectedSrc = ShareRef(src);
    auto hitTestResult = HitTestResultForDragStart(src, *state.Source, dragOrigin);
    if (!hitTestResult)
    {
      return false;
    }

    auto linkURL = hitTestResult->AbsoluteLinkURL();
    auto imageURL = hitTestResult->AbsoluteImageURL();

    IntPoint mouseDraggedPoint = src.View()->WindowToContents(FlooredIntPoint(dragEvent.Position()));

    _draggingImageURL = url::URL();
    _sourceDragOperationMask = sourceOperationMask;

    DragImage dragImage;
    IntPoint dragLoc(0, 0);
    IntPoint dragImageOffset(0, 0);

    krys_debug_assert(state.DataTransfer);

    Ref<dom::DataTransfer> dataTransfer = ShareRef(*state.DataTransfer);
    if (state.Type == EnumSet<boo::DragSourceAction> {DragSourceAction::DHTML})
    {
      dragImage = DragImage {dataTransfer->CreateDragImage(src.Document(), dragImageOffset)};
      // We allow DHTML/JS to set the drag image, even if its a link, image or text we're dragging.
      // This is in the spirit of the IE API, which allows overriding of pasteboard data and DragOp.
      if (dragImage)
      {
        dragLoc = DragLocForDHTMLDrag(mouseDraggedPoint, dragOrigin, dragImageOffset, !linkURL.IsEmpty());
        _dragOffset = dragImageOffset;
      }
    }

    if (state.Type == EnumSet<boo::DragSourceAction> {DragSourceAction::Selection} || !imageURL.IsEmpty()
        || !linkURL.IsEmpty())
    {
      // Selection, image, and link drags receive a default set of allowed drag operations.
      _sourceDragOperationMask.Add({DragOperation::Generic, DragOperation::Copy});
    }

    krys_debug_assert(state.Source);
    Ref<dom::Element> element = ShareRef(*state.Source);

    auto *image = GetImage(*element);
    auto &editor = src.Document()->GetEditor();
    auto &selection = src.Document()->Selection();
    if (state.Type == EnumSet<boo::DragSourceAction> {DragSourceAction::Selection})
    {
      if (hasData == HasNonDefaultPasteboardData::No)
      {
        if (selection.Selection().IsNone())
        {
          // The page may have cleared out the selection in the dragstart handler, in which case we should
          // bail out of the drag, since there is no content to write to the pasteboard.
          return false;
        }

        // FIXME: This entire block is almost identical to the code in Editor::copy, and the code should be
        // shared.
        if (html::EnclosingTextFormControl(selection.Selection().Start()))
        {
          dataTransfer->GetPasteboard().WritePlainText(editor.SelectedTextForDataTransfer(),
                                                       Pasteboard::CannotSmartReplace);
        }
        else
        {
          editor.WriteSelectionToPasteboard(dataTransfer->GetPasteboard());
        }
      }
      Client().WillPerformDragSourceAction(DragSourceAction::Selection, dragOrigin, *dataTransfer);
      if (!dragImage)
      {
        auto [dragImageRef] = CreateDragImageForSelection(src);
        dragImage = DragImage {DissolveDragImageToFraction(dragImageRef, DragImageAlpha)};
        dragLoc = DragLocForSelectionDrag(src);
        _dragOffset = IntPoint(dragOrigin.X() - dragLoc.X(), dragOrigin.Y() - dragLoc.Y());
      }

      if (!dragImage)
      {
        return false;
      }

      DoSystemDrag(krys::move(dragImage), dragLoc, dragOrigin, src, state);
      return true;
    }

    if (!imageURL.IsEmpty() && image && !image->IsNull()
        && _dragSourceAction.Contains(DragSourceAction::Image))
    {
      if (hasData == HasNonDefaultPasteboardData::No)
      {
        _draggingImageURL = imageURL;
        if (element->IsContentRichlyEditable())
        {
          SelectElement(*element);
        }
        DeclareAndWriteDragImage(*dataTransfer, *element, !linkURL.IsEmpty() ? linkURL : imageURL,
                                 hitTestResult->AltDisplayString());
      }

      Client().WillPerformDragSourceAction(DragSourceAction::Image, dragOrigin, *dataTransfer);

      if (!dragImage)
      {
        DoImageDrag(*element, dragOrigin, hitTestResult->ImageRect(), src, _dragOffset, state);
      }
      else
      {
        // DHTML defined drag image
        DoSystemDrag(krys::move(dragImage), dragLoc, dragOrigin, src, state);
      }

      return true;
    }

    if (!linkURL.IsEmpty() && _dragSourceAction.Contains(DragSourceAction::Link))
    {
      utf8_string textContentWithSimplifiedWhiteSpace = SimplifyWhiteSpace(hitTestResult->TextContent());

      if (hasData == HasNonDefaultPasteboardData::No)
      {
        // Simplify whitespace so the title put on the dataTransfer resembles what the user sees
        // on the web page. This includes replacing newlines with spaces.
        editor.CopyURL(linkURL, textContentWithSimplifiedWhiteSpace, dataTransfer->GetPasteboard());
      }

      const VisibleSelection &sourceSelection = selection.Selection();
      if (sourceSelection.IsCaret() && sourceSelection.IsContentEditable())
      {
        // a user can initiate a drag on a link without having any text
        // selected.  In this case, we should expand the selection to
        // the enclosing anchor element
        dom::Position pos = sourceSelection.Base();
        if (RefPtr<dom::Element> node = EnclosingAnchorElement(pos))
        {
          selection.SetSelection(VisibleSelection::SelectionFromContentsOfNode(node.get()));
        }
      }

      Client().WillPerformDragSourceAction(DragSourceAction::Link, dragOrigin, *dataTransfer);
      if (!dragImage)
      {
        float deviceScaleFactor = src.Document()->DeviceScaleFactor();
        auto [dragImageRef] =
          CreateDragImageForLink(*element, linkURL, textContentWithSimplifiedWhiteSpace, deviceScaleFactor);
        dragImage = DragImage {dragImageRef};
        if (dragImage)
        {
          _dragOffset = DragOffsetForLinkDragImage(dragImage.Get());
          dragLoc = IntPoint(dragOrigin.X() + _dragOffset.X(), dragOrigin.Y() + _dragOffset.Y());
          dragImage =
            DragImage {PlatformAdjustDragImageForDeviceScaleFactor(dragImage.Get(), deviceScaleFactor)};
        }
      }

      DoSystemDrag(krys::move(dragImage), dragLoc, dragOrigin, src, state);
      return true;
    }

    if (state.Type == EnumSet<boo::DragSourceAction> {DragSourceAction::DHTML} && dragImage)
    {
      krys_debug_assert(_dragSourceAction.Contains(DragSourceAction::DHTML));
      Client().WillPerformDragSourceAction(DragSourceAction::DHTML, dragOrigin, *dataTransfer);
      DoSystemDrag(krys::move(dragImage), dragLoc, dragOrigin, src, state);
      return true;
    }

    return false;
  }

  void DragController::DoImageDrag(dom::Element &element, const IntPoint &dragOrigin,
                                   const IntRect &layoutRect, LocalFrame &frame, IntPoint &dragImageOffset,
                                   const DragState &state) noexcept
  {
    IntPoint mouseDownPoint = dragOrigin;
    DragImage dragImage;
    IntPoint scaledOrigin;

    auto *renderer = element.Renderer();
    if (!renderer)
    {
      return;
    }

    auto orientation = renderer->GetImageOrientation();
    float deviceScaleFactor = element.NodeDocument().DeviceScaleFactor();

    auto *image = GetImage(element);
    if (image && !layoutRect.IsEmpty() && ShouldUseCachedImageForDragImage(*image)
        && (dragImage = DragImage {CreateDragImageFromImage(image, orientation)}))
    {
      dragImage = DragImage {FitDragImageToMaxSize(dragImage.Get(), layoutRect.Size(), MaxDragImageSize())};
      IntSize fittedSize = DragImageSize(dragImage.Get());

      dragImage = DragImage {PlatformAdjustDragImageForDeviceScaleFactor(dragImage.Get(), deviceScaleFactor)};
      dragImage = DragImage {DissolveDragImageToFraction(dragImage.Get(), DragImageAlpha)};

      // Properly orient the drag image and orient it differently if it's smaller than the original.
      float scale = fittedSize.Width() / static_cast<float>(layoutRect.Width());
      float dx = scale * (layoutRect.X() - mouseDownPoint.X());
      auto originY = static_cast<float>(layoutRect.Y());
      float dy = scale * (originY - mouseDownPoint.Y());
      scaledOrigin = IntPoint(static_cast<int32>(dx + 0.5), static_cast<int32>(dy + 0.5));
    }
    else
    {
      // FIXME: The response's suggested filename (ResourceResponse::suggestedFilename), once responses
      // parse it; the host has no icon for it either way.
      if (GetCachedImage(element))
      {
        dragImage = DragImage {CreateDragImageIconForCachedImageFilename({})};
        if (dragImage)
        {
          dragImage =
            DragImage {PlatformAdjustDragImageForDeviceScaleFactor(dragImage.Get(), deviceScaleFactor)};
          scaledOrigin =
            IntPoint(DragIconRightInset - DragImageSize(dragImage.Get()).Width(), DragIconBottomInset);
        }
      }
    }

    if (!dragImage)
    {
      return;
    }

    dragImageOffset = mouseDownPoint + scaledOrigin;
    DoSystemDrag(krys::move(dragImage), dragImageOffset, dragOrigin, frame, state);
  }

  // Booey has one frame, whose view is the root view: its window coordinates are the root view's.
  void DragController::DoSystemDrag(DragImage image, const IntPoint &dragLoc, const IntPoint &eventPos,
                                    LocalFrame &frame, const DragState &state) noexcept
  {
    _didInitiateDrag = true;
    _dragInitiator = ShareRefPtr(frame.Document());
    // Protect this frame and view, as a load may occur mid drag and attempt to unload this frame
    Ref<LocalFrame> mainFrame = ShareRef(_page.MainFrame());
    RefPtr<LocalFrameView> frameView = ShareRefPtr(frame.View());

    DragItem item;
    item.Image = krys::move(image);
    krys_debug_assert(state.Type.HasExactlyOneBitSet());
    item.SourceAction = state.Type.ToSingleValue();
    item.ContainsSelection = frame.Document()->Selection().Contains(LayoutPoint(eventPos));

    item.EventPositionInContentCoordinates = eventPos;
    item.DragLocationInContentCoordinates = dragLoc;
    item.DragLocationInWindowCoordinates = frameView->ContentsToWindow(dragLoc);

    if (RefPtr<dom::Element> element = state.Source)
    {
      RefPtr<dom::Element> dataTransferImageElement = state.DataTransfer->DragImageElement();
      if (state.Type == EnumSet<boo::DragSourceAction> {DragSourceAction::DHTML})
      {
        // If the drag image has been customized, fall back to positioning the preview relative to the drag
        // event location.
        IntSize dragPreviewSize;
        if (dataTransferImageElement)
        {
          dragPreviewSize = BoundsInRootViewSpace(*dataTransferImageElement).Size();
        }
        else
        {
          dragPreviewSize = DragImageSize(item.Image.Get());
          dragPreviewSize.Scale(1 / frame.Document()->DeviceScaleFactor());
        }
        item.DragPreviewFrameInRootViewCoordinates = {item.DragLocationInWindowCoordinates, dragPreviewSize};
      }
      else
      {
        // We can position the preview using the bounds of the drag source element.
        item.DragPreviewFrameInRootViewCoordinates = BoundsInRootViewSpace(*element);
      }

      if (RefPtr<dom::Element> link = ContainingLinkElement(*element))
      {
        auto titleAttribute = link->GetAttribute(u8"title");
        if (titleAttribute && !titleAttribute->empty())
        {
          item.Title = *titleAttribute;
        }
        else if (auto *htmlLink = DynamicDowncast<html::HTMLElement>(*link))
        {
          item.Title = htmlLink->InnerText();
        }
        item.Url = frame.Document()->CompleteURL(link->GetAttribute(u8"href").value_or(dom::DOMString()));
      }
    }
    Client().StartDrag(krys::move(item), *state.DataTransfer, *mainFrame);
  }

  const IntSize &DragController::MaxDragImageSize() noexcept
  {
    static const IntSize maxDragImageSize(200, 200);
    return maxDragImageSize;
  }

  bool DragController::ShouldUseCachedImageForDragImage(const Image &image) const noexcept
  {
    return image.Size().Height() * image.Size().Width() <= MaxOriginalImageArea;
  }

  void DragController::DeclareAndWriteDragImage(dom::DataTransfer &dataTransfer, dom::Element &element,
                                                const url::URL &url, utf8_stringview label) noexcept
  {
    auto *frame = element.NodeDocument().Frame();
    krys_debug_assert(frame);
    frame->Document()->GetEditor().WriteImageToPasteboard(dataTransfer.GetPasteboard(), element, url, label);
  }

  // Manual drag caret manipulation
  void DragController::PlaceDragCaret(const IntPoint &windowPoint) noexcept
  {
    auto &mainFrame = _page.MainFrame();
    MouseMovedIntoDocument(ShareRefPtr(mainFrame.DocumentAtPoint(windowPoint)));
    if (!_documentUnderMouse)
    {
      return;
    }
    auto *frame = _documentUnderMouse->Frame();
    auto *frameView = frame->View();
    if (!frameView)
    {
      return;
    }
    IntPoint framePoint = frameView->WindowToContents(windowPoint);

    _page.GetDragCaretController().SetCaretPosition(frame->VisiblePositionForPoint(framePoint));
  }
}
