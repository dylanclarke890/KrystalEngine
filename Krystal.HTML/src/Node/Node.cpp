#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/MutationObserver/RegisteredObserver.hpp"
#include "Krystal.HTML/MutationObserver/TransientRegisteredObserver.hpp"
#include "Krystal.HTML/Node/CharacterData.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"
#include "Krystal.HTML/NodeList/NodeList.hpp"
#include "Krystal.HTML/Tree/TreeMutationDispatcher.hpp"
#include "Krystal.HTML/Tree/TreeQueries.hpp"
#include "Krystal.HTML/Tree/TreeTraversal.hpp"

namespace Krys::HTML
{
  Node::Node(Document &document, NodeType type, NodeFlag flags) noexcept
      : EventTarget(EventTargetFlag::IsNode), _nodeType(type), _ownerDocument(ShareRefPtr(&document)),
        _parentNode(nullptr), _previousSibling(nullptr), _nextSibling(nullptr),
        _treeScope((IsDocumentNode() || IsShadowRootNode()) ? nullptr : &document)
  {
    _flags = flags;
  }

#pragma region Node

  URL Node::BaseURI() const noexcept
  {
    return NodeDocument().BaseURI();
  }

  RawPtr<Document> Node::OwnerDocument() const noexcept
  {
    Document &document = NodeDocument();
    return &document == this ? nullptr : &document;
  }

  Node &Node::GetRootNode(const GetRootNodeOptions &options) noexcept
  {
    return options.Composed ? TreeQueries::ShadowIncludingRoot(*this) : TreeQueries::Root(*this);
  }

  RawPtr<Element> Node::ParentElement() const noexcept
  {
    return DynamicDowncast<Element>(ParentNode());
  }

  bool Node::HasChildNodes() const noexcept
  {
    return FirstChild() != nullptr;
  }

  Ref<NodeList> Node::ChildNodes() noexcept
  {
    if (!_nodeRareData)
    {
      _nodeRareData = CreateUnique<NodeRareData>();
    }

    return _nodeRareData->ChildNodes(*this);
  }

  RawPtr<Node> Node::FirstChild() const noexcept
  {
    if (auto *containerNode = DynamicDowncast<ContainerNode>(*this))
    {
      return containerNode->FirstChild();
    }

    return nullptr;
  }

  RawPtr<Node> Node::LastChild() const noexcept
  {
    if (auto *containerNode = DynamicDowncast<ContainerNode>(*this))
    {
      return containerNode->LastChild();
    }

    return nullptr;
  }

  ExceptionOr<void> Node::Normalize() noexcept
  {
    for (auto *textNode = TreeTraversal::NextExclusiveTextNode(*this, this); textNode;
         textNode = TreeTraversal::NextExclusiveTextNode(*textNode, this))
    {
      auto length = textNode->Length();
      if (!length)
      {
        RemoveChild(*textNode);
      }
    }

    return {};
  }

  Ref<Node> Node::CloneNode(bool subtree) const noexcept
  {
    (void)subtree;
    // TODO(IMPL)
    return ShareRef<Node>(*const_cast<Node *>(this));
  }

  bool Node::IsEqualNode(RawPtr<const Node> otherNode) const noexcept
  {
    (void)otherNode;
    // TODO(IMPL)
    return false;
  }

  bool Node::IsSameNode(RawPtr<const Node> otherNode) const noexcept
  {
    return this == otherNode;
  }

  DocumentPosition Node::CompareDocumentPosition(Node &other) const noexcept
  {
    (void)other;
    // TODO(IMPL)
    return DocumentPosition::DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC;
  }

  bool Node::Contains(RawPtr<const Node> other) const noexcept
  {
    if (other == nullptr)
    {
      return false;
    }

    if (auto *container = DynamicDowncast<ContainerNode>(this))
    {
      return TreeQueries::IsInclusiveDescendantOf(*container, *other);
    }

    return false;
  }

