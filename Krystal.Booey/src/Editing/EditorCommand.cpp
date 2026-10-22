#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Page/EditorClient.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Platform/Scrollbar.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include <algorithm>
#include <array>

namespace krys::boo
{
  // FIXME: The editing commands (typing, deleting, cutting, pasting, styling, lists, indenting, links, undo
  // and redo, the mark and kill ring, find and spelling), with editing commands; the commands' state and
  // value (bold, the font and colours), with styling.
  class EditorInternalCommand
  {
  public:
    bool (*Execute)(LocalFrame &, dom::Event *, EditorCommandSource, utf8_stringview) noexcept;
    bool (*IsSupportedFromDOM)(LocalFrame *) noexcept;
    bool (*IsEnabled)(LocalFrame &, dom::Event *, EditorCommandSource) noexcept;
    bool IsTextInsertion;
    bool (*AllowExecutionWhenDisabled)(LocalFrame &, EditorCommandSource) noexcept;
  };

  namespace
  {
    constexpr bool IsNotTextInsertion = false;

    bool ExpandSelectionToGranularity(LocalFrame &frame, TextGranularity granularity) noexcept
    {
      auto &frameSelection = frame.Document()->Selection();
      VisibleSelection selection = frameSelection.Selection();
      auto oldRange = selection.ToNormalizedRange();
      (void)selection.ExpandUsingGranularity(granularity);
      auto newRange = selection.ToNormalizedRange();
      if (!newRange || newRange->Collapsed())
      {
        return false;
      }
      auto affinity = selection.GetAffinity();
      if (!frame.Document()->GetEditor().Client()->ShouldChangeSelectedRange(oldRange, newRange, affinity,
                                                                             false))
      {
        return false;
      }
      (void)frameSelection.SetSelectedRange(newRange, affinity, FrameSelection::ShouldCloseTyping::Yes);
      // FIXME: Why do we ignore the return value from SetSelectedRange here?
      return true;
    }

    KRYS_NODISCARD uint32 VerticalScrollDistance(LocalFrame &frame) noexcept
    {
      auto *focusedElement = frame.Document()->FocusedElement();
      if (!focusedElement)
      {
        return 0;
      }
      auto *renderBox = DynamicDowncast<RenderBox>(focusedElement->Renderer());
      if (!renderBox)
      {
        return 0;
      }
      const RenderStyle &style = renderBox->Style();
      if (!(style.OverflowY() == Overflow::Scroll || style.OverflowY() == Overflow::Auto
            || focusedElement->HasEditableStyle()))
      {
        return 0;
      }
      int32 height = std::min<int32>(renderBox->ClientHeight().ToInt(), frame.View()->VisibleHeight());
      return static_cast<uint32>(Scrollbar::PageStep(height));
    }

    // Execute command functions

