#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/CharacterData.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/PositionIterator.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/TextIterator.hpp"
#include "Krystal.Booey/Editing/VisiblePosition.hpp"
#include "Krystal.Booey/Editing/VisibleUnits.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLBRElement.hpp"
#include "Krystal.Booey/HTML/HTMLDListElement.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/HTMLOListElement.hpp"
#include "Krystal.Booey/HTML/HTMLPictureElement.hpp"
#include "Krystal.Booey/HTML/HTMLUListElement.hpp"
#include "Krystal.Booey/Rendering/HitTestSource.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderTable.hpp"
#include "Krystal.Booey/Rendering/RenderTableCell.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <unicode/uchar.h>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD bool IsHTMLElementOfType(const dom::Node *node, html::TagName tagName) noexcept
    {
      auto *element = DynamicDowncast<html::HTMLElement>(node);
      return element && element->TagQualifiedName().TagName() == tagName;
    }

    KRYS_NODISCARD bool IsHTMLBodyElement(const dom::Node *node) noexcept
    {
      return IsHTMLElementOfType(node, html::TagName::body);
    }
  }

  bool CanHaveChildrenForEditing(const dom::Node &node) noexcept
  {
    return !Is<dom::Text>(node) && !IsHTMLElementOfType(&node, html::TagName::picture)
           && node.CanContainRangeEndPoint();
  }

  // Atomic means that the node has no children, or has children which are ignored for the purposes of
  // editing.
  bool IsAtomicNode(const dom::Node *node) noexcept
  {
    return node && (!node->HasChildNodes() || EditingIgnoresContent(*node));
  }

  RefPtr<dom::ContainerNode> HighestEditableRoot(const dom::Position &position,
                                                 EditableType editableType) noexcept
  {
    RefPtr<dom::ContainerNode> highestEditableRoot =
      ShareRefPtr<dom::ContainerNode>(EditableRootForPosition(position, editableType));
    if (!highestEditableRoot)
    {
      return nullptr;
    }

    for (dom::ContainerNode *node = highestEditableRoot.get(); !IsHTMLBodyElement(node);)
    {
      node = node->ParentNode();
      if (!node)
      {
        break;
      }
      // FIXME: Can this ever be a Document or DocumentFragment? If not, this should return Element* instead.
      if (HasEditableStyle(*node, editableType))
      {
        highestEditableRoot = ShareRefPtr(node);
      }
    }

    return highestEditableRoot;
  }

  dom::Element *LowestEditableAncestor(dom::Node *node) noexcept
  {
    for (; node; node = node->ParentNode())
    {
      if (node->HasEditableStyle())
      {
        return node->RootEditableElement();
      }
      if (IsHTMLBodyElement(node))
      {
        break;
      }
    }
    return nullptr;
  }

  namespace
  {
    KRYS_NODISCARD bool ComputeEditability(const dom::Node &node, EditableType editableType,
                                           dom::Node::ShouldUpdateStyle shouldUpdateStyle) noexcept
    {
      if (node.ComputeEditability(dom::Node::UserSelectAllTreatment::NotEditable, shouldUpdateStyle)
          != dom::Node::Editability::ReadOnly)
      {
        return true;
      }

      switch (editableType)
      {
        case EditableType::ContentIsEditable: return false;
      }
      krys_unreachable();
    }
  }

  bool HasEditableStyle(const dom::Node &node, EditableType editableType) noexcept
  {
    return ComputeEditability(node, editableType, dom::Node::ShouldUpdateStyle::DoNotUpdate);
  }

  bool IsEditableNode(const dom::Node &node) noexcept
  {
    return ComputeEditability(node, EditableType::ContentIsEditable, dom::Node::ShouldUpdateStyle::Update);
  }

  bool IsEditablePosition(const dom::Position &position, EditableType editableType) noexcept
  {
    auto *node = position.GetContainerNode();
    return node && ComputeEditability(*node, editableType, dom::Node::ShouldUpdateStyle::Update);
  }

  bool IsAtUnsplittableElement(const dom::Position &position) noexcept
  {
    auto *node = position.GetContainerNode();
    return node == EditableRootForPosition(position)
           || node == EnclosingNodeOfType(position, IsTableCell).get();
  }

  bool IsRichlyEditablePosition(const dom::Position &position) noexcept
  {
    auto *node = position.GetContainerNode();
    return node && node->HasRichlyEditableStyle();
  }

  dom::Element *EditableRootForPosition(const dom::Position &position, EditableType editableType) noexcept
  {
    auto *node = position.GetContainerNode();
    if (!node)
    {
      return nullptr;
    }

    switch (editableType)
    {
      case EditableType::ContentIsEditable: return node->RootEditableElement();
    }
    return nullptr;
  }

  // Finds the enclosing element until which the tree can be split.
  // When a user hits ENTER, he/she won't expect this element to be split into two.
  // You may pass it as the second argument of splitTreeToNode.
  RefPtr<dom::Element> UnsplittableElementForPosition(const dom::Position &position) noexcept
  {
    // Since enclosingNodeOfType won't search beyond the highest root editable node,
    // this code works even if the closest table cell was outside of the root editable node.
    if (auto enclosingCell = EnclosingNodeOfType(position, &IsTableCell))
    {
      return ShareRefPtr(&Downcast<dom::Element>(*enclosingCell));
    }
    return ShareRefPtr(EditableRootForPosition(position));
  }

  dom::Position NextCandidate(const dom::Position &position) noexcept
  {
    for (dom::PositionIterator nextPosition = position; !nextPosition.AtEnd();)
    {
      nextPosition.Increment();
      if (nextPosition.IsCandidate())
      {
        return nextPosition;
      }
    }
    return {};
  }

  dom::Position NextVisuallyDistinctCandidate(const dom::Position &position,
                                              SkipDisplayContents skipDisplayContents) noexcept
  {
    // FIXME: Use PositionIterator instead.
    dom::Position nextPosition = position;
    dom::Position downstreamStart = nextPosition.Downstream();
    while (!nextPosition.AtEndOfTree())
    {
      nextPosition = nextPosition.Next(dom::PositionMoveType::Character);
      if (nextPosition.IsCandidate() && nextPosition.Downstream() != downstreamStart)
      {
        return nextPosition;
      }
      if (auto *node = nextPosition.GetContainerNode())
      {
        if (!node->Renderer())
        {
          if (skipDisplayContents == SkipDisplayContents::No)
          {
            if (auto *element = DynamicDowncast<dom::Element>(*node);
                element && element->HasDisplayContents())
            {
              continue;
            }
          }
          nextPosition = LastPositionInOrAfterNode(node);
        }
      }
    }
    return {};
  }

  dom::Position PreviousCandidate(const dom::Position &position) noexcept
  {
    dom::PositionIterator previousPosition = position;
    while (!previousPosition.AtStart())
    {
      previousPosition.Decrement();
      if (previousPosition.IsCandidate())
      {
        return previousPosition;
      }
    }
    return {};
  }

  dom::Position PreviousVisuallyDistinctCandidate(const dom::Position &position) noexcept
  {
    // FIXME: Use PositionIterator instead.
    dom::Position previousPosition = position;
    dom::Position downstreamStart = previousPosition.Downstream();
    while (!previousPosition.AtStartOfTree())
    {
      previousPosition = previousPosition.Previous(dom::PositionMoveType::Character);
      if (previousPosition.IsCandidate() && previousPosition.Downstream() != downstreamStart)
      {
        return previousPosition;
      }
      if (auto *node = previousPosition.GetContainerNode())
      {
        if (!node->Renderer())
        {
          previousPosition = FirstPositionInOrBeforeNode(node);
        }
      }
    }
    return {};
  }

  namespace
  {
    /// @brief The node in @p root's tree that is or contains @p node, when @p node is in a shadow tree inside
    /// it.
    KRYS_NODISCARD dom::Node *AncestorNodeInTreeOf(const dom::Node &root, dom::Node *node) noexcept
    {
      auto &rootTreeRoot = dom::TreeQueries::Root(root);
      for (auto *current = node; current;)
      {
        if (&dom::TreeQueries::Root(*current) == &rootTreeRoot)
        {
          return current;
        }
        current = dom::TreeQueries::ShadowHost(*current);
      }
      return nullptr;
    }
  }

  dom::Position FirstEditablePositionAfterPositionInRoot(const dom::Position &position,
                                                         dom::ContainerNode *highestRoot) noexcept
  {
    if (!highestRoot)
    {
      return {};
    }

    // position falls before highestRoot.
    if (position < dom::FirstPositionInNode(highestRoot) && highestRoot->HasEditableStyle())
    {
      return dom::FirstPositionInNode(highestRoot);
    }

    dom::Position candidate = position;

    if (&dom::TreeQueries::Root(*position.DeprecatedNode()) != &dom::TreeQueries::Root(*highestRoot))
    {
      auto *shadowAncestor = AncestorNodeInTreeOf(*highestRoot, position.DeprecatedNode());
      if (!shadowAncestor)
      {
        return {};
      }

      candidate = dom::PositionAfterNode(shadowAncestor);
    }

    while (candidate.DeprecatedNode() && !IsEditablePosition(candidate)
           && candidate.DeprecatedNode()->IsDescendantOf(*highestRoot))
    {
      candidate = IsAtomicNode(candidate.DeprecatedNode())
                    ? dom::PositionInParentAfterNode(candidate.DeprecatedNode())
                    : NextVisuallyDistinctCandidate(candidate);
    }

    if (candidate.DeprecatedNode() && !candidate.DeprecatedNode()->IsInclusiveDescendantOf(*highestRoot))
    {
      return {};
    }

    return candidate;
  }

  dom::Position LastEditablePositionBeforePositionInRoot(const dom::Position &position,
                                                         dom::ContainerNode *highestRoot) noexcept
  {
    if (!highestRoot)
    {
      return {};
    }

    // When position falls after highestRoot, the result is easy to compute.
    if (position > dom::LastPositionInNode(highestRoot))
    {
      return dom::LastPositionInNode(highestRoot);
    }

    dom::Position candidate = position;

    if (&dom::TreeQueries::Root(*position.DeprecatedNode()) != &dom::TreeQueries::Root(*highestRoot))
    {
      auto *shadowAncestor = AncestorNodeInTreeOf(*highestRoot, position.DeprecatedNode());
      if (!shadowAncestor)
      {
        return {};
      }

      candidate = FirstPositionInOrBeforeNode(shadowAncestor);
    }

    while (candidate.DeprecatedNode() && !IsEditablePosition(candidate)
           && candidate.DeprecatedNode()->IsDescendantOf(*highestRoot))
    {
      candidate = IsAtomicNode(candidate.DeprecatedNode())
                    ? dom::PositionInParentBeforeNode(candidate.DeprecatedNode())
                    : PreviousVisuallyDistinctCandidate(candidate);
    }

    if (candidate.DeprecatedNode() && !candidate.DeprecatedNode()->IsInclusiveDescendantOf(*highestRoot))
    {
      return {};
    }

    return candidate;
  }

  // FIXME: The function name, comment, and code say three different things here!
  // Whether or not content before and after this node will collapse onto the same line as it.
  bool IsBlock(const dom::Node &node) noexcept
  {
    return node.Renderer() && !node.Renderer()->IsInline();
  }

  bool IsInline(const dom::Node &node) noexcept
  {
    return node.Renderer() && node.Renderer()->IsInline();
  }

  // FIXME: Deploy this in all of the places where enclosingBlockFlow/enclosingBlockFlowOrTableElement are
  // used.
  // FIXME: Pass a position to this function. The enclosing block of [table, x] for example, should be the
  // block that contains the table and not the table, and this function should be the only one responsible for
  // knowing about these kinds of special cases.
  RefPtr<dom::Element> EnclosingBlock(RefPtr<dom::Node> node, EditingBoundaryCrossingRule rule) noexcept
  {
    auto block = EnclosingNodeOfType(FirstPositionInOrBeforeNode(node.get()), IsBlock, rule);
    return ShareRefPtr(DynamicDowncast<dom::Element>(block.get()));
  }

  TextDirection DirectionOfEnclosingBlock(const dom::Position &position) noexcept
  {
    auto block = EnclosingBlock(ShareRefPtr(position.GetContainerNode()));
    if (!block)
    {
      return TextDirection::LTR;
    }
    auto *renderer = block->Renderer();
    if (!renderer)
    {
      return TextDirection::LTR;
    }
    return renderer->GetWritingMode().BidiDirection();
  }

  // This method is used to create positions in the DOM. It returns the maximum valid offset
  // in a node. It returns 1 for some elements even though they do not have children, which
  // creates technically invalid DOM Positions. Be sure to call parentAnchoredEquivalent
  // on a Position before using it to create a DOM Range, or an exception will be thrown.
  int32 LastOffsetForEditing(const dom::Node &node) noexcept
  {
    if (Is<dom::CharacterData>(node) || node.HasChildNodes())
    {
      return static_cast<int32>(node.Length());
    }

    // FIXME: Might be more helpful to return 1 for any node where editingIgnoresContent is true, even one
    // that happens to have child nodes, like a select element with option node children.
    return EditingIgnoresContent(node) ? 1 : 0;
  }

  bool IsAmbiguousBoundaryCharacter(char32 character) noexcept
  {
    // These are characters that can behave as word boundaries, but can appear within words.
    // If they are just typed, i.e. if they are immediately followed by a caret, we want to delay text
    // checking until the next character has been typed.
    // FIXME: this is required until <rdar://problem/6853027> is fixed and text checking can do this for us.
    return character == '\'' || character == '@' || character == 0x2019 /* rightSingleQuotationMark */
           || character == 0x05F4 /* hebrewPunctuationGershayim */;
  }

  bool DeprecatedIsSpaceOrNewline(char32 character) noexcept
  {
    // Use IsUnicodeCompatibleASCIIWhitespace() for all Latin-1 characters, which is incorrect as it
    // excludes U+0085 and U+00A0.
    return character <= 0xFF ? text::IsUnicodeCompatibleASCIIWhitespace(character)
                             : u_charDirection(static_cast<UChar32>(character)) == U_WHITE_SPACE_NEUTRAL;
  }

  bool IsTableStructureNode(const dom::Node &node) noexcept
  {
    auto *renderer = node.Renderer();
    return renderer
           && (renderer->IsRenderTableCell() || renderer->IsRenderTableRow()
               || renderer->IsRenderTableSection() || renderer->IsRenderTableCol());
  }

  RefPtr<dom::Element> IsFirstPositionAfterTable(const VisiblePosition &position) noexcept
  {
    dom::Position upstream(position.DeepEquivalent().Upstream());
    auto *node = upstream.DeprecatedNode();
    if (!node)
    {
      return nullptr;
    }
    auto *renderer = node->Renderer();
    if (!renderer || !renderer->IsRenderTable() || !upstream.AtLastEditingPositionForNode())
    {
      return nullptr;
    }
    return ShareRefPtr(&Downcast<dom::Element>(*node));
  }

  RefPtr<dom::Element> IsLastPositionBeforeTable(const VisiblePosition &position) noexcept
  {
    dom::Position downstream(position.DeepEquivalent().Downstream());
    auto *node = downstream.DeprecatedNode();
    if (!node)
    {
      return nullptr;
    }
    auto *renderer = node->Renderer();
    if (!renderer || !renderer->IsRenderTable() || !downstream.AtFirstEditingPositionForNode())
    {
      return nullptr;
    }
    return ShareRefPtr(&Downcast<dom::Element>(*node));
  }

  // Returns the visible position at the beginning of a node
  VisiblePosition VisiblePositionBeforeNode(dom::Node &node) noexcept
  {
    if (node.HasChildNodes())
    {
      return VisiblePosition(FirstPositionInOrBeforeNode(&node));
    }
    krys_debug_assert(node.ParentNode());
    krys_debug_assert(!Is<dom::ShadowRoot>(*node.ParentNode()));
    return dom::PositionInParentBeforeNode(&node);
  }

  // Returns the visible position at the ending of a node
  VisiblePosition VisiblePositionAfterNode(dom::Node &node) noexcept
  {
    if (node.HasChildNodes())
    {
      return VisiblePosition(LastPositionInOrAfterNode(&node));
    }
    krys_debug_assert(node.ParentNode());
    krys_debug_assert(!Is<dom::ShadowRoot>(*node.ParentNode()));
    return dom::PositionInParentAfterNode(&node);
  }

  VisiblePosition ClosestEditablePositionInElementForAbsolutePoint(const dom::Element &element,
                                                                   const IntPoint &point) noexcept
  {
    // FIXME: The frame check (element.document().frame()), once documents know their frames.
    if (!element.IsConnected())
    {
      return {};
    }

    Ref protectedElement = ShareRef(element);
    element.NodeDocument().UpdateLayoutIgnorePendingStylesheets();

    auto *renderer = element.Renderer();
    // FIXME: Look at the inner element of a form control, not the control itself, as it is the editable part,
    // once there are text form controls.
    if (!renderer)
    {
      return {};
    }
    auto absoluteBoundingBox = renderer->AbsoluteBoundingBoxRect();
    auto constrainedAbsolutePoint =
      point.ConstrainedBetween(absoluteBoundingBox.MinXMinYCorner(), absoluteBoundingBox.MaxXMaxYCorner());
    auto localPoint =
      renderer->AbsoluteToLocal(FloatPoint(constrainedAbsolutePoint), MapCoordinatesMode::UseTransforms);
    auto visiblePosition =
      renderer->VisiblePositionForPoint(FlooredLayoutPoint(localPoint), HitTestSource::User);
    return IsEditablePosition(visiblePosition.DeepEquivalent()) ? visiblePosition : VisiblePosition {};
  }

  bool IsListHTMLElement(dom::Node *node) noexcept
  {
    return node
           && (IsHTMLElementOfType(node, html::TagName::ul) || IsHTMLElementOfType(node, html::TagName::ol)
               || IsHTMLElementOfType(node, html::TagName::dl));
  }

  bool IsListItem(const dom::Node &node) noexcept
  {
    return IsListHTMLElement(node.ParentNode()) || (node.Renderer() && node.Renderer()->IsRenderListItem());
  }

  RefPtr<dom::Node> EnclosingNodeOfType(const dom::Position &position,
                                        bool (*nodeIsOfType)(const dom::Node &),
                                        EditingBoundaryCrossingRule rule) noexcept
  {
    // FIXME: support CanSkipCrossEditingBoundary
    krys_debug_assert(rule == EditingBoundaryCrossingRule::CanCrossEditingBoundary
                      || rule == EditingBoundaryCrossingRule::CannotCrossEditingBoundary);
    auto root = rule == EditingBoundaryCrossingRule::CannotCrossEditingBoundary
                  ? HighestEditableRoot(position)
                  : nullptr;
    for (auto *n = position.DeprecatedNode(); n; n = n->ParentNode())
    {
      // Don't return a non-editable node if the input position was editable, since
      // the callers from editing will no doubt want to perform editing inside the returned node.
      if (root && !n->HasEditableStyle())
      {
        continue;
      }
      if (nodeIsOfType(*n))
      {
        return ShareRefPtr(n);
      }
      if (n == root.get())
      {
        return nullptr;
      }
    }
    return nullptr;
  }

  RefPtr<dom::Node> HighestEnclosingNodeOfType(const dom::Position &position,
                                               bool (*nodeIsOfType)(const dom::Node &),
                                               EditingBoundaryCrossingRule rule,
                                               dom::Node *stayWithin) noexcept
  {
    dom::Node *highest = nullptr;
    auto root = rule == EditingBoundaryCrossingRule::CannotCrossEditingBoundary
                  ? HighestEditableRoot(position)
                  : nullptr;
    for (auto *n = position.GetContainerNode(); n && n != stayWithin; n = n->ParentNode())
    {
      if (root && !n->HasEditableStyle())
      {
        continue;
      }
      if (nodeIsOfType(*n))
      {
        highest = n;
      }
      if (n == root.get())
      {
        break;
      }
    }
    return ShareRefPtr(highest);
  }

  RefPtr<dom::Element> EnclosingTableCell(const dom::Position &position) noexcept
  {
    auto node = EnclosingNodeOfType(position, IsTableCell);
    return node ? ShareRefPtr(&Downcast<dom::Element>(*node)) : nullptr;
  }

  RefPtr<dom::Element> EnclosingAnchorElement(const dom::Position &position) noexcept
  {
    for (auto *node = position.DeprecatedNode(); node; node = node->ParentNode())
    {
      if (auto *element = DynamicDowncast<dom::Element>(*node); element && css::IsLink(*element))
      {
        return ShareRefPtr(element);
      }
    }
    return nullptr;
  }

  namespace
  {
    KRYS_NODISCARD dom::Node *PreviousNodeConsideringAtomicNodes(const dom::Node *node) noexcept
    {
      if (node->PreviousSibling())
      {
        auto *n = node->PreviousSibling();
        while (!IsAtomicNode(n) && n->LastChild())
        {
          n = n->LastChild();
        }
        return n;
      }

      return node->ParentNode();
    }

    KRYS_NODISCARD dom::Node *NextNodeConsideringAtomicNodes(const dom::Node *node) noexcept
    {
      if (!IsAtomicNode(node) && node->FirstChild())
      {
        return node->FirstChild();
      }
      if (node->NextSibling())
      {
        return node->NextSibling();
      }
      const dom::Node *n = node;
      while (n && !n->NextSibling())
      {
        n = n->ParentNode();
      }
      if (n)
      {
        return n->NextSibling();
      }
      return nullptr;
    }
  }

  dom::Node *PreviousLeafNode(const dom::Node *node) noexcept
  {
    while ((node = PreviousNodeConsideringAtomicNodes(node)))
    {
      if (IsAtomicNode(node))
      {
        return const_cast<dom::Node *>(node);
      }
    }
    return nullptr;
  }

  dom::Node *NextLeafNode(const dom::Node *node) noexcept
  {
    while ((node = NextNodeConsideringAtomicNodes(node)))
    {
      if (IsAtomicNode(node))
      {
        return const_cast<dom::Node *>(node);
      }
    }
    return nullptr;
  }

  // FIXME: Do not require renderer, so that this can be used within fragments.
  bool IsRenderedTable(const dom::Node *node) noexcept
  {
    auto *element = DynamicDowncast<html::HTMLElement>(node);
    if (!element)
    {
      return false;
    }
    auto *renderer = element->Renderer();
    return renderer && renderer->IsRenderTable();
  }

  bool IsTableCell(const dom::Node &node) noexcept
  {
    auto *renderer = node.Renderer();
    if (!renderer)
    {
      return IsHTMLElementOfType(&node, html::TagName::td) || IsHTMLElementOfType(&node, html::TagName::th);
    }
    return renderer->IsRenderTableCell();
  }

  bool IsEmptyTableCell(const dom::Node *node) noexcept
  {
    // Returns true IFF the passed in node is one of:
    //   .) a table cell with no children,
    //   .) a table cell with a single BR child, and which has no other child renderers, including :before and
    //   :after renderers .) the BR child of such a table cell

    // Find rendered node
    while (node && !node->Renderer())
    {
      node = node->ParentNode();
    }
    if (!node)
    {
      return false;
    }

    // Make sure the rendered node is a table cell or <br>.
    // If it's a <br>, then the parent node has to be a table cell.
    const RenderObject *renderer = node->Renderer();
    if (renderer->IsBR())
    {
      renderer = renderer->Parent();
      if (!renderer)
      {
        return false;
      }
    }
    auto *renderTableCell = DynamicDowncast<RenderTableCell>(*renderer);
    if (!renderTableCell)
    {
      return false;
    }

    // Check that the table cell contains no child renderers except for perhaps a single <br>.
    auto *childRenderer = renderTableCell->FirstChild();
    if (!childRenderer)
    {
      return true;
    }
    if (!childRenderer->IsBR())
    {
      return false;
    }
    return !childRenderer->NextSibling();
  }

  void UpdatePositionForNodeRemoval(dom::Position &position, dom::Node &node) noexcept
  {
    if (position.IsNull())
    {
      return;
    }
    auto isShadowIncludingInclusiveAncestorOf = [&](const dom::Node *other)
    {
      return other && (other == &node || other->IsShadowIncludingDescendantOf(&node));
    };
    switch (position.GetAnchorType())
    {
      case dom::Position::AnchorType::PositionIsBeforeChildren:
        if (isShadowIncludingInclusiveAncestorOf(position.GetContainerNode()))
        {
          position = dom::PositionInParentBeforeNode(&node);
        }
        break;
      case dom::Position::AnchorType::PositionIsAfterChildren:
        if (isShadowIncludingInclusiveAncestorOf(position.GetContainerNode()))
        {
          position = dom::PositionInParentBeforeNode(&node);
        }
        break;
      case dom::Position::AnchorType::PositionIsOffsetInAnchor:
        if (position.GetContainerNode() == node.ParentNode()
            && static_cast<uint32>(position.OffsetInContainerNode()) > node.ComputeNodeIndex())
        {
          position.MoveToOffset(static_cast<uint32>(position.OffsetInContainerNode() - 1));
        }
        else if (isShadowIncludingInclusiveAncestorOf(position.GetContainerNode()))
        {
          position = dom::PositionInParentBeforeNode(&node);
        }
        break;
      case dom::Position::AnchorType::PositionIsAfterAnchor:
        if (isShadowIncludingInclusiveAncestorOf(position.AnchorNode()))
        {
          position = dom::PositionInParentAfterNode(&node);
        }
        break;
      case dom::Position::AnchorType::PositionIsBeforeAnchor:
        if (isShadowIncludingInclusiveAncestorOf(position.AnchorNode()))
        {
          position = dom::PositionInParentBeforeNode(&node);
        }
        break;
    }
  }

  int32 CaretMinOffset(const dom::Node &node) noexcept
  {
    auto *renderer = node.Renderer();
    krys_debug_assert(!Is<dom::CharacterData>(node) || !renderer || renderer->IsRenderText());

    if (renderer && renderer->IsRenderText())
    {
      return renderer->CaretMinOffset();
    }

    if (IsHTMLElementOfType(&node, html::TagName::picture))
    {
      for (auto *child = node.FirstChild(); child; child = child->NextSibling())
      {
        if (IsHTMLElementOfType(child, html::TagName::img))
        {
          return static_cast<int32>(child->ComputeNodeIndex());
        }
      }
    }

    return 0;
  }

  // If a node can contain candidates for VisiblePositions, return the offset of the last candidate, otherwise
  // return the number of children for container nodes and the length for unrendered text nodes.
  int32 CaretMaxOffset(const dom::Node &node) noexcept
  {
    // For rendered text nodes, return the last position that a caret could occupy.
    if (auto *text = DynamicDowncast<dom::Text>(node))
    {
      if (auto *renderer = text->Renderer())
      {
        return renderer->CaretMaxOffset();
      }
    }
    return LastOffsetForEditing(node);
  }

  bool LineBreakExistsAtVisiblePosition(const VisiblePosition &position) noexcept
  {
    return LineBreakExistsAtPosition(position.DeepEquivalent().Downstream());
  }

  bool LineBreakExistsAtPosition(const dom::Position &position) noexcept
  {
    if (position.IsNull())
    {
      return false;
    }

    if (IsHTMLElementOfType(position.AnchorNode(), html::TagName::br)
        && position.AtFirstEditingPositionForNode())
    {
      return true;
    }

    if (!position.AnchorNode()->Renderer())
    {
      return false;
    }

    auto *textNode = DynamicDowncast<dom::Text>(*position.AnchorNode());
    if (!textNode || !textNode->Renderer()->Style().PreserveNewline())
    {
      return false;
    }

    uint32 offset = static_cast<uint32>(position.OffsetInContainerNode());
    return offset < textNode->Length() && textNode->Data()[offset] == '\n';
  }

  // FIXME: indexForVisiblePosition and visiblePositionForIndex use TextIterators to convert between
  // VisiblePositions and indices. But TextIterator iteration using
  // TextIteratorBehavior::EmitsCharactersBetweenAllVisiblePositions does not exactly match VisiblePosition
  // iteration, so using them to preserve a selection during an editing opertion is unreliable. TextIterator's
  // TextIteratorBehavior::EmitsCharactersBetweenAllVisiblePositions mode needs to be fixed, or these
  // functions need to be changed to iterate using actual VisiblePositions.
  // FIXME: Deploy these functions everywhere that TextIterators are used to convert between VisiblePositions
  // and indices.
  int32 IndexForVisiblePosition(const VisiblePosition &visiblePosition,
                                RefPtr<dom::ContainerNode> &scope) noexcept
  {
    if (visiblePosition.IsNull())
    {
      return 0;
    }

    auto position = visiblePosition.DeepEquivalent();
    auto &document = *position.GetDocument();

    // FIXME: The document's design mode, once there's one.
    auto editableRoot = HighestEditableRoot(position, EditableType::ContentIsEditable);
    if (editableRoot)
    {
      scope = editableRoot;
    }
    else
    {
      if (position.GetContainerNode()->IsInShadowTree())
      {
        scope = ShareRefPtr<dom::ContainerNode>(
          dom::TreeQueries::ContainingShadowRoot(*position.GetContainerNode()));
      }
      else
      {
        scope = ShareRefPtr<dom::ContainerNode>(&document);
      }
    }

    auto range = *dom::MakeSimpleRange(dom::MakeBoundaryPointBeforeNodeContents(*scope), position);
    return static_cast<int32>(
      CharacterCount(range, TextIteratorBehavior::EmitsCharactersBetweenAllVisiblePositions));
  }

  // FIXME: Merge this function with the one above.
  int32 IndexForVisiblePosition(dom::Node &node, const VisiblePosition &visiblePosition,
                                TextIteratorBehaviors behaviors) noexcept
  {
    if (visiblePosition.IsNull())
    {
      return 0;
    }

    auto range = dom::MakeSimpleRange(dom::MakeBoundaryPointBeforeNodeContents(node), visiblePosition);
    return range ? static_cast<int32>(CharacterCount(*range, behaviors)) : 0;
  }

  VisiblePosition VisiblePositionForPositionWithOffset(const VisiblePosition &position, int32 offset) noexcept
  {
    RefPtr<dom::ContainerNode> root;
    uint32 startIndex = static_cast<uint32>(IndexForVisiblePosition(position, root));
    if (!root)
    {
      return {};
    }

    return VisiblePositionForIndex(static_cast<int32>(startIndex) + offset, root.get());
  }

  VisiblePosition VisiblePositionForIndex(int32 index, dom::Node *scope,
                                          TextIteratorBehaviors behaviors) noexcept
  {
    if (!scope)
    {
      return {};
    }
    return {dom::MakeDeprecatedLegacyPosition(ResolveCharacterLocation(
      dom::MakeRangeSelectingNodeContents(*scope), static_cast<uint64>(index), behaviors))};
  }

  VisiblePosition VisiblePositionForIndexUsingCharacterIterator(dom::Node &node, int32 index) noexcept
  {
    if (index <= 0)
    {
      return {FirstPositionInOrBeforeNode(&node)};
    }

    auto range = dom::MakeRangeSelectingNodeContents(node);
    CharacterIterator it(range);
    if (!it.AtEnd())
    {
      it.Advance(index - 1);
    }

    if (!it.AtEnd() && it.Text().size() == 1 && it.Text()[0] == '\n')
    {
      // FIXME: workaround for collapsed range (where only start position is correct) emitted for some emitted
      // newlines.
      it.Advance(1);
      if (!it.AtEnd())
      {
        return {dom::MakeDeprecatedLegacyPosition(it.Range().Start)};
      }
    }

    return {dom::MakeDeprecatedLegacyPosition((it.AtEnd() ? range : it.Range()).End), Affinity::Upstream};
  }

  // Determines whether a node is inside a range or visibly starts and ends at the boundaries of the range.
  // Call this function to determine whether a node is visibly fit inside selectedRange
  bool IsNodeVisiblyContainedWithin(dom::Node &node, const dom::SimpleRange &range) noexcept
  {
    if (dom::Contains<dom::TreeType::ComposedTree>(range, node))
    {
      return true;
    }

    auto startPosition = dom::MakeDeprecatedLegacyPosition(range.Start);
    auto endPosition = dom::MakeDeprecatedLegacyPosition(range.End);

    bool startIsVisuallySame = VisiblePositionBeforeNode(node) == VisiblePosition(startPosition);
    if (startIsVisuallySame && dom::PositionInParentAfterNode(&node) < endPosition)
    {
      return true;
    }

    bool endIsVisuallySame = VisiblePositionAfterNode(node) == VisiblePosition(endPosition);
    if (endIsVisuallySame && startPosition < dom::PositionInParentBeforeNode(&node))
    {
      return true;
    }

    return startIsVisuallySame && endIsVisuallySame;
  }

  bool IsRenderedAsNonInlineTableImageOrHR(const dom::Node *node) noexcept
  {
    if (!node)
    {
      return false;
    }
    auto *renderer = node->Renderer();
    return renderer && !renderer->IsInline()
           && (renderer->IsRenderTable() || renderer->IsRenderImage() || renderer->IsHR());
  }

  namespace
  {
    // FIXME: Should this be deprecated like deprecatedEnclosingBlockFlowElement is?
    KRYS_NODISCARD dom::Element *ElementIfBlockFlow(dom::Node &node) noexcept
    {
      auto *element = DynamicDowncast<dom::Element>(node);
      if (!element)
      {
        return nullptr;
      }
      auto *renderer = element->Renderer();
      return renderer && renderer->IsRenderBlockFlow() ? element : nullptr;
    }
  }

  bool IsBlockFlowElement(const dom::Node &node) noexcept
  {
    return ElementIfBlockFlow(const_cast<dom::Node &>(node));
  }

  dom::Element *DeprecatedEnclosingBlockFlowElement(dom::Node *node) noexcept
  {
    if (!node)
    {
      return nullptr;
    }
    if (auto *element = ElementIfBlockFlow(*node))
    {
      return element;
    }
    while ((node = node->ParentNode()))
    {
      if (auto *element = ElementIfBlockFlow(*node))
      {
        return element;
      }
      if (IsHTMLBodyElement(node))
      {
        return &Downcast<dom::Element>(*node);
      }
    }
    return nullptr;
  }

  namespace
  {
    KRYS_NODISCARD inline bool CaretRendersInsideNode(const dom::Node &node) noexcept
    {
      return !IsRenderedTable(&node) && !EditingIgnoresContent(node);
    }
  }

  RenderBlock *RendererForCaretPainting(const dom::Node *node) noexcept
  {
    if (!node)
    {
      return nullptr;
    }

    auto *renderer = node->Renderer();
    if (!renderer)
    {
      return nullptr;
    }

    // If caretNode is a block and caret is inside it, then caret should be painted by that block.
    if (auto *blockFlow = DynamicDowncast<RenderBlockFlow>(*renderer))
    {
      if (CaretRendersInsideNode(*node))
      {
        return blockFlow;
      }
    }
    return renderer->ContainingBlock();
  }

  LayoutRect LocalCaretRectInRendererForCaretPainting(const VisiblePosition &caretPosition,
                                                      RenderBlock *&caretPainter) noexcept
  {
    if (caretPosition.IsNull())
    {
      return LayoutRect();
    }
    krys_debug_assert(caretPosition.DeepEquivalent().DeprecatedNode()->Renderer());
    auto [localRect, renderer] = caretPosition.LocalCaretRect();
    return LocalCaretRectInRendererForRect(localRect, caretPosition.DeepEquivalent().DeprecatedNode(),
                                           renderer, caretPainter);
  }

  LayoutRect LocalCaretRectInRendererForRect(LayoutRect &localRect, dom::Node *node, RenderObject *renderer,
                                             RenderBlock *&caretPainter) noexcept
  {
    // Get the renderer that will be responsible for painting the caret
    // (which is either the renderer we just found, or one of its containers).
    caretPainter = RendererForCaretPainting(node);

    // Compute an offset between the renderer and the caretPainter.
    while (renderer != caretPainter)
    {
      auto *containerObject = renderer->Container();
      if (!containerObject)
      {
        return LayoutRect();
      }
      localRect.Move(renderer->OffsetFromContainer(*containerObject, localRect.Location()));
      renderer = containerObject;
    }

    return localRect;
  }

  IntRect AbsoluteBoundsForLocalCaretRect(RenderBlock *rendererForCaretPainting, const LayoutRect &rect,
                                          bool *insideFixed) noexcept
  {
    if (insideFixed)
    {
      *insideFixed = false;
    }

    if (!rendererForCaretPainting || rect.IsEmpty())
    {
      return IntRect();
    }

    LayoutRect localRect(rect);
    rendererForCaretPainting->FlipForWritingMode(localRect);
    return rendererForCaretPainting
      ->LocalToAbsoluteQuad(FloatQuad(FloatRect(localRect)), MapCoordinatesMode::UseTransforms, insideFixed)
      .EnclosingBoundingBox();
  }

  dom::PositionRange PositionsForRange(const dom::SimpleRange &range) noexcept
  {
    return {dom::MakeContainerOffsetPosition(range.Start).Downstream(),
            dom::MakeContainerOffsetPosition(range.End).Upstream()};
  }
}
