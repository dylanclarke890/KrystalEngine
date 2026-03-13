#include "Krystal.HTML/Ranges/Range.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/CharacterData.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"
#include "Krystal.HTML/Tree/TreeMutationAlgorithms.hpp"
#include "Krystal.HTML/Tree/TreeQueries.hpp"
#include "Krystal.HTML/Tree/TreeTraversal.hpp"

namespace Krys::HTML
{
  Range::Range(BoundaryPoint start, BoundaryPoint end) noexcept : AbstractRange(start, end)
  {
  }

  RawPtr<Node> Range::CommonAncestorContainer() const noexcept
  {
    return TreeQueries::CommonAncestorContainer(*_start.Container, *_end.Container);
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

    SetStartBoundaryPoint(*parent, TreeQueries::Index(node));
    return {};
  }

  ExceptionOr<void> Range::SetStartAfter(Node &node) noexcept
  {
    RawPtr<Node> parent = node.ParentNode();
    if (parent == nullptr)
    {
      return Exception(ExceptionCode::InvalidNodeTypeError);
    }

    SetStartBoundaryPoint(*parent, TreeQueries::Index(node) + 1uz);
    return {};
  }

  ExceptionOr<void> Range::SetEndBefore(Node &node) noexcept
  {
    RawPtr<Node> parent = node.ParentNode();
    if (parent == nullptr)
    {
      return Exception(ExceptionCode::InvalidNodeTypeError);
    }

    SetEndBoundaryPoint(*parent, TreeQueries::Index(node));
    return {};
  }

  ExceptionOr<void> Range::SetEndAfter(Node &node) noexcept
  {
    RawPtr<Node> parent = node.ParentNode();
    if (parent == nullptr)
    {
      return Exception(ExceptionCode::InvalidNodeTypeError);
    }

    SetEndBoundaryPoint(*parent, TreeQueries::Index(node) + 1uz);
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
    SetStartBoundaryPoint(*parent, index);
    SetEndBoundaryPoint(*parent, index + 1uz);

    return {};
  }

  ExceptionOr<void> Range::SelectNodeContents(Node &node) noexcept
  {
    if (node.IsDocumentTypeNode())
    {
      return Exception(ExceptionCode::InvalidNodeTypeError);
    }

    SetStartBoundaryPoint(node, 0);
    SetEndBoundaryPoint(node, node.Length());

    return {};
  }

  ExceptionOr<std::strong_ordering> Range::CompareBoundaryPoints(BoundaryPointComparator how,
                                                                 const Range &other) const noexcept
  {
    if (!TreeQueries::HasSameRoot(*_start.Container, *other._start.Container))
    {
      return Exception(ExceptionCode::WrongDocumentError);
    }

    switch (how)
    {
      case BoundaryPointComparator::StartToStart: return _start.ComparePositionTo(other._start);
      case BoundaryPointComparator::StartToEnd:   return _start.ComparePositionTo(other._end);
      case BoundaryPointComparator::EndToEnd:     return _end.ComparePositionTo(other._end);
      case BoundaryPointComparator::EndToStart:   return _end.ComparePositionTo(other._start);
      default:                                    return Exception(ExceptionCode::InvalidNodeTypeError);
    }
  }

