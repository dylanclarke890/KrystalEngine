#include "Krystal.HTML/Node.hpp"
#include "Krystal.HTML/ContainerNode.hpp"

namespace Krys::HTML
{
  bool Node::IsConnected() const noexcept
  {
    // TODO(IMPL): A node is connected if its shadow-including root(?) is a document.
    return false;
  }

  ExceptionOr<void> Node::InsertBefore(Node &newChild, RefPtr<Node> &&refChild)
  {
    if (auto *containerNode = DynamicDowncast<ContainerNode>(*this))
    {
      return containerNode->InsertBefore(newChild, Krys::Move(refChild));
    }

    return Exception {ExceptionCode::HierarchyRequestError};
  }

  ExceptionOr<void> Node::ReplaceChild(Node &newChild, Node &oldChild)
  {
    if (auto *containerNode = DynamicDowncast<ContainerNode>(*this))
    {
      return containerNode->ReplaceChild(newChild, oldChild);
    }

    return Exception {ExceptionCode::HierarchyRequestError};
  }

  ExceptionOr<void> Node::RemoveChild(Node &oldChild)
  {
    if (auto *containerNode = DynamicDowncast<ContainerNode>(*this))
    {
      return containerNode->RemoveChild(oldChild);
    }

    return Exception {ExceptionCode::NotFoundError};
  }

  ExceptionOr<void> Node::AppendChild(Node &newChild)
  {
    if (auto *containerNode = DynamicDowncast<ContainerNode>(*this))
    {
      return containerNode->AppendChild(newChild);
    }

    return Exception {ExceptionCode::HierarchyRequestError};
  }
}