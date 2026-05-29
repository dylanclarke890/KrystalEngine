#include "Krystal.HTML/DOM/Range.hpp"
#include "Krystal.HTML/DOM/Algorithms/LiveRangeUpdater.hpp"
#include "Krystal.HTML/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/DOM/Algorithms/TreeTraversal.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/CharacterData.hpp"
#include "Krystal.HTML/DOM/Comment.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/DOM/DocumentType.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/DOM/ProcessingInstruction.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/DOM/Text.hpp"

namespace Krys::HTML
{
  Range::Range(BoundaryPoint start, BoundaryPoint end) noexcept : AbstractRange(start, end)
  {
    LiveRangeUpdater::Created(*this);
  }

  Range::~Range() noexcept
  {
    LiveRangeUpdater::Destroyed(*this);
  }

  RawPtr<Node> Range::CommonAncestorContainer() const noexcept
  {
    if (!TreeQueries::SameRoot(*_start.Container, *_end.Container))
    {
      return nullptr;
    }

    return TreeQueries::CommonAncestor(*_start.Container, *_end.Container);
  }

  ExceptionOr<void> Range::SetStart(Node &node, uint64 offset) noexcept
  {
    return SetStartBoundaryPoint(node, offset);
  }

  ExceptionOr<void> Range::SetEnd(Node &node, uint64 offset) noexcept
  {
    return SetEndBoundaryPoint(node, offset);
  }

