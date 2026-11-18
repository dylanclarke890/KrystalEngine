/*
 * Copyright (C) 2005, 2006, 2008, 2009 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Editing/Commands/CompositeEditCommand.hpp"
#include "Krystal.Booey/Editing/EditingStyle.hpp"
#include "Krystal.Booey/Editing/WritingDirection.hpp"

namespace krys::boo
{
  class HTMLElement;

  enum ShouldIncludeTypingStyle : bool
  {
    IncludeTypingStyle,
    IgnoreTypingStyle,
  };

  /// @brief Applies an editing style to the selection or a range: block properties to the paragraphs' blocks,
  /// and the rest by pushing conflicting inline styles down and wrapping runs in the elements and spans that
  /// express it.
  class ApplyStyleCommand final : public CompositeEditCommand
  {
  public:
    enum class InlineStyleRemovalMode : uint8
    {
      IfNeeded,
      Always,
      None,
    };

    enum class AddStyledElement : bool
    {
      No,
      Yes,
    };

    using IsInlineElementToRemoveFunction = bool (*)(const Element *);

  private:
    RefPtr<EditingStyle> _style;
    ApplyStylePropertyLevel _propertyLevel {ApplyStylePropertyLevel::Default};
    dom::Position _start;
    dom::Position _end;
    bool _useEndingSelection;
    RefPtr<Element> _styledInlineElement;
    bool _removeOnly;
    IsInlineElementToRemoveFunction _isInlineElementToRemoveFunction {nullptr};

  public:
    ApplyStyleCommand(Document &document, const EditingStyle *style, EditAction action,
                      ApplyStylePropertyLevel level) noexcept;

    ApplyStyleCommand(Document &document, const EditingStyle *style, dom::Position start, dom::Position end,
                      EditAction action, ApplyStylePropertyLevel level) noexcept;

    ApplyStyleCommand(Element &element, bool removeOnly, EditAction action) noexcept;

    ApplyStyleCommand(Document &document, const EditingStyle *style,
                      IsInlineElementToRemoveFunction isInlineElementToRemove, EditAction action) noexcept;

    KRYS_NODISCARD static Ref<ApplyStyleCommand>
      Create(Document &document, const EditingStyle *style, EditAction action = EditAction::ChangeAttributes,
             ApplyStylePropertyLevel level = ApplyStylePropertyLevel::Default) noexcept;

    KRYS_NODISCARD static Ref<ApplyStyleCommand>
      Create(Document &document, const EditingStyle *style, const dom::Position &start,
             const dom::Position &end, EditAction action = EditAction::ChangeAttributes,
             ApplyStylePropertyLevel level = ApplyStylePropertyLevel::Default) noexcept;

    KRYS_NODISCARD static Ref<ApplyStyleCommand>
      Create(Element &element, bool removeOnly = false,
             EditAction action = EditAction::ChangeAttributes) noexcept;

    KRYS_NODISCARD static Ref<ApplyStyleCommand>
      Create(Document &document, const EditingStyle *style,
             IsInlineElementToRemoveFunction isInlineElementToRemoveFunction,
             EditAction action = EditAction::ChangeAttributes) noexcept;

  private:
    void DoApply() noexcept override;

    KRYS_NODISCARD bool ShouldDispatchInputEvents() const noexcept final
    {
      return false;
    }

    // style-removal helpers
    KRYS_NODISCARD bool IsStyledInlineElementToRemove(Element *element) const noexcept;

    KRYS_NODISCARD bool ShouldApplyInlineStyleToRun(EditingStyle &style, boo::Node *runStart,
                                                    boo::Node *pastEndNode) noexcept;

    void RemoveConflictingInlineStyleFromRun(EditingStyle &style, RefPtr<boo::Node> &runStart,
                                             RefPtr<boo::Node> &runEnd, boo::Node *pastEndNode) noexcept;

    bool RemoveInlineStyleFromElement(EditingStyle &style, HTMLElement &element,
                                      InlineStyleRemovalMode mode = InlineStyleRemovalMode::IfNeeded,
                                      EditingStyle *extractedStyle = nullptr) noexcept;

    KRYS_NODISCARD bool ShouldRemoveInlineStyleFromElement(EditingStyle &style, HTMLElement &element) noexcept
    {
      return RemoveInlineStyleFromElement(style, element, InlineStyleRemovalMode::None);
    }

    void ReplaceWithSpanOrRemoveIfWithoutAttributes(HTMLElement &element) noexcept;

    bool RemoveImplicitlyStyledElement(EditingStyle &style, HTMLElement &element, InlineStyleRemovalMode mode,
                                       EditingStyle *extractedStyle) noexcept;

    bool RemoveCSSStyle(EditingStyle &style, HTMLElement &element,
                        InlineStyleRemovalMode mode = InlineStyleRemovalMode::IfNeeded,
                        EditingStyle *extractedStyle = nullptr) noexcept;

    KRYS_NODISCARD RefPtr<HTMLElement> HighestAncestorWithConflictingInlineStyle(EditingStyle &style,
                                                                                 boo::Node *node) noexcept;

    void ApplyInlineStyleToPushDown(boo::Node &node, EditingStyle *style) noexcept;

    void PushDownInlineStyleAroundNode(EditingStyle &style, boo::Node *targetNode) noexcept;

    void RemoveInlineStyle(EditingStyle &style, const dom::Position &start,
                           const dom::Position &end) noexcept;

    KRYS_NODISCARD static bool NodeFullySelected(Element &element, const dom::Position &start,
                                                 const dom::Position &end) noexcept;

    // style-application helpers
    void ApplyBlockStyle(EditingStyle &style) noexcept;

    void ApplyInlineStyle(EditingStyle &style) noexcept;

    void FixRangeAndApplyInlineStyle(EditingStyle &style, const dom::Position &start,
                                     const dom::Position &end) noexcept;

    void ApplyInlineStyleToNodeRange(EditingStyle &style, boo::Node &startNode,
                                     boo::Node *pastEndNode) noexcept;

    void AddBlockStyle(const StyleChange &styleChange, HTMLElement &block) noexcept;

    void AddInlineStyleIfNeeded(EditingStyle *style, boo::Node &start, boo::Node &end,
                                AddStyledElement addStyledElement = AddStyledElement::Yes) noexcept;

    dom::Position PositionToComputeInlineStyleChange(boo::Node &startNode,
                                                     RefPtr<boo::Node> &dummyElement) noexcept;

    void ApplyInlineStyleChange(boo::Node &passedStart, boo::Node &passedEnd, StyleChange &styleChange,
                                AddStyledElement addStyledElement) noexcept;

    void SplitTextAtStart(const dom::Position &start, const dom::Position &end) noexcept;

    void SplitTextAtEnd(const dom::Position &start, const dom::Position &end) noexcept;

    void SplitTextElementAtStart(const dom::Position &start, const dom::Position &end) noexcept;

    void SplitTextElementAtEnd(const dom::Position &start, const dom::Position &end) noexcept;

    KRYS_NODISCARD bool ShouldSplitTextElement(Element *element, EditingStyle &style) noexcept;

    KRYS_NODISCARD static bool IsValidCaretPositionInTextNode(const dom::Position &position) noexcept;

    bool MergeStartWithPreviousIfIdentical(const dom::Position &start, const dom::Position &end) noexcept;

    bool MergeEndWithNextIfIdentical(const dom::Position &start, const dom::Position &end) noexcept;

    void CleanupUnstyledStyleSpans(ContainerNode *dummySpanAncestor) noexcept;

    bool SurroundNodeRangeWithElement(boo::Node &start, boo::Node &end, Ref<Element> &&element) noexcept;

    RefPtr<HTMLElement> SplitAncestorsWithUnicodeBidi(boo::Node *node, bool before,
                                                      WritingDirection allowedDirection) noexcept;

    void RemoveEmbeddingUpToEnclosingBlock(boo::Node *node, boo::Node *unsplitAncestor) noexcept;

    void UpdateStartEnd(const dom::Position &newStart, const dom::Position &newEnd) noexcept;

    KRYS_NODISCARD dom::Position StartPosition() noexcept;

    KRYS_NODISCARD dom::Position EndPosition() noexcept;
  };

  enum ShouldStyleAttributeBeEmpty : bool
  {
    AllowNonEmptyStyleAttribute,
    StyleAttributeShouldBeEmpty,
  };

  KRYS_NODISCARD bool IsEmptyFontTag(
    const Element *element,
    ShouldStyleAttributeBeEmpty shouldStyleAttributeBeEmpty = StyleAttributeShouldBeEmpty) noexcept;

  KRYS_NODISCARD bool IsStyleSpanOrSpanWithOnlyStyleAttribute(const Element &element) noexcept;

  KRYS_NODISCARD Ref<Element> CreateStyleSpanElement(Document &document) noexcept;
}
