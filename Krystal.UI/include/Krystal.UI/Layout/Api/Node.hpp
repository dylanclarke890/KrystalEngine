#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.UI/Layout/Api/Forward.hpp"
#include "Krystal.UI/Styles/Enums/Direction.hpp"
#include "Krystal.UI/Styles/Enums/NodeType.hpp"

namespace Krys::UI
{
  /// @brief Heap allocates and returns a new node using default settings.
  KRYS_NODISCARD NodeRef NodeCreate();

  /// @brief Heap allocates and returns a new node, with customized settings.
  KRYS_NODISCARD NodeRef NodeCreate(ConfigConstRef config);

  /// @brief Returns a mutable copy of an existing node, with the same context and children, but no owner
  /// set.Does not call the function set by ConfigSetCloneNodeFunc().
  KRYS_NODISCARD NodeRef NodeClone(NodeConstRef node);

  /// @brief Frees the node, disconnecting it from its owner and children.
  void NodeDestroy(NodeRef node);

  /// @brief Frees the subtree of Yoga nodes rooted at the given node.
  void NodeDestroyRecursive(NodeRef node);

  /// @brief Frees the node without disconnecting it from its owner or children. Allows garbage
  /// collecting nodes in parallel when the entire tree is unreachable.
  void NodeFinalize(NodeRef node);

  /// @brief Resets the node to its default state. Does not free any children or its owner.
  void NodeReset(NodeRef node);

  /// @brief Calculates the layout of the tree rooted at the given node.
  /// Layout results may be read after calling NodeCalculateLayout() using functions like NodeLayoutGetLeft(),
  /// NodeLayoutGetTop(), etc. NodeGetHasNewLayout() may be read to know if the layout of the node or its
  /// subtrees may have changed since the last time NodeCalculate() was called.
  void NodeCalculateLayout(NodeRef node, float availableWidth, float availableHeight,
                           Direction ownerDirection);

  /// @brief Whether the given node may have new layout results. Must be reset by calling
  /// NodeSetHasNewLayout().
  bool NodeGetHasNewLayout(NodeConstRef node);

  /// @brief Sets whether a nodes layout is considered new.
  void NodeSetHasNewLayout(NodeRef node, bool hasNewLayout);

  /// @brief Whether the node's layout results are dirty due to it or its children changing.
  bool NodeIsLayoutDirty(NodeConstRef node);

  /// @brief Marks a node with custom measure function as dirty.
  void NodeMarkLayoutDirty(NodeRef node);

  /// @brief Called when a change is made to the tree which dirties the layout of a node.
  void NodeSetLayoutDirtiedFunc(NodeRef node, DirtiedFunc dirtiedFunc);

  /// @brief Returns a layout dirtied func if set.
  DirtiedFunc NodeGetLayoutDirtiedFunc(NodeConstRef node);

  /// @brief Whether the node's styles are dirty due to it or its children changing.
  bool NodeIsStyleDirty(NodeConstRef node);

  /// @brief Sets whether the node's styles are dirty. Does not propagate the dirtying to children or parents.
  void NodeSetStyleDirty(const NodeRef node, bool isDirty);

  /// @brief Called when a change is made to the tree which dirties the style of a node.
  void NodeSetStyleDirtiedFunc(NodeRef node, DirtiedFunc dirtiedFunc);

  /// @brief Returns a style dirtied func if set.
  DirtiedFunc NodeGetStyleDirtiedFunc(NodeConstRef node);

  /// @brief Inserts a child node at the given index.
  void NodeInsertChild(NodeRef node, NodeRef child, size_t index);

  /// @brief Replaces the child node at a given index with a new one.
  void NodeSwapChild(NodeRef node, NodeRef child, size_t index);

  /// @brief Removes the given child node.
  void NodeRemoveChild(NodeRef node, NodeRef child);

  /// @brief Removes all children nodes.
  void NodeRemoveAllChildren(NodeRef node);

  /// @brief Sets children according to the given list of nodes.
  void NodeSetChildren(NodeRef owner, const NodeRef *children, size_t count);

  /// @brief Get the child node at a given index.
  KRYS_NODISCARD NodeRef NodeGetChild(NodeRef node, size_t index);

  /// @brief The number of child nodes.
  size_t NodeGetChildCount(NodeConstRef node);

  /// @brief Get the parent/owner currently set for a node.
  NodeRef NodeGetOwner(NodeRef node);

  /// @brief Get the parent/owner currently set for a node.
  NodeRef NodeGetParent(NodeRef node);

  /// @brief Set a new config for the node after creation.
  void NodeSetConfig(NodeRef node, ConfigRef config);

  /// @brief Get the config currently set on the node.
  ConfigConstRef NodeGetConfig(NodeConstRef node);

  /// @brief Sets extra data on the node which may be read from during callbacks.
  void NodeSetContext(NodeRef node, void *context);

  /// @brief Returns the context or NULL if no context has been set.
  void *NodeGetContext(NodeConstRef node);

  /// @brief Allows providing custom measurements for a  leaf node (usually for measuring text).
  /// NodeMarkDirty() must be set if content affecting the measurements of the node changes.
  void NodeSetMeasureFunc(NodeRef node, MeasureFunc measureFunc);

  /// @brief Whether a measure function is set.
  bool NodeHasMeasureFunc(NodeConstRef node);

  /// @brief Set a custom function for determining the text baseline for use in baseline alignment.
  void NodeSetBaselineFunc(NodeRef node, BaselineFunc baselineFunc);

  /// @brief Whether a baseline function is set.
  bool NodeHasBaselineFunc(NodeConstRef node);

  /// @brief Sets this node should be considered the reference baseline among siblings.
  void NodeSetIsReferenceBaseline(NodeRef node, bool isReferenceBaseline);

  /// @brief Whether this node is set as the reference baseline.
  bool NodeIsReferenceBaseline(NodeConstRef node);

  /// @brief Sets whether a leaf node's layout results may be truncated during layout rounding.
  void NodeSetNodeType(NodeRef node, NodeType nodeType);

  /// @brief Whether a leaf node's layout results may be truncated during layout rounding.
  NodeType NodeGetNodeType(NodeConstRef node);

  /// @brief Make it so that this node will always form a containing block for any descendant nodes. This is
  /// useful for when a node has a property outside of the engine that will form a containing block. For
  /// example, transforms or some of the others listed in
  /// https://developer.mozilla.org/en-US/docs/Web/CSS/Containing_block
  void NodeSetAlwaysFormsContainingBlock(NodeRef node, bool alwaysFormsContainingBlock);

  /// @brief Whether the node will always form a containing block for any descendant. This can happen in
  /// situation where the client implements something like a transform that can affect containing blocks but
  /// is not handled by the engine directly.
  bool NodeGetAlwaysFormsContainingBlock(NodeConstRef node);
}