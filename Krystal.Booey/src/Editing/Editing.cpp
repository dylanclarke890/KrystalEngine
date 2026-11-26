/*
 * Copyright (C) 2004-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2015 Google Inc. All rights reserved.
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

#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/Css/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/Dom/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/Dom/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/Dom/Nodes/Attr.hpp"
#include "Krystal.Booey/Dom/Nodes/CharacterData.hpp"
#include "Krystal.Booey/Dom/Nodes/Document.hpp"
#include "Krystal.Booey/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/Dom/Nodes/Text.hpp"
#include "Krystal.Booey/Dom/Ranges/SimpleRange.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/Markup/HTMLInterchange.hpp"
#include "Krystal.Booey/Editing/PositionIterator.hpp"
#include "Krystal.Booey/Editing/Selection/VisiblePosition.hpp"
#include "Krystal.Booey/Editing/Selection/VisibleSelection.hpp"
#include "Krystal.Booey/Editing/Selection/VisibleUnits.hpp"
#include "Krystal.Booey/Editing/TextIterator.hpp"
#include "Krystal.Booey/Html/Elements/HTMLBodyElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLBRElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLDListElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLImageElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLOListElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLPictureElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLTextFormControlElement.hpp"
#include "Krystal.Booey/Html/Elements/HTMLUListElement.hpp"
#include "Krystal.Booey/Html/Shadow/TextControlInnerElements.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderTextControlSingleLine.hpp"
#include "Krystal.Booey/Rendering/HitTesting/HitTestSource.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Tables/RenderTable.hpp"
#include "Krystal.Booey/Rendering/Tables/RenderTableCell.hpp"
#include "Krystal.Core/Geometry/Point.hpp"
#include "Krystal.Core/Geometry/Quad.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"
#include "Krystal.Text/Bidi.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD bool IsHTMLBodyElement(const Node *node) noexcept
    {
      return IsHTMLElementOfType(node, html::TagName::body);
    }
  }

  bool CanHaveChildrenForEditing(const Node &node) noexcept
  {
    return !Is<Text>(node) && !IsHTMLElementOfType(&node, html::TagName::picture)
           && node.CanContainRangeEndPoint();
  }

  // Atomic means that the node has no children, or has children which are ignored for the purposes of
  // editing.
  bool IsAtomicNode(const Node *node) noexcept
  {
    return node && (!node->HasChildNodes() || EditingIgnoresContent(*node));
  }

  RefPtr<ContainerNode> HighestEditableRoot(const Position &position, EditableType editableType) noexcept
  {
    RefPtr<ContainerNode> highestEditableRoot =
      ShareRefPtr<ContainerNode>(EditableRootForPosition(position, editableType));
    if (!highestEditableRoot)
    {
      return nullptr;
    }

    for (ContainerNode *node = highestEditableRoot.get(); !IsHTMLBodyElement(node);)
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

  Element *LowestEditableAncestor(Node *node) noexcept
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
    KRYS_NODISCARD bool ComputeEditability(const Node &node, EditableType editableType,
                                           Node::ShouldUpdateStyle shouldUpdateStyle) noexcept
    {
      if (node.ComputeEditability(Node::UserSelectAllTreatment::NotEditable, shouldUpdateStyle)
          != Node::Editability::ReadOnly)
      {
        return true;
      }

      switch (editableType)
      {
        case EditableType::ContentIsEditable: return false;
        case EditableType::HasEditableAXRole:
          if (auto *cache = node.NodeDocument().ExistingAXObjectCache())
          {
            return cache->RootAXEditableElement(node) != nullptr;
          }
          return false;
      }
      krys_unreachable();
    }
  }

  bool HasEditableStyle(const Node &node, EditableType editableType) noexcept
  {
    return ComputeEditability(node, editableType, Node::ShouldUpdateStyle::DoNotUpdate);
  }

  bool IsEditableNode(const Node &node) noexcept
  {
    return ComputeEditability(node, EditableType::ContentIsEditable, Node::ShouldUpdateStyle::Update);
  }

  bool IsEditablePosition(const Position &position, EditableType editableType) noexcept
  {
    auto *node = position.GetContainerNode();
    return node && ComputeEditability(*node, editableType, Node::ShouldUpdateStyle::Update);
  }

  bool IsAtUnsplittableElement(const Position &position) noexcept
  {
    auto *node = position.GetContainerNode();
    return node == EditableRootForPosition(position)
           || node == EnclosingNodeOfType(position, IsTableCell).get();
  }

  bool IsRichlyEditablePosition(const Position &position) noexcept
  {
    auto *node = position.GetContainerNode();
    return node && node->HasRichlyEditableStyle();
  }

  Element *EditableRootForPosition(const Position &position, EditableType editableType) noexcept
  {
    auto *node = position.GetContainerNode();
    if (!node)
    {
      return nullptr;
    }

    switch (editableType)
    {
      case EditableType::HasEditableAXRole:
        if (auto *cache = node->NodeDocument().ExistingAXObjectCache())
        {
          return cache->RootAXEditableElement(*node);
        }
        [[fallthrough]];
      case EditableType::ContentIsEditable: return node->RootEditableElement();
    }
    return nullptr;
  }

  // Finds the enclosing element until which the tree can be split.
  // When a user hits ENTER, he/she won't expect this element to be split into two.
  // You may pass it as the second argument of splitTreeToNode.
  RefPtr<Element> UnsplittableElementForPosition(const Position &position) noexcept
  {
    // Since enclosingNodeOfType won't search beyond the highest root editable node,
    // this code works even if the closest table cell was outside of the root editable node.
    if (auto enclosingCell = EnclosingNodeOfType(position, &IsTableCell))
    {
      return ShareRefPtr(&Downcast<Element>(*enclosingCell));
    }
    return ShareRefPtr(EditableRootForPosition(position));
  }

  Position NextCandidate(const Position &position) noexcept
  {
    for (PositionIterator nextPosition = position; !nextPosition.AtEnd();)
    {
      nextPosition.Increment();
      if (nextPosition.IsCandidate())
      {
        return nextPosition;
      }
    }
    return {};
  }

  Position NextVisuallyDistinctCandidate(const Position &position,
                                         SkipDisplayContents skipDisplayContents) noexcept
  {
    // FIXME: Use PositionIterator instead.
    Position nextPosition = position;
    Position downstreamStart = nextPosition.Downstream();
    while (!nextPosition.AtEndOfTree())
    {
      nextPosition = nextPosition.Next(PositionMoveType::Character);
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
            if (auto *element = DynamicDowncast<Element>(*node); element && element->HasDisplayContents())
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

  Position PreviousCandidate(const Position &position) noexcept
  {
    PositionIterator previousPosition = position;
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

  Position PreviousVisuallyDistinctCandidate(const Position &position) noexcept
  {
    // FIXME: Use PositionIterator instead.
    Position previousPosition = position;
    Position downstreamStart = previousPosition.Downstream();
    while (!previousPosition.AtStartOfTree())
    {
      previousPosition = previousPosition.Previous(PositionMoveType::Character);
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
    KRYS_NODISCARD Node *AncestorNodeInTreeOf(const Node &root, Node *node) noexcept
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

  Position FirstEditablePositionAfterPositionInRoot(const Position &position,
                                                    ContainerNode *highestRoot) noexcept
  {
    if (!highestRoot)
    {
      return {};
    }

    // position falls before highestRoot.
    if (position < FirstPositionInNode(highestRoot) && highestRoot->HasEditableStyle())
    {
      return FirstPositionInNode(highestRoot);
    }

    Position candidate = position;

    if (&dom::TreeQueries::Root(*position.DeprecatedNode()) != &dom::TreeQueries::Root(*highestRoot))
    {
      auto *shadowAncestor = AncestorNodeInTreeOf(*highestRoot, position.DeprecatedNode());
      if (!shadowAncestor)
      {
        return {};
      }

      candidate = PositionAfterNode(shadowAncestor);
    }

    while (candidate.DeprecatedNode() && !IsEditablePosition(candidate)
           && candidate.DeprecatedNode()->IsDescendantOf(*highestRoot))
    {
      candidate = IsAtomicNode(candidate.DeprecatedNode())
                    ? PositionInParentAfterNode(candidate.DeprecatedNode())
                    : NextVisuallyDistinctCandidate(candidate);
    }

    if (candidate.DeprecatedNode() && !candidate.DeprecatedNode()->IsInclusiveDescendantOf(*highestRoot))
    {
      return {};
    }

    return candidate;
  }

  Position LastEditablePositionBeforePositionInRoot(const Position &position,
                                                    ContainerNode *highestRoot) noexcept
  {
    if (!highestRoot)
    {
      return {};
    }

    // When position falls after highestRoot, the result is easy to compute.
    if (position > LastPositionInNode(highestRoot))
    {
      return LastPositionInNode(highestRoot);
    }

    Position candidate = position;

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
                    ? PositionInParentBeforeNode(candidate.DeprecatedNode())
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
  bool IsBlock(const Node &node) noexcept
  {
    return node.Renderer() && !node.Renderer()->IsInline();
  }

  bool IsInline(const Node &node) noexcept
  {
    return node.Renderer() && node.Renderer()->IsInline();
  }

  // FIXME: Deploy this in all of the places where enclosingBlockFlow/enclosingBlockFlowOrTableElement are
  // used.
  // FIXME: Pass a position to this function. The enclosing block of [table, x] for example, should be the
  // block that contains the table and not the table, and this function should be the only one responsible for
  // knowing about these kinds of special cases.
  RefPtr<Element> EnclosingBlock(const RefPtr<Node> &node, EditingBoundaryCrossingRule rule) noexcept
  {
    auto block = EnclosingNodeOfType(FirstPositionInOrBeforeNode(node.get()), IsBlock, rule);
    return DynamicDowncast<Element>(krys::move(block));
  }

  Node *AncestorNodeInThisScope(const Node &scopeRoot, Node *node) noexcept
  {
    for (; node; node = dom::TreeQueries::ShadowHost(*node))
    {
      if (&dom::TreeQueries::Root(*node) == &scopeRoot)
      {
        return node;
      }
      if (!node->IsInShadowTree())
      {
        return nullptr;
      }
    }
    return nullptr;
  }

  text::TextDirection DirectionOfEnclosingBlock(const Position &position) noexcept
  {
    auto block = EnclosingBlock(ShareRefPtr(position.GetContainerNode()));
    if (!block)
    {
      return text::TextDirection::LTR;
    }
    auto *renderer = block->Renderer();
    if (!renderer)
    {
      return text::TextDirection::LTR;
    }
    return renderer->GetWritingMode().BidiDirection();
  }

  // This method is used to create positions in the DOM. It returns the maximum valid offset
  // in a node. It returns 1 for some elements even though they do not have children, which
  // creates technically invalid DOM Positions. Be sure to call parentAnchoredEquivalent
  // on a Position before using it to create a DOM Range, or an exception will be thrown.
  int32 LastOffsetForEditing(const Node &node) noexcept
  {
    if (Is<CharacterData>(node) || node.HasChildNodes())
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
    return character == '\'' || character == '@' || character == 0x2019 /* rightSingleQuotationMark */
           || character == 0x05F4 /* hebrewPunctuationGershayim */;
  }

  bool DeprecatedIsSpaceOrNewline(char32 character) noexcept
  {
    // Use IsUnicodeCompatibleASCIIWhitespace() for all Latin-1 characters, which is incorrect as it
    // excludes U+0085 and U+00A0.
    return character <= 0xFF ? text::IsUnicodeCompatibleASCIIWhitespace(character)
                             : text::GetBidiClass(character) == text::BidiClass::WhiteSpaceNeutral;
  }

  bool IsTableStructureNode(const Node &node) noexcept
  {
    auto *renderer = node.Renderer();
    return renderer
           && (renderer->IsRenderTableCell() || renderer->IsRenderTableRow()
               || renderer->IsRenderTableSection() || renderer->IsRenderTableCol());
  }

  RefPtr<Element> IsFirstPositionAfterTable(const VisiblePosition &position) noexcept
  {
    Position upstream(position.DeepEquivalent().Upstream());
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
    return ShareRefPtr(&Downcast<Element>(*node));
  }

  RefPtr<Element> IsLastPositionBeforeTable(const VisiblePosition &position) noexcept
  {
    Position downstream(position.DeepEquivalent().Downstream());
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
    return ShareRefPtr(&Downcast<Element>(*node));
  }

  // Returns the visible position at the beginning of a node
  VisiblePosition VisiblePositionBeforeNode(Node &node) noexcept
  {
    if (node.HasChildNodes())
    {
      return VisiblePosition(FirstPositionInOrBeforeNode(&node));
    }
    krys_debug_assert(node.ParentNode());
    krys_debug_assert(!Is<ShadowRoot>(*node.ParentNode()));
    return PositionInParentBeforeNode(&node);
  }

  // Returns the visible position at the ending of a node
  VisiblePosition VisiblePositionAfterNode(Node &node) noexcept
  {
    if (node.HasChildNodes())
    {
      return VisiblePosition(LastPositionInOrAfterNode(&node));
    }
    krys_debug_assert(node.ParentNode());
    krys_debug_assert(!Is<ShadowRoot>(*node.ParentNode()));
    return PositionInParentAfterNode(&node);
  }

  VisiblePosition ClosestEditablePositionInElementForAbsolutePoint(const Element &element,
                                                                   const PointI &point) noexcept
  {
    // FIXME: The frame check (element.document().frame()), once documents know their frames.
    if (!element.IsConnected())
    {
      return {};
    }

    Ref protectedElement = ShareRef(element);
    element.NodeDocument().UpdateLayoutIgnorePendingStylesheets();

    RenderObject *renderer = element.Renderer();
    // Look at the inner element of a form control, not the control itself, as it is the editable part.
    if (auto *formControlElement = DynamicDowncast<HTMLTextFormControlElement>(element))
    {
      if (!formControlElement->IsInnerTextElementEditable())
      {
        return {};
      }
      if (auto innerTextElement = formControlElement->InnerTextElement())
      {
        renderer = innerTextElement->Renderer();
      }
    }
    if (!renderer)
    {
      return {};
    }
    auto absoluteBoundingBox = renderer->AbsoluteBoundingBoxRect();
    auto constrainedAbsolutePoint =
      point.ConstrainedBetween(absoluteBoundingBox.MinXMinYCorner(), absoluteBoundingBox.MaxXMaxYCorner());
    auto localPoint =
      renderer->AbsoluteToLocal(PointF(constrainedAbsolutePoint), MapCoordinatesMode::UseTransforms);
    auto visiblePosition =
      renderer->VisiblePositionForPoint(FlooredLayoutPoint(localPoint), HitTestSource::User);
    return IsEditablePosition(visiblePosition.DeepEquivalent()) ? visiblePosition : VisiblePosition {};
  }

  bool IsListHTMLElement(Node *node) noexcept
  {
    return node
           && (IsHTMLElementOfType(node, html::TagName::ul) || IsHTMLElementOfType(node, html::TagName::ol)
               || IsHTMLElementOfType(node, html::TagName::dl));
  }

  bool IsListItem(const Node &node) noexcept
  {
    return IsListHTMLElement(node.ParentNode()) || (node.Renderer() && node.Renderer()->IsRenderListItem());
  }

  RefPtr<Node> EnclosingNodeOfType(const Position &position, bool (*nodeIsOfType)(const Node &),
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

  RefPtr<Node> HighestEnclosingNodeOfType(const Position &position, bool (*nodeIsOfType)(const Node &),
                                          EditingBoundaryCrossingRule rule, Node *stayWithin) noexcept
  {
    Node *highest = nullptr;
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

  RefPtr<Element> EnclosingTableCell(const Position &position) noexcept
  {
    auto node = EnclosingNodeOfType(position, IsTableCell);
    return node ? ShareRefPtr(&Downcast<Element>(*node)) : nullptr;
  }

  RefPtr<Element> EnclosingAnchorElement(const Position &position) noexcept
  {
    for (auto *node = position.DeprecatedNode(); node; node = node->ParentNode())
    {
      if (auto *element = DynamicDowncast<Element>(*node); element && css::IsLink(*element))
      {
        return ShareRefPtr(element);
      }
    }
    return nullptr;
  }

  namespace
  {
    KRYS_NODISCARD Node *PreviousNodeConsideringAtomicNodes(const Node *node) noexcept
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

    KRYS_NODISCARD Node *NextNodeConsideringAtomicNodes(const Node *node) noexcept
    {
      if (!IsAtomicNode(node) && node->FirstChild())
      {
        return node->FirstChild();
      }
      if (node->NextSibling())
      {
        return node->NextSibling();
      }
      const Node *n = node;
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

  Node *PreviousLeafNode(const Node *node) noexcept
  {
    while ((node = PreviousNodeConsideringAtomicNodes(node)))
    {
      if (IsAtomicNode(node))
      {
        return const_cast<Node *>(node);
      }
    }
    return nullptr;
  }

  Node *NextLeafNode(const Node *node) noexcept
  {
    while ((node = NextNodeConsideringAtomicNodes(node)))
    {
      if (IsAtomicNode(node))
      {
        return const_cast<Node *>(node);
      }
    }
    return nullptr;
  }

  // FIXME: Do not require renderer, so that this can be used within fragments.
  bool IsRenderedTable(const Node *node) noexcept
  {
    auto *element = DynamicDowncast<HTMLElement>(node);
    if (!element)
    {
      return false;
    }
    auto *renderer = element->Renderer();
    return renderer && renderer->IsRenderTable();
  }

  bool IsTableCell(const Node &node) noexcept
  {
    auto *renderer = node.Renderer();
    if (!renderer)
    {
      return IsHTMLElementOfType(&node, html::TagName::td) || IsHTMLElementOfType(&node, html::TagName::th);
    }
    return renderer->IsRenderTableCell();
  }

  bool IsEmptyTableCell(const Node *node) noexcept
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

  void UpdatePositionForNodeRemoval(Position &position, Node &node) noexcept
  {
    if (position.IsNull())
    {
      return;
    }
    auto isShadowIncludingInclusiveAncestorOf = [&](const Node *other)
    {
      return other && (other == &node || other->IsShadowIncludingDescendantOf(&node));
    };
    switch (position.GetAnchorType())
    {
      case Position::AnchorType::PositionIsBeforeChildren:
      case Position::AnchorType::PositionIsAfterChildren:
        if (isShadowIncludingInclusiveAncestorOf(position.GetContainerNode()))
        {
          position = PositionInParentBeforeNode(&node);
        }
        break;
      case Position::AnchorType::PositionIsOffsetInAnchor:
        if (position.GetContainerNode() == node.ParentNode()
            && static_cast<uint32>(position.OffsetInContainerNode()) > node.ComputeNodeIndex())
        {
          position.MoveToOffset(static_cast<uint32>(position.OffsetInContainerNode() - 1));
        }
        else if (isShadowIncludingInclusiveAncestorOf(position.GetContainerNode()))
        {
          position = PositionInParentBeforeNode(&node);
        }
        break;
      case Position::AnchorType::PositionIsAfterAnchor:
        if (isShadowIncludingInclusiveAncestorOf(position.AnchorNode()))
        {
          position = PositionInParentAfterNode(&node);
        }
        break;
      case Position::AnchorType::PositionIsBeforeAnchor:
        if (isShadowIncludingInclusiveAncestorOf(position.AnchorNode()))
        {
          position = PositionInParentBeforeNode(&node);
        }
        break;
    }
  }

  int32 CaretMinOffset(const Node &node) noexcept
  {
    auto *renderer = node.Renderer();
    krys_debug_assert(!Is<CharacterData>(node) || !renderer || renderer->IsRenderText());

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
  int32 CaretMaxOffset(const Node &node) noexcept
  {
    // For rendered text nodes, return the last position that a caret could occupy.
    if (auto *text = DynamicDowncast<Text>(node))
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

  bool LineBreakExistsAtPosition(const Position &position) noexcept
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

    auto *textNode = DynamicDowncast<Text>(*position.AnchorNode());
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
  int32 IndexForVisiblePosition(const VisiblePosition &visiblePosition, RefPtr<ContainerNode> &scope) noexcept
  {
    if (visiblePosition.IsNull())
    {
      return 0;
    }

    auto position = visiblePosition.DeepEquivalent();
    auto &document = *position.GetDocument();

    auto editableRoot =
      HighestEditableRoot(position, AXObjectCache::AccessibilityEnabled() ? EditableType::HasEditableAXRole
                                                                          : EditableType::ContentIsEditable);
    if (editableRoot && !document.InDesignMode())
    {
      scope = editableRoot;
    }
    else
    {
      if (position.GetContainerNode()->IsInShadowTree())
      {
        scope =
          ShareRefPtr<ContainerNode>(dom::TreeQueries::ContainingShadowRoot(*position.GetContainerNode()));
      }
      else
      {
        scope = ShareRefPtr<ContainerNode>(&document);
      }
    }

    auto range = *dom::CreateSimpleRange(dom::CreateBoundaryPointBeforeNodeContents(*scope), position);
    return static_cast<int32>(
      CharacterCount(range, TextIteratorBehavior::EmitsCharactersBetweenAllVisiblePositions));
  }

  // FIXME: Merge this function with the one above.
  int32 IndexForVisiblePosition(Node &node, const VisiblePosition &visiblePosition,
                                TextIteratorBehaviors behaviors) noexcept
  {
    if (visiblePosition.IsNull())
    {
      return 0;
    }

    auto range = dom::CreateSimpleRange(dom::CreateBoundaryPointBeforeNodeContents(node), visiblePosition);
    return range ? static_cast<int32>(CharacterCount(*range, behaviors)) : 0;
  }

  VisiblePosition VisiblePositionForPositionWithOffset(const VisiblePosition &position, int32 offset) noexcept
  {
    RefPtr<ContainerNode> root;
    uint32 startIndex = static_cast<uint32>(IndexForVisiblePosition(position, root));
    if (!root)
    {
      return {};
    }

    return VisiblePositionForIndex(static_cast<int32>(startIndex) + offset, root.get());
  }

  VisiblePosition VisiblePositionForIndex(int32 index, Node *scope, TextIteratorBehaviors behaviors) noexcept
  {
    if (!scope)
    {
      return {};
    }
    return {CreateDeprecatedLegacyPosition(ResolveCharacterLocation(
      dom::CreateRangeSelectingNodeContents(*scope), static_cast<uint64>(index), behaviors))};
  }

  VisiblePosition VisiblePositionForIndexUsingCharacterIterator(Node &node, int32 index) noexcept
  {
    if (index <= 0)
    {
      return {FirstPositionInOrBeforeNode(&node)};
    }

    auto range = dom::CreateRangeSelectingNodeContents(node);
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
        return {CreateDeprecatedLegacyPosition(it.Range().Start)};
      }
    }

    return {CreateDeprecatedLegacyPosition((it.AtEnd() ? range : it.Range()).End), Affinity::Upstream};
  }

  // Determines whether a node is inside a range or visibly starts and ends at the boundaries of the range.
  // Call this function to determine whether a node is visibly fit inside selectedRange
  bool IsNodeVisiblyContainedWithin(Node &node, const dom::SimpleRange &range) noexcept
  {
    if (dom::Contains<dom::TreeType::ComposedTree>(range, node))
    {
      return true;
    }

    auto startPosition = CreateDeprecatedLegacyPosition(range.Start);
    auto endPosition = CreateDeprecatedLegacyPosition(range.End);

    bool startIsVisuallySame = VisiblePositionBeforeNode(node) == VisiblePosition(startPosition);
    if (startIsVisuallySame && PositionInParentAfterNode(&node) < endPosition)
    {
      return true;
    }

    bool endIsVisuallySame = VisiblePositionAfterNode(node) == VisiblePosition(endPosition);
    if (endIsVisuallySame && startPosition < PositionInParentBeforeNode(&node))
    {
      return true;
    }

    return startIsVisuallySame && endIsVisuallySame;
  }

  bool IsRenderedAsNonInlineTableImageOrHR(const Node *node) noexcept
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
    KRYS_NODISCARD Element *ElementIfBlockFlow(Node &node) noexcept
    {
      auto *element = DynamicDowncast<Element>(node);
      if (!element)
      {
        return nullptr;
      }
      auto *renderer = element->Renderer();
      return renderer && renderer->IsRenderBlockFlow() ? element : nullptr;
    }
  }

  bool IsBlockFlowElement(const Node &node) noexcept
  {
    return ElementIfBlockFlow(const_cast<Node &>(node));
  }

  Element *DeprecatedEnclosingBlockFlowElement(Node *node) noexcept
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
        return &Downcast<Element>(*node);
      }
    }
    return nullptr;
  }

  namespace
  {
    KRYS_NODISCARD inline bool CaretRendersInsideNode(const Node &node) noexcept
    {
      return !IsRenderedTable(&node) && !EditingIgnoresContent(node);
    }
  }

  RenderBlock *RendererForCaretPainting(const Node *node) noexcept
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

  LayoutRect LocalCaretRectInRendererForRect(LayoutRect &localRect, Node *node, RenderObject *renderer,
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

  RectI AbsoluteBoundsForLocalCaretRect(RenderBlock *rendererForCaretPainting, const LayoutRect &rect,
                                        bool *insideFixed) noexcept
  {
    if (insideFixed)
    {
      *insideFixed = false;
    }

    if (!rendererForCaretPainting || rect.IsEmpty())
    {
      return RectI();
    }

    LayoutRect localRect(rect);
    rendererForCaretPainting->FlipForWritingMode(localRect);
    return rendererForCaretPainting
      ->LocalToAbsoluteQuad(QuadF(RectF(localRect)), MapCoordinatesMode::UseTransforms, insideFixed)
      .EnclosingBoundingBox();
  }

  PositionRange PositionsForRange(const dom::SimpleRange &range) noexcept
  {
    return {CreateContainerOffsetPosition(range.Start).Downstream(),
            CreateContainerOffsetPosition(range.End).Upstream()};
  }

#pragma region Elements editing makes

  bool IsHTMLElementOfType(const Node *node, html::TagName tagName) noexcept
  {
    auto *element = DynamicDowncast<HTMLElement>(node);
    return element && element->TagQualifiedName().TagName() == tagName;
  }

  Ref<Element> CreateHTMLElement(Document &document, DOMStringView localName) noexcept
  {
    auto element = document.CreateElementNS(infra::namespaces::HTML, DOMStringAtom(localName));
    krys_debug_assert(!element.HasException());
    return element.ReleaseValue();
  }

  Ref<Element> CreateDefaultParagraphElement(Document &document) noexcept
  {
    switch (document.GetEditor().DefaultParagraphSeparator())
    {
      case EditorParagraphSeparator::div: return CreateHTMLElement(document, u8"div");
      case EditorParagraphSeparator::p:   break;
    }
    return CreateHTMLElement(document, u8"p");
  }

  namespace
  {
    KRYS_NODISCARD Ref<Element> CreateTabSpanElement(Document &document, Text &tabTextNode) noexcept
    {
      auto spanElement = CreateHTMLElement(document, u8"span");

      (void)spanElement->SetAttribute(u8"class", DOMString(AppleTabSpanClass));
      (void)spanElement->SetAttribute(u8"style", u8"white-space:pre");

      (void)spanElement->AppendChild(tabTextNode);

      return spanElement;
    }

    KRYS_NODISCARD bool HasARenderedDescendant(Node *node, Node *excludedNode) noexcept
    {
      for (auto *n = node->FirstChild(); n;)
      {
        if (n == excludedNode)
        {
          n = dom::TreeQueries::NextSkippingChildren(*n, node);
          continue;
        }
        if (n->Renderer())
        {
          return true;
        }
        n = dom::TreeQueries::Next(*n, node);
      }
      return false;
    }
  }

  Ref<Element> CreateTabSpanElement(Document &document, utf8_string tabText) noexcept
  {
    return CreateTabSpanElement(document, *document.CreateTextNode(krys::move(tabText)));
  }

  // FIXME: An editing text node (createEditingTextNode, which echoes passwords).
  Ref<Element> CreateTabSpanElement(Document &document) noexcept
  {
    return CreateTabSpanElement(document, *document.CreateTextNode(u8"\t"));
  }

  namespace
  {
    KRYS_NODISCARD bool HasEquivalentAttributes(const Element &first, const Element &second) noexcept
    {
      const auto &firstAttributes = first.AttributeList();
      const auto &secondAttributes = second.AttributeList();
      if (firstAttributes.size() != secondAttributes.size())
      {
        return false;
      }
      for (const auto &attribute : firstAttributes)
      {
        auto otherValue = second.GetAttributeNS(attribute->NamespaceURI(), attribute->LocalName());
        if (!otherValue || *otherValue != attribute->Value())
        {
          return false;
        }
      }
      return true;
    }
  }

  Ref<Element> CloneElementWithoutChildren(Element &element) noexcept
  {
    auto clone = dom::NodeAlgorithms::CloneNode(element, &element.NodeDocument(), false);
    krys_debug_assert(!clone.HasException());
    auto node = clone.ReleaseValue();
    return Downcast<Element>(krys::move(node));
  }

  void CloneDataFromElement(Element &target, const Element &source) noexcept
  {
    source.SynchronizeAllAttributes();
    for (const auto &attribute : source.AttributeList())
    {
      (void)target.SetAttributeNS(attribute->NamespaceURI(), DOMStringAtom(attribute->Name()),
                                  DOMString(attribute->Value()));
    }
  }

  Element *TabSpanNode(Node *node) noexcept
  {
    if (IsHTMLElementOfType(node, html::TagName::span))
    {
      auto &span = Downcast<Element>(*node);
      if (span.GetAttribute(u8"class") == DOMString(AppleTabSpanClass))
      {
        return &span;
      }
    }
    return nullptr;
  }

  Element *ParentTabSpanNode(Node *node) noexcept
  {
    return Is<Text>(node) ? TabSpanNode(node->ParentNode()) : nullptr;
  }

  RefPtr<Element> EnclosingList(Node *node) noexcept
  {
    if (!node)
    {
      return nullptr;
    }

    auto root = HighestEditableRoot(FirstPositionInOrBeforeNode(node));

    for (auto *ancestor = node->ParentNode(); ancestor; ancestor = ancestor->ParentNode())
    {
      if (IsHTMLElementOfType(ancestor, html::TagName::ul)
          || IsHTMLElementOfType(ancestor, html::TagName::ol))
      {
        return ShareRefPtr(&Downcast<Element>(*ancestor));
      }
      if (ancestor == root)
      {
        return nullptr;
      }
    }

    return nullptr;
  }

  RefPtr<Node> EnclosingListChild(Node *node) noexcept
  {
    if (!node)
    {
      return nullptr;
    }

    // Check for a list item element, or for a node whose parent is a list element. Such a node
    // will appear visually as a list item (but without a list marker)
    auto root = HighestEditableRoot(FirstPositionInOrBeforeNode(node));

    // FIXME: This function is inappropriately named since it starts with node instead of node->ParentNode()
    for (auto *n = node; n && n->ParentNode(); n = n->ParentNode())
    {
      if (IsHTMLElementOfType(n, html::TagName::li) || (IsListHTMLElement(n->ParentNode()) && n != root))
      {
        return ShareRefPtr(n);
      }
      if (n == root || IsTableCell(*n))
      {
        return nullptr;
      }
    }

    return nullptr;
  }

  RefPtr<Node> EnclosingEmptyListItem(const VisiblePosition &position) noexcept
  {
    // Check that position is on a line by itself inside a list item
    RefPtr<Node> listChildNode = EnclosingListChild(position.DeepEquivalent().DeprecatedNode());
    if (!listChildNode || !IsStartOfParagraph(position) || !IsEndOfParagraph(position))
    {
      return nullptr;
    }

    VisiblePosition firstInListChild(FirstPositionInOrBeforeNode(listChildNode.get()));
    VisiblePosition lastInListChild(LastPositionInOrAfterNode(listChildNode.get()));

    if (firstInListChild != position || lastInListChild != position)
    {
      return nullptr;
    }

    return listChildNode;
  }

  RefPtr<Element> OutermostEnclosingList(Node *node, Node *rootList) noexcept
  {
    RefPtr<Element> list = EnclosingList(node);
    if (!list)
    {
      return nullptr;
    }

    while (RefPtr<Element> nextList = EnclosingList(list.get()))
    {
      if (nextList.get() == rootList)
      {
        break;
      }
      list = krys::move(nextList);
    }

    return list;
  }

  bool CanMergeLists(Element *firstList, Element *secondList) noexcept
  {
    auto *first = DynamicDowncast<HTMLElement>(firstList);
    auto *second = DynamicDowncast<HTMLElement>(secondList);
    if (!first || !second)
    {
      return false;
    }

    return first->LocalName() == second->LocalName() // make sure the list types match (ol vs. ul)
           && first->HasEditableStyle() && second->HasEditableStyle()       // both lists are editable
           && first->RootEditableElement() == second->RootEditableElement() // don't cross editing boundaries
           // Make sure there is no visible content between this li and the previous list.
           && IsVisiblyAdjacent(PositionInParentAfterNode(first), PositionInParentBeforeNode(second));
  }

  Element *ElementIfEquivalent(const Element &first, Node &second) noexcept
  {
    auto *secondElement = DynamicDowncast<Element>(second);
    if (secondElement && first.TagQualifiedName() == secondElement->TagQualifiedName()
        && HasEquivalentAttributes(first, *secondElement))
    {
      return secondElement;
    }
    return nullptr;
  }

  Element *EnclosingElementWithTag(const Position &position, html::TagName tagName) noexcept
  {
    auto root = HighestEditableRoot(position);
    for (auto *node = position.DeprecatedNode(); node; node = node->ParentNode())
    {
      if (root && !node->HasEditableStyle())
      {
        continue;
      }
      auto *element = DynamicDowncast<Element>(*node);
      if (!element)
      {
        continue;
      }
      if (IsHTMLElementOfType(element, tagName))
      {
        return element;
      }
      if (node == root)
      {
        return nullptr;
      }
    }
    return nullptr;
  }

  bool IsNonTableCellHTMLBlockElement(const Node *node) noexcept
  {
    using html::TagName;
    for (auto tagName : {TagName::listing, TagName::ol, TagName::pre, TagName::table, TagName::ul,
                         TagName::xmp, TagName::h1, TagName::h2, TagName::h3, TagName::h4, TagName::h5})
    {
      if (IsHTMLElementOfType(node, tagName))
      {
        return true;
      }
    }
    return false;
  }

  Position AdjustedSelectionStartForStyleComputation(const VisibleSelection &selection) noexcept
  {
    // This function is used by range style computations to avoid bugs like:
    // you can only bold/unbold a selection starting from end of line once
    // It is important to skip certain irrelevant content at the start of the selection, so we do not wind up
    // with a spurious "mixed" style.

    auto visiblePosition = selection.VisibleStart();
    if (visiblePosition.IsNull())
    {
      return {};
    }

    // if the selection is a caret, just return the position, since the style
    // behind us is relevant
    if (selection.IsCaret())
    {
      return visiblePosition.DeepEquivalent();
    }

    // if the selection starts just before a paragraph break, skip over it
    if (IsEndOfParagraph(visiblePosition))
    {
      return visiblePosition.Next().DeepEquivalent().Downstream();
    }

    // otherwise, make sure to be at the start of the first selected node,
    // instead of possibly at the end of the last node before the selection
    return visiblePosition.DeepEquivalent().Downstream();
  }

  RefPtr<Node> HighestNodeToRemoveInPruning(Node *node) noexcept
  {
    RefPtr<Node> previousNode;
    auto *rootEditableElement = node ? node->RootEditableElement() : nullptr;
    for (RefPtr<Node> currentNode = ShareRefPtr(node); currentNode;
         currentNode = ShareRefPtr<Node>(currentNode->ParentNode()))
    {
      if (auto *renderer = currentNode->Renderer())
      {
        if (!renderer->CanHaveChildren() || HasARenderedDescendant(currentNode.get(), previousNode.get())
            || rootEditableElement == currentNode.get())
        {
          return previousNode;
        }
      }
      previousNode = currentNode;
    }
    return nullptr;
  }

  // Whitespace is ASCII, so it's found byte by byte; a no-break space is two bytes in UTF-8, so a rebalanced
  // string can be longer than the string it rebalances.
  utf8_string StringWithRebalancedWhitespace(utf8_stringview string, bool startIsStartOfParagraph,
                                             bool shouldEmitNBSPbeforeEnd) noexcept
  {
    constexpr utf8_stringview noBreakSpace = u8" ";
    auto isEditingWhitespaceAt = [&](size_t i, size_t &length)
    {
      if (string.substr(i).starts_with(noBreakSpace))
      {
        length = noBreakSpace.size();
        return true;
      }
      length = 1;
      return DeprecatedIsEditingWhitespace(string[i]);
    };

    utf8_string rebalancedString;
    bool changed = false;
    bool previousCharacterWasSpace = false;
    for (size_t i = 0; i < string.size();)
    {
      size_t length = 1;
      if (!isEditingWhitespaceAt(i, length))
      {
        previousCharacterWasSpace = false;
        rebalancedString.append(string.substr(i, length));
        i += length;
        continue;
      }
      bool isLast = i + length == string.size();
      utf8_stringview selectedWhitespaceCharacter;
      // We need to ensure there is no next sibling text node.
      if (previousCharacterWasSpace || (!i && startIsStartOfParagraph) || (isLast && shouldEmitNBSPbeforeEnd))
      {
        selectedWhitespaceCharacter = noBreakSpace;
        previousCharacterWasSpace = false;
      }
      else
      {
        selectedWhitespaceCharacter = u8" ";
        previousCharacterWasSpace = true;
      }
      if (string.substr(i, length) != selectedWhitespaceCharacter)
      {
        changed = true;
      }
      rebalancedString.append(selectedWhitespaceCharacter);
      i += length;
    }

    return changed ? rebalancedString : utf8_string(string);
  }

  utf8_stringview NonBreakingSpaceString() noexcept
  {
    return u8" ";
  }

  bool IsVisiblyAdjacent(const Position &first, const Position &second) noexcept
  {
    return VisiblePosition(first) == VisiblePosition(second.Upstream());
  }

#pragma endregion
}
