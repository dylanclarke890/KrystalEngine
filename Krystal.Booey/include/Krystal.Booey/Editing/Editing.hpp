#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/DOM/Position.hpp"
#include "Krystal.Booey/Editing/EditingBoundary.hpp"
#include "Krystal.Booey/Editing/TextIteratorBehavior.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class ContainerNode;
  class Element;
  class Node;
  struct SimpleRange;
}

namespace krys::boo::html
{
  class HTMLElement;
}

namespace krys::boo
{
  class IntPoint;
  class RenderBlock;
  class RenderObject;
  class VisiblePosition;

#pragma region Node

  KRYS_NODISCARD RefPtr<dom::ContainerNode>
    HighestEditableRoot(const dom::Position &position,
                        EditableType editableType = EditableType::ContentIsEditable) noexcept;
  KRYS_NODISCARD RefPtr<dom::Node> HighestEnclosingNodeOfType(
    const dom::Position &position, bool (*nodeIsOfType)(const dom::Node &),
    EditingBoundaryCrossingRule rule = EditingBoundaryCrossingRule::CannotCrossEditingBoundary,
    dom::Node *stayWithin = nullptr) noexcept;
  KRYS_NODISCARD dom::Element *LowestEditableAncestor(dom::Node *node) noexcept;
  /// @brief Use `EnclosingBlock` instead.
  KRYS_NODISCARD dom::Element *DeprecatedEnclosingBlockFlowElement(dom::Node *node) noexcept;
  KRYS_NODISCARD RefPtr<dom::Element> EnclosingBlock(
    RefPtr<dom::Node> node,
    EditingBoundaryCrossingRule rule = EditingBoundaryCrossingRule::CannotCrossEditingBoundary) noexcept;
  KRYS_NODISCARD RefPtr<dom::Element> EnclosingTableCell(const dom::Position &position) noexcept;
  KRYS_NODISCARD RefPtr<dom::Element> EnclosingAnchorElement(const dom::Position &position) noexcept;
  KRYS_NODISCARD RefPtr<dom::Node> EnclosingNodeOfType(
    const dom::Position &position, bool (*nodeIsOfType)(const dom::Node &),
    EditingBoundaryCrossingRule rule = EditingBoundaryCrossingRule::CannotCrossEditingBoundary) noexcept;
  // FIXME: Strange to name this isXXX, but return an element.
  KRYS_NODISCARD RefPtr<dom::Element> IsLastPositionBeforeTable(const VisiblePosition &position) noexcept;
  // FIXME: Strange to name this isXXX, but return an element.
  KRYS_NODISCARD RefPtr<dom::Element> IsFirstPositionAfterTable(const VisiblePosition &position) noexcept;

  // These two deliver leaf nodes as if the whole DOM tree were a linear chain of its leaf nodes.
  KRYS_NODISCARD dom::Node *NextLeafNode(const dom::Node *node) noexcept;
  KRYS_NODISCARD dom::Node *PreviousLeafNode(const dom::Node *node) noexcept;

  /// @brief The maximum valid offset in a node for editing positions: its length, or 1 for nodes whose
  /// content editing ignores even though they have no children (which makes technically invalid DOM
  /// positions; use `ParentAnchoredEquivalent` before making a range).
  KRYS_NODISCARD int32 LastOffsetForEditing(const dom::Node &node) noexcept;
  KRYS_NODISCARD int32 CaretMinOffset(const dom::Node &node) noexcept;
  KRYS_NODISCARD int32 CaretMaxOffset(const dom::Node &node) noexcept;

  KRYS_NODISCARD bool HasEditableStyle(const dom::Node &node, EditableType editableType) noexcept;
  KRYS_NODISCARD bool IsEditableNode(const dom::Node &node) noexcept;

  // FIXME: editingIgnoresContent, canHaveChildrenForEditing, and isAtomicNode should be named to clarify how
  // they differ.

  // Returns true for nodes that either have no content, or have content that is ignored (skipped over) while
  // editing. There are no VisiblePositions inside these nodes.
  KRYS_NODISCARD inline bool EditingIgnoresContent(const dom::Node &node) noexcept
  {
    return !node.CanContainRangeEndPoint();
  }

  KRYS_NODISCARD bool CanHaveChildrenForEditing(const dom::Node &node) noexcept;
  /// @brief Whether the node has no children, or has children which are ignored for the purposes of editing.
  KRYS_NODISCARD bool IsAtomicNode(const dom::Node *node) noexcept;
  KRYS_NODISCARD bool IsBlock(const dom::Node &node) noexcept;
  KRYS_NODISCARD bool IsBlockFlowElement(const dom::Node &node) noexcept;
  KRYS_NODISCARD bool IsInline(const dom::Node &node) noexcept;
  KRYS_NODISCARD bool IsRenderedTable(const dom::Node *node) noexcept;
  KRYS_NODISCARD bool IsTableCell(const dom::Node &node) noexcept;
  KRYS_NODISCARD bool IsEmptyTableCell(const dom::Node *node) noexcept;
  KRYS_NODISCARD bool IsTableStructureNode(const dom::Node &node) noexcept;
  KRYS_NODISCARD bool IsListHTMLElement(dom::Node *node) noexcept;
  KRYS_NODISCARD bool IsListItem(const dom::Node &node) noexcept;
  KRYS_NODISCARD bool IsRenderedAsNonInlineTableImageOrHR(const dom::Node *node) noexcept;
  KRYS_NODISCARD bool IsNodeVisiblyContainedWithin(dom::Node &node, const dom::SimpleRange &range) noexcept;

