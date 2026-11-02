#include "Krystal.Booey/DOM/Range.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/LiveRangeUpdater.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/CharacterData.hpp"
#include "Krystal.Booey/DOM/Comment.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DocumentType.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Internals/ElementFactory.hpp"
#include "Krystal.Booey/DOM/ProcessingInstruction.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Editing/Markup.hpp"
#include "Krystal.Booey/Graphics/GeometryUtilities.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLHtmlElement.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

namespace krys::boo::dom
{
  Range::~Range() noexcept
  {
    krys_debug_assert(!_isAssociatedWithSelection);
    LiveRangeUpdater::Destroyed(*this);
  }

  void Range::UpdateAssociatedSelection() noexcept
  {
    if (_isAssociatedWithSelection)
    {
      _start.Container->NodeDocument().Selection().UpdateFromAssociatedLiveRange();
    }
  }

#pragma region Range

  Range::Range(BoundaryPoint start, BoundaryPoint end) noexcept : AbstractRange(start, end)
  {
    LiveRangeUpdater::Created(*this);
  }

  Node *Range::CommonAncestorContainer() const noexcept
  {
    // Get the common ancestor:
    // 1. Let container be range's start node.
    // 2. While container is not an inclusive ancestor of range's end node: set container to container's
    //    parent.
    // 3. Return container.
    krys_debug_assert(TreeQueries::SameRoot(*_start.Container, *_end.Container));
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
    // 1. Let parent be node's parent.
    // 2. If parent is null, then throw an "InvalidNodeTypeError" DOMException.
    Node *parent = node.ParentNode();
    if (parent == nullptr)
    {
      return ExceptionCode::InvalidNodeTypeError;
    }

    // 3. Set the start of this to boundary point (parent, node's index).
    if (auto result = SetStartBoundaryPoint(*parent, TreeQueries::Index(node)); result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

  ExceptionOr<void> Range::SetStartAfter(Node &node) noexcept
  {
    // 1. Let parent be node's parent.
    // 2. If parent is null, then throw an "InvalidNodeTypeError" DOMException.
    Node *parent = node.ParentNode();
    if (parent == nullptr)
    {
      return ExceptionCode::InvalidNodeTypeError;
    }

    // 3. Set the start of this to boundary point (parent, node's index plus 1).
    if (auto result = SetStartBoundaryPoint(*parent, TreeQueries::Index(node) + 1uz); result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

  ExceptionOr<void> Range::SetEndBefore(Node &node) noexcept
  {
    // 1. Let parent be node's parent.
    // 2. If parent is null, then throw an "InvalidNodeTypeError" DOMException.
    Node *parent = node.ParentNode();
    if (parent == nullptr)
    {
      return Exception(ExceptionCode::InvalidNodeTypeError);
    }

    // 3. Set the end of this to boundary point (parent, node's index).
    if (auto result = SetEndBoundaryPoint(*parent, TreeQueries::Index(node)); result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

  ExceptionOr<void> Range::SetEndAfter(Node &node) noexcept
  {
    // 1. Let parent be node's parent.
    // 2. If parent is null, then throw an "InvalidNodeTypeError" DOMException.
    Node *parent = node.ParentNode();
    if (parent == nullptr)
    {
      return Exception(ExceptionCode::InvalidNodeTypeError);
    }

    // 3. Set the end of this to boundary point (parent, node's index plus 1).
    if (auto result = SetEndBoundaryPoint(*parent, TreeQueries::Index(node) + 1uz); result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

  void Range::Collapse(bool toStart) noexcept
  {
    // If toStart is true, set end to start; otherwise set start to end.
    if (toStart)
    {
      _end = _start;
    }
    else
    {
      _start = _end;
    }
    UpdateAssociatedSelection();
  }

  ExceptionOr<void> Range::SelectNode(Node &node) noexcept
  {
    // Select node within this:
    // 1. Let parent be node's parent.
    // 2. If parent is null, then throw an "InvalidNodeTypeError" DOMException.
    Node *parent = node.ParentNode();
    if (parent == nullptr)
    {
      return Exception(ExceptionCode::InvalidNodeTypeError);
    }

    // 3. Let index be node's index.
    auto index = TreeQueries::Index(node);
    // 4. Set range's start to boundary point (parent, index).
    if (auto result = SetStartBoundaryPoint(*parent, index); result.HasException())
    {
      return result.ReleaseException();
    }

    // 5. Set range's end to boundary point (parent, index plus 1).
    if (auto result = SetEndBoundaryPoint(*parent, index + 1uz); result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

  ExceptionOr<void> Range::SelectNodeContents(Node &node) noexcept
  {
    // 1. If node is a doctype, then throw an "InvalidNodeTypeError" DOMException.
    if (Is<DocumentType>(node))
    {
      return Exception(ExceptionCode::InvalidNodeTypeError);
    }

    // 2. Let length be the length of node.
    // 3. Set start to the boundary point (node, 0).
    if (auto result = SetStartBoundaryPoint(node, 0); result.HasException())
    {
      return result.ReleaseException();
    }

    // 4. Set end to the boundary point (node, length).
    if (auto result = SetEndBoundaryPoint(node, TreeQueries::Length(node)); result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

  ExceptionOr<std::strong_ordering> Range::CompareBoundaryPoints(BoundaryPointComparator how,
                                                                 const Range &other) const noexcept
  {
    // 1. If how is not one of START_TO_START, START_TO_END, END_TO_END, and END_TO_START, then throw a
    //    "NotSupportedError" DOMException.
    if (static_cast<uint32>(std::to_underlying(how)) >= EnumTraits<BoundaryPointComparator>::DistinctValues)
    {
      return Exception(ExceptionCode::NotSupportedError);
    }

    // 2. If this's root is not sourceRange's root, then throw a "WrongDocumentError" DOMException.
    if (!TreeQueries::SameRoot(*_start.Container, *other._start.Container))
    {
      return Exception(ExceptionCode::WrongDocumentError);
    }

    // 3. Let thisPoint and sourcePoint be null.
    // 4. Switch on how:
    // 5. Switch on the position of thisPoint relative to sourcePoint: before: return -1; equal: return 0;
    //    after: return 1.
    // (The names say which of sourceRange's points comes first: START_TO_END compares this's end with
    // sourceRange's start.)
    switch (how)
    {
      // START_TO_START: Set thisPoint to this's start and sourcePoint to sourceRange's start.
      case BoundaryPointComparator::StartToStart: return _start.ComparePositionTo(other._start);
      // START_TO_END: Set thisPoint to this's end and sourcePoint to sourceRange's start.
      case BoundaryPointComparator::StartToEnd:   return _end.ComparePositionTo(other._start);
      // END_TO_END: Set thisPoint to this's end and sourcePoint to sourceRange's end.
      case BoundaryPointComparator::EndToEnd:     return _end.ComparePositionTo(other._end);
      // END_TO_START: Set thisPoint to this's start and sourcePoint to sourceRange's end.
      case BoundaryPointComparator::EndToStart:   return _start.ComparePositionTo(other._end);
    }
    krys_unreachable();
  }

  ExceptionOr<void> Range::DeleteContents() noexcept
  {
    html::CEReactionsScope ceReactions;
    // 1. If this is collapsed, then return.
    if (Collapsed())
    {
      return {};
    }

    // 2. Let originalStartNode, originalStartOffset, originalEndNode, and originalEndOffset be this's start
    //    node, start offset, end node, and end offset, respectively.
    Ref<Node> originalStartNodeRef = _start.Container;
    Ref<Node> originalEndNodeRef = _end.Container;
    auto *originalStartNode = originalStartNodeRef.get();
    auto originalStartOffset = _start.Offset;
    auto *originalEndNode = originalEndNodeRef.get();
    auto originalEndOffset = _end.Offset;

    // 3. If originalStartNode is originalEndNode and it is a CharacterData node:
    if (originalStartNode == originalEndNode)
    {
      if (auto *characterData = DynamicDowncast<CharacterData>(originalStartNode))
      {
        // 3a. Replace data of originalStartNode with originalStartOffset, originalEndOffset -
        //     originalStartOffset, and the empty string.
        // 3b. Return.
        auto deleteData =
          characterData->DeleteData(originalStartOffset, originalEndOffset - originalStartOffset);
        if (deleteData.HasException())
        {
          return deleteData.ReleaseException();
        }
        return {};
      }
    }

    // 4. Let nodesToRemove be a list of all the nodes that are contained in this, in tree order, omitting any
    //    node whose parent is also contained in this.
    SmallNodeList nodesToRemove;

    Node *root = CommonAncestorContainer();
    Node *current = root->FirstChild();
    while (current != nullptr)
    {
      if (IsContained(*current))
      {
        nodesToRemove.emplace_back(ShareRef(*current));
        current = TreeQueries::NextSkippingChildren(*current, root);
      }
      else
      {
        current = TreeQueries::Next(*current, root);
      }
    }

    // 5. Let newNode and newOffset be null.
    Node *newNode = nullptr;
    size_t newOffset = 0uz;

    // 6. If originalStartNode is an inclusive ancestor of originalEndNode, then set newNode to
    //    originalStartNode and newOffset to originalStartOffset.
    if (TreeQueries::IsInclusiveAncestor(*originalStartNode, *originalEndNode))
    {
      newNode = originalStartNode;
      newOffset = originalStartOffset;
    }
    // 7. Otherwise:
    else
    {
      // 7a. Let referenceNode be originalStartNode.
      Node *refNode = originalStartNode;
      // 7b. While referenceNode's parent is non-null and is not an inclusive ancestor of originalEndNode: set
      //     referenceNode to its parent.
      while (refNode->ParentNode()
             && !TreeQueries::IsInclusiveAncestor(*refNode->ParentNode(), *originalEndNode))
      {
        refNode = refNode->ParentNode();
      }

      // 7c. Set newNode to referenceNode's parent and newOffset to referenceNode's index + 1.
      newNode = refNode->ParentNode();
      newOffset = TreeQueries::Index(*refNode) + 1uz;
    }

    // 8. Set this's start and end to (newNode, newOffset).
    krys_debug_assert(newNode != nullptr);
    _start = BoundaryPoint {ShareRef(*newNode), newOffset};
    _end = _start;

    // 9. If originalStartNode is a CharacterData node, then replace data of originalStartNode with
    //    originalStartOffset, originalStartNode's length - originalStartOffset, and the empty string.
    if (auto *characterData = DynamicDowncast<CharacterData>(originalStartNode))
    {
      auto length = TreeQueries::Length(*characterData);
      auto offset = originalStartOffset;

      if (auto deleteData = characterData->DeleteData(offset, length - offset); deleteData.HasException())
      {
        return deleteData.ReleaseException();
      }
    }

    // 10. For each node of nodesToRemove, in tree order: remove node.
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

    // 11. If originalEndNode is a CharacterData node, then replace data of originalEndNode with 0,
    //     originalEndOffset, and the empty string.
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
    html::CEReactionsScope ceReactions;
    // Extract:
    // 1. Let fragment be the result of creating a document fragment given range's start node's node document.
    auto fragment = CreateRef<DocumentFragment>(_start.Container->NodeDocument());

    // 2. If range is collapsed, then return fragment.
    if (Collapsed())
    {
      return fragment;
    }

    // 3. Let originalStartNode, originalStartOffset, originalEndNode, and originalEndOffset be range's start
    //    node, start offset, end node, and end offset, respectively.
    Ref<Node> originalStartNodeRef = _start.Container;
    Ref<Node> originalEndNodeRef = _end.Container;
    auto *originalStartNode = originalStartNodeRef.get();
    auto originalStartOffset = _start.Offset;
    auto *originalEndNode = originalEndNodeRef.get();
    auto originalEndOffset = _end.Offset;

    // 4. If originalStartNode is originalEndNode and it is a CharacterData node:
    if (originalStartNode == originalEndNode)
    {
      if (auto *characterData = DynamicDowncast<CharacterData>(originalStartNode))
      {
        // 4a.-4d. Clone it with the data from originalStartOffset to originalEndOffset, append the clone to
        //         fragment, and replace that data of originalStartNode with the empty string.
        // 4e. Return fragment.
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

    // 5. Let commonAncestor be the result of getting the common ancestor of range.
    Ref<Node> commonAncestorRef =
      ShareRef(*TreeQueries::CommonAncestor(*originalStartNode, *originalEndNode));
    auto *commonAncestor = commonAncestorRef.get();

    // 6. Let firstPartiallyContainedChild be null.
    // 7. If originalStartNode is not an inclusive ancestor of originalEndNode, then set
    //    firstPartiallyContainedChild to the first child of commonAncestor that is partially contained in
    //    range.
    RefPtr<Node> firstPartiallyContainedChild =
      ShareRefPtr(FirstPartiallyContainedChild(*commonAncestor, *originalStartNode, *originalEndNode));
    // 8. Let lastPartiallyContainedChild be null.
    // 9. If originalEndNode is not an inclusive ancestor of originalStartNode, then set
    //    lastPartiallyContainedChild to the last child of commonAncestor that is partially contained in
    //    range.
    RefPtr<Node> lastPartiallyContainedChild =
      ShareRefPtr(LastPartiallyContainedChild(*commonAncestor, *originalStartNode, *originalEndNode));

    // 10. Let containedChildren be a list of all children of commonAncestor that are contained in range, in
    //     tree order.
    // 11. If any member of containedChildren is a doctype, then throw a "HierarchyRequestError" DOMException.
    auto containedChildrenResult = GetContainedChildren(commonAncestor);
    if (containedChildrenResult.HasException())
    {
      return containedChildrenResult.ReleaseException();
    }
    SmallNodeList containedChildren = containedChildrenResult.ReleaseValue();

    // 12. Let newNode and newOffset be null.
    Node *newNode = nullptr;
    size_t newOffset = 0uz;

    // 13. If originalStartNode is an inclusive ancestor of originalEndNode, then set newNode to
    //     originalStartNode and newOffset to originalStartOffset.
    if (TreeQueries::IsInclusiveAncestor(*originalStartNode, *originalEndNode))
    {
      newNode = originalStartNode;
      newOffset = originalStartOffset;
    }
    // 14. Otherwise:
    else
    {
      // 14a. Let referenceNode be originalStartNode.
      Node *referenceNode = originalStartNode;
      // 14b. While referenceNode's parent is non-null and is not an inclusive ancestor of originalEndNode:
      //      set referenceNode to its parent.
      while (referenceNode->ParentNode() != nullptr
             && !TreeQueries::IsInclusiveAncestor(*referenceNode->ParentNode(), *originalEndNode))
      {
        referenceNode = referenceNode->ParentNode();
      }
      // 14c. Set newNode to the parent of referenceNode, and newOffset to referenceNode's index + 1.
      newNode = referenceNode->ParentNode();
      newOffset = TreeQueries::Index(*referenceNode) + 1uz;
    }

    // 15. Set range's start and end to (newNode, newOffset).
    krys_debug_assert(newNode != nullptr);
    _start = BoundaryPoint {ShareRef(*newNode), newOffset};
    _end = _start;

    // 16. If firstPartiallyContainedChild is a CharacterData node:
    if (Is<CharacterData>(firstPartiallyContainedChild.get()))
    {
      // 16a.-16d. Clone originalStartNode with its data from originalStartOffset to its end, append the clone
      //           to fragment, and replace that data of originalStartNode with the empty string.
      auto length = TreeQueries::Length(*originalStartNode) - originalStartOffset;
      auto cloneResult = CloneCharacterDataContents(*fragment, *originalStartNode, originalStartOffset,
                                                    length, DeleteClonedContent(true));
      if (cloneResult.HasException())
      {
        return cloneResult.ReleaseException();
      }
    }
    // 17. Otherwise, if firstPartiallyContainedChild is non-null:
    else if (firstPartiallyContainedChild != nullptr)
    {
      // 17a. Let clone be a clone of firstPartiallyContainedChild.
      auto cloneResult = NodeAlgorithms::CloneNode(*firstPartiallyContainedChild);
      if (cloneResult.HasException())
      {
        return cloneResult.ReleaseException();
      }

      // 17b. Append clone to fragment.
      if (auto appendResult = fragment->AppendChild(*cloneResult.Value()); appendResult.HasException())
      {
        return appendResult.ReleaseException();
      }

      // 17c. Let subrange be a new live range whose start is (originalStartNode, originalStartOffset) and
      //      whose end is (firstPartiallyContainedChild, firstPartiallyContainedChild's length).
      auto subrange = CreateRef<Range>(BoundaryPoint {ShareRef(*originalStartNode), originalStartOffset},
                                       BoundaryPoint {ShareRef(*firstPartiallyContainedChild),
                                                      TreeQueries::Length(*firstPartiallyContainedChild)});

      // 17d. Let subfragment be the result of extracting subrange.
      // 17e. Append subfragment to clone.
      auto subFragment = subrange->ExtractContents();
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

    // 18. For each contained child of containedChildren: append contained child to fragment.
    for (auto &contained : containedChildren)
    {
      auto appendResult = fragment->AppendChild(*contained);
      if (appendResult.HasException())
      {
        return appendResult.ReleaseException();
      }
    }

    // 19. If lastPartiallyContainedChild is a CharacterData node:
    if (Is<CharacterData>(lastPartiallyContainedChild.get()))
    {
      // 19a.-19d. Clone originalEndNode with its data up to originalEndOffset, append the clone to fragment,
      //           and replace that data of originalEndNode with the empty string.
      auto cloneResult = CloneCharacterDataContents(*fragment, *originalEndNode, 0uz, originalEndOffset,
                                                    DeleteClonedContent(true));
      if (cloneResult.HasException())
      {
        return cloneResult.ReleaseException();
      }
    }
    // 20. Otherwise, if lastPartiallyContainedChild is non-null:
    else if (lastPartiallyContainedChild != nullptr)
    {
      // 20a. Let clone be a clone of lastPartiallyContainedChild.
      auto cloneResult = NodeAlgorithms::CloneNode(*lastPartiallyContainedChild);
      if (cloneResult.HasException())
      {
        return cloneResult.ReleaseException();
      }

      // 20b. Append clone to fragment.
      if (auto appendResult = fragment->AppendChild(*cloneResult.Value()); appendResult.HasException())
      {
        return appendResult.ReleaseException();
      }

      // 20c. Let subrange be a new live range whose start is (lastPartiallyContainedChild, 0) and whose end
      // is
      //      (originalEndNode, originalEndOffset).
      auto subrange = CreateRef<Range>(BoundaryPoint {ShareRef(*lastPartiallyContainedChild), 0uz},
                                       BoundaryPoint {ShareRef(*originalEndNode), originalEndOffset});

      // 20d. Let subfragment be the result of extracting subrange.
      // 20e. Append subfragment to clone.
      auto subFragment = subrange->ExtractContents();
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

    // 21. Return fragment.
    return fragment;
  }

  ExceptionOr<Ref<DocumentFragment>> Range::CloneContents() const noexcept
  {
    // Clone the contents:
    // 1. Let fragment be the result of creating a document fragment given range's start node's node document.
    auto fragment = CreateRef<DocumentFragment>(_start.Container->NodeDocument());

    // 2. If range is collapsed, then return fragment.
    if (Collapsed())
    {
      return fragment;
    }

    // 3. Let originalStartNode, originalStartOffset, originalEndNode, and originalEndOffset be range's start
    //    node, start offset, end node, and end offset, respectively.
    auto *originalStartNode = _start.Container.get();
    auto originalStartOffset = _start.Offset;
    auto *originalEndNode = _end.Container.get();
    auto originalEndOffset = _end.Offset;

    // 4. If originalStartNode is originalEndNode and it is a CharacterData node:
    if (originalStartNode == originalEndNode)
    {
      if (auto *characterData = DynamicDowncast<CharacterData>(originalStartNode))
      {
        // 4a.-4c. Clone it with the data from originalStartOffset to originalEndOffset, and append the clone
        //         to fragment.
        // 4d. Return fragment.
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

    // 5. Let commonAncestor be the result of getting the common ancestor of range.
    auto *commonAncestor = TreeQueries::CommonAncestor(*originalStartNode, *originalEndNode);
    // 6.-7. Let firstPartiallyContainedChild be the first child of commonAncestor that is partially contained
    //       in range, if originalStartNode is not an inclusive ancestor of originalEndNode; otherwise null.
    auto *firstPartiallyContainedChild =
      FirstPartiallyContainedChild(*commonAncestor, *originalStartNode, *originalEndNode);
    // 8.-9. Let lastPartiallyContainedChild be the last child of commonAncestor that is partially contained
    // in
    //       range, if originalEndNode is not an inclusive ancestor of originalStartNode; otherwise null.
    auto *lastPartiallyContainedChild =
      LastPartiallyContainedChild(*commonAncestor, *originalStartNode, *originalEndNode);

    // 10. Let containedChildren be a list of all children of commonAncestor that are contained in range, in
    //     tree order.
    // 11. If any member of containedChildren is a doctype, then throw a "HierarchyRequestError" DOMException.
    auto containedChildren = GetContainedChildren(commonAncestor);
    if (containedChildren.HasException())
    {
      return containedChildren.ReleaseException();
    }

    // 12. If firstPartiallyContainedChild is a CharacterData node: clone originalStartNode with its data from
    //     originalStartOffset to its end, and append the clone to fragment.
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
    // 13. Otherwise, if firstPartiallyContainedChild is non-null: append a clone of it to fragment, and
    // append
    //     to the clone the result of cloning the contents of (originalStartNode, originalStartOffset) to
    //     (firstPartiallyContainedChild, firstPartiallyContainedChild's length).
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

      auto subrange = CreateRef<Range>(BoundaryPoint {ShareRef(*originalStartNode), originalStartOffset},
                                       BoundaryPoint {ShareRef(*firstPartiallyContainedChild),
                                                      TreeQueries::Length(*firstPartiallyContainedChild)});

      auto subFragment = subrange->CloneContents();
      if (subFragment.HasException())
      {
        return subFragment.ReleaseException();
      }

      if (auto appendSubFragment = clone->AppendChild(*subFragment.Value()); appendSubFragment.HasException())
      {
        return appendSubFragment.ReleaseException();
      }
    }

    // 14. For each contained child of containedChildren:
    for (auto &contained : containedChildren.Value())
    {
      // 14a. Let clone be a clone of contained child with subtree set to true.
      // 14b. Append clone to fragment.
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

    // 15. If lastPartiallyContainedChild is a CharacterData node: clone originalEndNode with its data up to
    //     originalEndOffset, and append the clone to fragment.
    if (Is<CharacterData>(lastPartiallyContainedChild))
    {
      auto clone = CloneCharacterDataContents(*fragment, *originalEndNode, 0uz, originalEndOffset,
                                              DeleteClonedContent(false));
      if (clone.HasException())
      {
        return clone.ReleaseException();
      }
    }
    // 16. Otherwise, if lastPartiallyContainedChild is non-null: append a clone of it to fragment, and append
    //     to the clone the result of cloning the contents of (lastPartiallyContainedChild, 0) to
    //     (originalEndNode, originalEndOffset).
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

      auto subrange = CreateRef<Range>(BoundaryPoint {ShareRef(*lastPartiallyContainedChild), 0uz},
                                       BoundaryPoint {ShareRef(*originalEndNode), originalEndOffset});

      auto subFragment = subrange->CloneContents();
      if (subFragment.HasException())
      {
        return subFragment.ReleaseException();
      }

      if (auto appendSubFragment = clone->AppendChild(*subFragment.Value()); appendSubFragment.HasException())
      {
        return appendSubFragment.ReleaseException();
      }
    }

    // 17. Return fragment.
    return fragment;
  }

  ExceptionOr<void> Range::InsertNode(Node &node) noexcept
  {
    html::CEReactionsScope ceReactions;
    // Insert node into this:
    // 1. If range's start node is a ProcessingInstruction or Comment node, is a Text node whose parent is
    // null,
    //    or is node, then throw a "HierarchyRequestError" DOMException.
    if (Is<ProcessingInstruction>(_start.Container.get()) || Is<Comment>(_start.Container.get())
        || (Is<Text>(_start.Container.get()) && _start.Container->ParentNode() == nullptr)
        || _start.Container == &node)
    {
      return ExceptionCode::HierarchyRequestError;
    }

    // 2. Let referenceNode be null.
    RefPtr<Node> referenceNode;
    // 3. If range's start node is a Text node, then set referenceNode to that Text node.
    if (Is<Text>(_start.Container.get()))
    {
      referenceNode = ShareRefPtr(_start.Container.get());
    }
    // 4. Otherwise, set referenceNode to the child of range's start node whose index is range's start offset
    //    if there is such a child; otherwise null.
    else
    {
      referenceNode =
        ShareRefPtr(TreeQueries::ChildAt(Downcast<ContainerNode>(*_start.Container), _start.Offset));
    }

    // 5. Let parent be range's start node if referenceNode is null; otherwise referenceNode's parent.
    RefPtr<ContainerNode> parent =
      ShareRefPtr(referenceNode == nullptr ? Downcast<ContainerNode>(_start.Container.get())
                                           : referenceNode->ParentNode());

    // 6. Ensure pre-insert validity given node, parent, referenceNode, and « ».
    if (auto preInsertValid = MutationAlgorithms::EnsurePreInsertValidity(node, *parent, referenceNode.get());
        preInsertValid.HasException())
    {
      return preInsertValid.ReleaseException();
    }

    // 7. If range's start node is a Text node, then set referenceNode to the result of splitting it with
    // offset
    //    range's start offset.
    if (Is<Text>(_start.Container.get()))
    {
      auto splitResult = Downcast<Text>(_start.Container.get())->SplitText(_start.Offset);
      if (splitResult.HasException())
      {
        return splitResult.ReleaseException();
      }

      referenceNode = splitResult.Value();
    }

    // 8. If node is referenceNode, then set referenceNode to its next sibling.
    if (&node == referenceNode)
    {
      referenceNode = ShareRefPtr(node.NextSibling());
    }

    // 9. If node's parent is non-null, then remove node.
    if (node.ParentNode() != nullptr)
    {
      if (auto remove = node.ParentNode()->RemoveChild(node); remove.HasException())
      {
        return remove.ReleaseException();
      }
    }

    // 10. Let newOffset be parent's length if referenceNode is null; otherwise referenceNode's index.
    auto newOffset =
      referenceNode == nullptr ? TreeQueries::Length(*parent) : TreeQueries::Index(*referenceNode);
    // 11. Increase newOffset by node's length if node is a DocumentFragment node; otherwise 1.
    newOffset += Is<DocumentFragment>(node) ? TreeQueries::Length(node) : 1uz;

    // 12. Pre-insert node into parent before referenceNode.
    if (auto preInsert = MutationAlgorithms::PreInsert(node, *parent, referenceNode.get());
        preInsert.HasException())
    {
      return preInsert.ReleaseException();
    }

    // 13. If range is collapsed, then set range's end to (parent, newOffset).
    if (Collapsed())
    {
      _end = BoundaryPoint {ShareRef(*parent), newOffset};
    }

    return {};
  }

  ExceptionOr<void> Range::SurroundContents(ContainerNode &newParent) noexcept
  {
    html::CEReactionsScope ceReactions;
    auto IsPartiallyContainedNonTextNode = [&](const Node &node)
    {
      return !Is<Text>(node) && IsPartiallyContained(node);
    };

    // 1. If a non-Text node is partially contained in this, then throw an "InvalidStateError" DOMException.
    //    (The partially contained nodes are the inclusive ancestors of one of its start and end nodes.)
    if (std::ranges::any_of(ConstInclusiveAncestorRange(*_start.Container), IsPartiallyContainedNonTextNode))
    {
      return ExceptionCode::InvalidStateError;
    }

    if (std::ranges::any_of(ConstInclusiveAncestorRange(*_end.Container), IsPartiallyContainedNonTextNode))
    {
      return ExceptionCode::InvalidStateError;
    }

    // 2. If newParent is a Document, DocumentType, or DocumentFragment node, then throw an
    //    "InvalidNodeTypeError" DOMException. (A DocumentType isn't a ContainerNode.)
    if (Is<Document>(newParent) || Is<DocumentFragment>(newParent))
    {
      return ExceptionCode::InvalidNodeTypeError;
    }

    // 3. Let fragment be the result of extracting this.
    auto fragment = ExtractContents();
    if (fragment.HasException())
    {
      return fragment.ReleaseException();
    }

    // 4. If newParent has children, then replace all with null within newParent.
    if (newParent.HasChildNodes())
    {
      if (auto replaceAll = MutationAlgorithms::ReplaceAll(nullptr, newParent); replaceAll.HasException())
      {
        return replaceAll.ReleaseException();
      }
    }

    // 5. Insert newParent into this.
    if (auto insert = InsertNode(newParent); insert.HasException())
    {
      return insert.ReleaseException();
    }

    // 6. Append fragment to newParent.
    if (auto append = MutationAlgorithms::Append(*fragment.Value(), newParent); append.HasException())
    {
      return append.ReleaseException();
    }

    // 7. Select newParent within this.
    if (auto select = SelectNode(newParent); select.HasException())
    {
      return select.ReleaseException();
    }

    return {};
  }

  Ref<Range> Range::CloneRange() const noexcept
  {
    // Return a new live range with the same start and end as this.
    auto start = BoundaryPoint {ShareRef(*StartContainer()), StartOffset()};
    auto end = BoundaryPoint {ShareRef(*EndContainer()), EndOffset()};
    return CreateRef<Range>(start, end);
  }

  void Range::Detach() noexcept
  {
    // Do nothing.
  }

  ExceptionOr<bool> Range::IsPointInRange(Node &node, uint64 offset) const noexcept
  {
    // 1. If node's root is not this's root, then return false.
    if (!TreeQueries::SameRoot(node, *_start.Container))
    {
      return false;
    }

    // 2. If node is a doctype, then throw an "InvalidNodeTypeError" DOMException.
    if (Is<DocumentType>(node))
    {
      return ExceptionCode::InvalidNodeTypeError;
    }

    // 3. If offset is greater than node's length, then throw an "IndexSizeError" DOMException.
    if (offset > TreeQueries::Length(node))
    {
      return ExceptionCode::IndexSizeError;
    }

    // 4. If (node, offset) is before start or after end, then return false.
    auto boundaryPoint = BoundaryPoint {ShareRef(node), offset};
    if (boundaryPoint.ComparePositionTo(_start) == std::strong_ordering::less
        || boundaryPoint.ComparePositionTo(_end) == std::strong_ordering::greater)
    {
      return false;
    }

    // 5. Return true.
    return true;
  }

  ExceptionOr<std::strong_ordering> Range::ComparePoint(Node &node, uint64 offset) const noexcept
  {
    // 1. If node's root is not this's root, then throw a "WrongDocumentError" DOMException.
    if (!TreeQueries::SameRoot(node, *_start.Container))
    {
      return ExceptionCode::WrongDocumentError;
    }

    // 2. If node is a doctype, then throw an "InvalidNodeTypeError" DOMException.
    if (Is<DocumentType>(node))
    {
      return ExceptionCode::InvalidNodeTypeError;
    }

    // 3. If offset is greater than node's length, then throw an "IndexSizeError" DOMException.
    if (offset > TreeQueries::Length(node))
    {
      return ExceptionCode::IndexSizeError;
    }

    // 4. If (node, offset) is before start, then return -1.
    auto boundaryPoint = BoundaryPoint {ShareRef(node), offset};
    if (boundaryPoint.ComparePositionTo(_start) == std::strong_ordering::less)
    {
      return std::strong_ordering::less;
    }

    // 5. If (node, offset) is after end, then return 1.
    if (boundaryPoint.ComparePositionTo(_end) == std::strong_ordering::greater)
    {
      return std::strong_ordering::greater;
    }

    // 6. Return 0.
    return std::strong_ordering::equal;
  }

  bool Range::IntersectsNode(const Node &node) const noexcept
  {
    // 1. If node's root is not this's root, then return false.
    if (!TreeQueries::SameRoot(node, *_start.Container))
    {
      return false;
    }

    // 2. Let parent be node's parent.
    // 3. If parent is null, then return true.
    Node *parent = node.ParentNode();
    if (parent == nullptr)
    {
      return true;
    }

    // 4. Let offset be node's index.
    auto offset = TreeQueries::Index(node);

    // 5. If (parent, offset) is before end and (parent, offset + 1) is after start, then return true.
    // 6. Return false.
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

  Ref<DOMRectList> Range::GetClientRects() const noexcept
  {
    (void)_start.Container->NodeDocument().UpdateLayout();
    return DOMRectList::Create(RenderObject::ClientBorderAndTextRects(MakeSimpleRange(*this)));
  }

  Ref<DOMRect> Range::GetBoundingClientRect() const noexcept
  {
    return BoundingClientRect(MakeSimpleRange(*this));
  }

  Ref<DOMRect> Range::BoundingClientRect(const SimpleRange &range) noexcept
  {
    (void)range.Start.GetDocument().UpdateLayout();
    auto rects = RenderObject::ClientBorderAndTextRects(range);
    return DOMRect::Create(UnionRectIgnoringZeroRects(Span<const FloatRect>(rects)));
  }

  ExceptionOr<DOMString> Range::ToString() const noexcept
  {
    // 1. Let string be the empty string.
    // 2. If this's start node is this's end node and it is a Text node, then return the substring of that
    // Text
    //    node's data beginning at this's start offset and ending at this's end offset.
    if (_start.Container == _end.Container)
    {
      if (auto *textNode = DynamicDowncast<Text>(_start.Container.get()))
      {
        return textNode->SubstringData(_start.Offset, _end.Offset - _start.Offset);
      }
    }

    DOMString string;
    // 3. If this's start node is a Text node, then append the substring of that node's data from this's start
    //    offset until the end to string.
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

    // 4. Append the concatenation of the data of all Text nodes that are contained in this, in tree order, to
    //    string. (They follow the start node, within the common ancestor.)
    auto *root = CommonAncestorContainer();
    for (auto *node = TreeQueries::Next(*_start.Container, root); node; node = TreeQueries::Next(*node, root))
    {
      if (!Is<Text>(node) || !IsContained(*node))
      {
        continue;
      }

      string += Downcast<Text>(*node).Data();
    }

    // 5. If this's end node is a Text node, then append the substring of that node's data from its start
    // until
    //    this's end offset to string.
    if (auto *endTextNode = DynamicDowncast<Text>(_end.Container.get()))
    {
      auto substringResult = endTextNode->SubstringData(0, _end.Offset);
      if (substringResult.HasException())
      {
        return substringResult.ReleaseException();
      }
      string += substringResult.ReleaseValue();
    }

    // 6. Return string.
    return string;
  }

#pragma endregion

#pragma region Range (DOM Parsing)

  // https://html.spec.whatwg.org/#dom-range-createcontextualfragment
  ExceptionOr<Ref<DocumentFragment>> Range::CreateContextualFragment(DOMStringView markup) const noexcept
  {
    // 1. Let node be this's start node.
    Ref<Node> node = ShareRef(*StartContainer());

    // 2. Let (compliantString, compliantOptions) be the result of invoking the get trusted type compliant
    //    input algorithm with this's relevant global object, string, a new SetHTMLUnsafeOptions dictionary
    //    whose runScripts member is set to true and whose other members are set to their default values,
    //    "Range createContextualFragment", node's node document's type, and false.
    // (There are no Trusted Types, and no scripts to run.)

    // 3. Let element be null.
    RefPtr<Element> element;
    // 4. If node implements Element, set element to node.
    if (auto *nodeElement = DynamicDowncast<Element>(node.get()))
    {
      element = ShareRefPtr(nodeElement);
    }
    // 5. Otherwise, if node implements Text or Comment, set element to node's parent element.
    else if (Is<Text>(*node) || Is<Comment>(*node))
    {
      element = ShareRefPtr(node->ParentElement());
    }

    // 6. If element is null or all of the following are true: element's node document is an HTML document;
    //    element's local name is "html"; and element's namespace is the HTML namespace, then set element to
    //    the result of creating an element given node's node document, "body", and the HTML namespace.
    if (!element || (Is<HTMLDocument>(element->NodeDocument()) && Is<html::HTMLHtmlElement>(*element)))
    {
      element = ElementFactory::CreateHTMLElement<html::HTMLBodyElement>(node->NodeDocument(), u8"body");
    }

    // 7. Return the result of invoking the fragment parsing algorithm steps given element, compliantString,
    //    compliantOptions, and Legacy.
    return ::krys::boo::CreateContextualFragment(*element, markup);
  }

#pragma endregion

  ExceptionOr<void> Range::SetStartBoundaryPoint(Node &node, uint64 offset) noexcept
  {
    // Set the start:
    // 1. If node is a doctype, then throw an "InvalidNodeTypeError" DOMException.
    if (Is<DocumentType>(node))
    {
      return Exception(ExceptionCode::InvalidNodeTypeError);
    }

    // 2. If offset is greater than node's length, then throw an "IndexSizeError" DOMException.
    if (offset > TreeQueries::Length(node))
    {
      return Exception(ExceptionCode::IndexSizeError);
    }

    // The range moves to the live ranges of node's document.
    if (&_start.Container->NodeDocument() != &node.NodeDocument())
    {
      LiveRangeUpdater::RootChanged(_start.Container->NodeDocument(), node.NodeDocument(), *this);
    }

    // 3. Let boundaryPoint be the boundary point (node, offset).
    BoundaryPoint boundaryPoint {ShareRef(node), offset};

    // 4a. If range's root is not node's root or boundaryPoint is after range's end, then set range's end to
    //     boundaryPoint.
    if (!TreeQueries::SameRoot(node, *_start.Container)
        || boundaryPoint.ComparePositionTo(_end) == std::strong_ordering::greater)
    {
      _end = boundaryPoint;
    }

    // 4b. Set range's start to boundaryPoint.
    _start = krys::move(boundaryPoint);
    UpdateAssociatedSelection();
    return {};
  }

  ExceptionOr<void> Range::SetEndBoundaryPoint(Node &node, uint64 offset) noexcept
  {
    // Set the end:
    // 1. If node is a doctype, then throw an "InvalidNodeTypeError" DOMException.
    if (Is<DocumentType>(node))
    {
      return Exception(ExceptionCode::InvalidNodeTypeError);
    }

    // 2. If offset is greater than node's length, then throw an "IndexSizeError" DOMException.
    if (offset > TreeQueries::Length(node))
    {
      return Exception(ExceptionCode::IndexSizeError);
    }

    // The range moves to the live ranges of node's document.
    if (&_start.Container->NodeDocument() != &node.NodeDocument())
    {
      LiveRangeUpdater::RootChanged(_start.Container->NodeDocument(), node.NodeDocument(), *this);
    }

    // 3. Let boundaryPoint be the boundary point (node, offset).
    BoundaryPoint boundaryPoint {ShareRef(node), offset};

    // 4a. If range's root is not node's root or boundaryPoint is before range's start, then set range's start
    //     to boundaryPoint.
    if (!TreeQueries::SameRoot(node, *_start.Container)
        || boundaryPoint.ComparePositionTo(_start) == std::strong_ordering::less)
    {
      _start = boundaryPoint;
    }

    // 4b. Set range's end to boundaryPoint.
    _end = krys::move(boundaryPoint);
    UpdateAssociatedSelection();
    return {};
  }

  bool Range::IsContained(Node &node) const noexcept
  {
    // A node node is contained in a live range range if node's root is range's root, and (node, 0) is after
    // range's start, and (node, node's length) is before range's end. (Callers pass nodes in range's root.)
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
    // A node is partially contained in a live range if it's an inclusive ancestor of the live range's start
    // node but not its end node, or vice versa.
    bool isStartInclusive = TreeQueries::IsInclusiveAncestor(node, *_start.Container);
    bool isEndInclusive = TreeQueries::IsInclusiveAncestor(node, *_end.Container);
    return isStartInclusive ^ isEndInclusive;
  }

  Node *Range::FirstPartiallyContainedChild(Node &commonAncestor, Node &startContainer,
                                            Node &endContainer) const noexcept
  {
    if (!TreeQueries::IsInclusiveAncestor(startContainer, endContainer))
    {
      for (Node *child = commonAncestor.FirstChild(); child; child = child->NextSibling())
      {
        if (IsPartiallyContained(*child))
        {
          return child;
        }
      }
    }

    return nullptr;
  }

  Node *Range::LastPartiallyContainedChild(Node &commonAncestor, Node &startContainer,
                                           Node &endContainer) const noexcept
  {
    if (!TreeQueries::IsInclusiveAncestor(endContainer, startContainer))
    {
      for (Node *child = commonAncestor.LastChild(); child; child = child->PreviousSibling())
      {
        if (IsPartiallyContained(*child))
        {
          return child;
        }
      }
    }

    return nullptr;
  }

  ExceptionOr<SmallNodeList> Range::GetContainedChildren(Node *commonAncestor) const noexcept
  {
    SmallNodeList containedChildren;

    for (Node *child = commonAncestor->FirstChild(); child; child = child->NextSibling())
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