  void Range::DeleteContents() noexcept
  {
    if (IsCollapsed())
    {
      return;
    }

    if (_start.Container.get() == _end.Container.get() && _start.Container->IsCharacterDataNode())
    {
      Downcast<CharacterData>(_start.Container.get())->DeleteData(_start.Offset, _end.Offset - _start.Offset);
      return;
    }

    // TODO(fix): this is probably wrong.
    List<Ref<Node>> nodesToRemove;
    auto *root = CommonAncestorContainer();
    for (auto *node = TreeTraversal::Next(*_start.Container, root); node;
         node = TreeTraversal::Next(*node, root))
    {
      auto startCmp = ComparePoint(*node, 0);
      if (startCmp.HasException() || startCmp.Value() != std::strong_ordering::greater)
        continue;

      auto endCmp = ComparePoint(*node, node->Length());
      if (endCmp.HasException() || endCmp.Value() != std::strong_ordering::less)
        continue;

      nodesToRemove.emplace_back(ShareRef(*node));

      // skip descendants
      node = TreeTraversal::NextSkippingChildren(*node, root);
    }

    RefPtr<Node> newNode;
    size_t newOffset = 0;

    if (TreeQueries::IsInclusiveAncestor(*_start.Container, *_end.Container))
    {
      newNode = _start.Container;
      newOffset = _start.Offset;
    }
    else
    {
      RawPtr<Node> refNode = _start.Container.get();
      while (refNode->ParentNode() && !TreeQueries::IsInclusiveAncestor(*refNode, *_end.Container))
      {
        refNode = refNode->ParentNode();
      }

      newNode = ShareRefPtr(refNode->ParentNode());
      newOffset = TreeQueries::Index(*refNode) + 1uz;
    }

    if (_start.Container->IsCharacterDataNode())
    {
      RawPtr<CharacterData> characterData = Downcast<CharacterData>(_start.Container.get());
      auto length = characterData->Length();
      characterData->DeleteData(_start.Offset, length - _start.Offset);
    }

    for (auto &node : nodesToRemove)
    {
      if (auto parent = ShareRefPtr(node->ParentNode()))
      {
        parent->RemoveChild(*node);
      }
    }

    if (_end.Container->IsCharacterDataNode())
    {
      auto *characterData = Downcast<CharacterData>(_end.Container.get());
      characterData->DeleteData(0uz, _end.Offset);
    }

    _start = BoundaryPoint {Krys::Move(newNode), newOffset};
    _end = _start;
  }

  ExceptionOr<Ref<DocumentFragment>> Range::ExtractContents() noexcept
  {
    auto fragment = CreateRef<DocumentFragment>(*_start.Container->OwnerDocument());
    if (IsCollapsed())
    {
      return fragment;
    }

    if (_start.Container.get() == _end.Container.get() && _start.Container->IsCharacterDataNode())
    {
      auto cloneResult = CloneCharacterDataContents(*fragment, *_start.Container, _start.Offset,
                                                    _end.Offset - _start.Offset, DeleteClonedContent(true));
      if (cloneResult.HasException())
      {
        return cloneResult.ReleaseException();
      }

      return fragment;
    }

    RawPtr<Node> commonAncestor = CommonAncestorContainer();
    RawPtr<Node> firstPartiallyContainedChild = GetFirstPartiallyContainedChild(commonAncestor);
    RawPtr<Node> lastPartiallyContainedChild = GetLastPartiallyContainedChild(commonAncestor);

    List<Ref<Node>> containedChildren = GetContainedChildren(commonAncestor);
    for (auto &contained : containedChildren)
    {
      if (contained->IsDocumentTypeNode())
      {
        return Exception(ExceptionCode::HierarchyRequestError);
      }
    }

    RefPtr<Node> newNode;
    size_t newOffset = 0uz;
    if (TreeQueries::IsInclusiveAncestor(*_start.Container, *_end.Container))
    {
      newNode = _start.Container;
      newOffset = _start.Offset;
    }
    else
    {
      RawPtr<Node> refNode = _start.Container.get();
      while (refNode->ParentNode() && !TreeQueries::IsInclusiveAncestor(*refNode, *_end.Container))
      {
        refNode = refNode->ParentNode();
      }

      newNode = ShareRefPtr(refNode->ParentNode());
      newOffset = TreeQueries::Index(*refNode) + 1uz;
    }

    auto cloneResult = CloneFirstPartiallyContainedChildContents(firstPartiallyContainedChild, *fragment,
                                                                 DeleteClonedContent(true));
    if (cloneResult.HasException())
    {
      return cloneResult.ReleaseException();
    }

    for (auto &contained : containedChildren)
    {
      fragment->AppendChild(*contained);
    }

    cloneResult = CloneLastPartiallyContainedChildContents(lastPartiallyContainedChild, *fragment,
                                                           DeleteClonedContent(true));
    if (cloneResult.HasException())
    {
      return cloneResult.ReleaseException();
    }

    _start = BoundaryPoint {Krys::Move(newNode), newOffset};
    _end = _start;

    return fragment;
  }

