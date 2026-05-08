#include "Krystal.HTML/Algorithms/ExtensibilityHooks.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  void ExtensibilityHooks::Inserted(Node &insertedNode) noexcept
  {
    auto *parent = insertedNode.ParentNode();
    if (parent == nullptr)
    {
      return;
    }

    if (parent->IsConnected())
    {
      insertedNode.SetEventTargetFlag(EventTargetFlag::IsConnected);
    }

    if (parent->IsInShadowTree())
    {
      insertedNode.SetEventTargetFlag(EventTargetFlag::IsInShadowTree);
    }
  }

  void ExtensibilityHooks::PostConnection(Node &connectedNode) noexcept
  {
    // TODO(impl): post connection steps
  }

  void ExtensibilityHooks::ChildrenChanged(ContainerNode &node) noexcept
  {
    node.OnChildrenChanged();
  }

  void ExtensibilityHooks::Moved(Node &node, Node &movedNode, bool isSubtreeRoot,
                                 ContainerNode &oldAncestor) noexcept
  {
    // TODO(impl): clear and set flags on node and movedNode as necessary
  }

  void ExtensibilityHooks::Removed(Node &removedNode, bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
  {
    removedNode.ClearEventTargetFlag(EventTargetFlag::IsConnected);
    removedNode.ClearEventTargetFlag(EventTargetFlag::IsInShadowTree);
  }

  void ExtensibilityHooks::Cloned(const Node &node, Node &copy, bool subtree) noexcept
  {
    // TODO(impl): Run any cloning steps defined for node in other applicable specifications and pass node,
    // copy, and subtree as parameters.
  }

  void ExtensibilityHooks::AttributeChanged(Element &element, DOMStringAtom localName, DOMStringView oldValue,
                                            DOMStringView value, DOMStringAtom namespaceURI) noexcept
  {
    // TODO(impl): Implement this method
  }
}