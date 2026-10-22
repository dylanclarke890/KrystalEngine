#pragma once

#include "Krystal.Booey/Editing/CompositeEditCommand.hpp"
#include "Krystal.Booey/Editing/EditingStyle.hpp"
#include "Krystal.Booey/Editing/WritingDirection.hpp"

namespace krys::boo::html
{
  class HTMLElement;
}

namespace krys::boo
{
  enum ShouldIncludeTypingStyle : bool
  {
    IncludeTypingStyle,
    IgnoreTypingStyle
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
      None
    };

    enum class AddStyledElement : bool
    {
      No,
      Yes
    };

    using IsInlineElementToRemoveFunction = bool (*)(const dom::Element *);

  private:
    RefPtr<EditingStyle> _style;
    ApplyStylePropertyLevel _propertyLevel {ApplyStylePropertyLevel::Default};
    dom::Position _start;
    dom::Position _end;
    bool _useEndingSelection;
    RefPtr<dom::Element> _styledInlineElement;
    bool _removeOnly;
    IsInlineElementToRemoveFunction _isInlineElementToRemoveFunction {nullptr};

  public:
    ApplyStyleCommand(dom::Document &document, const EditingStyle *style, EditAction action,
                      ApplyStylePropertyLevel level) noexcept;
    ApplyStyleCommand(dom::Document &document, const EditingStyle *style, const dom::Position &start,
                      const dom::Position &end, EditAction action, ApplyStylePropertyLevel level) noexcept;
    ApplyStyleCommand(dom::Element &element, bool removeOnly, EditAction action) noexcept;
    ApplyStyleCommand(dom::Document &document, const EditingStyle *style,
                      IsInlineElementToRemoveFunction isInlineElementToRemove, EditAction action) noexcept;

    KRYS_NODISCARD static Ref<ApplyStyleCommand>
      Create(dom::Document &document, const EditingStyle *style,
             EditAction action = EditAction::ChangeAttributes,
             ApplyStylePropertyLevel level = ApplyStylePropertyLevel::Default) noexcept;
    KRYS_NODISCARD static Ref<ApplyStyleCommand>
      Create(dom::Document &document, const EditingStyle *style, const dom::Position &start,
             const dom::Position &end, EditAction action = EditAction::ChangeAttributes,
             ApplyStylePropertyLevel level = ApplyStylePropertyLevel::Default) noexcept;
    KRYS_NODISCARD static Ref<ApplyStyleCommand>
      Create(dom::Element &element, bool removeOnly = false,
             EditAction action = EditAction::ChangeAttributes) noexcept;
    KRYS_NODISCARD static Ref<ApplyStyleCommand>
      Create(dom::Document &document, const EditingStyle *style,
             IsInlineElementToRemoveFunction isInlineElementToRemoveFunction,
             EditAction action = EditAction::ChangeAttributes) noexcept;

  private:
    void DoApply() noexcept override;
    KRYS_NODISCARD bool ShouldDispatchInputEvents() const noexcept final
    {
      return false;
    }

    // style-removal helpers
    KRYS_NODISCARD bool IsStyledInlineElementToRemove(dom::Element *element) const noexcept;
    KRYS_NODISCARD bool ShouldApplyInlineStyleToRun(EditingStyle &style, dom::Node *runStart,
                                                    dom::Node *pastEndNode) noexcept;
    void RemoveConflictingInlineStyleFromRun(EditingStyle &style, RefPtr<dom::Node> &runStart,
                                             RefPtr<dom::Node> &runEnd, dom::Node *pastEndNode) noexcept;
    bool RemoveInlineStyleFromElement(EditingStyle &style, html::HTMLElement &element,
                                      InlineStyleRemovalMode mode = InlineStyleRemovalMode::IfNeeded,
                                      EditingStyle *extractedStyle = nullptr) noexcept;
    KRYS_NODISCARD bool ShouldRemoveInlineStyleFromElement(EditingStyle &style,
                                                           html::HTMLElement &element) noexcept
    {
      return RemoveInlineStyleFromElement(style, element, InlineStyleRemovalMode::None);
    }
    void ReplaceWithSpanOrRemoveIfWithoutAttributes(html::HTMLElement &element) noexcept;
    bool RemoveImplicitlyStyledElement(EditingStyle &style, html::HTMLElement &element,
                                       InlineStyleRemovalMode mode, EditingStyle *extractedStyle) noexcept;
    bool RemoveCSSStyle(EditingStyle &style, html::HTMLElement &element,
                        InlineStyleRemovalMode mode = InlineStyleRemovalMode::IfNeeded,
                        EditingStyle *extractedStyle = nullptr) noexcept;
    KRYS_NODISCARD RefPtr<html::HTMLElement>
      HighestAncestorWithConflictingInlineStyle(EditingStyle &style, dom::Node *node) noexcept;
    void ApplyInlineStyleToPushDown(dom::Node &node, EditingStyle *style) noexcept;
    void PushDownInlineStyleAroundNode(EditingStyle &style, dom::Node *targetNode) noexcept;
    void RemoveInlineStyle(EditingStyle &style, const dom::Position &start,
                           const dom::Position &end) noexcept;
    KRYS_NODISCARD bool NodeFullySelected(dom::Element &element, const dom::Position &start,
                                          const dom::Position &end) const noexcept;