  ExceptionOr<Ref<DocumentFragment>> Range::CloneContents() const noexcept
  {
    auto fragment = CreateRef<DocumentFragment>(*_start.Container->OwnerDocument());
    if (IsCollapsed())
    {
      return fragment;
    }

    if (_start.Container.get() == _end.Container.get() && _start.Container->IsCharacterDataNode())
    {
      auto cloneResult = CloneCharacterDataContents(*fragment, *_start.Container, _start.Offset,
                                                    _end.Offset - _start.Offset, DeleteClonedContent(false));

      if (cloneResult.HasException())
      {
        return cloneResult.ReleaseException();
      }

      return fragment;
    }

    RawPtr<Node> commonAncestor = CommonAncestorContainer();
    RawPtr<Node> firstPartiallyContainedChild = GetFirstPartiallyContainedChild(commonAncestor);
    RawPtr<Node> lastPartiallyContainedChild = GetLastPartiallyContainedChild(commonAncestor);

    List<Ref<Node>> containedChildren = GetContainedChildren(commonAncestor);
    for (auto &contained : containedChildren)
    {
      if (contained->IsDocumentTypeNode())
      {
        return Exception(ExceptionCode::HierarchyRequestError);
      }
    }

    auto cloneResult = CloneFirstPartiallyContainedChildContents(firstPartiallyContainedChild, *fragment,
                                                                 DeleteClonedContent(false));
    if (cloneResult.HasException())
    {
      return cloneResult.ReleaseException();
    }

    for (auto &contained : containedChildren)
    {
      fragment->AppendChild(*contained->CloneNode(true));
    }

    cloneResult = CloneLastPartiallyContainedChildContents(lastPartiallyContainedChild, *fragment,
                                                           DeleteClonedContent(false));
    if (cloneResult.HasException())
    {
      return cloneResult.ReleaseException();
    }

    return fragment;
  }

