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

#include "Krystal.Web/Css/Properties/MutableStyleProperties.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Editing/Commands/TypingCommand.hpp"
#include "Krystal.Web/Editing/Editor.hpp"
#include "Krystal.Web/Editing/Selection/FrameSelection.hpp"
#include "Krystal.Web/Infra/StringAlgorithms.hpp"
#include "Krystal.Web/Input/Internals/EventHandler.hpp"
#include "Krystal.Web/Input/UIEvents/TextEvent.hpp"
#include "Krystal.Web/Page/Clients/EditorClient.hpp"
#include "Krystal.Web/Page/Frame.hpp"
#include "Krystal.Web/Page/FrameView.hpp"
#include "Krystal.Web/Page/Settings.hpp"
#include "Krystal.Web/Platform/Scrolling/Scrollbar.hpp"
#include "Krystal.Web/Rendering/RenderBox.hpp"
#include <algorithm>
#include <array>

namespace krys::web
{
  // FIXME: The editing commands (cutting, pasting, styling, lists, indenting, links, inserting HTML, images
  // and rules), with editing commands; the commands' state and value (bold, the font and colours), with
  // styling. (There's no mark and kill ring, find or spelling; see DIFFERENCES.md.)
  class EditorInternalCommand
  {
  public:
    bool (*Execute)(web::Frame &, Event *, EditorCommandSource, utf8_stringview) noexcept;
    bool (*IsSupportedFromDOM)(web::Frame *) noexcept;
    bool (*IsEnabled)(web::Frame &, Event *, EditorCommandSource) noexcept;
    bool IsTextInsertion;
    bool (*AllowExecutionWhenDisabled)(web::Frame &, EditorCommandSource) noexcept;
    /// @brief The command's state and value; null for a command without them.
    TriState (*State)(web::Frame &, Event *) noexcept {nullptr};
    utf8_string (*Value)(web::Frame &, Event *) noexcept {nullptr};
  };

  namespace
  {
    constexpr bool IsNotTextInsertion = false;
    constexpr bool IsTextInsertion = true;

    KRYS_NODISCARD web::Frame &TargetFrame(web::Frame &frame, Event *) noexcept
    {
      return frame;
    }

    bool ExpandSelectionToGranularity(web::Frame &frame, TextGranularity granularity) noexcept
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

    KRYS_NODISCARD uint32 VerticalScrollDistance(web::Frame &frame) noexcept
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

    bool ExecuteMakeTextWritingDirectionLeftToRight(web::Frame &frame, Event *, EditorCommandSource,
                                                    utf8_stringview) noexcept
    {
      auto style = css::MutableStyleProperties::Create();
      (void)style->SetProperty(css::PropertyId::UnicodeBidi, css::ValueId::Embed);
      (void)style->SetProperty(css::PropertyId::Direction, css::ValueId::Ltr);
      frame.Document()->GetEditor().ApplyStyle(style.get(), EditAction::SetInlineWritingDirection);
      return true;
    }

    bool ExecuteMakeTextWritingDirectionNatural(web::Frame &frame, Event *, EditorCommandSource,
                                                utf8_stringview) noexcept
    {
      auto style = css::MutableStyleProperties::Create();
      (void)style->SetProperty(css::PropertyId::UnicodeBidi, css::ValueId::Normal);
      frame.Document()->GetEditor().ApplyStyle(style.get(), EditAction::SetInlineWritingDirection);
      return true;
    }

    bool ExecuteMakeTextWritingDirectionRightToLeft(web::Frame &frame, Event *, EditorCommandSource,
                                                    utf8_stringview) noexcept
    {
      auto style = css::MutableStyleProperties::Create();
      (void)style->SetProperty(css::PropertyId::UnicodeBidi, css::ValueId::Embed);
      (void)style->SetProperty(css::PropertyId::Direction, css::ValueId::Rtl);
      frame.Document()->GetEditor().ApplyStyle(style.get(), EditAction::SetInlineWritingDirection);
      return true;
    }

