#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/CharacterData.hpp"
#include "Krystal.Booey/DOM/ClipboardEvent.hpp"
#include "Krystal.Booey/DOM/CompositionEvent.hpp"
#include "Krystal.Booey/DOM/DataTransfer.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/InputEvent.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/DOM/TextEvent.hpp"
#include "Krystal.Booey/Editing/ApplyStyleCommand.hpp"
#include "Krystal.Booey/Editing/CompositeEditCommand.hpp"
#include "Krystal.Booey/Editing/DeleteSelectionCommand.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/EditingStyle.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Editing/Markup.hpp"
#include "Krystal.Booey/Editing/RenderedPosition.hpp"
#include "Krystal.Booey/Editing/ReplaceSelectionCommand.hpp"
#include "Krystal.Booey/Editing/TextIterator.hpp"
#include "Krystal.Booey/Editing/TypingCommand.hpp"
#include "Krystal.Booey/Editing/VisibleUnits.hpp"
#include "Krystal.Booey/Editing/WebContentReader.hpp"
#include "Krystal.Booey/Editing/WritingDirection.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLEmbedElement.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/HTMLObjectElement.hpp"
#include "Krystal.Booey/HTML/HTMLTextFormControlElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Page/EditorClient.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Platform/Pasteboard.hpp"
#include "Krystal.Booey/Platform/ScrollAlignment.hpp"
#include "Krystal.Booey/Platform/StaticPasteboard.hpp"
#include "Krystal.Booey/Rendering/CaretRectComputation.hpp"
#include "Krystal.Booey/Rendering/RenderImage.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/SVG/SVGImageElement.hpp"

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
        case ClipboardEventKind::Cut:              return EventNames::Cut;
        case ClipboardEventKind::Paste:
        case ClipboardEventKind::PasteAsPlainText:
        case ClipboardEventKind::PasteAsQuotation:
        case ClipboardEventKind::PasteFont:        return EventNames::Paste;
        case ClipboardEventKind::BeforeCopy:       return EventNames::BeforeCopy;
        case ClipboardEventKind::BeforeCut:        return EventNames::BeforeCut;
        case ClipboardEventKind::BeforePaste:      return EventNames::BeforePaste;
      }
      krys_debug_assert(false);
      return {};
    }

    KRYS_NODISCARD Maybe<dom::DOMString>
      InputEventDataForEditingStyleAndAction(const css::StyleProperties *style, EditAction action) noexcept
    {
      if (!style)
      {
        return null;
      }

      switch (action)
      {
        case EditAction::SetColor: return dom::DOMString(style->GetPropertyValue(css::PropertyId::Color));
        case EditAction::SetInlineWritingDirection:
        case EditAction::SetBlockWritingDirection:
          return dom::DOMString(style->GetPropertyValue(css::PropertyId::Direction));
        default: return null;
      }
    }

    bool DispatchBeforeInputEvent(dom::Element &element, utf8_stringview inputType,
                                  dom::IsInputMethodComposing isInputMethodComposing,
                                  Maybe<dom::DOMString> data = null,
                                  RefPtr<dom::DataTransfer> &&dataTransfer = nullptr,
                                  const List<RefPtr<dom::StaticRange>> &targetRanges = {},
                                  bool cancelable = true) noexcept
    {
      auto event = dom::InputEvent::Create(dom::DOMStringAtom(EventNames::BeforeInput), inputType, cancelable,
                                           krys::move(data), krys::move(dataTransfer), targetRanges, 0,
                                           isInputMethodComposing);
      (void)dom::EventDispatcher::DispatchToTarget(*event, &element);
      return !event->DefaultPrevented();
    }

    // FIXME: Typing commands that are applied again fire `input` from within the command, so it's dispatched
    // to a scoped event queue rather than straight away, with a scoped event queue.
    void DispatchInputEvent(dom::Element &element, utf8_stringview inputType,
                            dom::IsInputMethodComposing isInputMethodComposing,
                            Maybe<dom::DOMString> data = null,
                            RefPtr<dom::DataTransfer> &&dataTransfer = nullptr,
                            const List<RefPtr<dom::StaticRange>> &targetRanges = {}) noexcept
    {
      auto event =
        dom::InputEvent::Create(dom::DOMStringAtom(EventNames::Input), inputType, false, krys::move(data),
                                krys::move(dataTransfer), targetRanges, 0, isInputMethodComposing);
      (void)dom::EventDispatcher::DispatchToTarget(*event, &element);
    }

    bool DispatchBeforeInputEvents(RefPtr<dom::Element> startRoot, RefPtr<dom::Element> endRoot,
                                   utf8_stringview inputTypeName,
                                   dom::IsInputMethodComposing isInputMethodComposing,
                                   const Maybe<dom::DOMString> &data = null,
                                   const RefPtr<dom::DataTransfer> &dataTransfer = nullptr,
                                   const List<RefPtr<dom::StaticRange>> &targetRanges = {},
                                   bool cancelable = true) noexcept
    {
      bool continueWithDefaultBehavior = true;
      if (startRoot)
      {
        continueWithDefaultBehavior &=
          DispatchBeforeInputEvent(*startRoot, inputTypeName, isInputMethodComposing, data,
                                   RefPtr<dom::DataTransfer>(dataTransfer), targetRanges, cancelable);
      }
      if (endRoot && endRoot != startRoot)
      {
        continueWithDefaultBehavior &=
          DispatchBeforeInputEvent(*endRoot, inputTypeName, isInputMethodComposing, data,
                                   RefPtr<dom::DataTransfer>(dataTransfer), targetRanges, cancelable);
      }
      return continueWithDefaultBehavior;
    }

    void DispatchInputEvents(RefPtr<dom::Element> startRoot, RefPtr<dom::Element> endRoot,
                             utf8_stringview inputTypeName,
                             dom::IsInputMethodComposing isInputMethodComposing,
                             const Maybe<dom::DOMString> &data = null,
                             const RefPtr<dom::DataTransfer> &dataTransfer = nullptr) noexcept
    {
      if (startRoot)
      {
        DispatchInputEvent(*startRoot, inputTypeName, isInputMethodComposing, data,
                           RefPtr<dom::DataTransfer>(dataTransfer));
      }
      if (endRoot && endRoot != startRoot)
      {
        DispatchInputEvent(*endRoot, inputTypeName, isInputMethodComposing, data,
                           RefPtr<dom::DataTransfer>(dataTransfer));
      }
    }

    KRYS_NODISCARD Maybe<dom::DOMString> InputEventData(const CompositeEditCommand &command) noexcept
    {
      auto data = command.InputEventData();
      if (data.empty())
      {
        return null;
      }
      return data;
    }

    KRYS_NODISCARD Ref<dom::DataTransfer>
      CreateDataTransferForClipboardEvent(dom::Document &document, ClipboardEventKind kind) noexcept
    {
      using StoreMode = dom::DataTransfer::StoreMode;
      switch (kind)
      {
        case ClipboardEventKind::Copy:
        case ClipboardEventKind::CopyFont:
        case ClipboardEventKind::Cut:
          return dom::DataTransfer::CreateForCopyAndPaste(document, StoreMode::ReadWrite,
                                                          CreateUnique<StaticPasteboard>());
        case ClipboardEventKind::PasteAsPlainText:
        {
          auto plainTextType = u8"text/plain";
          auto plainText = Pasteboard::CreateForCopyAndPaste()->ReadString(plainTextType);
          auto pasteboard = CreateUnique<StaticPasteboard>();
          pasteboard->WriteString(plainTextType, plainText);
          return dom::DataTransfer::CreateForCopyAndPaste(document, StoreMode::Readonly,
                                                          krys::move(pasteboard));
        }
        case ClipboardEventKind::Paste:
        case ClipboardEventKind::PasteAsQuotation:
        case ClipboardEventKind::PasteFont:
          return dom::DataTransfer::CreateForCopyAndPaste(document, StoreMode::Readonly,
                                                          Pasteboard::CreateForCopyAndPaste());
        case ClipboardEventKind::BeforeCopy:
        case ClipboardEventKind::BeforeCut:
        case ClipboardEventKind::BeforePaste:
          return dom::DataTransfer::CreateForCopyAndPaste(document, StoreMode::Invalid,
                                                          CreateUnique<StaticPasteboard>());
      }
      krys_debug_assert(false);
      return dom::DataTransfer::CreateForCopyAndPaste(document, StoreMode::Invalid,
                                                      CreateUnique<StaticPasteboard>());
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

  bool Editor::HandleKeyboardEvent(dom::KeyboardEvent &event) noexcept
  {
    if (auto *client = Client())
    {
      return client->HandleKeyboardEvent(event);
    }
    return false;
  }

  void Editor::HandleInputMethodKeydown(dom::KeyboardEvent &event) noexcept
  {
    if (auto *client = Client())
    {
      client->HandleInputMethodKeydown(event);
    }
  }

  void Editor::DidDispatchInputMethodKeydown(dom::KeyboardEvent &event) noexcept
  {
    if (auto *client = Client())
    {
      client->DidDispatchInputMethodKeydown(event);
    }
  }

  // Returns whether caller should continue with "the default processing", which is the same as
  // the event handler NOT setting the return value to false
  // https://w3c.github.io/clipboard-apis/#fire-a-clipboard-event
  bool Editor::DispatchClipboardEvent(RefPtr<dom::Element> &&target, ClipboardEventKind kind,
                                      Ref<dom::DataTransfer> &&dataTransfer) noexcept
  {
    // FIXME: Move the target selection code here.
    if (!target)
    {
      return true;
    }

    auto event =
      dom::ClipboardEvent::Create(dom::DOMStringAtom(EventNameForClipboardEvent(kind)), Ref(dataTransfer));

    (void)dom::EventDispatcher::DispatchToTarget(*event, target.get());
    bool noDefaultProcessing = event->DefaultPrevented();
    if (noDefaultProcessing && (kind == ClipboardEventKind::Copy || kind == ClipboardEventKind::Cut)
        && dataTransfer->GetPasteboard().HasData())
    {
      dataTransfer->CommitToPasteboard(*Pasteboard::CreateForCopyAndPaste());
    }

    dataTransfer->MakeInvalidForSecurity();

    return !noDefaultProcessing;
  }

  bool Editor::DispatchClipboardEvent(RefPtr<dom::Element> &&target, ClipboardEventKind kind) noexcept
  {
    if (!target)
    {
      return true;
    }

    auto &document = target->NodeDocument();
    return DispatchClipboardEvent(krys::move(target), kind,
                                  CreateDataTransferForClipboardEvent(document, kind));
  }

  // WinIE uses onbeforecut and onbeforepaste to enables the cut and paste menu items.  They
  // also send onbeforecopy, apparently for symmetry, but it doesn't affect the menu items.
  // We need to use onbeforecopy as a real menu enabler because we allow elements that are not
  // normally selectable to implement copy/paste (like divs, or a document body).

  bool Editor::CanDHTMLCut() noexcept
  {
    if (_document.Selection().Selection().IsInPasswordField())
    {
      return false;
    }

    return !DispatchClipboardEvent(FindEventTargetFromSelection(), ClipboardEventKind::BeforeCut);
  }

  bool Editor::CanDHTMLCopy() noexcept
  {
    if (_document.Selection().Selection().IsInPasswordField())
    {
      return false;
    }
    return !DispatchClipboardEvent(FindEventTargetFromSelection(), ClipboardEventKind::BeforeCopy);
  }

  bool Editor::CanDHTMLPaste() noexcept
  {
    return !DispatchClipboardEvent(FindEventTargetFromSelection(), ClipboardEventKind::BeforePaste);
  }

  bool Editor::CanCut() const noexcept
  {
    return CanCopy() && CanDelete();
  }

  // FIXME: Image documents, with them.
  bool Editor::CanCopy() const noexcept
  {
    const VisibleSelection &selection = _document.Selection().Selection();
    return (selection.IsRange() || !IsEditablePosition(selection.Start()))
           && (!selection.IsInPasswordField() || selection.IsInAutoFilledAndViewableField());
  }

  bool Editor::CanPaste() const noexcept
  {
    return CanEdit();
  }

  bool Editor::TryDHTMLCopy() noexcept
  {
    if (_document.Selection().Selection().IsInPasswordField())
    {
      return false;
    }

    return !DispatchClipboardEvent(FindEventTargetFromSelection(), ClipboardEventKind::Copy);
  }

  bool Editor::TryDHTMLCut() noexcept
  {
    if (_document.Selection().Selection().IsInPasswordField())
    {
      return false;
    }

    return !DispatchClipboardEvent(FindEventTargetFromSelection(), ClipboardEventKind::Cut);
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

  // FIXME: Beeping when there's nothing to cut (the system sound manager), once the host can.
  void Editor::Cut(FromMenuOrKeyBinding fromMenuOrKeyBinding) noexcept
  {
    bool oldCopying =
      std::exchange(_copyingFromMenuOrKeyBinding, fromMenuOrKeyBinding == FromMenuOrKeyBinding::Yes);
    if (TryDHTMLCut())
    {
      // DHTML did the whole operation
      _copyingFromMenuOrKeyBinding = oldCopying;
      return;
    }
    if (CanCut())
    {
      PerformCutOrCopy(EditorActionSpecifier::CutAction);
    }
    _copyingFromMenuOrKeyBinding = oldCopying;
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

  void Editor::Paste(FromMenuOrKeyBinding fromMenuOrKeyBinding) noexcept
  {
    Paste(*Pasteboard::CreateForCopyAndPaste(), fromMenuOrKeyBinding);
  }

  // FIXME: Spelling markers (updateMarkersForWordsAffectedByEditing), with text checking.
  void Editor::Paste(Pasteboard &pasteboard, FromMenuOrKeyBinding fromMenuOrKeyBinding) noexcept
  {
    bool oldPasting =
      std::exchange(_pastingFromMenuOrKeyBinding, fromMenuOrKeyBinding == FromMenuOrKeyBinding::Yes);

    if (DispatchClipboardEvent(FindEventTargetFromSelection(), ClipboardEventKind::Paste) && CanPaste())
    {
      if (_document.Selection().Selection().IsContentRichlyEditable())
      {
        PasteWithPasteboard(&pasteboard, {PasteOption::AllowPlainText});
      }
      else
      {
        PasteAsPlainTextWithPasteboard(pasteboard);
      }
    }
    _pastingFromMenuOrKeyBinding = oldPasting;
  }

  // FIXME: Spelling markers (updateMarkersForWordsAffectedByEditing), with text checking.
  void Editor::PasteAsPlainText(FromMenuOrKeyBinding fromMenuOrKeyBinding) noexcept
  {
    bool oldPasting =
      std::exchange(_pastingFromMenuOrKeyBinding, fromMenuOrKeyBinding == FromMenuOrKeyBinding::Yes);

    if (DispatchClipboardEvent(FindEventTargetFromSelection(), ClipboardEventKind::PasteAsPlainText)
        && CanPaste())
    {
      PasteAsPlainTextWithPasteboard(*Pasteboard::CreateForCopyAndPaste());
    }
    _pastingFromMenuOrKeyBinding = oldPasting;
  }

  void Editor::PasteAsPlainText(utf8_stringview pastingText, bool smartReplace) noexcept
  {
    RefPtr<dom::Element> target = FindEventTargetFromSelection();
    if (!target)
    {
      return;
    }
    auto event = dom::TextEvent::CreateForPlainTextPaste(pastingText, smartReplace);
    (void)dom::EventDispatcher::DispatchToTarget(*event, target.get());
    if (!event->DefaultPrevented())
    {
      (void)HandleTextEvent(*event);
    }
  }

  // FIXME: The edit action of the paste (the event's input type), with removing backgrounds.
  void Editor::PasteAsFragment(Ref<dom::DocumentFragment> &&pastingFragment, bool smartReplace,
                               bool matchStyle, MailBlockquoteHandling respectsMailBlockquote,
                               EditAction) noexcept
  {
    RefPtr<dom::Element> target = FindEventTargetFromSelection();
    if (!target)
    {
      return;
    }
    auto event =
      dom::TextEvent::CreateForFragmentPaste(krys::move(pastingFragment), dom::TextEventInputType::Paste,
                                             smartReplace, matchStyle, respectsMailBlockquote);
    (void)dom::EventDispatcher::DispatchToTarget(*event, target.get());
    if (!event->DefaultPrevented())
    {
      (void)HandleTextEvent(*event);
    }
  }

  void Editor::PasteAsPlainTextWithPasteboard(Pasteboard &pasteboard) noexcept
  {
    utf8_string text = ReadPlainTextFromPasteboard(pasteboard);
    if (ShouldInsertText(text, SelectedRange()))
    {
      PasteAsPlainText(text, CanSmartReplaceWithPasteboard(pasteboard));
    }
  }

  // FIXME: Quoting the fragment (quoteFragmentForPasting), with mail blockquotes.
  void Editor::PasteWithPasteboard(Pasteboard *pasteboard, EnumSet<PasteOption> options) noexcept
  {
    auto range = SelectedRange();
    if (!range)
    {
      return;
    }
    bool chosePlainText = false;
    auto fragment = WebContentFromPasteboard(*pasteboard, *range,
                                             options.Contains(PasteOption::AllowPlainText), chosePlainText);
    if (fragment && ShouldInsertFragment(*fragment, range))
    {
      PasteAsFragment(ShareRef(*fragment), CanSmartReplaceWithPasteboard(*pasteboard), chosePlainText,
                      options.Contains(PasteOption::IgnoreMailBlockquote)
                        ? MailBlockquoteHandling::IgnoreBlockquote
                        : MailBlockquoteHandling::RespectBlockquote);
    }
  }

  void Editor::WriteSelectionToPasteboard(Pasteboard &pasteboard) noexcept
  {
    PasteboardWebContent pasteboardContent;
    pasteboardContent.CanSmartCopyOrDelete = CanSmartCopyOrDelete();
    pasteboardContent.Text = SelectedTextForDataTransfer();
    pasteboardContent.Markup = SerializePreservingVisualAppearance(
      _document.Selection().Selection(), ResolveURLs::YesExcludingURLsForPrivacy, SerializeComposedTree::Yes,
      IgnoreUserSelectNone::Yes);
    pasteboardContent.ContentOrigin = _document.OriginIdentifierForPasteboard();
    pasteboard.Write(pasteboardContent);
  }

  IgnoreSelectionChangeForScope::IgnoreSelectionChangeForScope(LocalFrame &frame) noexcept
      : _selectionChange(*frame.Document(), null, TemporarySelectionOption::IgnoreSelectionChanges)
  {
  }

  // FIXME: Link decoration filtering (applyLinkDecorationFiltering), which is Cocoa's.
  void Editor::CopyURL(const url::URL &url, utf8_stringview title, Pasteboard &pasteboard) noexcept
  {
    PasteboardURL pasteboardURL;
    pasteboardURL.Url = url;
    pasteboardURL.Title = utf8_string(title);

    pasteboard.Write(pasteboardURL);
  }

  namespace
  {
    KRYS_NODISCARD dom::DOMString ElementURL(dom::Element &element) noexcept
    {
      if (Is<html::HTMLImageElement>(element) || Is<html::HTMLInputElement>(element))
      {
        return element.GetAttribute(u8"src").value_or(dom::DOMString());
      }
      if (Is<svg::SVGImageElement>(DynamicDowncast<svg::SVGElement>(&element)))
      {
        return element.GetAttributeNS(dom::DOMStringAtom(infra::Namespaces::XLink), u8"href")
          .value_or(dom::DOMString());
      }
      if (auto *htmlElement = DynamicDowncast<html::HTMLElement>(element);
          htmlElement
          && (Is<html::HTMLEmbedElement>(*htmlElement) || Is<html::HTMLObjectElement>(*htmlElement)))
      {
        return element.ImageSourceURL().value_or(dom::DOMString());
      }
      return {};
    }

    KRYS_NODISCARD bool HasImageForElement(dom::Element &element) noexcept
    {
      auto *renderer = DynamicDowncast<RenderImage>(element.Renderer());
      if (!renderer)
      {
        return false;
      }

      auto *cachedImage = renderer->GetCachedImage();
      if (!cachedImage || cachedImage->ErrorOccurred())
      {
        return false;
      }

      return cachedImage->ImageForRenderer(renderer);
    }
  }

  void Editor::WriteImageToPasteboard(Pasteboard &pasteboard, dom::Element &imageElement, const url::URL &,
                                      utf8_stringview title) noexcept
  {
    PasteboardImage pasteboardImage;

    if (!HasImageForElement(imageElement))
    {
      return;
    }

    pasteboardImage.Url.Url = imageElement.NodeDocument().CompleteURL(ElementURL(imageElement));
    pasteboardImage.Url.Title = utf8_string(title);
    pasteboardImage.Url.Markup =
      SerializeFragment(imageElement, SerializedNodes::SubtreeIncludingNode, nullptr, ResolveURLs::Yes);
    pasteboard.Write(pasteboardImage);
  }

  RefPtr<dom::DocumentFragment> Editor::WebContentFromPasteboard(Pasteboard &pasteboard,
                                                                 const dom::SimpleRange &context,
                                                                 bool allowPlainText,
                                                                 bool &chosePlainText) noexcept
  {
    WebContentReader reader(*_document.Frame(), context, allowPlainText);
    pasteboard.Read(reader);
    chosePlainText = reader.MadeFragmentFromPlainText();
    return reader.TakeFragment();
  }

  utf8_string Editor::ReadPlainTextFromPasteboard(Pasteboard &pasteboard) noexcept
  {
    PasteboardPlainText text;
    pasteboard.Read(text);
    return krys::move(text.Text);
  }

  bool Editor::CanSmartReplaceWithPasteboard(Pasteboard &pasteboard) noexcept
  {
    return SmartInsertDeleteEnabled() && pasteboard.CanSmartReplaceContents();
  }

  // FIXME: Asking the editor client (shouldInsertText and shouldInsertNode), with the editor client's
  // delegate callbacks.
  bool Editor::ShouldInsertFragment(dom::DocumentFragment &fragment,
                                    const Maybe<dom::SimpleRange> &replacingDOMRange) const noexcept
  {
    if (!Client())
    {
      return false;
    }
    if (auto *child = fragment.FirstChild();
        child && fragment.LastChild() == child && Is<dom::CharacterData>(*child))
    {
      return ShouldInsertText(Downcast<dom::CharacterData>(*child).Data(), replacingDOMRange);
    }
    return true;
  }

  // FIXME: Image documents and image overlays (writeImageToPasteboard), with images on the pasteboard; the
  // editor client's notifications around writing the selection; spelling markers
  // (updateMarkersForWordsAffectedByEditing), with text checking; and accessibility's notification of the cut
  // text, with accessibility.
  void Editor::PerformCutOrCopy(EditorActionSpecifier action) noexcept
  {
    auto selection = SelectedRange();
    if (action == EditorActionSpecifier::CutAction && !ShouldDeleteRange(selection))
    {
      return;
    }

    if (html::EnclosingTextFormControl(_document.Selection().Selection().Start()))
    {
      Pasteboard::CreateForCopyAndPaste()->WritePlainText(
        SelectedTextForDataTransfer(),
        CanSmartCopyOrDelete() ? Pasteboard::CanSmartReplace : Pasteboard::CannotSmartReplace);
    }
    else
    {
      WriteSelectionToPasteboard(*Pasteboard::CreateForCopyAndPaste());
    }

    if (action == EditorActionSpecifier::CutAction)
    {
      DeleteSelectionWithSmartDelete(CanSmartCopyOrDelete(), EditAction::Cut);
    }
  }

  EditingBehavior Editor::Behavior() const noexcept
  {
    return EditingBehavior(_document.GetSettings().EditingBehaviorType);
  }

  void Editor::TextFieldDidBeginEditing(dom::Element &e) noexcept
  {
    if (auto *client = Client())
    {
      client->TextFieldDidBeginEditing(e);
    }
  }

  void Editor::TextFieldDidEndEditing(dom::Element &e) noexcept
  {
    // FIXME: Dismissing the correction panel (dismissCorrectionPanelAsIgnored), with spelling correction.
    if (auto *client = Client())
    {
      client->TextFieldDidEndEditing(e);
    }
  }

  void Editor::TextDidChangeInTextField(dom::Element &e) noexcept
  {
    if (auto *client = Client())
    {
      client->TextDidChangeInTextField(e);
    }
  }

  bool Editor::DoTextFieldCommandFromEvent(dom::Element &e, dom::KeyboardEvent *ke) noexcept
  {
    if (auto *client = Client())
    {
      return client->DoTextFieldCommandFromEvent(e, ke);
    }

    return false;
  }

  void Editor::TextWillBeDeletedInTextField(dom::Element &input) noexcept
  {
    if (auto *client = Client())
    {
      client->TextWillBeDeletedInTextField(input);
    }
  }

  void Editor::TextDidChangeInTextArea(dom::Element &e) noexcept
  {
    if (auto *client = Client())
    {
      client->TextDidChangeInTextArea(e);
    }
  }

  // FIXME: Writing Tools suppressing editing, and stopping scrolling as content grows while typing.
  bool Editor::WillApplyEditing(CompositeEditCommand &command,
                                List<RefPtr<dom::StaticRange>> &&targetRanges) noexcept
  {
    if (!command.ShouldDispatchInputEvents())
    {
      return true;
    }

    RefPtr<EditCommandComposition> composition = ShareRefPtr(command.Composition());
    if (!composition)
    {
      return true;
    }

    auto isInputMethodComposing =
      command.IsInputMethodComposing() ? dom::IsInputMethodComposing::Yes : dom::IsInputMethodComposing::No;
    return DispatchBeforeInputEvents(ShareRefPtr(composition->StartingRootEditableElement()),
                                     ShareRefPtr(composition->EndingRootEditableElement()),
                                     command.InputEventTypeName(), isInputMethodComposing,
                                     InputEventData(command), command.InputEventDataTransfer(), targetRanges,
                                     command.IsBeforeInputEventCancelable());
  }

  namespace
  {
    void NotifyTextFromControls(dom::Element *startRoot, dom::Element *endRoot,
                                bool wasUserEdit = true) noexcept
    {
      RefPtr<html::HTMLTextFormControlElement> startingTextControl =
        ShareRefPtr(html::EnclosingTextFormControl(FirstPositionInOrBeforeNode(startRoot)));
      RefPtr<html::HTMLTextFormControlElement> endingTextControl =
        ShareRefPtr(html::EnclosingTextFormControl(FirstPositionInOrBeforeNode(endRoot)));
      if (startingTextControl)
      {
        startingTextControl->DidEditInnerTextValue(wasUserEdit);
      }
      if (endingTextControl && startingTextControl != endingTextControl)
      {
        endingTextControl->DidEditInnerTextValue(wasUserEdit);
      }
    }
  }

  // FIXME: Updating the editor's UI, with menus; and autocorrection's response (respondToAppliedEditing),
  // with text checking.
  void Editor::AppliedEditing(CompositeEditCommand &command) noexcept
  {
    _document.UpdateLayout();

    krys_debug_assert(command.Composition());
    Ref<EditCommandComposition> composition = ShareRef(*command.Composition());
    VisibleSelection newSelection(command.EndingSelection());

    bool wasUserEdit = [&command]
    {
      auto *typingCommand = DynamicDowncast<TypingCommand>(command);
      return !typingCommand || !typingCommand->TriggeringEventIsUntrusted();
    }();
    NotifyTextFromControls(composition->StartingRootEditableElement(),
                           composition->EndingRootEditableElement(), wasUserEdit);

    if (command.IsTopLevelCommand())
    {
      // Don't clear the typing style with this selection change. We do those things elsewhere if necessary.
      EnumSet<FrameSelection::SetSelectionOption> options;
      if (command.IsDictationCommand())
      {
        options.Add(FrameSelection::SetSelectionOption::DictationTriggered);
      }

      ChangeSelectionAfterCommand(newSelection, options);
    }

    auto isInputMethodComposing =
      command.IsInputMethodComposing() ? dom::IsInputMethodComposing::Yes : dom::IsInputMethodComposing::No;
    if (command.ShouldDispatchInputEvents())
    {
      DispatchInputEvents(ShareRefPtr(composition->StartingRootEditableElement()),
                          ShareRefPtr(composition->EndingRootEditableElement()), command.InputEventTypeName(),
                          isInputMethodComposing, InputEventData(command), command.InputEventDataTransfer());
    }

    if (command.IsTopLevelCommand())
    {
      if (!command.PreservesTypingStyle())
      {
        _document.Selection().ClearTypingStyle();
      }

      // Command will be equal to last edit command only in the case of typing
      if (_lastEditCommand.get() == &command)
      {
        krys_debug_assert(command.IsTypingCommand());
      }
      else
      {
        // Only register a new undo command if the command passed in is
        // different from the last command
        _lastEditCommand = ShareRefPtr(&command);
        if (auto *client = Client())
        {
          client->RegisterUndoStep(*_lastEditCommand->EnsureComposition());
        }
      }
      RespondToChangedContents(newSelection);
    }
  }

  bool Editor::WillUnapplyEditing(const EditCommandComposition &composition) const noexcept
  {
    TypingCommand::CloseTyping(_document);
    return DispatchBeforeInputEvents(ShareRefPtr(composition.StartingRootEditableElement()),
                                     ShareRefPtr(composition.EndingRootEditableElement()), u8"historyUndo",
                                     dom::IsInputMethodComposing::No);
  }

  void Editor::UnappliedEditing(EditCommandComposition &composition) noexcept
  {
    _document.UpdateLayout();

    NotifyTextFromControls(composition.StartingRootEditableElement(),
                           composition.EndingRootEditableElement());

    VisibleSelection newSelection(composition.StartingSelection());
    ChangeSelectionAfterCommand(newSelection, FrameSelection::DefaultSetSelectionOptions());
    DispatchInputEvents(ShareRefPtr(composition.StartingRootEditableElement()),
                        ShareRefPtr(composition.EndingRootEditableElement()), u8"historyUndo",
                        dom::IsInputMethodComposing::No);

    _lastEditCommand = nullptr;
    if (auto *client = Client())
    {
      client->RegisterRedoStep(composition);
    }
    RespondToChangedContents(newSelection);
  }

  bool Editor::WillReapplyEditing(const EditCommandComposition &composition) const noexcept
  {
    return DispatchBeforeInputEvents(ShareRefPtr(composition.StartingRootEditableElement()),
                                     ShareRefPtr(composition.EndingRootEditableElement()), u8"historyRedo",
                                     dom::IsInputMethodComposing::No);
  }

  void Editor::ReappliedEditing(EditCommandComposition &composition) noexcept
  {
    _document.UpdateLayout();

    NotifyTextFromControls(composition.StartingRootEditableElement(),
                           composition.EndingRootEditableElement());

    VisibleSelection newSelection(composition.EndingSelection());
    ChangeSelectionAfterCommand(newSelection, FrameSelection::DefaultSetSelectionOptions());
    DispatchInputEvents(ShareRefPtr(composition.StartingRootEditableElement()),
                        ShareRefPtr(composition.EndingRootEditableElement()), u8"historyRedo",
                        dom::IsInputMethodComposing::No);

    _lastEditCommand = nullptr;
    if (auto *client = Client())
    {
      client->RegisterUndoStep(composition);
    }
    RespondToChangedContents(newSelection);
  }

  bool Editor::CanUndo() const noexcept
  {
    auto *client = Client();
    return client && client->CanUndo();
  }

  void Editor::Undo() noexcept
  {
    if (auto *client = Client())
    {
      client->Undo();
    }
  }

  bool Editor::CanRedo() const noexcept
  {
    auto *client = Client();
    return client && client->CanRedo();
  }

  void Editor::Redo() noexcept
  {
    if (auto *client = Client())
    {
      client->Redo();
    }
  }

  CompositeEditCommand *Editor::LastEditCommand() const noexcept
  {
    return _lastEditCommand.get();
  }

  // FIXME: Accessibility's notification of the change, with accessibility; and updating the spelling and
  // grammar markers of the words the edit touched, with text checking.
  void Editor::RespondToChangedContents(const VisibleSelection &) noexcept
  {
    if (auto *client = Client())
    {
      client->RespondToChangedContents();
    }
  }

  void Editor::ChangeSelectionAfterCommand(const VisibleSelection &newSelection,
                                           EnumSet<FrameSelectionSetSelectionOption> options) noexcept
  {
    if (newSelection.IsOrphan() || newSelection.GetDocument() != &_document)
    {
      return;
    }

    // If there is no selection change, don't bother sending shouldChangeSelection, but still call
    // setSelection, because there is work that it must do in this situation. The old selection can be invalid
    // here and calling shouldChangeSelection can produce some strange calls. See <rdar://problem/5729315>
    // Some shouldChangeSelectedDOMRange contain Ranges for selections that are no longer valid
    bool selectionDidNotChangeDOMPosition = newSelection == _document.Selection().Selection();
    if (selectionDidNotChangeDOMPosition || _document.Selection().ShouldChangeSelection(newSelection))
    {
      _document.Selection().SetSelection(newSelection, options);
    }

    krys_debug_assert(!_document.Selection().Selection().IsOrphan());

    // FIXME: Telling the client the selection changed when the edit moved it visually without moving its DOM
    // position (respondToChangedSelection), with the rest of the editor client.
  }

  bool Editor::CanEdit() const noexcept
  {
    return _document.Selection().Selection().RootEditableElement();
  }

  bool Editor::CanEditRichly() const noexcept
  {
    return _document.Selection().Selection().IsContentRichlyEditable();
  }

  void Editor::ApplyStyle(const css::StyleProperties *style, EditAction editingAction) noexcept
  {
    if (style)
    {
      ApplyStyle(EditingStyle::Create(style), editingAction);
    }
  }

  void Editor::ApplyStyle(RefPtr<EditingStyle> &&style, EditAction editingAction) noexcept
  {
    if (!style)
    {
      return;
    }

    auto &selection = _document.Selection();
    if (selection.IsNone())
    {
      return;
    }

    auto inputTypeName = InputTypeNameForEditingAction(editingAction);
    auto isInputMethodComposing = IsInputMethodComposingForEditingAction(editingAction)
                                    ? dom::IsInputMethodComposing::Yes
                                    : dom::IsInputMethodComposing::No;
    auto inputEventData = InputEventDataForEditingStyleAndAction(style->Style(), editingAction);
    RefPtr<dom::Element> element = ShareRefPtr(selection.Selection().RootEditableElement());

    if (element && !DispatchBeforeInputEvent(*element, inputTypeName, isInputMethodComposing, inputEventData))
    {
      return;
    }

    if (selection.IsNone())
    {
      return;
    }

    if (selection.IsCaret())
    {
      ComputeAndSetTypingStyle(*style, editingAction);
    }
    else
    {
      ApplyStyleCommand::Create(_document, style.get(), editingAction)->Apply();
    }

    if (element)
    {
      DispatchInputEvent(*element, inputTypeName, isInputMethodComposing, krys::move(inputEventData));
    }
  }

  void Editor::ApplyParagraphStyle(const css::StyleProperties *style, EditAction editingAction) noexcept
  {
    if (!style)
    {
      return;
    }

    auto &selection = _document.Selection();
    if (selection.IsNone())
    {
      return;
    }

    auto inputTypeName = InputTypeNameForEditingAction(editingAction);
    auto isInputMethodComposing = IsInputMethodComposingForEditingAction(editingAction)
                                    ? dom::IsInputMethodComposing::Yes
                                    : dom::IsInputMethodComposing::No;
    auto inputEventData = InputEventDataForEditingStyleAndAction(style, editingAction);
    RefPtr<dom::Element> element = ShareRefPtr(selection.Selection().RootEditableElement());
    if (element && !DispatchBeforeInputEvent(*element, inputTypeName, isInputMethodComposing, inputEventData))
    {
      return;
    }
    if (selection.IsNone())
    {
      return;
    }

    ApplyStyleCommand::Create(_document, EditingStyle::Create(style).get(), editingAction,
                              ApplyStylePropertyLevel::ForceBlock)
      ->Apply();

    if (element)
    {
      DispatchInputEvent(*element, inputTypeName, isInputMethodComposing, krys::move(inputEventData));
    }
  }

  void Editor::ApplyStyleToSelection(const css::StyleProperties *style, EditAction editingAction) noexcept
  {
    if (!style || !style->PropertyCount() || !CanEditRichly())
    {
      return;
    }
    ApplyStyle(style, editingAction);
  }

  void Editor::ApplyParagraphStyleToSelection(const css::StyleProperties *style,
                                              EditAction editingAction) noexcept
  {
    if (!style || !style->PropertyCount() || !CanEditRichly())
    {
      return;
    }
    ApplyParagraphStyle(style, editingAction);
  }

  void Editor::ComputeAndSetTypingStyle(EditingStyle &style, EditAction editingAction) noexcept
  {
    auto &selection = _document.Selection();
    if (style.IsEmpty())
    {
      selection.ClearTypingStyle();
      return;
    }

    // Calculate the current typing style.
    RefPtr<EditingStyle> typingStyle;
    if (auto *existingTypingStyle = selection.TypingStyle())
    {
      typingStyle = existingTypingStyle->Copy();
    }
    else
    {
      typingStyle = EditingStyle::Create();
    }
    typingStyle->OverrideTypingStyleAt(style, selection.Selection().VisibleStart().DeepEquivalent());

    // Handle block styles, substracting these from the typing style.
    auto blockStyle = typingStyle->ExtractAndRemoveBlockProperties();
    if (!blockStyle->IsEmpty())
    {
      ApplyStyleCommand::Create(_document, blockStyle.get(), editingAction)->Apply();
    }

    // Set the remaining style as the typing style.
    selection.SetTypingStyle(krys::move(typingStyle));
  }

  void Editor::SetBaseWritingDirection(WritingDirection direction) noexcept
  {
    if (auto *focusedElement = DynamicDowncast<html::HTMLTextFormControlElement>(_document.FocusedElement());
        focusedElement && focusedElement->IsTextField())
    {
      if (direction == WritingDirection::Natural)
      {
        return;
      }

      auto element = ShareRef(*focusedElement);
      auto directionValue = direction == WritingDirection::LeftToRight ? u8"ltr" : u8"rtl";
      auto writingDirectionInputTypeName =
        InputTypeNameForEditingAction(EditAction::SetBlockWritingDirection);
      if (!DispatchBeforeInputEvent(*element, writingDirectionInputTypeName, dom::IsInputMethodComposing::No,
                                    dom::DOMString(directionValue)))
      {
        return;
      }

      (void)element->SetAttribute(dom::DOMStringAtom(u8"dir"), dom::DOMString(directionValue));
      DispatchInputEvent(*element, writingDirectionInputTypeName, dom::IsInputMethodComposing::No,
                         dom::DOMString(directionValue));
      (void)_document.UpdateStyleIfNeeded();
      return;
    }

    auto style = css::MutableStyleProperties::Create();
    (void)style->SetProperty(css::PropertyId::Direction,
                             direction == WritingDirection::LeftToRight   ? css::ValueId::Ltr
                             : direction == WritingDirection::RightToLeft ? css::ValueId::Rtl
                                                                          : css::ValueId::Inherit);
    ApplyParagraphStyleToSelection(style.get(), EditAction::SetBlockWritingDirection);
  }

  bool Editor::CanDelete() const noexcept
  {
    const auto &selection = _document.Selection().Selection();
    return selection.IsRange() && selection.RootEditableElement();
  }

  bool Editor::CanDeleteRange(const dom::SimpleRange &range) const noexcept
  {
    if (!range.Start.Container->HasEditableStyle() || !range.End.Container->HasEditableStyle())
    {
      return false;
    }

    if (range.Collapsed())
    {
      // FIXME: We sometimes allow deletions at the start of editable roots, like when the caret is in an
      // empty list item.
      auto previous = VisiblePosition {dom::MakeDeprecatedLegacyPosition(range.Start)}.Previous();
      if (previous.IsNull()
          || previous.DeepEquivalent().DeprecatedNode()->RootEditableElement()
               != range.Start.Container->RootEditableElement())
      {
        return false;
      }
    }

    return true;
  }

  // FIXME: Asking the editor client whether to delete the range (shouldDeleteRange), with the editing
  // delegates.
  bool Editor::ShouldDeleteRange(const Maybe<dom::SimpleRange> &range) const noexcept
  {
    return range && !range->Collapsed() && CanDeleteRange(*range) && Client();
  }

  bool Editor::ShouldSmartDelete() noexcept
  {
    if (Behavior().ShouldAlwaysSmartDelete())
    {
      return true;
    }
    return _document.Selection().Granularity() == TextGranularity::WordGranularity;
  }

  bool Editor::SmartInsertDeleteEnabled() noexcept
  {
    auto *client = Client();
    return client && client->SmartInsertDeleteEnabled();
  }

  bool Editor::CanSmartCopyOrDelete() noexcept
  {
    return SmartInsertDeleteEnabled() && ShouldSmartDelete();
  }

  Maybe<dom::SimpleRange> Editor::SelectedRange() noexcept
  {
    return _document.Selection().Selection().ToNormalizedRange();
  }

  // FIXME: Asking the editor client whether to insert the text (shouldInsertText), with the editing
  // delegates.
  bool Editor::ShouldInsertText(utf8_stringview, const Maybe<dom::SimpleRange> &) const noexcept
  {
    return Client();
  }

  bool Editor::InsertText(utf8_stringview text, dom::Event *triggeringEvent,
                          dom::TextEventInputType inputType) noexcept
  {
    auto *frame = _document.Frame();
    return frame && frame->GetEventHandler().HandleTextInputEvent(text, triggeringEvent, inputType);
  }

  bool Editor::InsertText(utf8_stringview text, dom::Event *triggeringEvent) noexcept
  {
    return InsertText(text, triggeringEvent, dom::TextEventInputType::Keyboard);
  }

  // FIXME: Dictation (DictationCommand), with dictation; autocorrection before and after the text, with text
  // checking; and revealing the selection when the client asks (shouldRevealCurrentSelectionAfterInsertion),
  // with the rest of the editor client.
  bool Editor::InsertTextWithoutSendingTextEvent(utf8_stringview text, bool selectInsertedText,
                                                 dom::TextEvent *triggeringEvent) noexcept
  {
    if (text.empty())
    {
      return false;
    }

    VisibleSelection selection = SelectionForCommand(triggeringEvent);
    if (!selection.IsContentEditable())
    {
      return false;
    }

    if (!ShouldInsertText(text, selection.ToNormalizedRange()))
    {
      return true;
    }

    // Get the selection to use for the event that triggered this insertText.
    // If the event handler changed the selection, we may want to use a different selection
    // that is contained in the event target.
    selection = SelectionForCommand(triggeringEvent);
    if (selection.IsContentEditable())
    {
      if (auto *selectionStart = selection.Start().DeprecatedNode())
      {
        Ref<dom::Document> document = ShareRef(selectionStart->NodeDocument());

        // Insert the text
        EnumSet<TypingCommand::Option> options {TypingCommand::Option::RetainAutocorrectionIndicator};
        if (selectInsertedText)
        {
          options.Add(TypingCommand::Option::SelectInsertedText);
        }
        if (triggeringEvent && triggeringEvent->IsAutocompletion())
        {
          options.Add(TypingCommand::Option::IsAutocompletion);
        }
        auto compositionType = triggeringEvent && triggeringEvent->IsComposition()
                                 ? TypingCommand::TextCompositionType::Final
                                 : TypingCommand::TextCompositionType::None;
        TypingCommand::InsertText(*document, text, triggeringEvent, selection, options, compositionType);

        // Reveal the current selection. Note that focus may have changed after insertion.
        // FIXME: Asking the client whether to (shouldRevealCurrentSelectionAfterInsertion, which every client
        // but one returns true from), and revealing the focused frame's selection, with frames.
        if (!triggeringEvent || triggeringEvent->IsTrusted())
        {
          document->Selection().RevealSelection(
            {SelectionRevealMode::Reveal, ScrollAlignment::AlignCenterIfNeeded});
        }
      }
    }

    return true;
  }

  bool Editor::InsertLineBreak() noexcept
  {
    if (!CanEdit())
    {
      return false;
    }

    if (!ShouldInsertText(u8"\n", _document.Selection().Selection().ToNormalizedRange()))
    {
      return true;
    }

    VisiblePosition caret = _document.Selection().Selection().VisibleStart();
    bool alignToEdge = IsEndOfEditableOrNonEditableContent(caret);
    TypingCommand::InsertLineBreak(_document, {});
    RevealSelectionAfterEditingOperation(alignToEdge ? ScrollAlignment::AlignToEdgeIfNeeded
                                                     : ScrollAlignment::AlignCenterIfNeeded,
                                         RevealExtentOption::DoNotRevealExtent);

    return true;
  }

  bool Editor::InsertParagraphSeparator() noexcept
  {
    if (!CanEdit())
    {
      return false;
    }

    if (!CanEditRichly())
    {
      return InsertLineBreak();
    }

    if (!ShouldInsertText(u8"\n", _document.Selection().Selection().ToNormalizedRange()))
    {
      return true;
    }

    VisiblePosition caret = _document.Selection().Selection().VisibleStart();
    bool alignToEdge = IsEndOfEditableOrNonEditableContent(caret);
    TypingCommand::InsertParagraphSeparator(_document, {});
    RevealSelectionAfterEditingOperation(alignToEdge ? ScrollAlignment::AlignToEdgeIfNeeded
                                                     : ScrollAlignment::AlignCenterIfNeeded,
                                         RevealExtentOption::DoNotRevealExtent);

    return true;
  }

  bool Editor::InsertParagraphSeparatorInQuotedContent() noexcept
  {
    // FIXME: Why is this missing calls to canEdit, canEditRichly, etc.?
    TypingCommand::InsertParagraphSeparatorInQuotedContent(_document);
    RevealSelectionAfterEditingOperation();
    return true;
  }

  bool Editor::HandleTextEvent(dom::TextEvent &event) noexcept
  {
    // Default event handling for Drag and Drop will be handled by DragController
    // so we leave the event for it.
    if (event.IsDrop())
    {
      return false;
    }

    if (event.IsPaste() || event.IsRemoveBackground())
    {
      auto action = event.IsRemoveBackground() ? EditAction::RemoveBackground : EditAction::Paste;
      if (event.PastingFragment())
      {
        ReplaceSelectionWithFragment(*event.PastingFragment(), false, event.ShouldSmartReplace(),
                                     event.ShouldMatchStyle(), action, event.GetMailBlockquoteHandling());
      }
      else
      {
        ReplaceSelectionWithText(event.Data(), false, event.ShouldSmartReplace(), action);
      }
      return true;
    }

    const auto &data = event.Data();
    if (data == u8"\n")
    {
      if (event.IsLineBreak())
      {
        return InsertLineBreak();
      }
      return InsertParagraphSeparator();
    }

    return InsertTextWithoutSendingTextEvent(data, false, &event);
  }

  // FIXME: The kill ring (addRangeToKillRing, setStartNewKillRingSequence), with the kill ring.
  bool Editor::DeleteWithDirection(SelectionDirection direction, TextGranularity granularity,
                                   bool shouldAddToKillRing, bool isTypingAction) noexcept
  {
    if (!CanEdit())
    {
      return false;
    }

    if (_document.Selection().IsRange())
    {
      if (isTypingAction)
      {
        TypingCommand::DeleteKeyPressed(
          _document,
          CanSmartCopyOrDelete() ? EnumSet<TypingCommand::Option> {TypingCommand::Option::SmartDelete}
                                 : EnumSet<TypingCommand::Option> {},
          granularity);
        RevealSelectionAfterEditingOperation();
      }
      else
      {
        DeleteSelectionWithSmartDelete(CanSmartCopyOrDelete());
        // Implicitly calls revealSelectionAfterEditingOperation().
      }
    }
    else
    {
      EnumSet<TypingCommand::Option> options;
      if (CanSmartCopyOrDelete())
      {
        options.Add(TypingCommand::Option::SmartDelete);
      }
      if (shouldAddToKillRing)
      {
        options.Add(TypingCommand::Option::AddsToKillRing);
      }
      switch (direction)
      {
        case SelectionDirection::Forward:
        case SelectionDirection::Right:
          TypingCommand::ForwardDeleteKeyPressed(_document, options, granularity);
          break;
        case SelectionDirection::Backward:
        case SelectionDirection::Left:
          TypingCommand::DeleteKeyPressed(_document, options, granularity);
          break;
      }
      RevealSelectionAfterEditingOperation();
    }

    return true;
  }

  void Editor::DeleteSelectionWithSmartDelete(bool smartDelete, EditAction editingAction) noexcept
  {
    if (_document.Selection().IsNone())
    {
      return;
    }

    DeleteSelectionCommand::Create(_document, smartDelete, true, false, false, true, editingAction)->Apply();
  }

  // FIXME: Beeping when there's nothing to delete (systemBeep), with system sounds; and the kill ring.
  void Editor::PerformDelete() noexcept
  {
    if (!CanDelete())
    {
      return;
    }

    DeleteSelectionWithSmartDelete(CanSmartCopyOrDelete());
  }

  // FIXME: Waiting for the inserted images to load before revealing the selection;
  // accessibility's notifications of the pasted text, with accessibility; and checking the spelling of the
  // editable root, with text checking.
  void Editor::ReplaceSelectionWithFragment(dom::DocumentFragment &fragment, bool selectReplacement,
                                            bool smartReplace, bool matchStyle, EditAction editingAction,
                                            MailBlockquoteHandling mailBlockquoteHandling) noexcept
  {
    VisibleSelection selection = _document.Selection().Selection();
    if (selection.IsNone() || !selection.IsContentEditable())
    {
      return;
    }

    EnumSet<ReplaceSelectionCommand::CommandOption> options {
      ReplaceSelectionCommand::CommandOption::PreventNesting,
      ReplaceSelectionCommand::CommandOption::SanitizeFragment};
    if (selectReplacement)
    {
      options.Add(ReplaceSelectionCommand::CommandOption::SelectReplacement);
    }
    if (smartReplace)
    {
      options.Add(ReplaceSelectionCommand::CommandOption::SmartReplace);
    }
    if (matchStyle)
    {
      options.Add(ReplaceSelectionCommand::CommandOption::MatchStyle);
    }
    if (mailBlockquoteHandling == MailBlockquoteHandling::IgnoreBlockquote)
    {
      options.Add(ReplaceSelectionCommand::CommandOption::IgnoreMailBlockquote);
    }

    auto command = ReplaceSelectionCommand::Create(_document, ShareRefPtr(&fragment), options, editingAction);
    command->Apply();

    RevealSelectionAfterEditingOperation();
  }

  void Editor::ReplaceSelectionWithText(utf8_stringview text, bool selectReplacement, bool smartReplace,
                                        EditAction editingAction) noexcept
  {
    auto range = SelectedRange();
    if (!range)
    {
      return;
    }

    ReplaceSelectionWithFragment(*CreateFragmentFromText(*range, text), selectReplacement, smartReplace, true,
                                 editingAction);
  }

  void Editor::RevealSelectionAfterEditingOperation(const ScrollAlignment &alignment,
                                                    RevealExtentOption revealExtentOption) noexcept
  {
    _document.Selection().RevealSelection({SelectionRevealMode::Reveal, alignment, revealExtentOption});
  }

  void Editor::RevealSelectionAfterEditingOperation() noexcept
  {
    RevealSelectionAfterEditingOperation(ScrollAlignment::AlignCenterIfNeeded,
                                         RevealExtentOption::DoNotRevealExtent);
  }

  bool Editor::HasComposition() const noexcept
  {
    return static_cast<bool>(_compositionNode);
  }

  dom::Text *Editor::CompositionNode() const noexcept
  {
    return _compositionNode.get();
  }

  bool Editor::InsertTextForConfirmedComposition(utf8_stringview text) noexcept
  {
    return _document.Frame()->GetEventHandler().HandleTextInputEvent(text, nullptr,
                                                                     dom::TextEventInputType::Composition);
  }

  void Editor::SelectComposition() noexcept
  {
    auto range = CompositionRange();
    if (!range)
    {
      return;
    }

    // The composition can start inside a composed character sequence, so we have to override checks.
    VisibleSelection selection;
    selection.SetWithoutValidation(dom::MakeDeprecatedLegacyPosition(range->Start),
                                   dom::MakeDeprecatedLegacyPosition(range->End));
    _document.Selection().SetSelection(selection, {});
  }

  void Editor::ConfirmComposition() noexcept
  {
    if (!_compositionNode)
    {
      return;
    }
    // A copy: confirming replaces the composition's text in its node.
    auto text = utf8_string(utf8_stringview(_compositionNode->Data())
                              .substr(_compositionStart, _compositionEnd - _compositionStart));
    SetComposition(text, SetCompositionMode::ConfirmComposition);
  }

  void Editor::ConfirmOrCancelCompositionAndNotifyClient() noexcept
  {
    if (!HasComposition())
    {
      return;
    }

    RefPtr<dom::Document> document = ShareRefPtr(&_document);
    auto *frame = document->Frame();
    if (!frame)
    {
      return;
    }

    if (CancelCompositionIfSelectionIsInvalid())
    {
      return;
    }

    ConfirmComposition();

    if (auto *editorClient = Client())
    {
      editorClient->RespondToChangedSelection(frame);
      editorClient->DiscardedComposition(*document);
    }
  }

  void Editor::CancelComposition() noexcept
  {
    if (!_compositionNode)
    {
      return;
    }
    SetComposition({}, SetCompositionMode::CancelComposition);
  }

  bool Editor::CancelCompositionIfSelectionIsInvalid() noexcept
  {
    uint32 start;
    uint32 end;
    if (!HasComposition() || IgnoreSelectionChanges() || GetCompositionSelection(start, end))
    {
      return false;
    }

    CancelComposition();
    return true;
  }

  void Editor::ConfirmComposition(utf8_stringview text) noexcept
  {
    SetComposition(text, SetCompositionMode::ConfirmComposition);
  }

  namespace
  {
    // FIXME: The user typing gesture (UserTypingGestureIndicator), with typing gestures.
    class SetCompositionScope
    {
      KRYS_NON_COPY_MOVABLE(SetCompositionScope);

    private:
      Ref<dom::Document> _document;

    public:
      explicit SetCompositionScope(dom::Document &document) noexcept : _document(ShareRef(document))
      {
        _document->GetEditor().SetIgnoreSelectionChanges(true);
      }

      ~SetCompositionScope() noexcept
      {
        _document->GetEditor().SetIgnoreSelectionChanges(false);
        if (auto *editorClient = _document->GetEditor().Client())
        {
          editorClient->DidUpdateComposition();
        }
      }
    };
  }

  // FIXME: Accessibility's composition notifications (onTextCompositionChange), with accessibility.
  void Editor::SetComposition(utf8_stringview text, SetCompositionMode mode) noexcept
  {
    Ref<dom::Document> document = ShareRef(_document);
    SetCompositionScope setCompositionScope(*document);

    if (mode == SetCompositionMode::CancelComposition)
    {
      krys_debug_assert(text.empty());
    }
    else
    {
      SelectComposition();
    }

    _compositionNode = nullptr;
    _customCompositionUnderlines.clear();

    if (document->Selection().IsNone())
    {
      return;
    }

    // Always delete the current composition before inserting the finalized composition text if we're
    // confirming our composition. Our default behavior (if the beforeinput event is not prevented) is to
    // insert the finalized composition text back in. We pass TypingCommand::TextCompositionType::Pending here
    // to indicate that we are deleting the pending composition.
    if (mode != SetCompositionMode::CancelComposition)
    {
      TypingCommand::DeleteSelection(*document, {}, TypingCommand::TextCompositionType::Pending);
    }

    (void)InsertTextForConfirmedComposition(text);

    if (RefPtr<dom::Element> target = ShareRefPtr(document->FocusedElement()))
    {
      auto event = dom::CompositionEvent::Create(dom::DOMStringAtom(EventNames::CompositionEnd), text);
      (void)dom::EventDispatcher::DispatchToTarget(*event, target.get());
    }

    if (mode == SetCompositionMode::CancelComposition)
    {
      // An open typing command that disagrees about current selection would cause issues with typing later
      // on.
      TypingCommand::CloseTyping(*document);
    }
  }

  void Editor::CloseTyping() noexcept
  {
    TypingCommand::CloseTyping(_document);
  }

  // FIXME: Accessibility's composition notifications (onTextCompositionChange), with accessibility.
  void Editor::SetComposition(utf8_stringview text, const List<CompositionUnderline> &underlines,
                              uint32 selectionStart, uint32 selectionEnd) noexcept
  {
    Ref<dom::Document> document = ShareRef(_document);
    SetCompositionScope setCompositionScope(*document);

    // Updates styles before setting selection for composition to prevent
    // inserting the previous composition text into text nodes oddly.
    (void)document->UpdateStyleIfNeeded();

    SelectComposition();

    if (document->Selection().IsNone())
    {
      return;
    }

    auto originalText = SelectedText();
    bool isStartingToRecomposeExistingRange =
      !text.empty() && selectionStart < selectionEnd && !HasComposition();
    if (isStartingToRecomposeExistingRange)
    {
      // We pass TypingCommand::TextCompositionType::Final here to indicate that we are removing composition
      // text that has been finalized.
      TypingCommand::DeleteSelection(*document, {}, TypingCommand::TextCompositionType::Final);
      const auto &currentSelection = document->Selection().Selection();
      if (currentSelection.IsRange())
      {
        // If deletion was prevented, then we need to collapse the selection to the end so that the original
        // text will not be recomposed.
        document->Selection().SetSelection(VisibleSelection {currentSelection.End(), currentSelection.End()});
      }
    }

    RefPtr<dom::CompositionEvent> event;
    RefPtr<dom::Element> target = ShareRefPtr(document->FocusedElement());
    if (target)
    {
      // Dispatch an appropriate composition event to the focused node.
      // We check the composition status and choose an appropriate composition event since this
      // function is used for three purposes:
      // 1. Starting a new composition.
      //    Send a compositionstart and a compositionupdate event when this function creates
      //    a new composition node, i.e.
      //    _compositionNode == 0 && !text.empty().
      //    Sending a compositionupdate event at this time ensures that at least one
      //    compositionupdate event is dispatched.
      // 2. Updating the existing composition node.
      //    Send a compositionupdate event when this function updates the existing composition
      //    node, i.e. _compositionNode != 0 && !text.empty().
      // 3. Canceling the ongoing composition.
      //    Send a compositionend event when function deletes the existing composition node, i.e.
      //    _compositionNode != 0 && text.empty().
      if (!_compositionNode)
      {
        // We should send a compositionstart event only when the given text is not empty because this
        // function doesn't create a composition node when the text is empty.
        if (!text.empty())
        {
          auto start =
            dom::CompositionEvent::Create(dom::DOMStringAtom(EventNames::CompositionStart), originalText);
          (void)dom::EventDispatcher::DispatchToTarget(*start, target.get());
          event = dom::CompositionEvent::Create(dom::DOMStringAtom(EventNames::CompositionUpdate), text);
        }
      }
      else if (!text.empty())
      {
        event = dom::CompositionEvent::Create(dom::DOMStringAtom(EventNames::CompositionUpdate), text);
      }

      if (event)
      {
        (void)dom::EventDispatcher::DispatchToTarget(*event, target.get());
      }
    }

    // If text is empty, then delete the old composition here. If text is non-empty, InsertTextCommand::input
    // will delete the old composition with an optimized replace operation.
    if (text.empty())
    {
      TypingCommand::DeleteSelection(*document, TypingCommand::Option::PreventSpellChecking,
                                     TypingCommand::TextCompositionType::Pending);
      if (target)
      {
        auto end = dom::CompositionEvent::Create(dom::DOMStringAtom(EventNames::CompositionEnd), text);
        (void)dom::EventDispatcher::DispatchToTarget(*end, target.get());
      }
    }

    _compositionNode = nullptr;
    _customCompositionUnderlines.clear();

    if (!text.empty())
    {
      TypingCommand::InsertText(
        *document, text, event.get(),
        {TypingCommand::Option::SelectInsertedText, TypingCommand::Option::PreventSpellChecking},
        TypingCommand::TextCompositionType::Pending);

      // Find out what node has the composition now.
      auto base = document->Selection().Selection().Base().Downstream();
      auto extent = document->Selection().Selection().Extent();
      RefPtr<dom::Node> baseNode = ShareRefPtr(base.DeprecatedNode());
      auto baseOffset = static_cast<uint32>(base.DeprecatedEditingOffset());
      RefPtr<dom::Node> extentNode = ShareRefPtr(extent.DeprecatedNode());
      auto extentOffset = static_cast<uint32>(extent.DeprecatedEditingOffset());

      if (auto *baseTextNode = DynamicDowncast<dom::Text>(baseNode.get());
          baseTextNode && baseNode == extentNode && baseOffset + text.size() == extentOffset)
      {
        _compositionNode = ShareRefPtr(baseTextNode);
        _compositionStart = baseOffset;
        _compositionEnd = extentOffset;
        _customCompositionUnderlines = underlines;
        for (auto &underline : _customCompositionUnderlines)
        {
          underline.StartOffset += baseOffset;
          underline.EndOffset += baseOffset;
        }

        if (auto *renderer = baseTextNode->Renderer())
        {
          renderer->Repaint();
        }

        auto start = std::min(baseOffset + selectionStart, extentOffset);
        auto end = std::min(std::max(start, baseOffset + selectionEnd), extentOffset);
        auto range = dom::SimpleRange {{ShareRef(*baseTextNode), start}, {ShareRef(*baseTextNode), end}};
        (void)document->Selection().SetSelectedRange(range, Affinity::Downstream,
                                                     FrameSelection::ShouldCloseTyping::No);
      }
    }
  }

  void Editor::SetIgnoreSelectionChanges(bool ignore, RevealSelection shouldRevealExistingSelection) noexcept
  {
    if (_ignoreSelectionChanges == ignore)
    {
      return;
    }

    _ignoreSelectionChanges = ignore;
    if (!ignore && shouldRevealExistingSelection == RevealSelection::Yes)
    {
      RevealSelectionAfterEditingOperation(ScrollAlignment::AlignToEdgeIfNeeded,
                                           RevealExtentOption::RevealExtent);
    }
  }

  Maybe<dom::SimpleRange> Editor::CompositionRange() const noexcept
  {
    if (!_compositionNode)
    {
      return {};
    }
    auto length = static_cast<uint32>(_compositionNode->Length());
    auto start = std::min(_compositionStart, length);
    auto end = std::clamp(_compositionEnd, start, length);
    if (start >= end)
    {
      return {};
    }
    return dom::SimpleRange {{ShareRef<dom::Node>(*_compositionNode), start},
                             {ShareRef<dom::Node>(*_compositionNode), end}};
  }

  bool Editor::GetCompositionSelection(uint32 &selectionStart, uint32 &selectionEnd) const noexcept
  {
    if (!_compositionNode)
    {
      return false;
    }
    const auto &selection = _document.Selection().Selection();
    auto start = selection.Start();
    if (start.DeprecatedNode() != _compositionNode.get())
    {
      return false;
    }
    auto end = selection.End();
    if (end.DeprecatedNode() != _compositionNode.get())
    {
      return false;
    }

    if (static_cast<uint32>(start.DeprecatedEditingOffset()) < _compositionStart)
    {
      return false;
    }
    if (static_cast<uint32>(end.DeprecatedEditingOffset()) > _compositionEnd)
    {
      return false;
    }

    selectionStart = static_cast<uint32>(start.DeprecatedEditingOffset()) - _compositionStart;
    selectionEnd = static_cast<uint32>(end.DeprecatedEditingOffset()) - _compositionStart;
    return true;
  }

  // FIXME: Spelling and grammar, the kill ring (setStartNewKillRingSequence), revealing images that load
  // (imageElementsToLoadBeforeRevealingSelection) and the editor's user interface updates
  // (scheduleEditorUIUpdate), with those.
  void Editor::RespondToChangedSelection(const VisibleSelection &,
                                         EnumSet<FrameSelectionSetSelectionOption>) noexcept
  {
    if (auto *client = Client())
    {
      client->RespondToChangedSelection(_document.Frame());
    }
  }

  namespace
  {
    void CollapseCaretWidth(IntRect &rect) noexcept
    {
      // FIXME: Width adjustment doesn't work for rotated text.
      if (rect.Width() == CaretWidth())
      {
        rect.SetWidth(0);
      }
      else if (rect.Height() == CaretWidth())
      {
        rect.SetHeight(0);
      }
    }
  }

  IntRect Editor::FirstRectForRange(const dom::SimpleRange &range) const noexcept
  {
    (void)range.Start.GetDocument().UpdateLayout();

    VisiblePosition start(dom::MakeDeprecatedLegacyPosition(range.Start));

    if (range.Collapsed())
    {
      // FIXME: Getting caret rect and removing caret width is a very roundabout way to get collapsed range
      // location. In particular, width adjustment doesn't work for rotated text.
      IntRect startCaretRect = RenderedPosition(start).AbsoluteRect();
      CollapseCaretWidth(startCaretRect);
      return startCaretRect;
    }

    VisiblePosition end(dom::MakeDeprecatedLegacyPosition(range.End), Affinity::Upstream);

    if (InSameLine(start, end))
    {
      return EnclosingIntRect(UnitedBoundingBoxes(RenderObject::AbsoluteTextQuads(range)));
    }

    return RenderedPosition(start).AbsoluteRect(CaretRectMode::ExpandToEndOfLine);
  }

  VisibleSelection Editor::SelectionForCommand(dom::Event *event) const noexcept
  {
    auto selection = _document.Selection().Selection();
    if (!event)
    {
      return selection;
    }
    // If the target is a text control, and the current selection is outside of its shadow tree,
    // then use the saved selection for that text control.
    if (auto *target = DynamicDowncast<html::HTMLTextFormControlElement>(event->Target().get());
        target && target->IsTextField())
    {
      auto start = selection.Start();
      if (start.IsNull() || target != html::EnclosingTextFormControl(start))
      {
        if (auto range = target->Selection())
        {
          return VisibleSelection(*range, Affinity::Downstream, selection.GetDirectionality());
        }
      }
    }
    return selection;
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
