#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Document/Document.hpp"
#include "Krystal.HTML/Document/ShadowRoot.hpp"
#include "Krystal.HTML/Element/Element.hpp"
#include "Krystal.HTML/Node/CharacterData.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/NodeTraversal.hpp"

namespace Krys::HTML
{
  Node::Node(Document &document, NodeType type, NodeFlag flags) noexcept
      : EventTarget(ConstructNodeTag {}), _nodeType(type), _ownerDocument(RefPtrRetain(&document)),
        _parentNode(nullptr), _previousSibling(nullptr), _nextSibling(nullptr),
        _treeScope((IsDocumentNode() || IsShadowRootNode()) ? nullptr : &document)
  {
    _flags = flags;
  }

  const Node &Node::GetRootNode(const GetRootNodeOptions &options) const noexcept
  {
    return options.Composed ? ShadowIncludingRoot() : Root();
  }

  Node &Node::GetRootNode(const GetRootNodeOptions &options) noexcept
  {
    return options.Composed ? ShadowIncludingRoot() : Root();
  }

  const Node &Node::Root() const noexcept
  {
    if (IsInTreeScope())
    {
      return GetTreeScope().RootNode();
    }

    return NodeTraversal::Root(*this);
  }

  Node &Node::Root() noexcept
  {
    if (IsInTreeScope())
    {
      return GetTreeScope().RootNode();
    }

    return NodeTraversal::Root(*this);
  }

  /// @see https://dom.spec.whatwg.org/#concept-shadow-including-root
  const Node &Node::ShadowIncludingRoot() const noexcept
  {
    const auto &root = this->Root();
    if (const auto *shadowRoot = DynamicDowncast<ShadowRoot>(root))
    {
      const auto *host = shadowRoot->Host();
      return host ? host->ShadowIncludingRoot() : root;
    }

    return root;
  }

  /// @see https://dom.spec.whatwg.org/#concept-shadow-including-root
  Node &Node::ShadowIncludingRoot() noexcept
  {
    auto &root = this->Root();
    if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(root))
    {
      auto *host = shadowRoot->Host();
      return host ? host->ShadowIncludingRoot() : root;
    }

    return root;
  }

  RawPtr<Element> Node::ParentElement() const noexcept
  {
    return DynamicDowncast<Element>(ParentNode());
  }

  bool Node::HasChildNodes() const noexcept
  {
    return FirstChild() != nullptr;
  }

  RefPtr<NodeList> Node::ChildNodes() noexcept
  {
    return nullptr;
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
    if (auto *containerNode = DynamicDowncast<ContainerNode>(*this))
    {
      return containerNode->CountChildNodes();
    }

    return 0;
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
}