  ExceptionOr<void> Range::InsertNode(Node &node) noexcept
  {
    if (_start.Container->IsProcessingInstructionNode() || _start.Container->IsCommentNode()
        || (_start.Container->IsTextNode() && !_start.Container->ParentNode())
        || _start.Container.get() == &node)
    {
      return Exception(ExceptionCode::HierarchyRequestError);
    }

    RefPtr<Node> refNode;
    if (_start.Container->IsTextNode())
    {
      refNode = _start.Container;
    }
    else
    {
      refNode = ShareRefPtr(TreeQueries::ChildAt(Downcast<ContainerNode>(*_start.Container), _start.Offset));
    }

    RefPtr<ContainerNode> parent =
      refNode ? ShareRefPtr(refNode->ParentNode()) : RefPtr<ContainerNode>(_start.Container);

    if (auto preInsertValid = TreeMutationAlgorithms::EnsurePreInsertValidity(node, *parent, refNode.get());
        preInsertValid.HasException())
    {
      return preInsertValid.ReleaseException();
    }

    if (_start.Container->IsTextNode())
    {
      auto splitResult = Downcast<Text>(_start.Container.get())->SplitText(_start.Offset);
      if (splitResult.HasException())
      {
        return splitResult.ReleaseException();
      }

      refNode = splitResult.Value();
    }

    if (&node == refNode.get())
    {
      refNode = ShareRefPtr(node.NextSibling());
    }

    if (node.ParentNode())
    {
      node.ParentNode()->RemoveChild(node);
    }

    auto newOffset = refNode ? TreeQueries::Index(*refNode) : parent->Length();
    newOffset += node.IsDocumentFragmentNode() ? node.Length() : 1uz;

    if (auto preInsert = TreeMutationAlgorithms::PreInsert(node, *parent, refNode.get());
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
    // TODO(fix): this is probably wrong.
    auto *commonAncestor = CommonAncestorContainer();
    auto *firstPartiallyContainedChild = GetFirstPartiallyContainedChild(commonAncestor);
    auto *lastPartiallyContainedChild = GetLastPartiallyContainedChild(commonAncestor);

    if ((firstPartiallyContainedChild && !firstPartiallyContainedChild->IsTextNode())
        || (lastPartiallyContainedChild && !lastPartiallyContainedChild->IsTextNode()))
    {
      return Exception(ExceptionCode::InvalidStateError);
    }

    if (_start.Container->IsTextNode() && IsPartiallyContained(*_start.Container))
    {
      return Exception(ExceptionCode::InvalidStateError);
    }

    if (newParent.IsDocumentNode() || newParent.IsDocumentTypeNode() || newParent.IsDocumentFragmentNode())
    {
      return Exception(ExceptionCode::InvalidNodeTypeError);
    }

    auto fragment = ExtractContents();
    if (fragment.HasException())
    {
      return fragment.ReleaseException();
    }

    auto removeAllChildrenResult = TreeMutationAlgorithms::RemoveAllChildren(newParent);
    if (removeAllChildrenResult.HasException())
    {
      return removeAllChildrenResult.ReleaseException();
    }

    auto surroundPreInsertValid =
      TreeMutationAlgorithms::EnsurePreInsertValidity(newParent, *fragment.Value(), newParent.FirstChild());
    if (surroundPreInsertValid.HasException())
    {
      return surroundPreInsertValid.ReleaseException();
    }

    auto surroundInsertResult =
      TreeMutationAlgorithms::Insert(*fragment.Value(), newParent, newParent.FirstChild());
    if (surroundInsertResult.HasException())
    {
      return surroundInsertResult.ReleaseException();
    }

    return SelectNode(newParent);
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

    return compareResult.Value() == std::strong_ordering::equal;
  }

  ExceptionOr<std::strong_ordering> Range::ComparePoint(Node &node, uint64 offset) const noexcept
  {
    if (!TreeQueries::HasSameRoot(node, *_start.Container))
    {
      return Exception(ExceptionCode::WrongDocumentError);
    }

    if (node.IsDocumentTypeNode())
    {
      return Exception(ExceptionCode::InvalidNodeTypeError);
    }

    if (offset > node.Length())
    {
      return Exception(ExceptionCode::IndexSizeError);
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
    if (!TreeQueries::HasSameRoot(node, *_start.Container))
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
    if (boundaryPoint.ComparePositionTo(_start) != std::strong_ordering::greater)
    {
      return false;
    }

    boundaryPoint = BoundaryPoint {ShareRef(*parent), offset + 1uz};
    if (boundaryPoint.ComparePositionTo(_end) != std::strong_ordering::less)
    {
      return false;
    }

    return true;
  }

  ExceptionOr<DOMString> Range::ToString() const noexcept
  {
    if (_start.Container.get() == _end.Container.get() && _start.Container->IsCharacterDataNode())
    {
      auto characterData = Downcast<CharacterData>(_start.Container.get());
      auto substringResult = characterData->SubstringData(_start.Offset, _end.Offset - _start.Offset);
      if (substringResult.HasException())
      {
        return substringResult.ReleaseException();
      }

      return substringResult.ReleaseValue();
    }

    DOMString string;
    if (auto *startTextNode = DynamicDowncast<Text>(_start.Container.get()))
    {
      auto substringResult =
        startTextNode->SubstringData(_start.Offset, startTextNode->Length() - _start.Offset);
      if (substringResult.HasException())
      {
        return substringResult.ReleaseException();
      }
      string += substringResult.ReleaseValue();
    }

    auto containedNodes = GetContainedChildren(CommonAncestorContainer());
    for (auto &node : containedNodes)
    {
      if (auto *textNode = DynamicDowncast<Text>(node.get()))
      {
        string += textNode->Data();
      }
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
    if (node.IsDocumentTypeNode())
    {
      return Exception(ExceptionCode::InvalidNodeTypeError);
    }

    if (offset > node.Length())
    {
      return Exception(ExceptionCode::IndexSizeError);
    }

    BoundaryPoint boundaryPoint {ShareRef(node), offset};

    if (!TreeQueries::HasSameRoot(node, *_start.Container)
        || boundaryPoint.ComparePositionTo(_end) == std::strong_ordering::greater)
    {
      _end = boundaryPoint;
    }

    _start = Krys::Move(boundaryPoint);
    return {};
  }

  ExceptionOr<void> Range::SetEndBoundaryPoint(Node &node, uint64 offset) noexcept
  {
    if (node.IsDocumentTypeNode())
    {
      return Exception(ExceptionCode::InvalidNodeTypeError);
    }

    if (offset > node.Length())
    {
      return Exception(ExceptionCode::IndexSizeError);
    }

    BoundaryPoint boundaryPoint {ShareRef(node), offset};

    if (!TreeQueries::HasSameRoot(node, *_start.Container)
        || boundaryPoint.ComparePositionTo(_start) == std::strong_ordering::less)
    {
      _start = boundaryPoint;
    }

    _end = Krys::Move(boundaryPoint);
    return {};
  }

  ExceptionOr<void> Range::CloneCharacterDataContents(DocumentFragment &fragment, Node &container,
                                                      size_t offset, size_t length,
                                                      DeleteClonedContent deleteClonedContent) const noexcept
  {
    CharacterData &characterData = Downcast<CharacterData>(container);
    Ref<Node> clone = characterData.CloneNode();

    auto data = characterData.SubstringData(offset, length);
    if (data.HasException())
    {
      return data.ReleaseException();
    }

    Downcast<CharacterData>(clone.get())->SetData(data.ReleaseValue());
    fragment.AppendChild(*clone);

    if (deleteClonedContent)
    {
      characterData.DeleteData(offset, length);
    }

    return {};
  }

  bool Range::IsPartiallyContained(const Node &node) const noexcept
  {
    bool isStartInclusive = TreeQueries::IsInclusiveAncestor(node, *_start.Container);
    bool isEndInclusive = TreeQueries::IsInclusiveAncestor(node, *_end.Container);
    return (isStartInclusive && !isEndInclusive) || (!isStartInclusive && isEndInclusive);
  }

  RawPtr<Node> Range::GetFirstPartiallyContainedChild(RawPtr<Node> commonAncestor) const noexcept
  {
    if (!TreeQueries::IsInclusiveAncestor(*_start.Container, *_end.Container))
    {
      for (RawPtr<Node> child = commonAncestor->FirstChild(); child; child = child->NextSibling())
      {
        if (IsPartiallyContained(*child))
        {
          return child;
        }
      }
    }

    return nullptr;
  }

  RawPtr<Node> Range::GetLastPartiallyContainedChild(RawPtr<Node> commonAncestor) const noexcept
  {
    if (!TreeQueries::IsInclusiveAncestor(*_end.Container, *_start.Container))
    {
      for (auto *child = commonAncestor->LastChild(); child; child = child->PreviousSibling())
      {
        if (IsPartiallyContained(*child))
        {
          return child;
        }
      }
    }

    return nullptr;
  }

  List<Ref<Node>> Range::GetContainedChildren(RawPtr<Node> commonAncestor) const noexcept
  {
    List<Ref<Node>> containedChildren;

    for (RawPtr<Node> child = commonAncestor->FirstChild(); child; child = child->NextSibling())
    {
      auto afterStart = ComparePoint(*child, 0);
      if (afterStart.HasException() || afterStart.Value() != std::strong_ordering::greater)
      {
        continue;
      }

      auto beforeEnd = ComparePoint(*child, child->Length());
      if (beforeEnd.HasException() || beforeEnd.Value() != std::strong_ordering::less)
      {
        continue;
      }

      containedChildren.emplace_back(ShareRef(*child));
    }

    return containedChildren;
  }

  ExceptionOr<void>
    Range::CloneFirstPartiallyContainedChildContents(RawPtr<Node> child, DocumentFragment &fragment,
                                                     DeleteClonedContent deleteClonedContent) const noexcept
  {
    if (child && child->IsCharacterDataNode())
    {
      auto length = Downcast<CharacterData>(_start.Container.get())->Length() - _start.Offset;
      auto cloneResult =
        CloneCharacterDataContents(fragment, *_start.Container, _start.Offset, length, deleteClonedContent);
      if (cloneResult.HasException())
      {
        return cloneResult.ReleaseException();
      }
    }
    else if (child != nullptr)
    {
      Ref<Node> clone = child->CloneNode();
      fragment.AppendChild(*clone);

      auto subrange = Range {_start, BoundaryPoint {ShareRef(*child), child->Length()}};
      auto subFragment = deleteClonedContent ? subrange.ExtractContents() : subrange.CloneContents();

      if (subFragment.HasException())
      {
        return subFragment.ReleaseException();
      }

      clone->AppendChild(*subFragment.Value());
    }

    return {};
  }

  ExceptionOr<void>
    Range::CloneLastPartiallyContainedChildContents(RawPtr<Node> child, DocumentFragment &fragment,
                                                    DeleteClonedContent deleteClonedContent) const noexcept
  {
    if (child && child->IsCharacterDataNode())
    {
      auto cloneResult =
        CloneCharacterDataContents(fragment, *_end.Container, 0uz, _end.Offset, deleteClonedContent);

      if (cloneResult.HasException())
      {
        return cloneResult.ReleaseException();
      }
    }
    else if (child != nullptr)
    {
      Ref<Node> clone = child->CloneNode();
      fragment.AppendChild(*clone);

      auto subrange = Range {BoundaryPoint {ShareRef(*child), 0}, _end};
      auto subFragment = subrange.ExtractContents();
      if (subFragment.HasException())
      {
        return subFragment.ReleaseException();
      }

      clone->AppendChild(*subFragment.Value());
    }

    return {};
  }
}
