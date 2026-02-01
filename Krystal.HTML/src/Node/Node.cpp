#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Document/Document.hpp"
#include "Krystal.HTML/Node/CharacterData.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"

namespace Krys::HTML
{
  Node::Node(Document &document, NodeType type, NodeFlags flags) noexcept
      : EventTarget(ConstructNodeTag {}), _nodeType(type), _ownerDocument(RefPtrRetain(&document)),
        _parentNode(nullptr), _previousSibling(nullptr), _nextSibling(nullptr)
  {
    _flags = flags;
  }

  bool Node::IsConnected() const noexcept
  {
    // TODO(IMPL): A node is connected if its shadow-including root(?) is a document.
    return false;
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
}