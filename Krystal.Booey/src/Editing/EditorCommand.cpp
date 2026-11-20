/*
 * Copyright (C) 2006-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2017 Google Inc. All rights reserved.
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2009 Igalia S.L.
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

#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/DOM/Events/TextEvent.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Editing/Commands/TypingCommand.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/Selection/FrameSelection.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Input/EventHandler.hpp"
#include "Krystal.Booey/Page/Clients/EditorClient.hpp"
#include "Krystal.Booey/Page/Frame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Platform/Scrolling/Scrollbar.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include <algorithm>
#include <array>

namespace krys::boo
{
  // FIXME: The editing commands (cutting, pasting, styling, lists, indenting, links, inserting HTML, images
  // and rules), with editing commands; the commands' state and value (bold, the font and colours), with
  // styling. (There's no mark and kill ring, find or spelling; see DIFFERENCES.md.)
  class EditorInternalCommand
  {
  public:
    bool (*Execute)(boo::Frame &, Event *, EditorCommandSource, utf8_stringview) noexcept;
    bool (*IsSupportedFromDOM)(boo::Frame *) noexcept;
    bool (*IsEnabled)(boo::Frame &, Event *, EditorCommandSource) noexcept;
    bool IsTextInsertion;
    bool (*AllowExecutionWhenDisabled)(boo::Frame &, EditorCommandSource) noexcept;
    /// @brief The command's state and value; null for a command without them.
    TriState (*State)(boo::Frame &, Event *) noexcept {nullptr};
    utf8_string (*Value)(boo::Frame &, Event *) noexcept {nullptr};
  };

  namespace
  {
    constexpr bool IsNotTextInsertion = false;
    constexpr bool IsTextInsertion = true;

    KRYS_NODISCARD boo::Frame &TargetFrame(boo::Frame &frame, Event *) noexcept
    {
      return frame;
    }

    bool ExpandSelectionToGranularity(boo::Frame &frame, TextGranularity granularity) noexcept
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

    KRYS_NODISCARD uint32 VerticalScrollDistance(boo::Frame &frame) noexcept
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

    bool ExecuteMakeTextWritingDirectionLeftToRight(boo::Frame &frame, Event *, EditorCommandSource,
                                                    utf8_stringview) noexcept
    {
      auto style = css::MutableStyleProperties::Create();
      (void)style->SetProperty(css::PropertyId::UnicodeBidi, css::ValueId::Embed);
      (void)style->SetProperty(css::PropertyId::Direction, css::ValueId::Ltr);
      frame.Document()->GetEditor().ApplyStyle(style.get(), EditAction::SetInlineWritingDirection);
      return true;
    }

    bool ExecuteMakeTextWritingDirectionNatural(boo::Frame &frame, Event *, EditorCommandSource,
                                                utf8_stringview) noexcept
    {
      auto style = css::MutableStyleProperties::Create();
      (void)style->SetProperty(css::PropertyId::UnicodeBidi, css::ValueId::Normal);
      frame.Document()->GetEditor().ApplyStyle(style.get(), EditAction::SetInlineWritingDirection);
      return true;
    }

    bool ExecuteMakeTextWritingDirectionRightToLeft(boo::Frame &frame, Event *, EditorCommandSource,
                                                    utf8_stringview) noexcept
    {
      auto style = css::MutableStyleProperties::Create();
      (void)style->SetProperty(css::PropertyId::UnicodeBidi, css::ValueId::Embed);
      (void)style->SetProperty(css::PropertyId::Direction, css::ValueId::Rtl);
      frame.Document()->GetEditor().ApplyStyle(style.get(), EditAction::SetInlineWritingDirection);
      return true;
    }

    bool ExecuteMoveBackward(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::CharacterGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveBackwardAndModifySelection(boo::Frame &frame, Event *, EditorCommandSource,
                                               utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::CharacterGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveDown(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                  SelectionDirection::Forward,
                                                  TextGranularity::LineGranularity, UserTriggered::Yes);
    }

    bool ExecuteMoveDownAndModifySelection(boo::Frame &frame, Event *, EditorCommandSource,
                                           utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::LineGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveForward(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::CharacterGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveForwardAndModifySelection(boo::Frame &frame, Event *, EditorCommandSource,
                                              utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::CharacterGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveLeft(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return frame.Document()->Selection().Modify(FrameSelection::Alteration::Move, SelectionDirection::Left,
                                                  TextGranularity::CharacterGranularity, UserTriggered::Yes);
    }

    bool ExecuteMoveLeftAndModifySelection(boo::Frame &frame, Event *, EditorCommandSource,
                                           utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend, SelectionDirection::Left,
                                                 TextGranularity::CharacterGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveRight(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return frame.Document()->Selection().Modify(FrameSelection::Alteration::Move, SelectionDirection::Right,
                                                  TextGranularity::CharacterGranularity, UserTriggered::Yes);
    }

    bool ExecuteMoveRightAndModifySelection(boo::Frame &frame, Event *, EditorCommandSource,
                                            utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Right,
                                                 TextGranularity::CharacterGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToBeginningOfDocument(boo::Frame &frame, Event *, EditorCommandSource,
                                          utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::DocumentBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToBeginningOfDocumentAndModifySelection(boo::Frame &frame, Event *, EditorCommandSource,
                                                            utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::DocumentBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToBeginningOfLine(boo::Frame &frame, Event *, EditorCommandSource,
                                      utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Backward, TextGranularity::LineBoundary,
                                                 UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToBeginningOfLineAndModifySelection(boo::Frame &frame, Event *, EditorCommandSource,
                                                        utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Backward, TextGranularity::LineBoundary,
                                                 UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToBeginningOfParagraph(boo::Frame &frame, Event *, EditorCommandSource,
                                           utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::ParagraphBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToBeginningOfParagraphAndModifySelection(boo::Frame &frame, Event *, EditorCommandSource,
                                                             utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::ParagraphBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToBeginningOfSentence(boo::Frame &frame, Event *, EditorCommandSource,
                                          utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::SentenceBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToBeginningOfSentenceAndModifySelection(boo::Frame &frame, Event *, EditorCommandSource,
                                                            utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::SentenceBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToEndOfDocument(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::DocumentBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToEndOfDocumentAndModifySelection(boo::Frame &frame, Event *, EditorCommandSource,
                                                      utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::DocumentBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToEndOfLine(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Forward, TextGranularity::LineBoundary,
                                                 UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToEndOfLineAndModifySelection(boo::Frame &frame, Event *, EditorCommandSource,
                                                  utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Forward, TextGranularity::LineBoundary,
                                                 UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToEndOfParagraph(boo::Frame &frame, Event *, EditorCommandSource,
                                     utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::ParagraphBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToEndOfParagraphAndModifySelection(boo::Frame &frame, Event *, EditorCommandSource,
                                                       utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::ParagraphBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToEndOfSentence(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::SentenceBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToEndOfSentenceAndModifySelection(boo::Frame &frame, Event *, EditorCommandSource,
                                                      utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::SentenceBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToLeftEndOfLine(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move, SelectionDirection::Left,
                                                 TextGranularity::LineBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToLeftEndOfLineAndModifySelection(boo::Frame &frame, Event *, EditorCommandSource,
                                                      utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend, SelectionDirection::Left,
                                                 TextGranularity::LineBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToRightEndOfLine(boo::Frame &frame, Event *, EditorCommandSource,
                                     utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move, SelectionDirection::Right,
                                                 TextGranularity::LineBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToRightEndOfLineAndModifySelection(boo::Frame &frame, Event *, EditorCommandSource,
                                                       utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Right, TextGranularity::LineBoundary,
                                                 UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveUp(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                  SelectionDirection::Backward,
                                                  TextGranularity::LineGranularity, UserTriggered::Yes);
    }

    bool ExecuteMoveUpAndModifySelection(boo::Frame &frame, Event *, EditorCommandSource,
                                         utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::LineGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveWordBackward(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::WordGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveWordBackwardAndModifySelection(boo::Frame &frame, Event *, EditorCommandSource,
                                                   utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::WordGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveWordForward(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::WordGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveWordForwardAndModifySelection(boo::Frame &frame, Event *, EditorCommandSource,
                                                  utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::WordGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveWordLeft(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move, SelectionDirection::Left,
                                                 TextGranularity::WordGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveWordLeftAndModifySelection(boo::Frame &frame, Event *, EditorCommandSource,
                                               utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend, SelectionDirection::Left,
                                                 TextGranularity::WordGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveWordRight(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move, SelectionDirection::Right,
                                                 TextGranularity::WordGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveWordRightAndModifySelection(boo::Frame &frame, Event *, EditorCommandSource,
                                                utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Right, TextGranularity::WordGranularity,
                                                 UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveParagraphBackwardAndModifySelection(boo::Frame &frame, Event *, EditorCommandSource,
                                                        utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::ParagraphGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveParagraphForwardAndModifySelection(boo::Frame &frame, Event *, EditorCommandSource,
                                                       utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::ParagraphGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMovePageDown(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
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

    bool ExecuteMovePageDownAndModifySelection(boo::Frame &frame, Event *, EditorCommandSource,
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

    bool ExecuteMovePageUp(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
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

    bool ExecuteMovePageUpAndModifySelection(boo::Frame &frame, Event *, EditorCommandSource,
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

    bool ExecuteScrollPageBackward(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return frame.GetEventHandler().LogicalScrollRecursively(
        ScrollLogicalDirection::ScrollBlockDirectionBackward, ScrollGranularity::Page);
    }

    bool ExecuteScrollPageForward(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return frame.GetEventHandler().LogicalScrollRecursively(
        ScrollLogicalDirection::ScrollBlockDirectionForward, ScrollGranularity::Page);
    }

    bool ExecuteScrollLineUp(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return frame.GetEventHandler().ScrollRecursively(ScrollDirection::ScrollUp, ScrollGranularity::Line);
    }

    bool ExecuteScrollLineDown(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return frame.GetEventHandler().ScrollRecursively(ScrollDirection::ScrollDown, ScrollGranularity::Line);
    }

    bool ExecuteScrollToBeginningOfDocument(boo::Frame &frame, Event *, EditorCommandSource,
                                            utf8_stringview) noexcept
    {
      return frame.GetEventHandler().LogicalScrollRecursively(
        ScrollLogicalDirection::ScrollBlockDirectionBackward, ScrollGranularity::Document);
    }

    bool ExecuteScrollToEndOfDocument(boo::Frame &frame, Event *, EditorCommandSource,
                                      utf8_stringview) noexcept
    {
      return frame.GetEventHandler().LogicalScrollRecursively(
        ScrollLogicalDirection::ScrollBlockDirectionForward, ScrollGranularity::Document);
    }

    KRYS_NODISCARD Editor::FromMenuOrKeyBinding FromMenuOrKeyBindingFor(EditorCommandSource source) noexcept
    {
      return source == EditorCommandSource::MenuOrKeyBinding ? Editor::FromMenuOrKeyBinding::Yes
                                                             : Editor::FromMenuOrKeyBinding::No;
    }

    bool ExecuteCopy(boo::Frame &frame, Event *, EditorCommandSource source, utf8_stringview) noexcept
    {
      frame.Document()->GetEditor().Copy(FromMenuOrKeyBindingFor(source));
      return true;
    }

    bool ExecuteCut(boo::Frame &frame, Event *, EditorCommandSource source, utf8_stringview) noexcept
    {
      frame.Document()->GetEditor().Cut(FromMenuOrKeyBindingFor(source));
      return true;
    }

    bool ExecutePaste(boo::Frame &frame, Event *, EditorCommandSource source, utf8_stringview) noexcept
    {
      frame.Document()->GetEditor().Paste(FromMenuOrKeyBindingFor(source));
      return true;
    }

    bool ExecutePasteAndMatchStyle(boo::Frame &frame, Event *, EditorCommandSource source,
                                   utf8_stringview) noexcept
    {
      frame.Document()->GetEditor().PasteAsPlainText(FromMenuOrKeyBindingFor(source));
      return true;
    }

    bool ExecutePasteAsPlainText(boo::Frame &frame, Event *, EditorCommandSource source,
                                 utf8_stringview) noexcept
    {
      frame.Document()->GetEditor().PasteAsPlainText(FromMenuOrKeyBindingFor(source));
      return true;
    }

    bool ExecuteDelete(boo::Frame &frame, Event *, EditorCommandSource source, utf8_stringview) noexcept
    {
      switch (source)
      {
        case EditorCommandSource::MenuOrKeyBinding:
        {
          // Doesn't modify the text if the current selection isn't a range.
          frame.Document()->GetEditor().PerformDelete();
          return true;
        }
        case EditorCommandSource::DOM:
        case EditorCommandSource::DOMWithUserInterface:
          // If the current selection is a caret, delete the preceding character. IE performs forwardDelete,
          // but we currently side with Firefox. Doesn't scroll to make the selection visible, or modify the
          // kill ring (this time, siding with IE, not Firefox).
          TypingCommand::DeleteKeyPressed(
            *frame.Document(), frame.Document()->GetEditor().CanSmartCopyOrDelete()
                                 ? EnumSet<TypingCommand::Option> {TypingCommand::Option::SmartDelete}
                                 : EnumSet<TypingCommand::Option> {});
          return true;
      }
      krys_debug_assert(false);
      return false;
    }

    bool ExecuteDeleteBackward(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->GetEditor().DeleteWithDirection(
        SelectionDirection::Backward, TextGranularity::CharacterGranularity, false, true);
      return true;
    }

    bool ExecuteDeleteBackwardByDecomposingPreviousCharacter(boo::Frame &frame, Event *, EditorCommandSource,
                                                             utf8_stringview) noexcept
    {
      (void)frame.Document()->GetEditor().DeleteWithDirection(
        SelectionDirection::Backward, TextGranularity::CharacterGranularity, false, true);
      return true;
    }

    bool ExecuteDeleteForward(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->GetEditor().DeleteWithDirection(
        SelectionDirection::Forward, TextGranularity::CharacterGranularity, false, true);
      return true;
    }

    bool ExecuteDeleteToBeginningOfLine(boo::Frame &frame, Event *, EditorCommandSource,
                                        utf8_stringview) noexcept
    {
      (void)frame.Document()->GetEditor().DeleteWithDirection(SelectionDirection::Backward,
                                                              TextGranularity::LineBoundary, true, false);
      return true;
    }

    bool ExecuteDeleteToBeginningOfParagraph(boo::Frame &frame, Event *, EditorCommandSource,
                                             utf8_stringview) noexcept
    {
      (void)frame.Document()->GetEditor().DeleteWithDirection(
        SelectionDirection::Backward, TextGranularity::ParagraphBoundary, true, false);
      return true;
    }

    bool ExecuteDeleteToEndOfLine(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      // Despite its name, this command should delete the newline at the end of
      // a paragraph if you are at the end of a paragraph (like DeleteToEndOfParagraph).
      (void)frame.Document()->GetEditor().DeleteWithDirection(SelectionDirection::Forward,
                                                              TextGranularity::LineBoundary, true, false);
      return true;
    }

    bool ExecuteDeleteToEndOfParagraph(boo::Frame &frame, Event *, EditorCommandSource,
                                       utf8_stringview) noexcept
    {
      // Despite its name, this command should delete the newline at the end of
      // a paragraph if you are at the end of a paragraph.
      (void)frame.Document()->GetEditor().DeleteWithDirection(
        SelectionDirection::Forward, TextGranularity::ParagraphBoundary, true, false);
      return true;
    }

    bool ExecuteDeleteWordBackward(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->GetEditor().DeleteWithDirection(SelectionDirection::Backward,
                                                              TextGranularity::WordGranularity, true, false);
      return true;
    }

    bool ExecuteDeleteWordForward(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->GetEditor().DeleteWithDirection(SelectionDirection::Forward,
                                                              TextGranularity::WordGranularity, true, false);
      return true;
    }

    bool ExecuteForwardDelete(boo::Frame &frame, Event *, EditorCommandSource source,
                              utf8_stringview) noexcept
    {
      switch (source)
      {
        case EditorCommandSource::MenuOrKeyBinding:
          (void)frame.Document()->GetEditor().DeleteWithDirection(
            SelectionDirection::Forward, TextGranularity::CharacterGranularity, false, true);
          return true;
        case EditorCommandSource::DOM:
        case EditorCommandSource::DOMWithUserInterface:
          // Doesn't scroll to make the selection visible, or modify the kill ring.
          // ForwardDelete is not implemented in IE or Firefox, so this behavior is only needed for
          // backward compatibility with ourselves, and for consistency with Delete.
          TypingCommand::ForwardDeleteKeyPressed(*frame.Document());
          return true;
      }
      krys_debug_assert(false);
      return false;
    }

    bool ExecuteInsertBacktab(boo::Frame &frame, Event *event, EditorCommandSource, utf8_stringview) noexcept
    {
      return TargetFrame(frame, event)
        .GetEventHandler()
        .HandleTextInputEvent(u8"\t", event, dom::TextEventInputType::BackTab);
    }

    bool ExecuteInsertLineBreak(boo::Frame &frame, Event *event, EditorCommandSource source,
                                utf8_stringview) noexcept
    {
      switch (source)
      {
        case EditorCommandSource::MenuOrKeyBinding:
          return TargetFrame(frame, event)
            .GetEventHandler()
            .HandleTextInputEvent(u8"\n", event, dom::TextEventInputType::LineBreak);
        case EditorCommandSource::DOM:
        case EditorCommandSource::DOMWithUserInterface:
          // Doesn't scroll to make the selection visible, or modify the kill ring.
          // InsertLineBreak is not implemented in IE or Firefox, so this behavior is only needed for
          // backward compatibility with ourselves, and for consistency with other commands.
          TypingCommand::InsertLineBreak(*frame.Document(), {});
          return true;
      }
      krys_debug_assert(false);
      return false;
    }

    bool ExecuteInsertNewline(boo::Frame &frame, Event *event, EditorCommandSource, utf8_stringview) noexcept
    {
      auto &targetFrame = TargetFrame(frame, event);
      return targetFrame.GetEventHandler().HandleTextInputEvent(
        u8"\n", event,
        targetFrame.Document()->GetEditor().CanEditRichly() ? dom::TextEventInputType::Keyboard
                                                            : dom::TextEventInputType::LineBreak);
    }

    bool ExecuteInsertParagraph(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      TypingCommand::InsertParagraphSeparator(*frame.Document(), {});
      return true;
    }

    bool ExecuteInsertTab(boo::Frame &frame, Event *event, EditorCommandSource, utf8_stringview) noexcept
    {
      return TargetFrame(frame, event)
        .GetEventHandler()
        .HandleTextInputEvent(u8"\t", event, dom::TextEventInputType::Keyboard);
    }

    bool ExecuteInsertText(boo::Frame &frame, Event *event, EditorCommandSource,
                           utf8_stringview value) noexcept
    {
      TypingCommand::InsertText(*frame.Document(), value, event, {});
      return true;
    }

    bool ExecuteRedo(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      frame.Document()->GetEditor().Redo();
      return true;
    }

    bool ExecuteUndo(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      frame.Document()->GetEditor().Undo();
      return true;
    }

    bool ExecuteSelectAll(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      frame.Document()->Selection().SelectAll();
      return true;
    }

    bool ExecuteSelectLine(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return ExpandSelectionToGranularity(frame, TextGranularity::LineGranularity);
    }

    bool ExecuteSelectParagraph(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return ExpandSelectionToGranularity(frame, TextGranularity::ParagraphGranularity);
    }

    bool ExecuteSelectSentence(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return ExpandSelectionToGranularity(frame, TextGranularity::SentenceGranularity);
    }

    bool ExecuteSelectWord(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return ExpandSelectionToGranularity(frame, TextGranularity::WordGranularity);
    }

    bool ExecuteUnselect(boo::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      frame.Document()->Selection().Clear();
      return true;
    }

    // Supported functions

    bool Supported(boo::Frame *) noexcept
    {
      return true;
    }

    bool SupportedFromMenuOrKeyBinding(boo::Frame *) noexcept
    {
      return false;
    }

    // With no script access to the clipboard, copying and cutting are allowed, as the default policy allows
    // them.
    KRYS_NODISCARD bool DefaultValueForSupportedCopyCut(boo::Frame &) noexcept
    {
      return true;
    }

    bool SupportedCopyCut(boo::Frame *frame) noexcept
    {
      if (!frame)
      {
        return false;
      }

      bool defaultValue = DefaultValueForSupportedCopyCut(*frame);

      EditorClient *client = frame->Document()->GetEditor().Client();
      return client ? client->CanCopyCut(frame, defaultValue) : defaultValue;
    }

    // With no script access to the clipboard, pasting from the DOM is disallowed, as the default settings
    // disallow it.
    bool SupportedPaste(boo::Frame *frame) noexcept
    {
      if (!frame)
      {
        return false;
      }

      bool defaultValue = false;

      EditorClient *client = frame->Document()->GetEditor().Client();
      return client ? client->CanPaste(frame, defaultValue) : defaultValue;
    }

    // Enabled functions

    bool Enabled(boo::Frame &, Event *, EditorCommandSource) noexcept
    {
      return true;
    }

    bool EnabledRedo(boo::Frame &frame, Event *, EditorCommandSource) noexcept
    {
      return frame.Document()->GetEditor().CanRedo();
    }

    bool EnabledUndo(boo::Frame &frame, Event *, EditorCommandSource) noexcept
    {
      return frame.Document()->GetEditor().CanUndo();
    }

    bool EnabledVisibleSelection(boo::Frame &frame, Event *event, EditorCommandSource) noexcept
    {
      // The term "visible" here includes a caret in editable text or a range in any text.
      const VisibleSelection &selection = frame.Document()->GetEditor().SelectionForCommand(event);
      return (selection.IsCaret() && selection.IsContentEditable()) || selection.IsRange();
    }

    KRYS_NODISCARD bool CaretBrowsingEnabled(boo::Frame &frame) noexcept
    {
      return frame.Document()->GetSettings().CaretBrowsingEnabled;
    }

    bool EnabledVisibleSelectionOrCaretBrowsing(boo::Frame &frame, Event *event,
                                                EditorCommandSource source) noexcept
    {
      return CaretBrowsingEnabled(frame) || EnabledVisibleSelection(frame, event, source);
    }

    bool EnabledCopy(boo::Frame &frame, Event *, EditorCommandSource) noexcept
    {
      auto &editor = frame.Document()->GetEditor();
      return editor.CanDHTMLCopy() || editor.CanCopy();
    }

    bool EnabledCut(boo::Frame &frame, Event *, EditorCommandSource) noexcept
    {
      auto &editor = frame.Document()->GetEditor();
      return editor.CanDHTMLCut() || editor.CanCut();
    }

    bool EnabledPaste(boo::Frame &frame, Event *, EditorCommandSource) noexcept
    {
      auto &editor = frame.Document()->GetEditor();
      return editor.CanDHTMLPaste() || editor.CanPaste();
    }

    bool EnabledInEditableText(boo::Frame &frame, Event *event, EditorCommandSource) noexcept
    {
      return frame.Document()->GetEditor().SelectionForCommand(event).RootEditableElement();
    }

    bool EnabledInEditableTextOrCaretBrowsing(boo::Frame &frame, Event *event,
                                              EditorCommandSource source) noexcept
    {
      return CaretBrowsingEnabled(frame) || EnabledInEditableText(frame, event, source);
    }

    bool EnabledDelete(boo::Frame &frame, Event *event, EditorCommandSource source) noexcept
    {
      switch (source)
      {
        case EditorCommandSource::MenuOrKeyBinding: return frame.Document()->GetEditor().CanDelete();
        case EditorCommandSource::DOM:
        case EditorCommandSource::DOMWithUserInterface:
          // "Delete" from DOM is like delete/backspace keypress, affects selected range if non-empty,
          // otherwise removes a character
          return EnabledInEditableText(frame, event, source);
      }
      krys_debug_assert(false);
      return false;
    }

    bool EnabledInRichlyEditableText(boo::Frame &frame, Event *, EditorCommandSource) noexcept
    {
      const auto &selection = frame.Document()->Selection().Selection();
      return selection.IsCaretOrRange() && selection.IsContentRichlyEditable()
             && selection.RootEditableElement();
    }

    // AllowExecutionWhenDisabled functions

    bool DoNotAllowExecutionWhenDisabled(boo::Frame &, EditorCommandSource) noexcept
    {
      return false;
    }

    bool AllowExecutionWhenDisabledCopyCut(boo::Frame &, EditorCommandSource source) noexcept
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

    bool AllowExecutionWhenDisabledPaste(boo::Frame &, EditorCommandSource source) noexcept
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
      {u8"Cut",
       {ExecuteCut, SupportedCopyCut, EnabledCut, IsNotTextInsertion, AllowExecutionWhenDisabledCopyCut}},
      {u8"Delete",
       {ExecuteDelete, Supported, EnabledDelete, IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"DeleteBackward",
       {ExecuteDeleteBackward, SupportedFromMenuOrKeyBinding, EnabledInEditableText, IsNotTextInsertion,
        DoNotAllowExecutionWhenDisabled}},
      {u8"DeleteBackwardByDecomposingPreviousCharacter",
       {ExecuteDeleteBackwardByDecomposingPreviousCharacter, SupportedFromMenuOrKeyBinding,
        EnabledInEditableText, IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"DeleteForward",
       {ExecuteDeleteForward, SupportedFromMenuOrKeyBinding, EnabledInEditableText, IsNotTextInsertion,
        DoNotAllowExecutionWhenDisabled}},
      {u8"DeleteToBeginningOfLine",
       {ExecuteDeleteToBeginningOfLine, SupportedFromMenuOrKeyBinding, EnabledInEditableText,
        IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"DeleteToBeginningOfParagraph",
       {ExecuteDeleteToBeginningOfParagraph, SupportedFromMenuOrKeyBinding, EnabledInEditableText,
        IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"DeleteToEndOfLine",
       {ExecuteDeleteToEndOfLine, SupportedFromMenuOrKeyBinding, EnabledInEditableText, IsNotTextInsertion,
        DoNotAllowExecutionWhenDisabled}},
      {u8"DeleteToEndOfParagraph",
       {ExecuteDeleteToEndOfParagraph, SupportedFromMenuOrKeyBinding, EnabledInEditableText,
        IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"DeleteWordBackward",
       {ExecuteDeleteWordBackward, SupportedFromMenuOrKeyBinding, EnabledInEditableText, IsNotTextInsertion,
        DoNotAllowExecutionWhenDisabled}},
      {u8"DeleteWordForward",
       {ExecuteDeleteWordForward, SupportedFromMenuOrKeyBinding, EnabledInEditableText, IsNotTextInsertion,
        DoNotAllowExecutionWhenDisabled}},
      {u8"ForwardDelete",
       {ExecuteForwardDelete, Supported, EnabledInEditableText, IsNotTextInsertion,
        DoNotAllowExecutionWhenDisabled}},
      {u8"InsertBacktab",
       {ExecuteInsertBacktab, SupportedFromMenuOrKeyBinding, EnabledInEditableText, IsTextInsertion,
        DoNotAllowExecutionWhenDisabled}},
      {u8"InsertLineBreak",
       {ExecuteInsertLineBreak, Supported, EnabledInEditableText, IsTextInsertion,
        DoNotAllowExecutionWhenDisabled}},
      {u8"InsertNewline",
       {ExecuteInsertNewline, SupportedFromMenuOrKeyBinding, EnabledInEditableText, IsTextInsertion,
        DoNotAllowExecutionWhenDisabled}},
      {u8"InsertParagraph",
       {ExecuteInsertParagraph, Supported, EnabledInEditableText, IsNotTextInsertion,
        DoNotAllowExecutionWhenDisabled}},
      {u8"InsertTab",
       {ExecuteInsertTab, SupportedFromMenuOrKeyBinding, EnabledInEditableText, IsTextInsertion,
        DoNotAllowExecutionWhenDisabled}},
      {u8"InsertText",
       {ExecuteInsertText, Supported, EnabledInEditableText, IsTextInsertion,
        DoNotAllowExecutionWhenDisabled}},
      {u8"MakeTextWritingDirectionLeftToRight",
       {ExecuteMakeTextWritingDirectionLeftToRight, SupportedFromMenuOrKeyBinding,
        EnabledInRichlyEditableText, IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MakeTextWritingDirectionNatural",
       {ExecuteMakeTextWritingDirectionNatural, SupportedFromMenuOrKeyBinding, EnabledInRichlyEditableText,
        IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
      {u8"MakeTextWritingDirectionRightToLeft",
       {ExecuteMakeTextWritingDirectionRightToLeft, SupportedFromMenuOrKeyBinding,
        EnabledInRichlyEditableText, IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
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
      {u8"Paste",
       {ExecutePaste, SupportedPaste, EnabledPaste, IsNotTextInsertion, AllowExecutionWhenDisabledPaste}},
      {u8"PasteAndMatchStyle",
       {ExecutePasteAndMatchStyle, SupportedPaste, EnabledPaste, IsNotTextInsertion,
        AllowExecutionWhenDisabledPaste}},
      {u8"PasteAsPlainText",
       {ExecutePasteAsPlainText, SupportedPaste, EnabledPaste, IsNotTextInsertion,
        AllowExecutionWhenDisabledPaste}},
      {u8"Redo", {ExecuteRedo, Supported, EnabledRedo, IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
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
      {u8"Undo", {ExecuteUndo, Supported, EnabledUndo, IsNotTextInsertion, DoNotAllowExecutionWhenDisabled}},
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
                           Document &document) noexcept
      : _command(command), _source(source), _document(command ? ShareRefPtr(&document) : nullptr)
  {
    krys_debug_assert(command || !_document);
  }

  boo::Frame *Editor::Command::Frame() const noexcept
  {
    return _document ? _document->Frame() : nullptr;
  }

  bool Editor::Command::Execute(utf8_stringview parameter, Event *triggeringEvent) const noexcept
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
    RefPtr<boo::Frame> frame = ShareRefPtr(Frame());
    if (!frame)
    {
      return false;
    }

    return _command->Execute(*frame, triggeringEvent, _source, parameter);
  }

  bool Editor::Command::Execute(Event *triggeringEvent) const noexcept
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

  bool Editor::Command::IsEnabled(Event *triggeringEvent) const noexcept
  {
    auto *frame = Frame();
    if (!IsSupported() || !frame)
    {
      return false;
    }
    return _command->IsEnabled(*frame, triggeringEvent, _source);
  }

  TriState Editor::Command::State(Event *triggeringEvent) const noexcept
  {
    auto *frame = Frame();
    if (!IsSupported() || !frame || !_command->State)
    {
      return TriState::False;
    }
    return _command->State(*frame, triggeringEvent);
  }

  utf8_string Editor::Command::Value(Event *triggeringEvent) const noexcept
  {
    auto *frame = Frame();
    if (!IsSupported() || !frame)
    {
      return {};
    }
    if (!_command->Value)
    {
      if (!_command->State)
      {
        return {};
      }
      return _command->State(*frame, triggeringEvent) == TriState::True ? u8"true" : u8"false";
    }
    return _command->Value(*frame, triggeringEvent);
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
