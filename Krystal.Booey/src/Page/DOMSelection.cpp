#include "Krystal.Booey/Page/DOMSelection.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DocumentType.hpp"
#include "Krystal.Booey/DOM/Position.hpp"
#include "Krystal.Booey/DOM/Range.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/DOM/StaticRange.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Editing/TextIterator.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include <algorithm>

namespace krys::boo
{
  namespace
  {
    using infra::StringAlgorithms;

    /// @see https://dom.spec.whatwg.org/#concept-range-bp-set (its first two steps)
    KRYS_NODISCARD dom::ExceptionOr<void> CheckNodeOffsetPair(const dom::Node &node, uint32 offset) noexcept
    {
      if (Is<dom::DocumentType>(node))
      {
        return dom::Exception(dom::ExceptionCode::InvalidNodeTypeError);
      }
      if (offset > dom::TreeQueries::Length(node))
      {
        return dom::Exception(dom::ExceptionCode::IndexSizeError);
      }
      return {};
    }

    KRYS_NODISCARD RefPtr<dom::Node> SelectionShadowAncestor(LocalFrame &frame) noexcept
    {
      auto *node = frame.Document()->Selection().Selection().Base().AnchorNode();
      if (!node || !node->IsInShadowTree())
      {
        return nullptr;
      }
      return ShareRefPtr(AncestorNodeInThisScope(node->NodeDocument(), node));
    }

    KRYS_NODISCARD RefPtr<dom::Range> CreateLiveRangeBeforeShadowHostWithSelection(LocalFrame &frame) noexcept
    {
      if (RefPtr<dom::Node> shadowAncestor = SelectionShadowAncestor(frame))
      {
        auto *parent = shadowAncestor->ParentNode();
        dom::BoundaryPoint point {ShareRef(*parent), shadowAncestor->ComputeNodeIndex()};
        return CreateRef<dom::Range>(point, point);
      }

      return nullptr;
    }
  }

  DOMSelection::DOMSelection(dom::Document &document) noexcept
      : _document(WeakPtr<dom::Document>::WithRef(document))
  {
  }

  LocalFrame *DOMSelection::Frame() const noexcept
  {
    auto *document = _document.get();
    return document ? document->Frame() : nullptr;
  }

