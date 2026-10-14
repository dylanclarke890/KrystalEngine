#include "Krystal.Booey/DOM/Algorithms/ExtensibilityHooks.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"

namespace krys::boo::dom
{
  void ExtensibilityHooks::NodeInserted(Node &insertedNode) noexcept
  {
    auto *parent = insertedNode.ParentNode();
    if (parent == nullptr)
    {
      return;
    }

    insertedNode.AddRef();

    if (parent->IsConnected())
    {
      insertedNode.SetNodeFlag(NodeFlags::IsConnected);
    }

    if (parent->IsInShadowTree())
    {
      insertedNode.SetNodeFlag(NodeFlags::IsInShadowTree);
    }

    insertedNode.OnInsert();
  }

  void ExtensibilityHooks::NodePostConnection(Node &connectedNode) noexcept
  {
    connectedNode.OnPostConnection();
  }

  void ExtensibilityHooks::NodeChildrenChanged(ContainerNode &node) noexcept
  {
    node.OnChildrenChanged();
  }

  void ExtensibilityHooks::NodeMoved(Node &movedNode, bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
  {
    auto *parent = movedNode.ParentNode();
    assert(parent != nullptr);

    // NOTE: a node can be moved from a shadow tree to a light tree and vice versa but only as long as
    // they share the same shadow-including root (i.e. they're in the same document). This means that
    // `IsConnected` will be the same for the node before and after the move but `IsInShadowTree` may be
    // different.

    if (parent->IsInShadowTree())
    {
      movedNode.SetNodeFlag(NodeFlags::IsInShadowTree);
    }
    else
    {
      movedNode.ClearNodeFlag(NodeFlags::IsInShadowTree);
    }

    movedNode.OnMove(isSubtreeRoot, oldAncestor);
  }

  void ExtensibilityHooks::NodeRemoved(Node &removedNode, bool isSubtreeRoot,
                                       ContainerNode &oldAncestor) noexcept
  {
    removedNode.ClearNodeFlag(NodeFlags::IsConnected);
    removedNode.ClearNodeFlag(NodeFlags::IsInShadowTree);

    removedNode.OnRemove(isSubtreeRoot, oldAncestor);

    if (removedNode.ParentNode() == nullptr)
    {
      removedNode.SubRef();
    }
  }

  void ExtensibilityHooks::NodeCloned(Node &node, Node &copy, bool subtree) noexcept
  {
    node.OnClone(copy, subtree);
  }

  void ExtensibilityHooks::NodeAdopted(Node &node, Document &oldDocument) noexcept
  {
    node.OnAdopt(oldDocument);
  }

  void ExtensibilityHooks::ElementAttributeChanged(Element &element, DOMStringAtom localName,
                                                   Maybe<DOMStringView> oldValue, Maybe<DOMStringView> value,
                                                   DOMStringAtom namespaceURI) noexcept
  {
    element.OnAttributeChanged(localName, oldValue, value, namespaceURI);
  }
}
