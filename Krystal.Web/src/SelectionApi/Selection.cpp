/*
 * Copyright (C) 2007-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2012 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Web/SelectionApi/Selection.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/DocumentType.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Dom/Ranges/Range.hpp"
#include "Krystal.Web/Dom/Ranges/SimpleRange.hpp"
#include "Krystal.Web/Dom/Ranges/StaticRange.hpp"
#include "Krystal.Web/Dom/Traversal/TreeTraversal.hpp"
#include "Krystal.Web/Dom/Trees/TreeQueries.hpp"
#include "Krystal.Web/Editing/Position.hpp"
#include "Krystal.Web/Editing/Selection/FrameSelection.hpp"
#include "Krystal.Web/Editing/TextIterator.hpp"
#include "Krystal.Web/Html/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Web/Infra/StringAlgorithms.hpp"
#include "Krystal.Web/Page/Frame.hpp"
#include <algorithm>

namespace krys::web
{
  namespace
  {
    using infra::StringAlgorithms;

    /// @see https://dom.spec.whatwg.org/#concept-range-bp-set (its first two steps)
    KRYS_NODISCARD ExceptionOr<void> CheckNodeOffsetPair(const Node &node, uint32 offset) noexcept
    {
      if (Is<DocumentType>(node))
      {
        return Exception(ExceptionCode::InvalidNodeTypeError);
      }
      if (offset > dom::TreeQueries::Length(node))
      {
        return Exception(ExceptionCode::IndexSizeError);
      }
      return {};
    }
  }

  Selection::Selection(Document &document) noexcept : _document(CreateWeakPtr(&document))
  {
  }

  web::Frame *Selection::Frame() const noexcept
  {
    auto *document = _document.get();
    return document ? document->Frame() : nullptr;
  }

  Maybe<dom::SimpleRange> Selection::GetRange() const noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return null;
    }

    auto range = frame->Document()->Selection().Selection().Range();
    if (!range || !IsInDocumentTree(*range->Start.Container) || !IsInDocumentTree(*range->End.Container))
    {
      return null;
    }
    
    return range;
  }

  bool Selection::IsInDocumentTree(const Node &node) const noexcept
  {
    return &dom::TreeQueries::Root(node) == _document.get();
  }

  bool Selection::IsInDocumentTree(const Position &position) const noexcept
  {
    auto *node = position.GetContainerNode();
    return node && IsInDocumentTree(*node);
  }

  bool Selection::AreAnchorAndFocusInDocumentTree() const noexcept
  {
    return IsInDocumentTree(AnchorPosition()) && IsInDocumentTree(FocusPosition());
  }

  Position Selection::AnchorPosition() const noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return {};
    }
    return frame->Document()->Selection().Selection().Anchor();
  }

  Position Selection::FocusPosition() const noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return {};
    }
    return frame->Document()->Selection().Selection().Focus();
  }

  RefPtr<Node> Selection::AnchorNode() const noexcept
  {
    auto anchor = AnchorPosition();
    return IsInDocumentTree(anchor) ? ShareRefPtr(anchor.GetContainerNode()) : nullptr;
  }

  uint32 Selection::AnchorOffset() const noexcept
  {
    auto anchor = AnchorPosition();
    return IsInDocumentTree(anchor) ? static_cast<uint32>(anchor.ComputeOffsetInContainerNode()) : 0;
  }

  RefPtr<Node> Selection::FocusNode() const noexcept
  {
    auto focus = FocusPosition();
    return IsInDocumentTree(focus) ? ShareRefPtr(focus.GetContainerNode()) : nullptr;
  }

  uint32 Selection::FocusOffset() const noexcept
  {
    auto focus = FocusPosition();
    return IsInDocumentTree(focus) ? static_cast<uint32>(focus.ComputeOffsetInContainerNode()) : 0;
  }

  bool Selection::IsCollapsed() const noexcept
  {
    // Wherever they are: in a shadow tree too.
    auto anchor = AnchorPosition();
    auto focus = FocusPosition();
    if (anchor.IsNull() || focus.IsNull())
    {
      return anchor.IsNull() && focus.IsNull();
    }
    return anchor.GetContainerNode() == focus.GetContainerNode()
           && anchor.ComputeOffsetInContainerNode() == focus.ComputeOffsetInContainerNode();
  }

  DOMString Selection::Type() const noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return u8"None";
    }

    auto range = frame->Document()->Selection().Selection().Range();
    if (!range || !AreAnchorAndFocusInDocumentTree())
    {
      return u8"None";
    }
    
    if (range->Collapsed())
    {
      return u8"Caret";
    }
    
    return u8"Range";
  }

  DOMString Selection::Direction() const noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return u8"none";
    }
    auto &selection = frame->Document()->Selection().Selection();
    // FIXME: This can return a direction for a caret, which does not make logical sense.
    if (selection.GetDirectionality() == Directionality::None || selection.IsNone())
    {
      return u8"none";
    }
    return selection.IsBaseFirst() ? u8"forward" : u8"backward";
  }

  uint32 Selection::RangeCount() const noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return 0;
    }
    return !frame->Document()->Selection().IsNone() && AreAnchorAndFocusInDocumentTree() ? 1 : 0;
  }

  ExceptionOr<void> Selection::Collapse(Node *node, uint32 offset) noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return {};
    }
    if (!node)
    {
      RemoveAllRanges();
      return {};
    }
    if (auto result = CheckNodeOffsetPair(*node, offset); result.HasException())
    {
      return result.ReleaseException();
    }
    if (!(node->IsConnected() && frame->Document() == &node->NodeDocument())
        && &dom::TreeQueries::Root(*node) != frame->Document())
    {
      return {};
    }
    auto &selection = frame->Document()->Selection();
    selection.DisassociateLiveRange();
    selection.MoveTo(CreateContainerOffsetPosition(ShareRefPtr(node), offset), Affinity::Downstream);
    return {};
  }

  ExceptionOr<void> Selection::CollapseToEnd() noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return {};
    }
    auto &selection = frame->Document()->Selection();
    if (selection.IsNone())
    {
      return Exception(ExceptionCode::InvalidStateError);
    }
    selection.DisassociateLiveRange();
    selection.MoveTo(selection.Selection().UncanonicalizedEnd(), Affinity::Downstream);
    return {};
  }

  ExceptionOr<void> Selection::CollapseToStart() noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return {};
    }
    auto &selection = frame->Document()->Selection();
    if (selection.IsNone())
    {
      return Exception(ExceptionCode::InvalidStateError);
    }
    selection.DisassociateLiveRange();
    selection.MoveTo(selection.Selection().UncanonicalizedStart(), Affinity::Downstream);
    return {};
  }

  void Selection::Empty() noexcept
  {
    RemoveAllRanges();
  }

  ExceptionOr<void> Selection::SetBaseAndExtent(Node &anchorNode, uint32 anchorOffset, Node &focusNode,
                                                uint32 focusOffset) noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return {};
    }
    if (auto result = CheckNodeOffsetPair(anchorNode, anchorOffset); result.HasException())
    {
      return result.ReleaseException();
    }
    if (auto result = CheckNodeOffsetPair(focusNode, focusOffset); result.HasException())
    {
      return result.ReleaseException();
    }
    auto &document = *frame->Document();
    if (!dom::TreeQueries::IsShadowIncludingInclusiveAncestor(document, anchorNode)
        || !dom::TreeQueries::IsShadowIncludingInclusiveAncestor(document, focusNode))
    {
      return {};
    }
    auto &selection = document.Selection();
    selection.DisassociateLiveRange();
    selection.MoveTo(CreateContainerOffsetPosition(ShareRefPtr(&anchorNode), anchorOffset),
                     CreateContainerOffsetPosition(ShareRefPtr(&focusNode), focusOffset),
                     Affinity::Downstream);
    return {};
  }

  ExceptionOr<void> Selection::SetPosition(Node *node, uint32 offset) noexcept
  {
    return Collapse(node, offset);
  }

  void Selection::Modify(DOMStringView alterString, DOMStringView directionString,
                         DOMStringView granularityString) noexcept
  {
    FrameSelection::Alteration alter;
    if (StringAlgorithms::ASCIICaseInsensitiveMatch(alterString, u8"extend"))
    {
      alter = FrameSelection::Alteration::Extend;
    }
    else if (StringAlgorithms::ASCIICaseInsensitiveMatch(alterString, u8"move"))
    {
      alter = FrameSelection::Alteration::Move;
    }
    else
    {
      return;
    }

    SelectionDirection direction;
    if (StringAlgorithms::ASCIICaseInsensitiveMatch(directionString, u8"forward"))
    {
      direction = SelectionDirection::Forward;
    }
    else if (StringAlgorithms::ASCIICaseInsensitiveMatch(directionString, u8"backward"))
    {
      direction = SelectionDirection::Backward;
    }
    else if (StringAlgorithms::ASCIICaseInsensitiveMatch(directionString, u8"left"))
    {
      direction = SelectionDirection::Left;
    }
    else if (StringAlgorithms::ASCIICaseInsensitiveMatch(directionString, u8"right"))
    {
      direction = SelectionDirection::Right;
    }
    else
    {
      return;
    }

    TextGranularity granularity;
    if (StringAlgorithms::ASCIICaseInsensitiveMatch(granularityString, u8"character"))
    {
      granularity = TextGranularity::CharacterGranularity;
    }
    else if (StringAlgorithms::ASCIICaseInsensitiveMatch(granularityString, u8"word"))
    {
      granularity = TextGranularity::WordGranularity;
    }
    else if (StringAlgorithms::ASCIICaseInsensitiveMatch(granularityString, u8"sentence"))
    {
      granularity = TextGranularity::SentenceGranularity;
    }
    else if (StringAlgorithms::ASCIICaseInsensitiveMatch(granularityString, u8"line"))
    {
      granularity = TextGranularity::LineGranularity;
    }
    else if (StringAlgorithms::ASCIICaseInsensitiveMatch(granularityString, u8"paragraph"))
    {
      granularity = TextGranularity::ParagraphGranularity;
    }
    else if (StringAlgorithms::ASCIICaseInsensitiveMatch(granularityString, u8"lineboundary"))
    {
      granularity = TextGranularity::LineBoundary;
    }
    else if (StringAlgorithms::ASCIICaseInsensitiveMatch(granularityString, u8"sentenceboundary"))
    {
      granularity = TextGranularity::SentenceBoundary;
    }
    else if (StringAlgorithms::ASCIICaseInsensitiveMatch(granularityString, u8"paragraphboundary"))
    {
      granularity = TextGranularity::ParagraphBoundary;
    }
    else if (StringAlgorithms::ASCIICaseInsensitiveMatch(granularityString, u8"documentboundary"))
    {
      granularity = TextGranularity::DocumentBoundary;
    }
    else
    {
      return;
    }

    if (auto *frame = Frame())
    {
      (void)frame->Document()->Selection().Modify(alter, direction, granularity);
    }
  }

  ExceptionOr<void> Selection::Extend(Node &node, uint32 offset) noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return {};
    }

    // 1. If the document associated with this is not a shadow-including inclusive ancestor of node, abort
    // these steps.
    if (!dom::TreeQueries::IsShadowIncludingInclusiveAncestor(*frame->Document(), node))
    {
      return {};
    }

    // 2. If this is empty, throw an InvalidStateError exception and abort these steps.
    if (RangeCount() < 1 && !frame->Document()->Selection().IsCaretOrRange())
    {
      return Exception(ExceptionCode::InvalidStateError,
                       u8"extend() requires a Range to be added to the Selection");
    }

    // 3-9. (The frame selection's new focus, keeping its anchor, sets the range and direction.)
    if (auto result = CheckNodeOffsetPair(node, offset); result.HasException())
    {
      return result.ReleaseException();
    }
    auto &selection = frame->Document()->Selection();
    auto newSelection = selection.Selection();
    newSelection.SetFocus(CreateContainerOffsetPosition(ShareRefPtr(&node), offset));
    selection.DisassociateLiveRange();
    selection.SetSelection(newSelection);
    return {};
  }

  ExceptionOr<Ref<dom::Range>> Selection::GetRangeAt(uint32 index) noexcept
  {
    if (index >= RangeCount())
    {
      return Exception(ExceptionCode::IndexSizeError);
    }
    // The associated live range is made on first use, and kept until the selection changes.
    RefPtr<dom::Range> liveRange = Frame()->Document()->Selection().AssociatedLiveRange();
    if (!liveRange)
    {
      return Exception(ExceptionCode::IndexSizeError);
    }
    return ShareRef(*liveRange);
  }

  void Selection::RemoveAllRanges() noexcept
  {
    if (auto *frame = Frame())
    {
      frame->Document()->Selection().Clear();
    }
  }

  void Selection::AddRange(dom::Range &liveRange) noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return;
    }
    // 1. If the root of the range's boundary points are not the document associated with this, abort these
    // steps.
    if (&dom::TreeQueries::Root(*liveRange.StartContainer()) != frame->Document())
    {
      return;
    }

    // 2. If rangeCount is not 0, abort these steps.
    auto &selection = frame->Document()->Selection();
    if (!selection.IsNone())
    {
      return;
    }

    // 3. Set this's range to range by a strong reference (not by making a copy).
    selection.AssociateLiveRange(liveRange);
  }

  ExceptionOr<void> Selection::RemoveRange(dom::Range &liveRange) noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return {};
    }
    if (&liveRange != frame->Document()->Selection().AssociatedLiveRange().get())
    {
      return Exception(ExceptionCode::NotFoundError);
    }
    RemoveAllRanges();
    return {};
  }

  List<Ref<dom::StaticRange>> Selection::GetComposedRanges(const List<Ref<ShadowRoot>> &shadowRoots) noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return {};
    }
    // 1. If this is empty, return an empty array.
    auto range = frame->Document()->Selection().Selection().Range();
    if (!range)
    {
      return {};
    }

    // Whether node's root is a shadow root that isn't a shadow-including inclusive ancestor of any of
    // shadowRoots.
    auto isRescoped = [&](const Node &node)
    {
      auto *root = dom::TreeTraversal::ContainingShadowRoot(node);
      return root
             && std::ranges::none_of(
               shadowRoots, [&](const Ref<ShadowRoot> &shadowRoot)
               { return dom::TreeQueries::IsShadowIncludingInclusiveAncestor(*root, *shadowRoot); });
    };

    // 2. Otherwise, let startNode be start node of the range associated with this, and let startOffset be
    // start offset of the range.
    Ref<Node> startNode = range->Start.Container;
    uint32 startOffset = static_cast<uint32>(range->Start.Offset);
    // 3. While startNode is a node, startNode's root is a shadow root, and startNode's root is not a
    // shadow-including inclusive ancestor of any of options["shadowRoots"], repeat these steps:
    while (isRescoped(*startNode))
    {
      // 3a. Set startOffset to index of startNode's root's host.
      // 3b. Set startNode to startNode's root's host's parent.
      auto *host = dom::TreeTraversal::ShadowHost(*startNode);
      krys_debug_assert(host && host->ParentNode());
      startNode = ShareRef(*host->ParentNode());
      startOffset = static_cast<uint32>(dom::TreeQueries::Index(*host));
    }

    // 4. Let endNode be end node of the range associated with this, and let endOffset be end offset of the
    // range.
    Ref<Node> endNode = range->End.Container;
    uint32 endOffset = static_cast<uint32>(range->End.Offset);
    // 5. While endNode is a node, endNode's root is a shadow root, and endNode's root is not a
    // shadow-including inclusive ancestor of any of options["shadowRoots"], repeat these steps:
    while (isRescoped(*endNode))
    {
      // 5a. Set endOffset to index of endNode's root's host plus 1.
      // 5b. Set endNode to endNode's root's host's parent.
      auto *host = dom::TreeTraversal::ShadowHost(*endNode);
      krys_debug_assert(host && host->ParentNode());
      endNode = ShareRef(*host->ParentNode());
      endOffset = static_cast<uint32>(dom::TreeQueries::Index(*host) + 1);
    }

    // 6. Return an array consisting of new StaticRange whose start node is startNode, start offset is
    // startOffset, end node is endNode, and end offset is endOffset.
    auto init = dom::StaticRangeInit::Create(dom::BoundaryPoint {krys::move(startNode), startOffset},
                                             dom::BoundaryPoint {krys::move(endNode), endOffset});
    krys_debug_assert(!init.HasException());
    return {CreateRef<dom::StaticRange>(init.ReleaseValue())};
  }

  void Selection::DeleteFromDocument() noexcept
  {
    html::CEReactionsScope ceReactions;
    auto *frame = Frame();
    if (!frame)
    {
      return;
    }
    // The associated live range is only ever one whose boundary points are in the document tree.
    if (RefPtr<dom::Range> range = frame->Document()->Selection().AssociatedLiveRange())
    {
      (void)range->DeleteContents();
    }
  }

  bool Selection::ContainsNode(Node &node, bool allowPartial) const noexcept
  {
    if (!IsInDocumentTree(node))
    {
      return false;
    }
    auto range = GetRange();
    return range && (allowPartial ? Intersects(*range, node) : Contains(*range, node));
  }

  ExceptionOr<void> Selection::SelectAllChildren(Node &node) noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return {};
    }

    // 1. If node is a DocumentType, throw an InvalidNodeTypeError exception and abort these steps.
    if (Is<DocumentType>(node))
    {
      return Exception(ExceptionCode::InvalidNodeTypeError);
    }

    // 2. If node's root is not the document associated with this, abort these steps.
    if (&dom::TreeQueries::Root(node) != frame->Document())
    {
      return {};
    }

    // 3. Let newRange be a new range and childCount be the number of children of node.
    // 4. Set newRange's start to (node, 0).
    // 5. Set newRange's end to (node, childCount).
    // 6. Set this's range to newRange.
    // 7. Set this's direction to forwards.
    auto childCount = static_cast<uint32>(dom::TreeQueries::CountChildNodes(node));
    return SetBaseAndExtent(node, 0, node, childCount);
  }

  // FIXME: Copying text that isn't selectable (the needsToCopyUserSelectNoneQuirk), with quirks.
  DOMString Selection::ToString() const noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return {};
    }

    TextIteratorBehaviors options {TextIteratorBehavior::IgnoresUserSelectNone};

    auto range = frame->Document()->Selection().Selection().Range();
    return range ? PlainText(*range, options) : DOMString();
  }
}