  // const StringAtom &Node::LookupPrefix(const StringAtom &namespaceURI) const noexcept
  //{
  // }

  // const StringAtom &Node::LookupNamespaceURI(const StringAtom &prefix) const noexcept
  //{
  // }

  // bool Node::IsDefaultNamespace(const StringAtom &namespaceURI) const noexcept
  //{
  // }

  ExceptionOr<void> Node::InsertBefore(Node &newChild, RefPtr<Node> &&refChild) noexcept
  {
    if (auto *containerNode = DynamicDowncast<ContainerNode>(*this))
    {
      return containerNode->InsertBefore(newChild, Krys::Move(refChild));
    }

    return Exception {ExceptionCode::HierarchyRequestError};
  }

  ExceptionOr<void> Node::ReplaceChild(Node &newChild, Node &oldChild) noexcept
  {
    if (auto *containerNode = DynamicDowncast<ContainerNode>(*this))
    {
      return containerNode->ReplaceChild(newChild, oldChild);
    }

    return Exception {ExceptionCode::HierarchyRequestError};
  }

  ExceptionOr<void> Node::RemoveChild(Node &oldChild) noexcept
  {
    if (auto *containerNode = DynamicDowncast<ContainerNode>(*this))
    {
      return containerNode->RemoveChild(oldChild);
    }

    return Exception {ExceptionCode::NotFoundError};
  }

  ExceptionOr<void> Node::AppendChild(Node &newChild) noexcept
  {
    if (auto *containerNode = DynamicDowncast<ContainerNode>(*this))
    {
      return containerNode->AppendChild(newChild);
    }

    return Exception {ExceptionCode::HierarchyRequestError};
  }

  /// @see https://dom.spec.whatwg.org/#concept-node-length
  size_t Node::Length() const noexcept
  {
    if (auto *characterData = DynamicDowncast<CharacterData>(*this))
    {
      return characterData->Length();
    }

    return CountChildNodes();
  }

  size_t Node::CountChildNodes() const noexcept
  {
    if (const auto *containerNode = DynamicDowncast<ContainerNode>(*this))
    {
      return containerNode->CountChildNodes();
    }

    return 0;
  }

#pragma endregion

  void Node::SetTreeScopeRecursively(TreeScope &newTreeScope) noexcept
  {
    assert(!IsDocumentNode());
    if (_treeScope != &newTreeScope)
    {
      // Ref<TreeScope> oldTreeScope = CreateRef<TreeScope>(*_treeScope);
      //  MoveTreeToNewScope(*this, oldTreeScope, newTreeScope);
    }
  }

  void Node::InsertedIntoAncestor(const NodeInsertedContext &context) noexcept
  {
    if (context.ConnectedToDocument)
    {
      SetEventTargetFlag(EventTargetFlag::IsConnected);
    }

    if (context.InsertedInto.get().IsInShadowTree())
    {
      SetEventTargetFlag(EventTargetFlag::IsInShadowTree);
    }
  }

  void Node::RemovedFromAncestor(const NodeRemovedContext &context) noexcept
  {
    if (context.DisconnectedFromDocument)
    {
      ClearEventTargetFlag(EventTargetFlag::IsConnected);
    }

    if (IsInShadowTree() && !GetTreeScope().RootNode().IsShadowRootNode())
    {
      ClearEventTargetFlag(EventTargetFlag::IsInShadowTree);
    }
  }

  List<Ref<RegisteredObserver>> &Node::RegisteredObserverList() noexcept
  {
    if (!_nodeRareData)
    {
      _nodeRareData = CreateUnique<NodeRareData>();
    }

    return _nodeRareData->RegisteredObserverList();
  }

  List<Ref<TransientRegisteredObserver>> &Node::TransientRegisteredObservers() noexcept
  {
    if (!_nodeRareData)
    {
      _nodeRareData = CreateUnique<NodeRareData>();
    }

    return _nodeRareData->TransientRegisteredObservers();
  }
}