    bool ExecuteMoveBackward(LocalFrame &frame, dom::Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::CharacterGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveBackwardAndModifySelection(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                               utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::CharacterGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveDown(LocalFrame &frame, dom::Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                  SelectionDirection::Forward,
                                                  TextGranularity::LineGranularity, UserTriggered::Yes);
    }

    bool ExecuteMoveDownAndModifySelection(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                           utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::LineGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveForward(LocalFrame &frame, dom::Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::CharacterGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveForwardAndModifySelection(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                              utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::CharacterGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveLeft(LocalFrame &frame, dom::Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return frame.Document()->Selection().Modify(FrameSelection::Alteration::Move, SelectionDirection::Left,
                                                  TextGranularity::CharacterGranularity, UserTriggered::Yes);
    }

    bool ExecuteMoveLeftAndModifySelection(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                           utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend, SelectionDirection::Left,
                                                 TextGranularity::CharacterGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveRight(LocalFrame &frame, dom::Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return frame.Document()->Selection().Modify(FrameSelection::Alteration::Move, SelectionDirection::Right,
                                                  TextGranularity::CharacterGranularity, UserTriggered::Yes);
    }

    bool ExecuteMoveRightAndModifySelection(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                            utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Right,
                                                 TextGranularity::CharacterGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToBeginningOfDocument(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                          utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::DocumentBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToBeginningOfDocumentAndModifySelection(LocalFrame &frame, dom::Event *,
                                                            EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::DocumentBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToBeginningOfLine(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                      utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Backward, TextGranularity::LineBoundary,
                                                 UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToBeginningOfLineAndModifySelection(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                                        utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Backward, TextGranularity::LineBoundary,
                                                 UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToBeginningOfParagraph(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                           utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::ParagraphBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToBeginningOfParagraphAndModifySelection(LocalFrame &frame, dom::Event *,
                                                             EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::ParagraphBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToBeginningOfSentence(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                          utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::SentenceBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToBeginningOfSentenceAndModifySelection(LocalFrame &frame, dom::Event *,
                                                            EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::SentenceBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToEndOfDocument(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                    utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::DocumentBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToEndOfDocumentAndModifySelection(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                                      utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::DocumentBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToEndOfLine(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Forward, TextGranularity::LineBoundary,
                                                 UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToEndOfLineAndModifySelection(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                                  utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Forward, TextGranularity::LineBoundary,
                                                 UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToEndOfParagraph(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                     utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::ParagraphBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToEndOfParagraphAndModifySelection(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                                       utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::ParagraphBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToEndOfSentence(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                    utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::SentenceBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToEndOfSentenceAndModifySelection(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                                      utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::SentenceBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToLeftEndOfLine(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                    utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move, SelectionDirection::Left,
                                                 TextGranularity::LineBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToLeftEndOfLineAndModifySelection(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                                      utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend, SelectionDirection::Left,
                                                 TextGranularity::LineBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToRightEndOfLine(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                     utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move, SelectionDirection::Right,
                                                 TextGranularity::LineBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToRightEndOfLineAndModifySelection(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                                       utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Right, TextGranularity::LineBoundary,
                                                 UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveUp(LocalFrame &frame, dom::Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                  SelectionDirection::Backward,
                                                  TextGranularity::LineGranularity, UserTriggered::Yes);
    }

    bool ExecuteMoveUpAndModifySelection(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                         utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::LineGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveWordBackward(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                 utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::WordGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveWordBackwardAndModifySelection(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                                   utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::WordGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveWordForward(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::WordGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveWordForwardAndModifySelection(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                                  utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::WordGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveWordLeft(LocalFrame &frame, dom::Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move, SelectionDirection::Left,
                                                 TextGranularity::WordGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveWordLeftAndModifySelection(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                               utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend, SelectionDirection::Left,
                                                 TextGranularity::WordGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveWordRight(LocalFrame &frame, dom::Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move, SelectionDirection::Right,
                                                 TextGranularity::WordGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveWordRightAndModifySelection(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                                utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Right, TextGranularity::WordGranularity,
                                                 UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveParagraphBackwardAndModifySelection(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                                        utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::ParagraphGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveParagraphForwardAndModifySelection(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                                       utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::ParagraphGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMovePageDown(LocalFrame &frame, dom::Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      uint32 distance = VerticalScrollDistance(frame);
      if (!distance)
      {
        return false;
      }
      return frame.Document()->Selection().Modify(FrameSelection::Alteration::Move, distance,
                                                  FrameSelection::VerticalDirection::Down, UserTriggered::Yes,
                                                  FrameSelection::CursorAlignOnScroll::Always);
    }

    bool ExecuteMovePageDownAndModifySelection(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                               utf8_stringview) noexcept
    {
      uint32 distance = VerticalScrollDistance(frame);
      if (!distance)
      {
        return false;
      }
      return frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend, distance,
                                                  FrameSelection::VerticalDirection::Down, UserTriggered::Yes,
                                                  FrameSelection::CursorAlignOnScroll::Always);
    }

    bool ExecuteMovePageUp(LocalFrame &frame, dom::Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      uint32 distance = VerticalScrollDistance(frame);
      if (!distance)
      {
        return false;
      }
      return frame.Document()->Selection().Modify(FrameSelection::Alteration::Move, distance,
                                                  FrameSelection::VerticalDirection::Up, UserTriggered::Yes,
                                                  FrameSelection::CursorAlignOnScroll::Always);
    }

    bool ExecuteMovePageUpAndModifySelection(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                             utf8_stringview) noexcept
    {
      uint32 distance = VerticalScrollDistance(frame);
      if (!distance)
      {
        return false;
      }
      return frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend, distance,
                                                  FrameSelection::VerticalDirection::Up, UserTriggered::Yes,
                                                  FrameSelection::CursorAlignOnScroll::Always);
    }

    bool ExecuteScrollPageBackward(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                   utf8_stringview) noexcept
    {
      return frame.GetEventHandler().LogicalScrollRecursively(
        ScrollLogicalDirection::ScrollBlockDirectionBackward, ScrollGranularity::Page);
    }

    bool ExecuteScrollPageForward(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                  utf8_stringview) noexcept
    {
      return frame.GetEventHandler().LogicalScrollRecursively(
        ScrollLogicalDirection::ScrollBlockDirectionForward, ScrollGranularity::Page);
    }

    bool ExecuteScrollLineUp(LocalFrame &frame, dom::Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return frame.GetEventHandler().ScrollRecursively(ScrollDirection::ScrollUp, ScrollGranularity::Line);
    }

    bool ExecuteScrollLineDown(LocalFrame &frame, dom::Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return frame.GetEventHandler().ScrollRecursively(ScrollDirection::ScrollDown, ScrollGranularity::Line);
    }

    bool ExecuteScrollToBeginningOfDocument(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                            utf8_stringview) noexcept
    {
      return frame.GetEventHandler().LogicalScrollRecursively(
        ScrollLogicalDirection::ScrollBlockDirectionBackward, ScrollGranularity::Document);
    }

    bool ExecuteScrollToEndOfDocument(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                      utf8_stringview) noexcept
    {
      return frame.GetEventHandler().LogicalScrollRecursively(
        ScrollLogicalDirection::ScrollBlockDirectionForward, ScrollGranularity::Document);
    }

    bool ExecuteCopy(LocalFrame &frame, dom::Event *, EditorCommandSource source, utf8_stringview) noexcept
    {
      frame.Document()->GetEditor().Copy(source == EditorCommandSource::MenuOrKeyBinding
                                           ? Editor::FromMenuOrKeyBinding::Yes
                                           : Editor::FromMenuOrKeyBinding::No);
      return true;
    }

    bool ExecuteSelectAll(LocalFrame &frame, dom::Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      frame.Document()->Selection().SelectAll();
      return true;
    }

    bool ExecuteSelectLine(LocalFrame &frame, dom::Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return ExpandSelectionToGranularity(frame, TextGranularity::LineGranularity);
    }

    bool ExecuteSelectParagraph(LocalFrame &frame, dom::Event *, EditorCommandSource,
                                utf8_stringview) noexcept
    {
      return ExpandSelectionToGranularity(frame, TextGranularity::ParagraphGranularity);
    }

    bool ExecuteSelectSentence(LocalFrame &frame, dom::Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return ExpandSelectionToGranularity(frame, TextGranularity::SentenceGranularity);
    }

    bool ExecuteSelectWord(LocalFrame &frame, dom::Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return ExpandSelectionToGranularity(frame, TextGranularity::WordGranularity);
    }

    bool ExecuteUnselect(LocalFrame &frame, dom::Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      frame.Document()->Selection().Clear();
      return true;
    }

    // Supported functions

    bool Supported(LocalFrame *) noexcept
    {
      return true;
    }

    bool SupportedFromMenuOrKeyBinding(LocalFrame *) noexcept
    {
      return false;
    }

    // FIXME: Script access to the clipboard (javaScriptCanAccessClipboard, the clipboard access policy), with
    // scripting; until then copying and cutting are allowed, as the default policy allows them.
    KRYS_NODISCARD bool DefaultValueForSupportedCopyCut(LocalFrame &) noexcept
    {
      return true;
    }

    bool SupportedCopyCut(LocalFrame *frame) noexcept
    {
      if (!frame)
      {
        return false;
      }

      bool defaultValue = DefaultValueForSupportedCopyCut(*frame);

      EditorClient *client = frame->Document()->GetEditor().Client();
      return client ? client->CanCopyCut(frame, defaultValue) : defaultValue;
    }

    // Enabled functions

    bool Enabled(LocalFrame &, dom::Event *, EditorCommandSource) noexcept
    {
      return true;
    }

    bool EnabledVisibleSelection(LocalFrame &frame, dom::Event *event, EditorCommandSource) noexcept
    {
      // The term "visible" here includes a caret in editable text or a range in any text.
      const VisibleSelection &selection = frame.Document()->GetEditor().SelectionForCommand(event);
      return (selection.IsCaret() && selection.IsContentEditable()) || selection.IsRange();
    }

    KRYS_NODISCARD bool CaretBrowsingEnabled(LocalFrame &frame) noexcept
    {
      return frame.Document()->GetSettings().CaretBrowsingEnabled;
    }

    bool EnabledVisibleSelectionOrCaretBrowsing(LocalFrame &frame, dom::Event *event,
                                                EditorCommandSource source) noexcept
    {
      return CaretBrowsingEnabled(frame) || EnabledVisibleSelection(frame, event, source);
    }

    // FIXME: Script access to the clipboard (allowCopyCutFromDOM), with scripting.
    bool EnabledCopy(LocalFrame &frame, dom::Event *, EditorCommandSource) noexcept
    {
      auto &editor = frame.Document()->GetEditor();
      return editor.CanDHTMLCopy() || editor.CanCopy();
    }

    bool EnabledInEditableText(LocalFrame &frame, dom::Event *event, EditorCommandSource) noexcept
    {
      return frame.Document()->GetEditor().SelectionForCommand(event).RootEditableElement();
    }

    bool EnabledInEditableTextOrCaretBrowsing(LocalFrame &frame, dom::Event *event,
                                              EditorCommandSource source) noexcept
    {
      return CaretBrowsingEnabled(frame) || EnabledInEditableText(frame, event, source);
    }

    // AllowExecutionWhenDisabled functions

    bool DoNotAllowExecutionWhenDisabled(LocalFrame &, EditorCommandSource) noexcept
    {
      return false;
    }

    bool AllowExecutionWhenDisabledCopyCut(LocalFrame &, EditorCommandSource source) noexcept
    {
      switch (source)
      {
        case EditorCommandSource::MenuOrKeyBinding:     return true;
        case EditorCommandSource::DOM:
        case EditorCommandSource::DOMWithUserInterface: return false;
      }
      krys_debug_assert(false);
      return false;
    }

    struct CommandEntry
    {
      utf8_stringview Name;
      EditorInternalCommand Command;
    };

    // Sorted by name, ignoring ASCII case.
    constexpr auto Commands = std::to_array<CommandEntry>({
      {u8"Copy",
       {ExecuteCopy, SupportedCopyCut, EnabledCopy, IsNotTextInsertion, AllowExecutionWhenDisabledCopyCut}},
      {u8"MoveBackward",
       {ExecuteMoveBackward, SupportedFromMenuOrKeyBinding, EnabledInEditableTextOrCaretBrowsing,
        IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveBackwardAndModifySelection",
       {ExecuteMoveBackwardAndModifySelection, SupportedFromMenuOrKeyBinding,
        EnabledVisibleSelectionOrCaretBrowsing, IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveDown",
       {ExecuteMoveDown, SupportedFromMenuOrKeyBinding, EnabledInEditableTextOrCaretBrowsing,
        IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveDownAndModifySelection",
       {ExecuteMoveDownAndModifySelection, SupportedFromMenuOrKeyBinding,
        EnabledVisibleSelectionOrCaretBrowsing, IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveForward",
       {ExecuteMoveForward, SupportedFromMenuOrKeyBinding, EnabledInEditableTextOrCaretBrowsing,
        IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveForwardAndModifySelection",
       {ExecuteMoveForwardAndModifySelection, SupportedFromMenuOrKeyBinding,
        EnabledVisibleSelectionOrCaretBrowsing, IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveLeft",
       {ExecuteMoveLeft, SupportedFromMenuOrKeyBinding, EnabledInEditableTextOrCaretBrowsing,
        IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveLeftAndModifySelection",
       {ExecuteMoveLeftAndModifySelection, SupportedFromMenuOrKeyBinding,
        EnabledVisibleSelectionOrCaretBrowsing, IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MovePageDown",
       {ExecuteMovePageDown, SupportedFromMenuOrKeyBinding, EnabledInEditableText, IsNotTextInsertion,
        DoNotAllowExecutionWhenDisabled}},
      {u8"MovePageDownAndModifySelection",
       {ExecuteMovePageDownAndModifySelection, SupportedFromMenuOrKeyBinding, EnabledVisibleSelection,
        IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MovePageUp",
       {ExecuteMovePageUp, SupportedFromMenuOrKeyBinding, EnabledInEditableText, IsNotTextInsertion,
        DoNotAllowExecutionWhenDisabled}},
      {u8"MovePageUpAndModifySelection",
       {ExecuteMovePageUpAndModifySelection, SupportedFromMenuOrKeyBinding, EnabledVisibleSelection,
        IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveParagraphBackwardAndModifySelection",
       {ExecuteMoveParagraphBackwardAndModifySelection, SupportedFromMenuOrKeyBinding,
        EnabledVisibleSelectionOrCaretBrowsing, IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveParagraphForwardAndModifySelection",
       {ExecuteMoveParagraphForwardAndModifySelection, SupportedFromMenuOrKeyBinding,
        EnabledVisibleSelectionOrCaretBrowsing, IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveRight",
       {ExecuteMoveRight, SupportedFromMenuOrKeyBinding, EnabledInEditableTextOrCaretBrowsing,
        IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveRightAndModifySelection",
       {ExecuteMoveRightAndModifySelection, SupportedFromMenuOrKeyBinding,
        EnabledVisibleSelectionOrCaretBrowsing, IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveToBeginningOfDocument",
       {ExecuteMoveToBeginningOfDocument, SupportedFromMenuOrKeyBinding, EnabledInEditableTextOrCaretBrowsing,
        IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveToBeginningOfDocumentAndModifySelection",
       {ExecuteMoveToBeginningOfDocumentAndModifySelection, SupportedFromMenuOrKeyBinding,
        EnabledVisibleSelectionOrCaretBrowsing, IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveToBeginningOfLine",
       {ExecuteMoveToBeginningOfLine, SupportedFromMenuOrKeyBinding, EnabledInEditableTextOrCaretBrowsing,
        IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveToBeginningOfLineAndModifySelection",
       {ExecuteMoveToBeginningOfLineAndModifySelection, SupportedFromMenuOrKeyBinding,
        EnabledVisibleSelectionOrCaretBrowsing, IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveToBeginningOfParagraph",
       {ExecuteMoveToBeginningOfParagraph, SupportedFromMenuOrKeyBinding,
        EnabledInEditableTextOrCaretBrowsing, IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveToBeginningOfParagraphAndModifySelection",
       {ExecuteMoveToBeginningOfParagraphAndModifySelection, SupportedFromMenuOrKeyBinding,
        EnabledVisibleSelectionOrCaretBrowsing, IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveToBeginningOfSentence",
       {ExecuteMoveToBeginningOfSentence, SupportedFromMenuOrKeyBinding, EnabledInEditableTextOrCaretBrowsing,
        IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveToBeginningOfSentenceAndModifySelection",
       {ExecuteMoveToBeginningOfSentenceAndModifySelection, SupportedFromMenuOrKeyBinding,
        EnabledVisibleSelectionOrCaretBrowsing, IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveToEndOfDocument",
       {ExecuteMoveToEndOfDocument, SupportedFromMenuOrKeyBinding, EnabledInEditableTextOrCaretBrowsing,
        IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveToEndOfDocumentAndModifySelection",
       {ExecuteMoveToEndOfDocumentAndModifySelection, SupportedFromMenuOrKeyBinding,
        EnabledVisibleSelectionOrCaretBrowsing, IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveToEndOfLine",
       {ExecuteMoveToEndOfLine, SupportedFromMenuOrKeyBinding, EnabledInEditableTextOrCaretBrowsing,
        IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveToEndOfLineAndModifySelection",
       {ExecuteMoveToEndOfLineAndModifySelection, SupportedFromMenuOrKeyBinding,
        EnabledVisibleSelectionOrCaretBrowsing, IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveToEndOfParagraph",
       {ExecuteMoveToEndOfParagraph, SupportedFromMenuOrKeyBinding, EnabledInEditableTextOrCaretBrowsing,
        IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveToEndOfParagraphAndModifySelection",
       {ExecuteMoveToEndOfParagraphAndModifySelection, SupportedFromMenuOrKeyBinding,
        EnabledVisibleSelectionOrCaretBrowsing, IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveToEndOfSentence",
       {ExecuteMoveToEndOfSentence, SupportedFromMenuOrKeyBinding, EnabledInEditableTextOrCaretBrowsing,
        IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveToEndOfSentenceAndModifySelection",
       {ExecuteMoveToEndOfSentenceAndModifySelection, SupportedFromMenuOrKeyBinding,
        EnabledVisibleSelectionOrCaretBrowsing, IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveToLeftEndOfLine",
       {ExecuteMoveToLeftEndOfLine, SupportedFromMenuOrKeyBinding, EnabledInEditableText, IsNotTextInsertion,
        DoNotAllowExecutionWhenDisabled}},
      {u8"MoveToLeftEndOfLineAndModifySelection",
       {ExecuteMoveToLeftEndOfLineAndModifySelection, SupportedFromMenuOrKeyBinding, EnabledInEditableText,
        IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveToRightEndOfLine",
       {ExecuteMoveToRightEndOfLine, SupportedFromMenuOrKeyBinding, EnabledInEditableText, IsNotTextInsertion,
        DoNotAllowExecutionWhenDisabled}},
      {u8"MoveToRightEndOfLineAndModifySelection",
       {ExecuteMoveToRightEndOfLineAndModifySelection, SupportedFromMenuOrKeyBinding, EnabledInEditableText,
        IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveUp",
       {ExecuteMoveUp, SupportedFromMenuOrKeyBinding, EnabledInEditableTextOrCaretBrowsing,
        IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveUpAndModifySelection",
       {ExecuteMoveUpAndModifySelection, SupportedFromMenuOrKeyBinding,
        EnabledVisibleSelectionOrCaretBrowsing, IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveWordBackward",
       {ExecuteMoveWordBackward, SupportedFromMenuOrKeyBinding, EnabledInEditableTextOrCaretBrowsing,
        IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveWordBackwardAndModifySelection",
       {ExecuteMoveWordBackwardAndModifySelection, SupportedFromMenuOrKeyBinding,
        EnabledVisibleSelectionOrCaretBrowsing, IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveWordForward",
       {ExecuteMoveWordForward, SupportedFromMenuOrKeyBinding, EnabledInEditableTextOrCaretBrowsing,
        IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveWordForwardAndModifySelection",
       {ExecuteMoveWordForwardAndModifySelection, SupportedFromMenuOrKeyBinding,
        EnabledVisibleSelectionOrCaretBrowsing, IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveWordLeft",
       {ExecuteMoveWordLeft, SupportedFromMenuOrKeyBinding, EnabledInEditableTextOrCaretBrowsing,
        IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveWordLeftAndModifySelection",
       {ExecuteMoveWordLeftAndModifySelection, SupportedFromMenuOrKeyBinding,
        EnabledVisibleSelectionOrCaretBrowsing, IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveWordRight",
       {ExecuteMoveWordRight, SupportedFromMenuOrKeyBinding, EnabledInEditableTextOrCaretBrowsing,
        IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MoveWordRightAndModifySelection",
       {ExecuteMoveWordRightAndModifySelection, SupportedFromMenuOrKeyBinding,
        EnabledVisibleSelectionOrCaretBrowsing, IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"ScrollLineDown",
       {ExecuteScrollLineDown, SupportedFromMenuOrKeyBinding, Enabled, IsNotTextInsertion,
        DoNotAllowExecutionWhenDisabled}},
      {u8"ScrollLineUp",
       {ExecuteScrollLineUp, SupportedFromMenuOrKeyBinding, Enabled, IsNotTextInsertion,
        DoNotAllowExecutionWhenDisabled}},
      {u8"ScrollPageBackward",
       {ExecuteScrollPageBackward, SupportedFromMenuOrKeyBinding, Enabled, IsNotTextInsertion,
        DoNotAllowExecutionWhenDisabled}},
      {u8"ScrollPageForward",
       {ExecuteScrollPageForward, SupportedFromMenuOrKeyBinding, Enabled, IsNotTextInsertion,
        DoNotAllowExecutionWhenDisabled}},
      {u8"ScrollToBeginningOfDocument",
       {ExecuteScrollToBeginningOfDocument, SupportedFromMenuOrKeyBinding, Enabled, IsNotTextInsertion,
        DoNotAllowExecutionWhenDisabled}},
      {u8"ScrollToEndOfDocument",
       {ExecuteScrollToEndOfDocument, SupportedFromMenuOrKeyBinding, Enabled, IsNotTextInsertion,
        DoNotAllowExecutionWhenDisabled}},
      {u8"SelectAll",
       {ExecuteSelectAll, Supported, Enabled, IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"SelectLine",
       {ExecuteSelectLine, SupportedFromMenuOrKeyBinding, EnabledVisibleSelection, IsNotTextInsertion,
        DoNotAllowExecutionWhenDisabled}},
      {u8"SelectParagraph",
       {ExecuteSelectParagraph, SupportedFromMenuOrKeyBinding, EnabledVisibleSelection, IsNotTextInsertion,
        DoNotAllowExecutionWhenDisabled}},
      {u8"SelectSentence",
       {ExecuteSelectSentence, SupportedFromMenuOrKeyBinding, EnabledVisibleSelection, IsNotTextInsertion,
        DoNotAllowExecutionWhenDisabled}},
      {u8"SelectWord",
       {ExecuteSelectWord, SupportedFromMenuOrKeyBinding, EnabledVisibleSelection, IsNotTextInsertion,
        DoNotAllowExecutionWhenDisabled}},
      {u8"Unselect",
       {ExecuteUnselect, Supported, EnabledVisibleSelection, IsNotTextInsertion,
        DoNotAllowExecutionWhenDisabled}},
    });

    KRYS_NODISCARD const EditorInternalCommand *InternalCommand(utf8_stringview commandName) noexcept
    {
      if (commandName.empty())
      {
        return nullptr;
      }
      auto it = std::ranges::find_if(
        Commands, [&](const CommandEntry &entry)
        { return infra::StringAlgorithms::ASCIICaseInsensitiveMatch(entry.Name, commandName); });
      return it != Commands.end() ? &it->Command : nullptr;
    }
  }

  Editor::Command Editor::GetCommand(utf8_stringview commandName) noexcept
  {
    return Command(InternalCommand(commandName), EditorCommandSource::MenuOrKeyBinding, _document);
  }

  Editor::Command Editor::GetCommand(utf8_stringview commandName, EditorCommandSource source) noexcept
  {
    return Command(InternalCommand(commandName), source, _document);
  }

  bool Editor::CommandIsSupportedFromMenuOrKeyBinding(utf8_stringview commandName) noexcept
  {
    return InternalCommand(commandName);
  }

  Editor::Command::Command(const EditorInternalCommand *command, EditorCommandSource source,
                           dom::Document &document) noexcept
      : _command(command), _source(source), _document(command ? ShareRefPtr(&document) : nullptr)
  {
    krys_debug_assert(command || !_document);
  }

  LocalFrame *Editor::Command::Frame() const noexcept
  {
    return _document ? _document->Frame() : nullptr;
  }

  bool Editor::Command::Execute(utf8_stringview parameter, dom::Event *triggeringEvent) const noexcept
  {
    if (!IsEnabled(triggeringEvent))
    {
      // Let certain commands be executed when performed explicitly even if they are disabled.
      if (!AllowExecutionWhenDisabled())
      {
        return false;
      }
    }

    (void)_document->UpdateLayoutIgnorePendingStylesheets();
    RefPtr<LocalFrame> frame = ShareRefPtr(Frame());
    if (!frame)
    {
      return false;
    }

    return _command->Execute(*frame, triggeringEvent, _source, parameter);
  }

  bool Editor::Command::Execute(dom::Event *triggeringEvent) const noexcept
  {
    return Execute({}, triggeringEvent);
  }

  bool Editor::Command::IsSupported() const noexcept
  {
    if (!_command)
    {
      return false;
    }
    switch (_source)
    {
      case EditorCommandSource::MenuOrKeyBinding:     return true;
      case EditorCommandSource::DOM:
      case EditorCommandSource::DOMWithUserInterface: return _command->IsSupportedFromDOM(Frame());
    }
    krys_debug_assert(false);
    return false;
  }

  bool Editor::Command::IsEnabled(dom::Event *triggeringEvent) const noexcept
  {
    auto *frame = Frame();
    if (!IsSupported() || !frame)
    {
      return false;
    }
    return _command->IsEnabled(*frame, triggeringEvent, _source);
  }

  bool Editor::Command::IsTextInsertion() const noexcept
  {
    return _command && _command->IsTextInsertion;
  }

  bool Editor::Command::AllowExecutionWhenDisabled() const noexcept
  {
    auto *frame = Frame();
    if (!IsSupported() || !frame)
    {
      return false;
    }
    return _command->AllowExecutionWhenDisabled(*frame, _source);
  }
}