    bool ExecuteMoveBackward(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::CharacterGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveBackwardAndModifySelection(web::Frame &frame, Event *, EditorCommandSource,
                                               utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::CharacterGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveDown(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                  SelectionDirection::Forward,
                                                  TextGranularity::LineGranularity, UserTriggered::Yes);
    }

    bool ExecuteMoveDownAndModifySelection(web::Frame &frame, Event *, EditorCommandSource,
                                           utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::LineGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveForward(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::CharacterGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveForwardAndModifySelection(web::Frame &frame, Event *, EditorCommandSource,
                                              utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::CharacterGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveLeft(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return frame.Document()->Selection().Modify(FrameSelection::Alteration::Move, SelectionDirection::Left,
                                                  TextGranularity::CharacterGranularity, UserTriggered::Yes);
    }

    bool ExecuteMoveLeftAndModifySelection(web::Frame &frame, Event *, EditorCommandSource,
                                           utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend, SelectionDirection::Left,
                                                 TextGranularity::CharacterGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveRight(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return frame.Document()->Selection().Modify(FrameSelection::Alteration::Move, SelectionDirection::Right,
                                                  TextGranularity::CharacterGranularity, UserTriggered::Yes);
    }

    bool ExecuteMoveRightAndModifySelection(web::Frame &frame, Event *, EditorCommandSource,
                                            utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Right,
                                                 TextGranularity::CharacterGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToBeginningOfDocument(web::Frame &frame, Event *, EditorCommandSource,
                                          utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::DocumentBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToBeginningOfDocumentAndModifySelection(web::Frame &frame, Event *, EditorCommandSource,
                                                            utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::DocumentBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToBeginningOfLine(web::Frame &frame, Event *, EditorCommandSource,
                                      utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Backward, TextGranularity::LineBoundary,
                                                 UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToBeginningOfLineAndModifySelection(web::Frame &frame, Event *, EditorCommandSource,
                                                        utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Backward, TextGranularity::LineBoundary,
                                                 UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToBeginningOfParagraph(web::Frame &frame, Event *, EditorCommandSource,
                                           utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::ParagraphBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToBeginningOfParagraphAndModifySelection(web::Frame &frame, Event *, EditorCommandSource,
                                                             utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::ParagraphBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToBeginningOfSentence(web::Frame &frame, Event *, EditorCommandSource,
                                          utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::SentenceBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToBeginningOfSentenceAndModifySelection(web::Frame &frame, Event *, EditorCommandSource,
                                                            utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::SentenceBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToEndOfDocument(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::DocumentBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToEndOfDocumentAndModifySelection(web::Frame &frame, Event *, EditorCommandSource,
                                                      utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::DocumentBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToEndOfLine(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Forward, TextGranularity::LineBoundary,
                                                 UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToEndOfLineAndModifySelection(web::Frame &frame, Event *, EditorCommandSource,
                                                  utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Forward, TextGranularity::LineBoundary,
                                                 UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToEndOfParagraph(web::Frame &frame, Event *, EditorCommandSource,
                                     utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::ParagraphBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToEndOfParagraphAndModifySelection(web::Frame &frame, Event *, EditorCommandSource,
                                                       utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::ParagraphBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToEndOfSentence(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::SentenceBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToEndOfSentenceAndModifySelection(web::Frame &frame, Event *, EditorCommandSource,
                                                      utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::SentenceBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToLeftEndOfLine(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move, SelectionDirection::Left,
                                                 TextGranularity::LineBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToLeftEndOfLineAndModifySelection(web::Frame &frame, Event *, EditorCommandSource,
                                                      utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend, SelectionDirection::Left,
                                                 TextGranularity::LineBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToRightEndOfLine(web::Frame &frame, Event *, EditorCommandSource,
                                     utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move, SelectionDirection::Right,
                                                 TextGranularity::LineBoundary, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveToRightEndOfLineAndModifySelection(web::Frame &frame, Event *, EditorCommandSource,
                                                       utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Right, TextGranularity::LineBoundary,
                                                 UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveUp(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                  SelectionDirection::Backward,
                                                  TextGranularity::LineGranularity, UserTriggered::Yes);
    }

    bool ExecuteMoveUpAndModifySelection(web::Frame &frame, Event *, EditorCommandSource,
                                         utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::LineGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveWordBackward(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::WordGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveWordBackwardAndModifySelection(web::Frame &frame, Event *, EditorCommandSource,
                                                   utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::WordGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveWordForward(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::WordGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveWordForwardAndModifySelection(web::Frame &frame, Event *, EditorCommandSource,
                                                  utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::WordGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveWordLeft(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move, SelectionDirection::Left,
                                                 TextGranularity::WordGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveWordLeftAndModifySelection(web::Frame &frame, Event *, EditorCommandSource,
                                               utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend, SelectionDirection::Left,
                                                 TextGranularity::WordGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveWordRight(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Move, SelectionDirection::Right,
                                                 TextGranularity::WordGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveWordRightAndModifySelection(web::Frame &frame, Event *, EditorCommandSource,
                                                utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Right, TextGranularity::WordGranularity,
                                                 UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveParagraphBackwardAndModifySelection(web::Frame &frame, Event *, EditorCommandSource,
                                                        utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Backward,
                                                 TextGranularity::ParagraphGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMoveParagraphForwardAndModifySelection(web::Frame &frame, Event *, EditorCommandSource,
                                                       utf8_stringview) noexcept
    {
      (void)frame.Document()->Selection().Modify(FrameSelection::Alteration::Extend,
                                                 SelectionDirection::Forward,
                                                 TextGranularity::ParagraphGranularity, UserTriggered::Yes);
      return true;
    }

    bool ExecuteMovePageDown(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
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

    bool ExecuteMovePageDownAndModifySelection(web::Frame &frame, Event *, EditorCommandSource,
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

    bool ExecuteMovePageUp(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
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

    bool ExecuteMovePageUpAndModifySelection(web::Frame &frame, Event *, EditorCommandSource,
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

    bool ExecuteScrollPageBackward(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return frame.GetEventHandler().LogicalScrollRecursively(
        ScrollLogicalDirection::ScrollBlockDirectionBackward, ScrollGranularity::Page);
    }

    bool ExecuteScrollPageForward(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return frame.GetEventHandler().LogicalScrollRecursively(
        ScrollLogicalDirection::ScrollBlockDirectionForward, ScrollGranularity::Page);
    }

    bool ExecuteScrollLineUp(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return frame.GetEventHandler().ScrollRecursively(ScrollDirection::ScrollUp, ScrollGranularity::Line);
    }

    bool ExecuteScrollLineDown(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return frame.GetEventHandler().ScrollRecursively(ScrollDirection::ScrollDown, ScrollGranularity::Line);
    }

    bool ExecuteScrollToBeginningOfDocument(web::Frame &frame, Event *, EditorCommandSource,
                                            utf8_stringview) noexcept
    {
      return frame.GetEventHandler().LogicalScrollRecursively(
        ScrollLogicalDirection::ScrollBlockDirectionBackward, ScrollGranularity::Document);
    }

    bool ExecuteScrollToEndOfDocument(web::Frame &frame, Event *, EditorCommandSource,
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

    bool ExecuteCopy(web::Frame &frame, Event *, EditorCommandSource source, utf8_stringview) noexcept
    {
      frame.Document()->GetEditor().Copy(FromMenuOrKeyBindingFor(source));
      return true;
    }

    bool ExecuteCut(web::Frame &frame, Event *, EditorCommandSource source, utf8_stringview) noexcept
    {
      frame.Document()->GetEditor().Cut(FromMenuOrKeyBindingFor(source));
      return true;
    }

    bool ExecutePaste(web::Frame &frame, Event *, EditorCommandSource source, utf8_stringview) noexcept
    {
      frame.Document()->GetEditor().Paste(FromMenuOrKeyBindingFor(source));
      return true;
    }

    bool ExecutePasteAndMatchStyle(web::Frame &frame, Event *, EditorCommandSource source,
                                   utf8_stringview) noexcept
    {
      frame.Document()->GetEditor().PasteAsPlainText(FromMenuOrKeyBindingFor(source));
      return true;
    }

    bool ExecutePasteAsPlainText(web::Frame &frame, Event *, EditorCommandSource source,
                                 utf8_stringview) noexcept
    {
      frame.Document()->GetEditor().PasteAsPlainText(FromMenuOrKeyBindingFor(source));
      return true;
    }

    bool ExecuteDelete(web::Frame &frame, Event *, EditorCommandSource source, utf8_stringview) noexcept
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

    bool ExecuteDeleteBackward(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->GetEditor().DeleteWithDirection(
        SelectionDirection::Backward, TextGranularity::CharacterGranularity, false, true);
      return true;
    }

    bool ExecuteDeleteBackwardByDecomposingPreviousCharacter(web::Frame &frame, Event *, EditorCommandSource,
                                                             utf8_stringview) noexcept
    {
      (void)frame.Document()->GetEditor().DeleteWithDirection(
        SelectionDirection::Backward, TextGranularity::CharacterGranularity, false, true);
      return true;
    }

    bool ExecuteDeleteForward(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->GetEditor().DeleteWithDirection(
        SelectionDirection::Forward, TextGranularity::CharacterGranularity, false, true);
      return true;
    }

    bool ExecuteDeleteToBeginningOfLine(web::Frame &frame, Event *, EditorCommandSource,
                                        utf8_stringview) noexcept
    {
      (void)frame.Document()->GetEditor().DeleteWithDirection(SelectionDirection::Backward,
                                                              TextGranularity::LineBoundary, true, false);
      return true;
    }

    bool ExecuteDeleteToBeginningOfParagraph(web::Frame &frame, Event *, EditorCommandSource,
                                             utf8_stringview) noexcept
    {
      (void)frame.Document()->GetEditor().DeleteWithDirection(
        SelectionDirection::Backward, TextGranularity::ParagraphBoundary, true, false);
      return true;
    }

    bool ExecuteDeleteToEndOfLine(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      // Despite its name, this command should delete the newline at the end of
      // a paragraph if you are at the end of a paragraph (like DeleteToEndOfParagraph).
      (void)frame.Document()->GetEditor().DeleteWithDirection(SelectionDirection::Forward,
                                                              TextGranularity::LineBoundary, true, false);
      return true;
    }

    bool ExecuteDeleteToEndOfParagraph(web::Frame &frame, Event *, EditorCommandSource,
                                       utf8_stringview) noexcept
    {
      // Despite its name, this command should delete the newline at the end of
      // a paragraph if you are at the end of a paragraph.
      (void)frame.Document()->GetEditor().DeleteWithDirection(
        SelectionDirection::Forward, TextGranularity::ParagraphBoundary, true, false);
      return true;
    }

    bool ExecuteDeleteWordBackward(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->GetEditor().DeleteWithDirection(SelectionDirection::Backward,
                                                              TextGranularity::WordGranularity, true, false);
      return true;
    }

    bool ExecuteDeleteWordForward(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      (void)frame.Document()->GetEditor().DeleteWithDirection(SelectionDirection::Forward,
                                                              TextGranularity::WordGranularity, true, false);
      return true;
    }

    bool ExecuteForwardDelete(web::Frame &frame, Event *, EditorCommandSource source,
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

    bool ExecuteInsertBacktab(web::Frame &frame, Event *event, EditorCommandSource, utf8_stringview) noexcept
    {
      return TargetFrame(frame, event)
        .GetEventHandler()
        .HandleTextInputEvent(u8"\t", event, ui_events::TextEventInputType::BackTab);
    }

    bool ExecuteInsertLineBreak(web::Frame &frame, Event *event, EditorCommandSource source,
                                utf8_stringview) noexcept
    {
      switch (source)
      {
        case EditorCommandSource::MenuOrKeyBinding:
          return TargetFrame(frame, event)
            .GetEventHandler()
            .HandleTextInputEvent(u8"\n", event, ui_events::TextEventInputType::LineBreak);
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

    bool ExecuteInsertNewline(web::Frame &frame, Event *event, EditorCommandSource, utf8_stringview) noexcept
    {
      auto &targetFrame = TargetFrame(frame, event);
      return targetFrame.GetEventHandler().HandleTextInputEvent(
        u8"\n", event,
        targetFrame.Document()->GetEditor().CanEditRichly() ? ui_events::TextEventInputType::Keyboard
                                                            : ui_events::TextEventInputType::LineBreak);
    }

    bool ExecuteInsertParagraph(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      TypingCommand::InsertParagraphSeparator(*frame.Document(), {});
      return true;
    }

    bool ExecuteInsertTab(web::Frame &frame, Event *event, EditorCommandSource, utf8_stringview) noexcept
    {
      return TargetFrame(frame, event)
        .GetEventHandler()
        .HandleTextInputEvent(u8"\t", event, ui_events::TextEventInputType::Keyboard);
    }

    bool ExecuteInsertText(web::Frame &frame, Event *event, EditorCommandSource,
                           utf8_stringview value) noexcept
    {
      TypingCommand::InsertText(*frame.Document(), value, event, {});
      return true;
    }

    bool ExecuteRedo(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      frame.Document()->GetEditor().Redo();
      return true;
    }

    bool ExecuteUndo(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      frame.Document()->GetEditor().Undo();
      return true;
    }

    bool ExecuteSelectAll(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      frame.Document()->Selection().SelectAll();
      return true;
    }

    bool ExecuteSelectLine(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return ExpandSelectionToGranularity(frame, TextGranularity::LineGranularity);
    }

    bool ExecuteSelectParagraph(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return ExpandSelectionToGranularity(frame, TextGranularity::ParagraphGranularity);
    }

    bool ExecuteSelectSentence(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return ExpandSelectionToGranularity(frame, TextGranularity::SentenceGranularity);
    }

    bool ExecuteSelectWord(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      return ExpandSelectionToGranularity(frame, TextGranularity::WordGranularity);
    }

    bool ExecuteUnselect(web::Frame &frame, Event *, EditorCommandSource, utf8_stringview) noexcept
    {
      frame.Document()->Selection().Clear();
      return true;
    }

    // Supported functions

    bool Supported(web::Frame *) noexcept
    {
      return true;
    }

    bool SupportedFromMenuOrKeyBinding(web::Frame *) noexcept
    {
      return false;
    }

    // With no script access to the clipboard, copying and cutting are allowed, as the default policy allows
    // them.
    KRYS_NODISCARD bool DefaultValueForSupportedCopyCut(web::Frame &) noexcept
    {
      return true;
    }

    bool SupportedCopyCut(web::Frame *frame) noexcept
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
    bool SupportedPaste(web::Frame *frame) noexcept
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

    bool Enabled(web::Frame &, Event *, EditorCommandSource) noexcept
    {
      return true;
    }

    bool EnabledRedo(web::Frame &frame, Event *, EditorCommandSource) noexcept
    {
      return frame.Document()->GetEditor().CanRedo();
    }

    bool EnabledUndo(web::Frame &frame, Event *, EditorCommandSource) noexcept
    {
      return frame.Document()->GetEditor().CanUndo();
    }

    bool EnabledVisibleSelection(web::Frame &frame, Event *event, EditorCommandSource) noexcept
    {
      // The term "visible" here includes a caret in editable text or a range in any text.
      const VisibleSelection &selection = frame.Document()->GetEditor().SelectionForCommand(event);
      return (selection.IsCaret() && selection.IsContentEditable()) || selection.IsRange();
    }

    KRYS_NODISCARD bool CaretBrowsingEnabled(web::Frame &frame) noexcept
    {
      return frame.Document()->GetSettings().CaretBrowsingEnabled;
    }

    bool EnabledVisibleSelectionOrCaretBrowsing(web::Frame &frame, Event *event,
                                                EditorCommandSource source) noexcept
    {
      return CaretBrowsingEnabled(frame) || EnabledVisibleSelection(frame, event, source);
    }

    bool EnabledCopy(web::Frame &frame, Event *, EditorCommandSource) noexcept
    {
      auto &editor = frame.Document()->GetEditor();
      return editor.CanDHTMLCopy() || editor.CanCopy();
    }

    bool EnabledCut(web::Frame &frame, Event *, EditorCommandSource) noexcept
    {
      auto &editor = frame.Document()->GetEditor();
      return editor.CanDHTMLCut() || editor.CanCut();
    }

    bool EnabledPaste(web::Frame &frame, Event *, EditorCommandSource) noexcept
    {
      auto &editor = frame.Document()->GetEditor();
      return editor.CanDHTMLPaste() || editor.CanPaste();
    }

    bool EnabledInEditableText(web::Frame &frame, Event *event, EditorCommandSource) noexcept
    {
      return frame.Document()->GetEditor().SelectionForCommand(event).RootEditableElement();
    }

    bool EnabledInEditableTextOrCaretBrowsing(web::Frame &frame, Event *event,
                                              EditorCommandSource source) noexcept
    {
      return CaretBrowsingEnabled(frame) || EnabledInEditableText(frame, event, source);
    }

    bool EnabledDelete(web::Frame &frame, Event *event, EditorCommandSource source) noexcept
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

    bool EnabledInRichlyEditableText(web::Frame &frame, Event *, EditorCommandSource) noexcept
    {
      const auto &selection = frame.Document()->Selection().Selection();
      return selection.IsCaretOrRange() && selection.IsContentRichlyEditable()
             && selection.RootEditableElement();
    }

    // AllowExecutionWhenDisabled functions

    bool DoNotAllowExecutionWhenDisabled(web::Frame &, EditorCommandSource) noexcept
    {
      return false;
    }

    bool AllowExecutionWhenDisabledCopyCut(web::Frame &, EditorCommandSource source) noexcept
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

    bool AllowExecutionWhenDisabledPaste(web::Frame &, EditorCommandSource source) noexcept
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

  web::Frame *Editor::Command::Frame() const noexcept
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
    RefPtr<web::Frame> frame = ShareRefPtr(Frame());
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
