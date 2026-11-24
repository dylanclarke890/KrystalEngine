#include "Krystal.Booey/DOM/Nodes/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Core/Containers/List.hpp"

namespace krys::boo
{
  namespace
  {
    /// @brief Nodes whose tree references are being dropped, and whether they're being dropped now: a node
    /// deleted while dropping them adds its children here rather than recursing, so deep trees don't overflow
    /// the stack.
    struct DetachedChildrenQueue
    {
      List<Node *> Nodes;
      bool IsDraining {false};
    };

    DetachedChildrenQueue &GetDetachedChildrenQueue() noexcept
    {
      static DetachedChildrenQueue queue;
      return queue;
    }
  }

  ContainerNode::ContainerNode(Document &document, dom::NodeType type, dom::NodeFlags flags) noexcept
      : Node(document, type, flags | dom::NodeFlag::IsContainerNode), _firstChild(nullptr),
        _lastChild(nullptr)
  {
  }

  ContainerNode::~ContainerNode() noexcept
  {
    RemoveDetachedChildren();
  }

  void ContainerNode::RemoveDetachedChildren() noexcept
  {
    auto &queue = GetDetachedChildrenQueue();
    _lastChild = nullptr;
    while (auto *child = _firstChild.get())
    {
      _firstChild = child->_nextSibling;
      child->_nextSibling = nullptr;
      child->_previousSibling = nullptr;
      child->_parentNode = nullptr;
      child->_nodeFlags.Remove(dom::NodeFlag::IsConnected);
      queue.Nodes.push_back(child);
    }

    if (queue.IsDraining)
    {
      return;
    }
    queue.IsDraining = true;
    while (!queue.Nodes.empty())
    {
      auto *node = queue.Nodes.back();
      queue.Nodes.pop_back();
      // The parent's tree reference (which may delete the node, adding its own children).
      node->SubRef();
    }
    queue.IsDraining = false;
  }
}
