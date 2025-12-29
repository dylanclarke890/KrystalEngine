#include "Krystal.UI/Layout/Algorithm/Cache.hpp"
#include "Krystal.UI/Layout/Algorithm/CalculateLayout.hpp"
#include "Krystal.UI/Layout/Api/Forward.hpp"
#include "Krystal.UI/Layout/Api/Node.hpp"
#include "Krystal.UI/Layout/Event/Event.hpp"
#include "Krystal.UI/Layout/Node/Node.hpp"

namespace Krys::UI
{
  NodeRef NodeCreate()
  {
    return NodeCreate(&Config::Defaults());
  }

  NodeRef NodeCreate(ConfigConstRef config)
  {
    auto *node = new Node {config};
    assert(config != nullptr && "Tried to construct YGNode with null config");
    Event::Publish<Event::NodeAllocation>(node, {config});

    return node;
  }

  NodeRef NodeClone(NodeConstRef oldNode)
  {
    const auto node = new Node(*oldNode);
    Event::Publish<Event::NodeAllocation>(node, {node->GetConfig()});
    node->SetOwner(nullptr);
    return node;
  }

  void NodeDestroy(const NodeRef node)
  {
    if (auto owner = node->GetOwner())
    {
      owner->RemoveChild(node);
      node->SetOwner(nullptr);
    }

    const size_t childCount = node->GetChildCount();
    for (size_t i = 0; i < childCount; i++)
    {
      auto child = node->GetChild(i);
      child->SetOwner(nullptr);
    }

    node->ClearChildren();

    Event::Publish<Event::NodeDeallocation>(node, {node->GetConfig()});
    delete node;
  }

  void NodeDestroyRecursive(NodeRef root)
  {
    size_t skipped = 0;
    while (root->GetChildCount() > skipped)
    {
      const auto child = root->GetChild(skipped);
      if (child->GetOwner() != root)
      {
        // Don't free shared nodes that we don't own.
        skipped += 1;
      }
      else
      {
        root->RemoveChild(child);
        NodeDestroyRecursive(child);
      }
    }
    NodeDestroy(root);
  }

  void NodeFinalize(const NodeRef node)
  {
    Event::Publish<Event::NodeDeallocation>(node, {node->GetConfig()});
    delete node;
  }

  void NodeReset(NodeRef node)
  {
    assert(node != nullptr);
    node->Reset();
  }

  void NodeCalculateLayout(NodeRef node, float ownerWidth, float ownerHeight, Direction ownerDirection)
  {
    CalculateLayout(node, ownerWidth, ownerHeight, ownerDirection);
  }

  bool NodeGetHasNewLayout(NodeConstRef node)
  {
    return node->GetHasNewLayout();
  }

  void NodeSetHasNewLayout(NodeRef node, bool hasNewLayout)
  {
    node->SetHasNewLayout(hasNewLayout);
  }

  bool NodeIsLayoutDirty(NodeConstRef node)
  {
    return node->IsLayoutDirty();
  }

  bool NodeIsStyleDirty(NodeConstRef node)
  {
    return node->IsStyleDirty();
  }

  void NodeMarkLayoutDirty(const NodeRef node)
  {
    assert(node->HasMeasureFunc()
           && "Only leaf nodes with custom measure functions should manually mark themselves as dirty");

    node->MarkLayoutDirtyAndPropagate();
  }

  void NodeSetStyleDirty(const NodeRef node, bool isDirty)
  {
    node->SetStyleDirty(isDirty);
  }

  void NodeSetLayoutDirtiedFunc(NodeRef node, DirtiedFunc dirtiedFunc)
  {
    node->SetLayoutDirtiedFunc(dirtiedFunc);
  }

  DirtiedFunc NodeGetLayoutDirtiedFunc(NodeConstRef node)
  {
    return node->GetLayoutDirtiedFunc();
  }

  void NodeSetStyleDirtiedFunc(NodeRef node, DirtiedFunc dirtiedFunc)
  {
    node->SetStyleDirtiedFunc(dirtiedFunc);
  }

  DirtiedFunc NodeGetStyleDirtiedFunc(NodeConstRef node)
  {
    return node->GetStyleDirtiedFunc();
  }

  void NodeInsertChild(const NodeRef owner, const NodeRef child, const size_t index)
  {
    assert(child->GetOwner() == nullptr && "Child already has a owner, it must be removed first.");

    assert(!owner->HasMeasureFunc()
           && "Cannot add child: Nodes with measure functions cannot have children.");

    owner->InsertChild(child, index);
    child->SetOwner(owner);
    owner->MarkLayoutDirtyAndPropagate();
  }

  void NodeSwapChild(const NodeRef owner, const NodeRef child, const size_t index)
  {
    owner->ReplaceChild(child, index);
    child->SetOwner(owner);
  }