  KRYS_NODISCARD inline bool PositionBeforeOrAfterNodeIsCandidate(dom::Node &node) noexcept
  {
    return IsRenderedTable(&node) || EditingIgnoresContent(node);
  }

#pragma endregion

#pragma region SimpleRange

  KRYS_NODISCARD dom::PositionRange PositionsForRange(const dom::SimpleRange &range) noexcept;

#pragma endregion

#pragma region Position

  KRYS_NODISCARD dom::Position NextCandidate(const dom::Position &position) noexcept;
  KRYS_NODISCARD dom::Position PreviousCandidate(const dom::Position &position) noexcept;

  enum class SkipDisplayContents : bool
  {
    No,
    Yes
  };

  KRYS_NODISCARD dom::Position NextVisuallyDistinctCandidate(
    const dom::Position &position,
    SkipDisplayContents skipDisplayContents = SkipDisplayContents::Yes) noexcept;
  KRYS_NODISCARD dom::Position PreviousVisuallyDistinctCandidate(const dom::Position &position) noexcept;

  KRYS_NODISCARD inline dom::Position FirstPositionInOrBeforeNode(dom::Node *node) noexcept
  {
    if (!node)
    {
      return {};
    }
    return EditingIgnoresContent(*node) ? dom::PositionBeforeNode(node) : dom::FirstPositionInNode(node);
  }

  KRYS_NODISCARD inline dom::Position LastPositionInOrAfterNode(dom::Node *node) noexcept
  {
    if (!node)
    {
      return {};
    }
    return EditingIgnoresContent(*node) ? dom::PositionAfterNode(node) : dom::LastPositionInNode(node);
  }

  KRYS_NODISCARD dom::Position
    FirstEditablePositionAfterPositionInRoot(const dom::Position &position,
                                             dom::ContainerNode *highestRoot) noexcept;
  KRYS_NODISCARD dom::Position
    LastEditablePositionBeforePositionInRoot(const dom::Position &position,
                                             dom::ContainerNode *highestRoot) noexcept;

  KRYS_NODISCARD bool
    IsEditablePosition(const dom::Position &position,
                       EditableType editableType = EditableType::ContentIsEditable) noexcept;
  KRYS_NODISCARD bool IsRichlyEditablePosition(const dom::Position &position) noexcept;
  KRYS_NODISCARD bool LineBreakExistsAtPosition(const dom::Position &position) noexcept;
  KRYS_NODISCARD bool IsAtUnsplittableElement(const dom::Position &position) noexcept;
  void UpdatePositionForNodeRemoval(dom::Position &position, dom::Node &node) noexcept;
  KRYS_NODISCARD TextDirection DirectionOfEnclosingBlock(const dom::Position &position) noexcept;

#pragma endregion

#pragma region VisiblePosition

  KRYS_NODISCARD VisiblePosition VisiblePositionBeforeNode(dom::Node &node) noexcept;
  KRYS_NODISCARD VisiblePosition VisiblePositionAfterNode(dom::Node &node) noexcept;
  KRYS_NODISCARD bool LineBreakExistsAtVisiblePosition(const VisiblePosition &position) noexcept;
  KRYS_NODISCARD int32 IndexForVisiblePosition(const VisiblePosition &visiblePosition,
                                               RefPtr<dom::ContainerNode> &scope) noexcept;
  KRYS_NODISCARD int32 IndexForVisiblePosition(dom::Node &node, const VisiblePosition &visiblePosition,
                                               TextIteratorBehaviors behaviors) noexcept;
  KRYS_NODISCARD VisiblePosition VisiblePositionForPositionWithOffset(const VisiblePosition &position,
                                                                      int32 offset) noexcept;
  KRYS_NODISCARD VisiblePosition
    VisiblePositionForIndex(int32 index, dom::Node *scope,
                            TextIteratorBehaviors behaviors =
                              TextIteratorBehavior::EmitsCharactersBetweenAllVisiblePositions) noexcept;
  // FIXME: Why do we need this version?
  KRYS_NODISCARD VisiblePosition VisiblePositionForIndexUsingCharacterIterator(dom::Node &node,
                                                                               int32 index) noexcept;
  KRYS_NODISCARD VisiblePosition ClosestEditablePositionInElementForAbsolutePoint(
    const dom::Element &element, const IntPoint &point) noexcept;

#pragma endregion

#pragma region Element

  KRYS_NODISCARD dom::Element *
    EditableRootForPosition(const dom::Position &position,
                            EditableType editableType = EditableType::ContentIsEditable) noexcept;
  KRYS_NODISCARD RefPtr<dom::Element> UnsplittableElementForPosition(const dom::Position &position) noexcept;

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

#pragma region Caret rendering

  KRYS_NODISCARD RenderBlock *RendererForCaretPainting(const dom::Node *node) noexcept;
  KRYS_NODISCARD LayoutRect LocalCaretRectInRendererForCaretPainting(const VisiblePosition &caretPosition,
                                                                     RenderBlock *&caretPainter) noexcept;
  KRYS_NODISCARD LayoutRect LocalCaretRectInRendererForRect(LayoutRect &localRect, dom::Node *node,
                                                            RenderObject *renderer,
                                                            RenderBlock *&caretPainter) noexcept;
  KRYS_NODISCARD IntRect AbsoluteBoundsForLocalCaretRect(RenderBlock *rendererForCaretPainting,
                                                         const LayoutRect &rect,
                                                         bool *insideFixed = nullptr) noexcept;

#pragma endregion
}