  Maybe<dom::SimpleRange> DOMSelection::GetRange() const noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return null;
    }
    auto range = frame->Document()->Selection().Selection().Range();
    if (!range || range->Start.Container->IsInShadowTree())
    {
      return null;
    }
    return range;
  }

  dom::Position DOMSelection::AnchorPosition() const noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return {};
    }
    return frame->Document()->Selection().Selection().Anchor();
  }

  dom::Position DOMSelection::FocusPosition() const noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return {};
    }
    return frame->Document()->Selection().Selection().Focus();
  }

  RefPtr<dom::Node> DOMSelection::AnchorNode() const noexcept
  {
    return ShadowAdjustedNode(AnchorPosition());
  }

  uint32 DOMSelection::AnchorOffset() const noexcept
  {
    return ShadowAdjustedOffset(AnchorPosition());
  }

  RefPtr<dom::Node> DOMSelection::FocusNode() const noexcept
  {
    return ShadowAdjustedNode(FocusPosition());
  }

  uint32 DOMSelection::FocusOffset() const noexcept
  {
    return ShadowAdjustedOffset(FocusPosition());
  }

  bool DOMSelection::IsCollapsed() const noexcept
  {
    if (!Frame())
    {
      return true;
    }
    auto range = GetRange();
    return !range || range->Collapsed();
  }

  dom::DOMString DOMSelection::Type() const noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return u8"None";
    }
    auto range = frame->Document()->Selection().Selection().Range();
    if (!range)
    {
      return u8"None";
    }
    if (range->Collapsed())
    {
      return u8"Caret";
    }
    return u8"Range";
  }

  dom::DOMString DOMSelection::Direction() const noexcept
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

  uint32 DOMSelection::RangeCount() const noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return 0;
    }
    if (frame->Document()->Selection().AssociatedLiveRange())
    {
      return 1;
    }
    if (SelectionShadowAncestor(*frame))
    {
      return 1;
    }
    return 0;
  }

  dom::ExceptionOr<void> DOMSelection::Collapse(dom::Node *node, uint32 offset) noexcept
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
    selection.MoveTo(dom::MakeContainerOffsetPosition(ShareRefPtr(node), offset), Affinity::Downstream);
    return {};
  }

  dom::ExceptionOr<void> DOMSelection::CollapseToEnd() noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return {};
    }
    auto &selection = frame->Document()->Selection();
    if (selection.IsNone())
    {
      return dom::Exception(dom::ExceptionCode::InvalidStateError);
    }
    selection.DisassociateLiveRange();
    selection.MoveTo(selection.Selection().UncanonicalizedEnd(), Affinity::Downstream);
    return {};
  }

  dom::ExceptionOr<void> DOMSelection::CollapseToStart() noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return {};
    }
    auto &selection = frame->Document()->Selection();
    if (selection.IsNone())
    {
      return dom::Exception(dom::ExceptionCode::InvalidStateError);
    }
    selection.DisassociateLiveRange();
    selection.MoveTo(selection.Selection().UncanonicalizedStart(), Affinity::Downstream);
    return {};
  }

  void DOMSelection::Empty() noexcept
  {
    RemoveAllRanges();
  }

  dom::ExceptionOr<void> DOMSelection::SetBaseAndExtent(dom::Node &anchorNode, uint32 anchorOffset,
                                                        dom::Node &focusNode, uint32 focusOffset) noexcept
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
    selection.MoveTo(dom::MakeContainerOffsetPosition(ShareRefPtr(&anchorNode), anchorOffset),
                     dom::MakeContainerOffsetPosition(ShareRefPtr(&focusNode), focusOffset),
                     Affinity::Downstream);
    return {};
  }

  dom::ExceptionOr<void> DOMSelection::SetPosition(dom::Node *node, uint32 offset) noexcept
  {
    return Collapse(node, offset);
  }

  void DOMSelection::Modify(dom::DOMStringView alterString, dom::DOMStringView directionString,
                            dom::DOMStringView granularityString) noexcept
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

  dom::ExceptionOr<void> DOMSelection::Extend(dom::Node &node, uint32 offset) noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return {};
    }

    if (RangeCount() < 1 && !frame->Document()->Selection().IsCaretOrRange())
    {
      return dom::Exception(dom::ExceptionCode::InvalidStateError,
                            u8"extend() requires a Range to be added to the Selection");
    }

    if (!(node.IsConnected() && frame->Document() == &node.NodeDocument())
        && &dom::TreeQueries::Root(node) != frame->Document())
    {
      return {};
    }
    if (auto result = CheckNodeOffsetPair(node, offset); result.HasException())
    {
      return result.ReleaseException();
    }
    auto &selection = frame->Document()->Selection();
    auto newSelection = selection.Selection();
    newSelection.SetExtent(dom::MakeContainerOffsetPosition(ShareRefPtr(&node), offset));
    selection.DisassociateLiveRange();
    selection.SetSelection(newSelection);
    return {};
  }

  dom::ExceptionOr<Ref<dom::Range>> DOMSelection::GetRangeAt(uint32 index) noexcept
  {
    if (index >= RangeCount())
    {
      return dom::Exception(dom::ExceptionCode::IndexSizeError);
    }
    auto &frame = *Frame();
    if (RefPtr<dom::Range> liveRange = frame.Document()->Selection().AssociatedLiveRange())
    {
      return ShareRef(*liveRange);
    }
    return ShareRef(*CreateLiveRangeBeforeShadowHostWithSelection(frame));
  }

  void DOMSelection::RemoveAllRanges() noexcept
  {
    if (auto *frame = Frame())
    {
      frame->Document()->Selection().Clear();
    }
  }

  void DOMSelection::AddRange(dom::Range &liveRange) noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return;
    }
    auto &selection = frame->Document()->Selection();
    if (selection.IsNone())
    {
      selection.AssociateLiveRange(liveRange);
    }
  }

  dom::ExceptionOr<void> DOMSelection::RemoveRange(dom::Range &liveRange) noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return {};
    }
    if (&liveRange != frame->Document()->Selection().AssociatedLiveRange().get())
    {
      return dom::Exception(dom::ExceptionCode::NotFoundError);
    }
    RemoveAllRanges();
    return {};
  }

  List<Ref<dom::StaticRange>>
    DOMSelection::GetComposedRanges(const List<Ref<dom::ShadowRoot>> &shadowRoots) noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return {};
    }
    auto range = frame->Document()->Selection().Selection().Range();
    if (!range)
    {
      return {};
    }

    auto isListed = [&](const dom::ShadowRoot *shadowRoot)
    {
      return std::ranges::any_of(shadowRoots,
                                 [&](const Ref<dom::ShadowRoot> &root) { return root.get() == shadowRoot; });
    };

    Ref<dom::Node> startNode = range->Start.Container;
    uint32 startOffset = static_cast<uint32>(range->Start.Offset);
    while (startNode->IsInShadowTree() && !isListed(dom::TreeQueries::ContainingShadowRoot(*startNode)))
    {
      auto *host = dom::TreeQueries::ShadowHost(*startNode);
      krys_debug_assert(host && host->ParentNode());
      startNode = ShareRef(*host->ParentNode());
      startOffset = host->ComputeNodeIndex();
    }

    Ref<dom::Node> endNode = range->End.Container;
    uint32 endOffset = static_cast<uint32>(range->End.Offset);
    while (endNode->IsInShadowTree() && !isListed(dom::TreeQueries::ContainingShadowRoot(*endNode)))
    {
      auto *host = dom::TreeQueries::ShadowHost(*endNode);
      krys_debug_assert(host && host->ParentNode());
      endNode = ShareRef(*host->ParentNode());
      endOffset = host->ComputeNodeIndex() + 1;
    }

    auto init = dom::StaticRangeInit::Create(dom::BoundaryPoint {krys::move(startNode), startOffset},
                                             dom::BoundaryPoint {krys::move(endNode), endOffset});
    krys_debug_assert(!init.HasException());
    return {CreateRef<dom::StaticRange>(init.ReleaseValue())};
  }

  void DOMSelection::DeleteFromDocument() noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return;
    }
    if (RefPtr<dom::Range> range = frame->Document()->Selection().AssociatedLiveRange())
    {
      (void)range->DeleteContents();
    }
  }

  bool DOMSelection::ContainsNode(dom::Node &node, bool allowPartial) const noexcept
  {
    auto range = GetRange();
    return range && (allowPartial ? Intersects(*range, node) : Contains(*range, node));
  }

  dom::ExceptionOr<void> DOMSelection::SelectAllChildren(dom::Node &node) noexcept
  {
    // This doesn't (and shouldn't) select the characters in a node if passed a text node.
    // Selection API specification seems to have this wrong: https://github.com/w3c/selection-api/issues/125
    return SetBaseAndExtent(node, 0, node, node.CountChildNodes());
  }

  // FIXME: Copying text that isn't selectable (the needsToCopyUserSelectNoneQuirk), with quirks.
  dom::DOMString DOMSelection::ToString() const noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return {};
    }

    TextIteratorBehaviors options {TextIteratorBehavior::IgnoresUserSelectNone};

    auto range = frame->Document()->Selection().Selection().Range();
    return range ? PlainText(*range, options) : dom::DOMString();
  }

  RefPtr<dom::Node> DOMSelection::ShadowAdjustedNode(const dom::Position &position) const noexcept
  {
    if (position.IsNull())
    {
      return nullptr;
    }

    auto *containerNode = position.GetContainerNode();
    auto *adjustedNode = AncestorNodeInThisScope(*Frame()->Document(), containerNode);
    if (!adjustedNode)
    {
      return nullptr;
    }

    if (containerNode == adjustedNode)
    {
      return ShareRefPtr(containerNode);
    }

    return ShareRefPtr(adjustedNode->ParentNode());
  }

  uint32 DOMSelection::ShadowAdjustedOffset(const dom::Position &position) const noexcept
  {
    if (position.IsNull())
    {
      return 0;
    }

    auto *containerNode = position.GetContainerNode();
    auto *adjustedNode = AncestorNodeInThisScope(*Frame()->Document(), containerNode);
    if (!adjustedNode)
    {
      return 0;
    }

    if (containerNode == adjustedNode)
    {
      return static_cast<uint32>(position.ComputeOffsetInContainerNode());
    }

    return adjustedNode->ComputeNodeIndex();
  }
}