  void NodeRemoveChild(const NodeRef owner, const NodeRef excludedChild)
  {
    if (owner->GetChildCount() == 0)
    {
      return; // This is an empty set. Nothing to remove.
    }

    // Children may be shared between parents, which is indicated by not having an owner. We only want to
    // reset the child completely if it is owned exclusively by one node.
    auto childOwner = excludedChild->GetOwner();
    if (owner->RemoveChild(excludedChild))
    {
      if (owner == childOwner)
      {
        excludedChild->SetLayout({}); // layout is no longer valid
        excludedChild->SetOwner(nullptr);
      }
      owner->MarkLayoutDirtyAndPropagate();
    }
  }

  void NodeRemoveAllChildren(const NodeRef owner)
  {
    const size_t childCount = owner->GetChildCount();
    if (childCount == 0)
    {
      return; // This is an empty set already. Nothing to do.
    }

    auto *firstChild = owner->GetChild(0);
    if (firstChild->GetOwner() == owner)
    {
      // If the first child has this node as its owner, we assume that this child
      // set is unique.
      for (size_t i = 0; i < childCount; i++)
      {
        Node *oldChild = owner->GetChild(i);
        oldChild->SetLayout({}); // layout is no longer valid
        oldChild->SetOwner(nullptr);
      }
      owner->ClearChildren();
      owner->MarkLayoutDirtyAndPropagate();
    }
    else
    {
      // Otherwise, we are not the owner of the child set. We don't have to do
      // anything to clear it.
      owner->SetChildren({});
      owner->MarkLayoutDirtyAndPropagate();
    }
  }

  void NodeSetChildren(const NodeRef owner, const NodeRef *children, const size_t count)
  {
    if (owner == nullptr)
    {
      return;
    }

    const List<Node *> childrenVector = {children, children + count};
    if (childrenVector.empty())
    {
      if (owner->GetChildCount() > 0)
      {
        for (auto *child : owner->GetChildren())
        {
          child->SetLayout({});
          child->SetOwner(nullptr);
        }
        owner->SetChildren({});
        owner->MarkLayoutDirtyAndPropagate();
      }
    }
    else
    {
      if (owner->GetChildCount() > 0)
      {
        for (auto *oldChild : owner->GetChildren())
        {
          // Our new children may have nodes in common with the old children. We
          // don't reset these common nodes.
          if (std::find(childrenVector.begin(), childrenVector.end(), oldChild) == childrenVector.end())
          {
            oldChild->SetLayout({});
            oldChild->SetOwner(nullptr);
          }
        }
      }
      owner->SetChildren(childrenVector);
      for (Node *child : childrenVector)
      {
        child->SetOwner(owner);
      }
      owner->MarkLayoutDirtyAndPropagate();
    }
  }

  NodeRef NodeGetChild(const NodeRef node, const size_t index)
  {
    if (index < node->GetChildren().size())
    {
      return node->GetChild(index);
    }
    return nullptr;
  }

  size_t NodeGetChildCount(NodeConstRef node)
  {
    return node->GetChildCount();
  }

  NodeRef NodeGetOwner(NodeRef node)
  {
    return node->GetOwner();
  }

  NodeRef NodeGetParent(NodeRef node)
  {
    return node->GetOwner();
  }

  void NodeSetConfig(NodeRef node, ConfigRef config)
  {
    node->SetConfig(config);
  }

  ConfigConstRef NodeGetConfig(NodeConstRef node)
  {
    return node->GetConfig();
  }

  void NodeSetContext(NodeRef node, void *context)
  {
    node->SetContext(context);
  }

  void *NodeGetContext(NodeConstRef node)
  {
    return node->GetContext();
  }

  void NodeSetMeasureFunc(NodeRef node, MeasureFunc measureFunc)
  {
    node->SetMeasureFunc(measureFunc); // TODO: should we mark dirty?
  }

  bool NodeHasMeasureFunc(NodeConstRef node)
  {
    return node->HasMeasureFunc();
  }

  void NodeSetBaselineFunc(NodeRef node, BaselineFunc baselineFunc)
  {
    node->SetBaselineFunc(baselineFunc); // TODO: should we mark dirty?
  }

  bool NodeHasBaselineFunc(NodeConstRef node)
  {
    return node->HasBaselineFunc();
  }

  void NodeSetIsReferenceBaseline(NodeRef node, bool isReferenceBaseline)
  {
    if (node->IsReferenceBaseline() != isReferenceBaseline)
    {
      node->SetIsReferenceBaseline(isReferenceBaseline);
      node->MarkLayoutDirtyAndPropagate();
    }
  }

  bool NodeIsReferenceBaseline(NodeConstRef node)
  {
    return node->IsReferenceBaseline();
  }

  void NodeSetNodeType(NodeRef node, NodeType nodeType)
  {
    node->SetNodeType(nodeType);
  }

  NodeType NodeGetNodeType(NodeConstRef node)
  {
    return node->GetNodeType();
  }

  void NodeSetAlwaysFormsContainingBlock(NodeRef node, bool alwaysFormsContainingBlock)
  {
    node->SetAlwaysFormsContainingBlock(alwaysFormsContainingBlock);
  }

  bool NodeGetAlwaysFormsContainingBlock(NodeConstRef node)
  {
    return node->AlwaysFormsContainingBlock();
  }
}