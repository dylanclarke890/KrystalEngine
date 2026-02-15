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
      : EventTarget(ConstructNodeTag {}), _nodeType(type), _ownerDocument(ShareRefPtr(&document)),
        _parentNode(nullptr), _previousSibling(nullptr), _nextSibling(nullptr),
        _treeScope((IsDocumentNode() || IsShadowRootNode()) ? nullptr : &document)
  {
    _flags = flags;
  }

  RawPtr<Document> Node::OwnerDocument() const noexcept
  {
    return _ownerDocument.get();
  }

  Node &Node::GetRootNode(const GetRootNodeOptions &options) noexcept
  {
    return options.Composed ? ShadowIncludingRoot() : Root();
  }

  Node &Node::Root() noexcept
  {
    if (IsInTreeScope())
    {
      auto &root = GetTreeScope().RootNode();
      return root;
    }

    return NodeTraversal::Root(*this);
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

  RawPtr<ContainerNode> Node::ParentNode() const noexcept
  {
    if (_parentNode)
    {
      return _parentNode;
    }

    return nullptr;
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

  DOMString Node::NodeValue() const noexcept
  {
    return {};
  }

  ExceptionOr<void> Node::SetNodeValue(DOMStringView) noexcept
  {
    return {};
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