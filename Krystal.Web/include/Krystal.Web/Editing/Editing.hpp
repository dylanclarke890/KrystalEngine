/*
 * Copyright (C) 2004, 2006, 2008, 2016 Apple Inc. All rights reserved.
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
#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Strings/String.hpp"
#include "Krystal.Web/Dom/Types/DOMString.hpp"
#include "Krystal.Web/Editing/Position.hpp"
#include "Krystal.Web/Editing/Selection/EditingBoundary.hpp"
#include "Krystal.Web/Editing/TextIteratorBehavior.hpp"
#include "Krystal.Web/Graphics/Geometry/LayoutRect.hpp"
#include "Krystal.Web/Html/Types/TagName.hpp"
#include "Krystal.Web/Text/WritingMode.hpp"

namespace krys::web::dom
{
  struct SimpleRange;
}

namespace krys::web
{
  class ContainerNode;
  class Document;
  class Element;
  class HTMLElement;
  class Node;
  class RenderBlock;
  class RenderObject;
  class VisiblePosition;
  class VisibleSelection;

#pragma region Node

  KRYS_NODISCARD RefPtr<ContainerNode>
    HighestEditableRoot(const Position &position,
                        EditableType editableType = EditableType::ContentIsEditable) noexcept;

  KRYS_NODISCARD RefPtr<Node> HighestEnclosingNodeOfType(
    const Position &position, bool (*nodeIsOfType)(const Node &),
    EditingBoundaryCrossingRule rule = EditingBoundaryCrossingRule::CannotCrossEditingBoundary,
    Node *stayWithin = nullptr) noexcept;

  KRYS_NODISCARD Element *LowestEditableAncestor(Node *node) noexcept;

  /// @brief Use `EnclosingBlock` instead.
  KRYS_NODISCARD Element *DeprecatedEnclosingBlockFlowElement(Node *node) noexcept;

  KRYS_NODISCARD RefPtr<Element> EnclosingBlock(
    const RefPtr<Node> &node,
    EditingBoundaryCrossingRule rule = EditingBoundaryCrossingRule::CannotCrossEditingBoundary) noexcept;

  KRYS_NODISCARD RefPtr<Element> EnclosingTableCell(const Position &position) noexcept;

  KRYS_NODISCARD RefPtr<Element> EnclosingAnchorElement(const Position &position) noexcept;

  KRYS_NODISCARD RefPtr<Node> EnclosingNodeOfType(
    const Position &position, bool (*nodeIsOfType)(const Node &),
    EditingBoundaryCrossingRule rule = EditingBoundaryCrossingRule::CannotCrossEditingBoundary) noexcept;

  // FIXME: Strange to name this isXXX, but return an element.
  KRYS_NODISCARD RefPtr<Element> IsLastPositionBeforeTable(const VisiblePosition &position) noexcept;

  // FIXME: Strange to name this isXXX, but return an element.
  KRYS_NODISCARD RefPtr<Element> IsFirstPositionAfterTable(const VisiblePosition &position) noexcept;

  // These two deliver leaf nodes as if the whole DOM tree were a linear chain of its leaf nodes.
  KRYS_NODISCARD Node *NextLeafNode(const Node *node) noexcept;

  KRYS_NODISCARD Node *PreviousLeafNode(const Node *node) noexcept;

  /// @brief The maximum valid offset in a node for editing positions: its length, or 1 for nodes whose
  /// content editing ignores even though they have no children (which makes technically invalid DOM
  /// positions; use `ParentAnchoredEquivalent` before making a range).
  KRYS_NODISCARD int32 LastOffsetForEditing(const Node &node) noexcept;

  KRYS_NODISCARD int32 CaretMinOffset(const Node &node) noexcept;

  KRYS_NODISCARD int32 CaretMaxOffset(const Node &node) noexcept;

  KRYS_NODISCARD bool HasEditableStyle(const Node &node, EditableType editableType) noexcept;

  KRYS_NODISCARD bool IsEditableNode(const Node &node) noexcept;

  // FIXME: editingIgnoresContent, canHaveChildrenForEditing, and isAtomicNode should be named to clarify how
  // they differ.

  // Returns true for nodes that either have no content, or have content that is ignored (skipped over) while
  // editing. There are no VisiblePositions inside these nodes.
  KRYS_NODISCARD inline bool EditingIgnoresContent(const Node &node) noexcept
  {
    return !node.CanContainRangeEndPoint();
  }

  KRYS_NODISCARD bool CanHaveChildrenForEditing(const Node &node) noexcept;

  /// @brief Whether the node has no children, or has children which are ignored for the purposes of editing.
  KRYS_NODISCARD bool IsAtomicNode(const Node *node) noexcept;

  KRYS_NODISCARD bool IsBlock(const Node &node) noexcept;

  KRYS_NODISCARD bool IsBlockFlowElement(const Node &node) noexcept;

  KRYS_NODISCARD bool IsInline(const Node &node) noexcept;

  KRYS_NODISCARD bool IsRenderedTable(const Node *node) noexcept;

  KRYS_NODISCARD bool IsTableCell(const Node &node) noexcept;

  KRYS_NODISCARD bool IsEmptyTableCell(const Node *node) noexcept;

  KRYS_NODISCARD bool IsTableStructureNode(const Node &node) noexcept;

  KRYS_NODISCARD bool IsListHTMLElement(Node *node) noexcept;

  KRYS_NODISCARD bool IsListItem(const Node &node) noexcept;

  KRYS_NODISCARD bool IsRenderedAsNonInlineTableImageOrHR(const Node *node) noexcept;

  KRYS_NODISCARD bool IsNodeVisiblyContainedWithin(Node &node, const dom::SimpleRange &range) noexcept;

  KRYS_NODISCARD inline bool PositionBeforeOrAfterNodeIsCandidate(Node &node) noexcept
  {
    return IsRenderedTable(&node) || EditingIgnoresContent(node);
  }

#pragma endregion

#pragma region SimpleRange

  KRYS_NODISCARD PositionRange PositionsForRange(const dom::SimpleRange &range) noexcept;

#pragma endregion

#pragma region Position

  KRYS_NODISCARD Position NextCandidate(const Position &position) noexcept;

  KRYS_NODISCARD Position PreviousCandidate(const Position &position) noexcept;

  enum class SkipDisplayContents : bool
  {
    No,
    Yes,
  };

  KRYS_NODISCARD Position NextVisuallyDistinctCandidate(
    const Position &position, SkipDisplayContents skipDisplayContents = SkipDisplayContents::Yes) noexcept;

  KRYS_NODISCARD Position PreviousVisuallyDistinctCandidate(const Position &position) noexcept;

  KRYS_NODISCARD inline Position FirstPositionInOrBeforeNode(Node *node) noexcept
  {
    if (!node)
    {
      return {};
    }
    return EditingIgnoresContent(*node) ? PositionBeforeNode(node) : FirstPositionInNode(node);
  }

  KRYS_NODISCARD inline Position LastPositionInOrAfterNode(Node *node) noexcept
  {
    if (!node)
    {
      return {};
    }
    return EditingIgnoresContent(*node) ? PositionAfterNode(node) : LastPositionInNode(node);
  }

  KRYS_NODISCARD Position FirstEditablePositionAfterPositionInRoot(const Position &position,
                                                                   ContainerNode *highestRoot) noexcept;

  KRYS_NODISCARD Position LastEditablePositionBeforePositionInRoot(const Position &position,
                                                                   ContainerNode *highestRoot) noexcept;

  KRYS_NODISCARD bool
    IsEditablePosition(const Position &position,
                       EditableType editableType = EditableType::ContentIsEditable) noexcept;

  KRYS_NODISCARD bool IsRichlyEditablePosition(const Position &position) noexcept;

  KRYS_NODISCARD bool LineBreakExistsAtPosition(const Position &position) noexcept;

  KRYS_NODISCARD bool IsAtUnsplittableElement(const Position &position) noexcept;

  void UpdatePositionForNodeRemoval(Position &position, Node &node) noexcept;

  KRYS_NODISCARD text::TextDirection DirectionOfEnclosingBlock(const Position &position) noexcept;

  /// @brief The node, or the shadow host it's in (or that host's, and so on), that's in the tree whose root
  /// is
  /// @p scopeRoot.
  KRYS_NODISCARD Node *AncestorNodeInThisScope(const Node &scopeRoot, Node *node) noexcept;

#pragma endregion

#pragma region VisiblePosition

  KRYS_NODISCARD VisiblePosition VisiblePositionBeforeNode(Node &node) noexcept;

  KRYS_NODISCARD VisiblePosition VisiblePositionAfterNode(Node &node) noexcept;

  KRYS_NODISCARD bool LineBreakExistsAtVisiblePosition(const VisiblePosition &position) noexcept;

  KRYS_NODISCARD int32 IndexForVisiblePosition(const VisiblePosition &visiblePosition,
                                               RefPtr<ContainerNode> &scope) noexcept;

  KRYS_NODISCARD int32 IndexForVisiblePosition(Node &node, const VisiblePosition &visiblePosition,
                                               TextIteratorBehaviors behaviors) noexcept;

  KRYS_NODISCARD VisiblePosition VisiblePositionForPositionWithOffset(const VisiblePosition &position,
                                                                      int32 offset) noexcept;

  KRYS_NODISCARD VisiblePosition
    VisiblePositionForIndex(int32 index, Node *scope,
                            TextIteratorBehaviors behaviors =
                              TextIteratorBehavior::EmitsCharactersBetweenAllVisiblePositions) noexcept;

  // FIXME: Why do we need this version?
  KRYS_NODISCARD VisiblePosition VisiblePositionForIndexUsingCharacterIterator(Node &node,
                                                                               int32 index) noexcept;

  KRYS_NODISCARD VisiblePosition
    ClosestEditablePositionInElementForAbsolutePoint(const Element &element, const PointI &point) noexcept;

#pragma endregion

#pragma region Element

  KRYS_NODISCARD Element *
    EditableRootForPosition(const Position &position,
                            EditableType editableType = EditableType::ContentIsEditable) noexcept;

  KRYS_NODISCARD RefPtr<Element> UnsplittableElementForPosition(const Position &position) noexcept;

#pragma endregion

  // FIXME: This is only one of many definitions of whitespace. Possibly never the right one to use.
  KRYS_NODISCARD constexpr bool DeprecatedIsEditingWhitespace(char32 c) noexcept
  {
    return c == 0xA0 || c == ' ' || c == '\n' || c == '\t';
  }

  // FIXME: Can't answer this question correctly without being passed the white-space mode.
  KRYS_NODISCARD constexpr bool DeprecatedIsCollapsibleWhitespace(char32 c) noexcept
  {
    return c == ' ' || c == '\n';
  }

  /// @brief ASCII whitespace for Latin-1 characters (which excludes U+0085 and U+00A0), and characters whose
  /// bidi class is whitespace above it.
  KRYS_NODISCARD bool DeprecatedIsSpaceOrNewline(char32 character) noexcept;

  KRYS_NODISCARD bool IsAmbiguousBoundaryCharacter(char32 character) noexcept;

#pragma region Elements editing makes

  /// @brief Whether @p node is an HTML element with the tag name @p tagName.
  KRYS_NODISCARD bool IsHTMLElementOfType(const Node *node, html::TagName tagName) noexcept;

  KRYS_NODISCARD Ref<Element> CreateHTMLElement(Document &document, DOMStringView localName) noexcept;

  KRYS_NODISCARD Ref<Element> CreateDefaultParagraphElement(Document &document) noexcept;

  KRYS_NODISCARD Ref<Element> CreateTabSpanElement(Document &document) noexcept;

  KRYS_NODISCARD Ref<Element> CreateTabSpanElement(Document &document, utf8_string tabText) noexcept;

  /// @brief A copy of @p element, with its attributes, without its children.
  KRYS_NODISCARD Ref<Element> CloneElementWithoutChildren(Element &element) noexcept;

  /// @brief Gives @p target the attributes of @p source.
  void CloneDataFromElement(Element &target, const Element &source) noexcept;

  KRYS_NODISCARD Element *TabSpanNode(Node *node) noexcept;

  KRYS_NODISCARD Element *ParentTabSpanNode(Node *node) noexcept;

  KRYS_NODISCARD RefPtr<Element> EnclosingList(Node *node) noexcept;

  KRYS_NODISCARD RefPtr<Node> EnclosingListChild(Node *node) noexcept;

  KRYS_NODISCARD RefPtr<Node> EnclosingEmptyListItem(const VisiblePosition &position) noexcept;

  KRYS_NODISCARD RefPtr<Element> OutermostEnclosingList(Node *node, Node *rootList = nullptr) noexcept;

  KRYS_NODISCARD bool CanMergeLists(Element *firstList, Element *secondList) noexcept;

  KRYS_NODISCARD Element *ElementIfEquivalent(const Element &first, Node &second) noexcept;

  KRYS_NODISCARD Element *EnclosingElementWithTag(const Position &position, html::TagName tagName) noexcept;

  KRYS_NODISCARD bool IsNonTableCellHTMLBlockElement(const Node *node) noexcept;

  KRYS_NODISCARD Position
    AdjustedSelectionStartForStyleComputation(const VisibleSelection &selection) noexcept;

  /// @brief The highest node, from @p node up, that removing would leave nothing rendered behind.
  KRYS_NODISCARD RefPtr<Node> HighestNodeToRemoveInPruning(Node *node) noexcept;

  /// @brief @p string with its whitespace alternating between spaces and no-break spaces, so that none of it
  /// collapses.
  KRYS_NODISCARD utf8_string StringWithRebalancedWhitespace(utf8_stringview string,
                                                            bool startIsStartOfParagraph,
                                                            bool shouldEmitNBSPbeforeEnd) noexcept;

  KRYS_NODISCARD utf8_stringview NonBreakingSpaceString() noexcept;

  KRYS_NODISCARD bool IsVisiblyAdjacent(const Position &first, const Position &second) noexcept;

#pragma endregion

#pragma region Caret rendering

  KRYS_NODISCARD RenderBlock *RendererForCaretPainting(const Node *node) noexcept;

  KRYS_NODISCARD LayoutRect LocalCaretRectInRendererForCaretPainting(const VisiblePosition &caretPosition,
                                                                     RenderBlock *&caretPainter) noexcept;

  KRYS_NODISCARD LayoutRect LocalCaretRectInRendererForRect(LayoutRect &localRect, Node *node,
                                                            RenderObject *renderer,
                                                            RenderBlock *&caretPainter) noexcept;

  KRYS_NODISCARD RectI AbsoluteBoundsForLocalCaretRect(RenderBlock *rendererForCaretPainting,
                                                       const LayoutRect &rect,
                                                       bool *insideFixed = nullptr) noexcept;

#pragma endregion
}
