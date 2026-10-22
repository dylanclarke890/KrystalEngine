#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/ClipboardEvent.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Editing/TextIterator.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Page/EditorClient.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Platform/Pasteboard.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD utf8_stringview EventNameForClipboardEvent(ClipboardEventKind kind) noexcept
    {
      switch (kind)
      {
        case ClipboardEventKind::Copy:
        case ClipboardEventKind::CopyFont:         return EventNames::Copy;
        case ClipboardEventKind::BeforeCopy:       return EventNames::BeforeCopy;
        // FIXME: Cutting and pasting, with editing commands.
        case ClipboardEventKind::Cut:
        case ClipboardEventKind::Paste:
        case ClipboardEventKind::PasteAsPlainText:
        case ClipboardEventKind::PasteAsQuotation:
        case ClipboardEventKind::PasteFont:
        case ClipboardEventKind::BeforeCut:
        case ClipboardEventKind::BeforePaste:      break;
      }
      krys_debug_assert(false);
      return {};
    }
  }

  Editor::Editor(dom::Document &document) noexcept : _document(document)
  {
  }

  Editor::~Editor() noexcept = default;

  EditorClient *Editor::Client() const noexcept
  {
    if (auto *page = _document.GetPage())
    {
      return &page->GetEditorClient();
    }
    return nullptr;
  }

  // FIXME: Input methods (handleInputMethodKeydown, didDispatchInputMethodKeydown), with text input.
  bool Editor::HandleKeyboardEvent(dom::KeyboardEvent &event) noexcept
  {
    if (auto *client = Client())
    {
      return client->HandleKeyboardEvent(event);
    }
    return false;
  }

  // Returns whether caller should continue with "the default processing", which is the same as
  // the event handler NOT setting the return value to false
  // FIXME: The clipboard data the event's listeners write, committed to the pasteboard when they cancel a
  // copy or cut, with DataTransfer.
  bool Editor::DispatchClipboardEvent(RefPtr<dom::Element> &&target, ClipboardEventKind kind) noexcept
  {
    // FIXME: Move the target selection code here.
    if (!target)
    {
      return true;
    }

    auto event = dom::ClipboardEvent::Create(dom::DOMStringAtom(EventNameForClipboardEvent(kind)));

    (void)dom::EventDispatcher::DispatchToTarget(*event, target.get());
    bool noDefaultProcessing = event->DefaultPrevented();

    return !noDefaultProcessing;
  }

  // WinIE uses onbeforecut and onbeforepaste to enables the cut and paste menu items.  They
  // also send onbeforecopy, apparently for symmetry, but it doesn't affect the menu items.
  // We need to use onbeforecopy as a real menu enabler because we allow elements that are not
  // normally selectable to implement copy/paste (like divs, or a document body).

  // FIXME: Password fields, with form controls.
  bool Editor::CanDHTMLCopy() noexcept
  {
    return !DispatchClipboardEvent(FindEventTargetFromSelection(), ClipboardEventKind::BeforeCopy);
  }

  // FIXME: Image documents and password fields, with them.
  bool Editor::CanCopy() const noexcept
  {
    const VisibleSelection &selection = _document.Selection().Selection();
    return selection.IsRange() || !IsEditablePosition(selection.Start());
  }

  // FIXME: Password fields, with form controls.
  bool Editor::TryDHTMLCopy() noexcept
  {
    return !DispatchClipboardEvent(FindEventTargetFromSelection(), ClipboardEventKind::Copy);
  }

  RefPtr<dom::Element> Editor::FindEventTargetFrom(const VisibleSelection &selection) const noexcept
  {
    RefPtr<dom::Element> target {selection.Start().AnchorElementAncestor()};
    if (!target)
    {
      target = ShareRefPtr<dom::Element>(_document.BodyOrFrameset());
    }

    return target;
  }

  RefPtr<dom::Element> Editor::FindEventTargetFromSelection() const noexcept
  {
    return FindEventTargetFrom(_document.Selection().Selection());
  }

  // FIXME: Beeping when there's nothing to copy (the system sound manager), once the host can.
  void Editor::Copy(FromMenuOrKeyBinding fromMenuOrKeyBinding) noexcept
  {
    bool oldCopying =
      std::exchange(_copyingFromMenuOrKeyBinding, fromMenuOrKeyBinding == FromMenuOrKeyBinding::Yes);
    if (TryDHTMLCopy())
    {
      // DHTML did the whole operation
      _copyingFromMenuOrKeyBinding = oldCopying;
      return;
    }
    if (CanCopy())
    {
      PerformCutOrCopy(EditorActionSpecifier::CopyAction);
    }
    _copyingFromMenuOrKeyBinding = oldCopying;
  }

  // FIXME: Cutting (deleting the selection after copying it), with editing commands; text form controls,
  // image documents, image overlays and writing the selection as web content (writeSelectionToPasteboard),
  // with form controls, images and DataTransfer; the editor client's notifications around writing the
  // selection.
  void Editor::PerformCutOrCopy(EditorActionSpecifier action) noexcept
  {
    krys_debug_assert(action == EditorActionSpecifier::CopyAction);
    (void)action;

    Pasteboard::CreateForCopyAndPaste()->WritePlainText(SelectedTextForDataTransfer(),
                                                        Pasteboard::CannotSmartReplace);
  }

  EditingBehavior Editor::Behavior() const noexcept
  {
    return EditingBehavior(_document.GetSettings().EditingBehaviorType);
  }

  // FIXME: A text control's own selection when the event's target is in one, with form controls.
  const VisibleSelection &Editor::SelectionForCommand(dom::Event *) const noexcept
  {
    return _document.Selection().Selection();
  }

  utf8_string Editor::SelectedText() const noexcept
  {
    return SelectedText(TextIteratorBehavior::TraversesFlatTree);
  }

  utf8_string Editor::SelectedTextForDataTransfer() const noexcept
  {
    return SelectedText({TextIteratorBehavior::EmitsImageAltText, TextIteratorBehavior::TraversesFlatTree});
  }

  utf8_string Editor::SelectedText(TextIteratorBehaviors behaviors) const noexcept
  {
    // We remove '\0' characters because they are not visibly rendered to the user.
    auto range = _document.Selection().Selection().FirstRange();
    if (!range)
    {
      return {};
    }
    auto text = PlainText(*range, behaviors);
    std::erase(text, u8'\0');
    return text;
  }
}