  ExceptionOr<void> Range::SetStartBefore(Node &node) noexcept
  {
    RawPtr<Node> parent = node.ParentNode();
    if (parent == nullptr)
    {
      return Exception(ExceptionCode::InvalidNodeTypeError);
    }

    if (auto result = SetStartBoundaryPoint(*parent, TreeQueries::Index(node)); result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

  ExceptionOr<void> Range::SetStartAfter(Node &node) noexcept
  {
    RawPtr<Node> parent = node.ParentNode();
    if (parent == nullptr)
    {
      return Exception(ExceptionCode::InvalidNodeTypeError);
    }

    if (auto result = SetStartBoundaryPoint(*parent, TreeQueries::Index(node) + 1uz); result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

  ExceptionOr<void> Range::SetEndBefore(Node &node) noexcept
  {
    RawPtr<Node> parent = node.ParentNode();
    if (parent == nullptr)
    {
      return Exception(ExceptionCode::InvalidNodeTypeError);
    }

    if (auto result = SetEndBoundaryPoint(*parent, TreeQueries::Index(node)); result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

  ExceptionOr<void> Range::SetEndAfter(Node &node) noexcept
  {
    RawPtr<Node> parent = node.ParentNode();
    if (parent == nullptr)
    {
      return Exception(ExceptionCode::InvalidNodeTypeError);
    }

    if (auto result = SetEndBoundaryPoint(*parent, TreeQueries::Index(node) + 1uz); result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

  void Range::Collapse(bool toStart) noexcept
  {
    if (toStart)
    {
      _end = _start;
    }
    else
    {
      _start = _end;
    }
  }

  ExceptionOr<void> Range::SelectNode(Node &node) noexcept
  {
    RawPtr<Node> parent = node.ParentNode();
    if (parent == nullptr)
    {
      return Exception(ExceptionCode::InvalidNodeTypeError);
    }

    auto index = TreeQueries::Index(node);
    if (auto result = SetStartBoundaryPoint(*parent, index); result.HasException())
    {
      return result.ReleaseException();
    }

    if (auto result = SetEndBoundaryPoint(*parent, index + 1uz); result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

  ExceptionOr<void> Range::SelectNodeContents(Node &node) noexcept
  {
    if (Is<DocumentType>(node))
    {
      return Exception(ExceptionCode::InvalidNodeTypeError);
    }

    if (auto result = SetStartBoundaryPoint(node, 0); result.HasException())
    {
      return result.ReleaseException();
    }

    if (auto result = SetEndBoundaryPoint(node, TreeQueries::Length(node)); result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

  ExceptionOr<std::strong_ordering> Range::CompareBoundaryPoints(BoundaryPointComparator how,
                                                                 const Range &other) const noexcept
  {
    if (!TreeQueries::SameRoot(*_start.Container, *other._start.Container))
    {
      return Exception(ExceptionCode::WrongDocumentError);
    }

    switch (how)
    {
      case BoundaryPointComparator::StartToStart: return _start.ComparePositionTo(other._start);
      case BoundaryPointComparator::StartToEnd:   return _start.ComparePositionTo(other._end);
      case BoundaryPointComparator::EndToEnd:     return _end.ComparePositionTo(other._end);
      case BoundaryPointComparator::EndToStart:   return _end.ComparePositionTo(other._start);
      default:                                    return Exception(ExceptionCode::NotSupportedError);
    }
  }

  ExceptionOr<void> Range::DeleteContents() noexcept
  {
    if (IsCollapsed())
    {
      return {};
    }

    auto originalStartNode = _start.Container.get();
    auto originalStartOffset = _start.Offset;
    auto originalEndNode = _end.Container.get();
    auto originalEndOffset = _end.Offset;

    if (originalStartNode == originalEndNode)
    {
      if (auto *characterData = DynamicDowncast<CharacterData>(originalStartNode))
      {
        auto deleteData =
          characterData->DeleteData(originalStartOffset, originalEndOffset - originalStartOffset);
        if (deleteData.HasException())
        {
          return deleteData.ReleaseException();
        }
        return {};
      }
    }

    SmallNodeList nodesToRemove;

    RawPtr<Node> root = CommonAncestorContainer();
    RawPtr<Node> current = root->FirstChild();
    while (current != nullptr)
    {
      if (IsContained(*current))
      {
        nodesToRemove.emplace_back(ShareRef(*current));
        current = TreeTraversal::NextSkippingChildren(*current, root);
      }
      else
      {
        current = TreeTraversal::Next(*current, root);
      }
    }

    RawPtr<Node> newNode = nullptr;
    size_t newOffset = 0uz;

    if (TreeQueries::IsInclusiveAncestor(*originalStartNode, *originalEndNode))
    {
      newNode = originalStartNode;
      newOffset = originalStartOffset;
    }
    else
    {
      RawPtr<Node> refNode = originalStartNode;
      while (refNode->ParentNode() && !TreeQueries::IsInclusiveAncestor(*refNode, *originalEndNode))
      {
        refNode = refNode->ParentNode();
      }

      newNode = refNode->ParentNode();
      newOffset = TreeQueries::Index(*refNode) + 1uz;
    }

    assert(newNode != nullptr);
    _start = BoundaryPoint {ShareRef(*newNode), newOffset};
    _end = _start;

    if (auto *characterData = DynamicDowncast<CharacterData>(originalStartNode))
    {
      auto length = TreeQueries::Length(*characterData);
      auto offset = originalStartOffset;

      if (auto deleteData = characterData->DeleteData(offset, length - offset); deleteData.HasException())
      {
        return deleteData.ReleaseException();
      }
    }

    for (auto &node : nodesToRemove)
    {
      if (auto parent = node->ParentNode())
      {
        if (auto remove = parent->RemoveChild(*node); remove.HasException())
        {
          return remove.ReleaseException();
        }
      }
    }

    if (auto *characterData = DynamicDowncast<CharacterData>(originalEndNode))
    {
      if (auto deleteData = characterData->DeleteData(0uz, originalEndOffset); deleteData.HasException())
      {
        return deleteData.ReleaseException();
      }
    }

    return {};
  }

  ExceptionOr<Ref<DocumentFragment>> Range::ExtractContents() noexcept
  {
    auto fragment = CreateRef<DocumentFragment>(_start.Container->NodeDocument());

    if (IsCollapsed())
    {
      return fragment;
    }

    auto *originalStartNode = _start.Container.get();
    auto originalStartOffset = _start.Offset;
    auto *originalEndNode = _end.Container.get();
    auto originalEndOffset = _end.Offset;

    if (originalStartNode == originalEndNode)
    {
      if (auto *characterData = DynamicDowncast<CharacterData>(originalStartNode))
      {
        auto cloneResult =
          CloneCharacterDataContents(*fragment, *originalStartNode, originalStartOffset,
                                     originalEndOffset - originalStartOffset, DeleteClonedContent(true));

        if (cloneResult.HasException())
        {
          return cloneResult.ReleaseException();
        }

        return fragment;
      }
    }

    auto *commonAncestor = TreeQueries::CommonAncestor(*originalStartNode, *originalEndNode);

    auto *firstPartiallyContainedChild =
      FirstPartiallyContainedChild(*commonAncestor, *originalStartNode, *originalEndNode);
    auto *lastPartiallyContainedChild =
      LastPartiallyContainedChild(*commonAncestor, *originalStartNode, *originalEndNode);

    auto containedChildrenResult = GetContainedChildren(commonAncestor);
    if (containedChildrenResult.HasException())
    {
      return containedChildrenResult.ReleaseException();
    }
    SmallNodeList containedChildren = containedChildrenResult.ReleaseValue();

    RawPtr<Node> newNode = nullptr;
    size_t newOffset = 0uz;

    if (TreeQueries::IsInclusiveAncestor(*originalStartNode, *originalEndNode))
    {
      newNode = originalStartNode;
      newOffset = originalStartOffset;
    }
    else
    {
      RawPtr<Node> referenceNode = originalStartNode;
      while (referenceNode->ParentNode() != nullptr
             && !TreeQueries::IsInclusiveAncestor(*referenceNode, *originalEndNode))
      {
        referenceNode = referenceNode->ParentNode();
      }
      newNode = referenceNode->ParentNode();
      newOffset = TreeQueries::Index(*referenceNode) + 1uz;
    }

    assert(newNode != nullptr);
    _start = BoundaryPoint {ShareRef(*newNode), newOffset};
    _end = _start;

    if (Is<CharacterData>(firstPartiallyContainedChild))
    {
      auto length = TreeQueries::Length(*originalStartNode) - originalStartOffset;
      auto cloneResult = CloneCharacterDataContents(*fragment, *originalStartNode, originalStartOffset,
                                                    length, DeleteClonedContent(true));
      if (cloneResult.HasException())
      {
        return cloneResult.ReleaseException();
      }
    }
    else if (firstPartiallyContainedChild != nullptr)
    {
      auto cloneResult = NodeAlgorithms::CloneNode(*firstPartiallyContainedChild);
      if (cloneResult.HasException())
      {
        return cloneResult.ReleaseException();
      }

      if (auto appendResult = fragment->AppendChild(*cloneResult.Value()); appendResult.HasException())
      {
        return appendResult.ReleaseException();
      }

      auto subrange = Range {BoundaryPoint {ShareRef(*originalStartNode), originalStartOffset},
                             BoundaryPoint {ShareRef(*firstPartiallyContainedChild),
                                            TreeQueries::Length(*firstPartiallyContainedChild)}};

      auto subFragment = subrange.ExtractContents();
      if (subFragment.HasException())
      {
        return subFragment.ReleaseException();
      }

      if (auto appendSubFragmentResult = cloneResult.Value()->AppendChild(*subFragment.Value());
          appendSubFragmentResult.HasException())
      {
        return appendSubFragmentResult.ReleaseException();
      }
    }

    for (auto &contained : containedChildren)
    {
      auto appendResult = fragment->AppendChild(*contained);
      if (appendResult.HasException())
      {
        return appendResult.ReleaseException();
      }
    }

    if (Is<CharacterData>(lastPartiallyContainedChild))
    {
      auto cloneResult = CloneCharacterDataContents(*fragment, *originalEndNode, 0uz, originalEndOffset,
                                                    DeleteClonedContent(true));
      if (cloneResult.HasException())
      {
        return cloneResult.ReleaseException();
      }
    }
    else if (lastPartiallyContainedChild != nullptr)
    {
      auto cloneResult = NodeAlgorithms::CloneNode(*lastPartiallyContainedChild);
      if (cloneResult.HasException())
      {
        return cloneResult.ReleaseException();
      }

      if (auto appendResult = fragment->AppendChild(*cloneResult.Value()); appendResult.HasException())
      {
        return appendResult.ReleaseException();
      }

      auto subrange = Range {BoundaryPoint {ShareRef(*lastPartiallyContainedChild), 0uz},
                             BoundaryPoint {ShareRef(*originalEndNode), originalEndOffset}};

      auto subFragment = subrange.ExtractContents();
      if (subFragment.HasException())
      {
        return subFragment.ReleaseException();
      }

      if (auto appendSubFragmentResult = cloneResult.Value()->AppendChild(*subFragment.Value());
          appendSubFragmentResult.HasException())
      {
        return appendSubFragmentResult.ReleaseException();
      }
    }

    return fragment;
  }

  ExceptionOr<Ref<DocumentFragment>> Range::CloneContents() const noexcept
  {
    auto fragment = CreateRef<DocumentFragment>(_start.Container->NodeDocument());

    if (IsCollapsed())
    {
      return fragment;
    }

    auto *originalStartNode = _start.Container.get();
    auto originalStartOffset = _start.Offset;
    auto *originalEndNode = _end.Container.get();
    auto originalEndOffset = _end.Offset;

    if (originalStartNode == originalEndNode)
    {
      if (auto *characterData = DynamicDowncast<CharacterData>(originalStartNode))
      {
        auto clone =
          CloneCharacterDataContents(*fragment, *originalStartNode, originalStartOffset,
                                     originalEndOffset - originalStartOffset, DeleteClonedContent(false));

        if (clone.HasException())
        {
          return clone.ReleaseException();
        }

        return fragment;
      }
    }

    auto *commonAncestor = TreeQueries::CommonAncestor(*originalStartNode, *originalEndNode);
    auto *firstPartiallyContainedChild =
      FirstPartiallyContainedChild(*commonAncestor, *originalStartNode, *originalEndNode);
    auto *lastPartiallyContainedChild =
      LastPartiallyContainedChild(*commonAncestor, *originalStartNode, *originalEndNode);

    auto containedChildren = GetContainedChildren(commonAncestor);
    if (containedChildren.HasException())
    {
      return containedChildren.ReleaseException();
    }

    if (Is<CharacterData>(firstPartiallyContainedChild))
    {
      auto length = TreeQueries::Length(*originalStartNode) - originalStartOffset;
      auto clone = CloneCharacterDataContents(*fragment, *originalStartNode, originalStartOffset, length,
                                              DeleteClonedContent(false));
      if (clone.HasException())
      {
        return clone.ReleaseException();
      }
    }
    else if (firstPartiallyContainedChild != nullptr)
    {
      auto clone = NodeAlgorithms::CloneNode(*firstPartiallyContainedChild);
      if (clone.HasException())
      {
        return clone.ReleaseException();
      }

      if (auto append = fragment->AppendChild(*clone.Value()); append.HasException())
      {
        return append.ReleaseException();
      }

      auto subrange = Range {BoundaryPoint {ShareRef(*originalStartNode), originalStartOffset},
                             BoundaryPoint {ShareRef(*firstPartiallyContainedChild),
                                            TreeQueries::Length(*firstPartiallyContainedChild)}};

      auto subFragment = subrange.CloneContents();
      if (subFragment.HasException())
      {
        return subFragment.ReleaseException();
      }

      if (auto appendSubFragment = clone->AppendChild(*subFragment.Value()); appendSubFragment.HasException())
      {
        return appendSubFragment.ReleaseException();
      }
    }

    for (auto &contained : containedChildren.Value())
    {
      auto clone = NodeAlgorithms::CloneNode(*contained, nullptr, true);
      if (clone.HasException())
      {
        return clone.ReleaseException();
      }

      if (auto append = fragment->AppendChild(*clone.Value()); append.HasException())
      {
        return append.ReleaseException();
      }
    }

    if (Is<CharacterData>(lastPartiallyContainedChild))
    {
      auto clone = CloneCharacterDataContents(*fragment, *originalEndNode, 0uz, originalEndOffset,
                                              DeleteClonedContent(false));
      if (clone.HasException())
      {
        return clone.ReleaseException();
      }
    }
    else if (lastPartiallyContainedChild != nullptr)
    {
      auto clone = NodeAlgorithms::CloneNode(*lastPartiallyContainedChild);
      if (clone.HasException())
      {
        return clone.ReleaseException();
      }

      if (auto append = fragment->AppendChild(*clone.Value()); append.HasException())
      {
        return append.ReleaseException();
      }

      auto subrange = Range {BoundaryPoint {ShareRef(*lastPartiallyContainedChild), 0uz},
                             BoundaryPoint {ShareRef(*originalEndNode), originalEndOffset}};

      auto subFragment = subrange.CloneContents();
      if (subFragment.HasException())
      {
        return subFragment.ReleaseException();
      }

      if (auto appendSubFragment = clone->AppendChild(*subFragment.Value()); appendSubFragment.HasException())
      {
        return appendSubFragment.ReleaseException();
      }
    }

    return fragment;
  }

  ExceptionOr<void> Range::InsertNode(Node &node) noexcept
  {
    if (Is<ProcessingInstruction>(_start.Container.get()) || Is<Comment>(_start.Container.get())
        || (Is<Text>(_start.Container.get()) && _start.Container->ParentNode() == nullptr)
        || _start.Container == &node)
    {
      return ExceptionCode::HierarchyRequestError;
    }

    RawPtr<Node> referenceNode;
    if (Is<Text>(_start.Container.get()))
    {
      referenceNode = _start.Container.get();
    }
    else
    {
      referenceNode = TreeQueries::ChildAt(Downcast<ContainerNode>(*_start.Container), _start.Offset);
    }

    RawPtr<ContainerNode> parent = referenceNode == nullptr ? Downcast<ContainerNode>(_start.Container.get())
                                                            : referenceNode->ParentNode();

    if (auto preInsertValid = MutationAlgorithms::EnsurePreInsertValidity(node, *parent, referenceNode);
        preInsertValid.HasException())
    {
      return preInsertValid.ReleaseException();
    }

    if (Is<Text>(_start.Container.get()))
    {
      auto splitResult = Downcast<Text>(_start.Container.get())->SplitText(_start.Offset);
      if (splitResult.HasException())
      {
        return splitResult.ReleaseException();
      }

      referenceNode = splitResult.Value().get();
    }

    if (&node == referenceNode)
    {
      referenceNode = node.NextSibling();
    }

    if (node.ParentNode() != nullptr)
    {
      if (auto remove = node.ParentNode()->RemoveChild(node); remove.HasException())
      {
        return remove.ReleaseException();
      }
    }

    auto newOffset =
      referenceNode == nullptr ? TreeQueries::Length(*parent) : TreeQueries::Index(*referenceNode);
    newOffset += Is<DocumentFragment>(node) ? TreeQueries::Length(node) : 1uz;

    if (auto preInsert = MutationAlgorithms::PreInsert(node, *parent, referenceNode);
        preInsert.HasException())
    {
      return preInsert.ReleaseException();
    }

    if (IsCollapsed())
    {
      _end = BoundaryPoint {ShareRef(*parent), newOffset};
    }

    return {};
  }

  ExceptionOr<void> Range::SurroundContents(ContainerNode &newParent) noexcept
  {
    auto IsPartiallyContainedNonTextNode = [&](const Node &node)
    {
      return !Is<Text>(node) && IsPartiallyContained(node);
    };

    if (std::ranges::any_of(ConstInclusiveAncestorRange(*_start.Container), IsPartiallyContainedNonTextNode))
    {
      return ExceptionCode::InvalidStateError;
    }

    if (std::ranges::any_of(ConstInclusiveAncestorRange(*_end.Container), IsPartiallyContainedNonTextNode))
    {
      return ExceptionCode::InvalidStateError;
    }

    // NOTE: Is<DocumentType>(newParent) isn't needed here as a DocumentType can never be a ContainerNode.
    if (Is<Document>(newParent) || Is<DocumentFragment>(newParent))
    {
      return ExceptionCode::InvalidNodeTypeError;
    }

    auto fragment = ExtractContents();
    if (fragment.HasException())
    {
      return fragment.ReleaseException();
    }

    if (newParent.HasChildNodes())
    {
      if (auto replaceAll = MutationAlgorithms::ReplaceAll(nullptr, newParent); replaceAll.HasException())
      {
        return replaceAll.ReleaseException();
      }
    }

    if (auto insert = InsertNode(newParent); insert.HasException())
    {
      return insert.ReleaseException();
    }

    if (auto append = MutationAlgorithms::Append(*fragment.Value(), newParent); append.HasException())
    {
      return append.ReleaseException();
    }

    if (auto select = SelectNode(newParent); select.HasException())
    {
      return select.ReleaseException();
    }

    return {};
  }

  Ref<Range> Range::CloneRange() const noexcept
  {
    auto start = BoundaryPoint {ShareRef(*StartContainer()), StartOffset()};
    auto end = BoundaryPoint {ShareRef(*EndContainer()), EndOffset()};
    return CreateRef<Range>(start, end);
  }

  void Range::Detach() noexcept
  {
    // legacy method, does nothing.
  }

  ExceptionOr<bool> Range::IsPointInRange(Node &node, uint64 offset) const noexcept
  {
    auto compareResult = ComparePoint(node, offset);
    if (compareResult.HasException())
    {
      return compareResult.ReleaseException();
    }

    return compareResult == std::strong_ordering::equal;
  }

  ExceptionOr<std::strong_ordering> Range::ComparePoint(Node &node, uint64 offset) const noexcept
  {
    if (!TreeQueries::SameRoot(node, *_start.Container))
    {
      return ExceptionCode::WrongDocumentError;
    }

    if (Is<DocumentType>(node))
    {
      return ExceptionCode::InvalidNodeTypeError;
    }

    if (offset > TreeQueries::Length(node))
    {
      return ExceptionCode::IndexSizeError;
    }

    auto boundaryPoint = BoundaryPoint {ShareRef(node), offset};
    if (boundaryPoint.ComparePositionTo(_start) == std::strong_ordering::less)
    {
      return std::strong_ordering::less;
    }

    if (boundaryPoint.ComparePositionTo(_end) == std::strong_ordering::greater)
    {
      return std::strong_ordering::greater;
    }

    return std::strong_ordering::equal;
  }

  bool Range::IntersectsNode(const Node &node) const noexcept
  {
    if (!TreeQueries::SameRoot(node, *_start.Container))
    {
      return false;
    }

    RawPtr<Node> parent = node.ParentNode();
    if (parent == nullptr)
    {
      return true;
    }

    auto offset = TreeQueries::Index(node);

    auto boundaryPoint = BoundaryPoint {ShareRef(*parent), offset};
    if (boundaryPoint.ComparePositionTo(_end) != std::strong_ordering::less)
    {
      return false;
    }

    boundaryPoint.Offset = offset + 1uz;
    if (boundaryPoint.ComparePositionTo(_start) != std::strong_ordering::greater)
    {
      return false;
    }

    return true;
  }

  ExceptionOr<DOMString> Range::ToString() const noexcept
  {
    if (_start.Container == _end.Container)
    {
      if (auto *textNode = DynamicDowncast<Text>(_start.Container.get()))
      {
        return textNode->SubstringData(_start.Offset, _end.Offset - _start.Offset);
      }
    }

    DOMString string;
    if (auto *startTextNode = DynamicDowncast<Text>(_start.Container.get()))
    {
      auto substringResult =
        startTextNode->SubstringData(_start.Offset, TreeQueries::Length(*startTextNode) - _start.Offset);

      if (substringResult.HasException())
      {
        return substringResult.ReleaseException();
      }

      string += substringResult.ReleaseValue();
    }

    auto *root = CommonAncestorContainer();
    for (auto *node = TreeTraversal::Next(*_start.Container, root); node;
         node = TreeTraversal::Next(*node, root))
    {
      if (!Is<Text>(node) || !IsContained(*node))
      {
        continue;
      }

      string += Downcast<Text>(*node).Data();
    }

    if (auto *endTextNode = DynamicDowncast<Text>(_end.Container.get()))
    {
      auto substringResult = endTextNode->SubstringData(0, _end.Offset);
      if (substringResult.HasException())
      {
        return substringResult.ReleaseException();
      }
      string += substringResult.ReleaseValue();
    }

    return string;
  }

  ExceptionOr<void> Range::SetStartBoundaryPoint(Node &node, uint64 offset) noexcept
  {
    if (Is<DocumentType>(node))
    {
      return Exception(ExceptionCode::InvalidNodeTypeError);
    }

    if (offset > TreeQueries::Length(node))
    {
      return Exception(ExceptionCode::IndexSizeError);
    }

    if (&_start.Container->NodeDocument() != &node.NodeDocument())
    {
      LiveRangeUpdater::RootChanged(_start.Container->NodeDocument(), node.NodeDocument(), *this);
    }

    BoundaryPoint boundaryPoint {ShareRef(node), offset};

    if (!TreeQueries::SameRoot(node, *_start.Container)
        || boundaryPoint.ComparePositionTo(_end) == std::strong_ordering::greater)
    {
      _end = boundaryPoint;
    }

    _start = Krys::Move(boundaryPoint);
    return {};
  }

  ExceptionOr<void> Range::SetEndBoundaryPoint(Node &node, uint64 offset) noexcept
  {
    if (Is<DocumentType>(node))
    {
      return Exception(ExceptionCode::InvalidNodeTypeError);
    }

    if (offset > TreeQueries::Length(node))
    {
      return Exception(ExceptionCode::IndexSizeError);
    }

    if (&_start.Container->NodeDocument() != &node.NodeDocument())
    {
      LiveRangeUpdater::RootChanged(_start.Container->NodeDocument(), node.NodeDocument(), *this);
    }

    BoundaryPoint boundaryPoint {ShareRef(node), offset};

    if (!TreeQueries::SameRoot(node, *_start.Container)
        || boundaryPoint.ComparePositionTo(_start) == std::strong_ordering::less)
    {
      _start = boundaryPoint;
    }

    _end = Krys::Move(boundaryPoint);
    return {};
  }

  bool Range::IsContained(Node &node) const noexcept
  {
    auto boundaryPoint = BoundaryPoint {ShareRef(node), 0uz};
    if (boundaryPoint.ComparePositionTo(_start) != std::strong_ordering::greater)
    {
      return false;
    }

    boundaryPoint.Offset = TreeQueries::Length(node);
    if (boundaryPoint.ComparePositionTo(_end) != std::strong_ordering::less)
    {
      return false;
    }

    return true;
  }

  bool Range::IsPartiallyContained(const Node &node) const noexcept
  {
    bool isStartInclusive = TreeQueries::IsInclusiveAncestor(node, *_start.Container);
    bool isEndInclusive = TreeQueries::IsInclusiveAncestor(node, *_end.Container);
    return isStartInclusive ^ isEndInclusive;
  }

  RawPtr<Node> Range::FirstPartiallyContainedChild(Node &commonAncestor, Node &startContainer,
                                                   Node &endContainer) const noexcept
  {
    if (!TreeQueries::IsInclusiveAncestor(startContainer, endContainer))
    {
      for (RawPtr<Node> child = commonAncestor.FirstChild(); child; child = child->NextSibling())
      {
        if (IsPartiallyContained(*child))
        {
          return child;
        }
      }
    }

    return nullptr;
  }

  RawPtr<Node> Range::LastPartiallyContainedChild(Node &commonAncestor, Node &startContainer,
                                                  Node &endContainer) const noexcept
  {
    if (!TreeQueries::IsInclusiveAncestor(endContainer, startContainer))
    {
      for (RawPtr<Node> child = commonAncestor.LastChild(); child; child = child->PreviousSibling())
      {
        if (IsPartiallyContained(*child))
        {
          return child;
        }
      }
    }

    return nullptr;
  }

  ExceptionOr<SmallNodeList> Range::GetContainedChildren(RawPtr<Node> commonAncestor) const noexcept
  {
    SmallNodeList containedChildren;

    for (RawPtr<Node> child = commonAncestor->FirstChild(); child; child = child->NextSibling())
    {
      if (Is<DocumentType>(*child))
      {
        return ExceptionCode::HierarchyRequestError;
      }

      if (IsContained(*child))
      {
        containedChildren.emplace_back(ShareRef(*child));
      }
    }

    return containedChildren;
  }

  ExceptionOr<void> Range::CloneCharacterDataContents(DocumentFragment &fragment, Node &node, size_t offset,
                                                      size_t length,
                                                      DeleteClonedContent deleteClonedContent) const noexcept
  {
    CharacterData &characterData = Downcast<CharacterData>(node);

    auto cloneResult = NodeAlgorithms::CloneNode(characterData);
    if (cloneResult.HasException())
    {
      return cloneResult.ReleaseException();
    }

    auto data = characterData.SubstringData(offset, length);
    if (data.HasException())
    {
      return data.ReleaseException();
    }

    auto characterDataClone = Downcast<CharacterData>(cloneResult.Value().get());
    if (auto setDataResult = characterDataClone->Data(data.ReleaseValue()); setDataResult.HasException())
    {
      return setDataResult.ReleaseException();
    }

    if (auto appendResult = fragment.AppendChild(*characterDataClone); appendResult.HasException())
    {
      return appendResult.ReleaseException();
    }

    if (deleteClonedContent)
    {
      if (auto deleteDataResult = characterData.DeleteData(offset, length); deleteDataResult.HasException())
      {
        return deleteDataResult.ReleaseException();
      }
    }

    return {};
  }
}