    // style-application helpers
    void ApplyBlockStyle(EditingStyle &style) noexcept;
    void ApplyRelativeFontStyleChange(EditingStyle *style) noexcept;
    void ApplyInlineStyle(EditingStyle &style) noexcept;
    void FixRangeAndApplyInlineStyle(EditingStyle &style, const dom::Position &start,
                                     const dom::Position &end) noexcept;
    void ApplyInlineStyleToNodeRange(EditingStyle &style, dom::Node &startNode,
                                     dom::Node *pastEndNode) noexcept;
    void AddBlockStyle(const StyleChange &styleChange, html::HTMLElement &block) noexcept;
    void AddInlineStyleIfNeeded(EditingStyle *style, dom::Node &start, dom::Node &end,
                                AddStyledElement addStyledElement = AddStyledElement::Yes) noexcept;
    dom::Position PositionToComputeInlineStyleChange(dom::Node &startNode,
                                                     RefPtr<dom::Node> &dummyElement) noexcept;
    void ApplyInlineStyleChange(dom::Node &startNode, dom::Node &endNode, StyleChange &styleChange,
                                AddStyledElement addStyledElement) noexcept;
    void SplitTextAtStart(const dom::Position &start, const dom::Position &end) noexcept;
    void SplitTextAtEnd(const dom::Position &start, const dom::Position &end) noexcept;
    void SplitTextElementAtStart(const dom::Position &start, const dom::Position &end) noexcept;
    void SplitTextElementAtEnd(const dom::Position &start, const dom::Position &end) noexcept;
    KRYS_NODISCARD bool ShouldSplitTextElement(dom::Element *element, EditingStyle &style) noexcept;
    KRYS_NODISCARD bool IsValidCaretPositionInTextNode(const dom::Position &position) noexcept;
    bool MergeStartWithPreviousIfIdentical(const dom::Position &start, const dom::Position &end) noexcept;
    bool MergeEndWithNextIfIdentical(const dom::Position &start, const dom::Position &end) noexcept;
    void CleanupUnstyledAppleStyleSpans(dom::ContainerNode *dummySpanAncestor) noexcept;
    bool SurroundNodeRangeWithElement(dom::Node &start, dom::Node &end, Ref<dom::Element> &&element) noexcept;
    KRYS_NODISCARD float ComputedFontSize(dom::Node *node) noexcept;
    void JoinChildTextNodes(dom::Node *node, const dom::Position &start, const dom::Position &end) noexcept;

    RefPtr<html::HTMLElement> SplitAncestorsWithUnicodeBidi(dom::Node *node, bool before,
                                                            WritingDirection allowedDirection) noexcept;
    void RemoveEmbeddingUpToEnclosingBlock(dom::Node *node, dom::Node *unsplitAncestor) noexcept;

    void UpdateStartEnd(const dom::Position &newStart, const dom::Position &newEnd) noexcept;
    KRYS_NODISCARD dom::Position StartPosition() noexcept;
    KRYS_NODISCARD dom::Position EndPosition() noexcept;
  };

  enum ShouldStyleAttributeBeEmpty : bool
  {
    AllowNonEmptyStyleAttribute,
    StyleAttributeShouldBeEmpty
  };

  KRYS_NODISCARD bool IsEmptyFontTag(
    const dom::Element *element,
    ShouldStyleAttributeBeEmpty shouldStyleAttributeBeEmpty = StyleAttributeShouldBeEmpty) noexcept;
  KRYS_NODISCARD bool IsLegacyAppleStyleSpan(const dom::Node *node) noexcept;
  KRYS_NODISCARD bool IsStyleSpanOrSpanWithOnlyStyleAttribute(const dom::Element &element) noexcept;
  KRYS_NODISCARD Ref<dom::Element> CreateStyleSpanElement(dom::Document &document) noexcept;
}
