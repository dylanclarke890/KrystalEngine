#include "Krystal.Booey/DOM/Algorithms/ExtensibilityHooks.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/UserActionElementSet.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeUpdater.hpp"

namespace krys::boo::dom
{
  void ExtensibilityHooks::NodeInserted(Node &insertedNode, bool isSubtreeRoot) noexcept
  {
    // NOTE: a shadow root has no parent: it's connected when its host is, and it's always in a shadow tree
    // (its own). Its host owns it, so it holds no tree reference.
    if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(insertedNode))
    {
      if (shadowRoot->Host()->IsConnected())
      {
        shadowRoot->SetNodeFlag(NodeFlags::IsConnected);
      }

      insertedNode.InvalidateStyle(style::Validity::SubtreeInvalid,
                                   style::InvalidationMode::InsertedIntoAncestor);
      insertedNode.OnInsert();
      return;
    }

    auto *parent = insertedNode.ParentNode();
    krys_debug_assert(parent);

    // The parent owns its children: the inserted node gains one tree reference. Its descendants keep the ones
    // they already hold.
    if (isSubtreeRoot)
    {
      insertedNode.AddRef();
    }

    if (parent->IsConnected())
    {
      insertedNode.SetNodeFlag(NodeFlags::IsConnected);
    }

    if (parent->IsInShadowTree())
    {
      insertedNode.SetNodeFlag(NodeFlags::IsInShadowTree);
    }

    insertedNode.InvalidateStyle(style::Validity::SubtreeInvalid,
                                 style::InvalidationMode::InsertedIntoAncestor);
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
    // A moved subtree is styled again in its new place.
    if (auto *element = DynamicDowncast<Element>(movedNode); element && isSubtreeRoot)
    {
      RenderTreeUpdater::TearDownRenderers(*element);
    }

    if (Is<ShadowRoot>(movedNode))
    {
      movedNode.OnMove(isSubtreeRoot, oldAncestor);
      return;
    }

    auto *parent = movedNode.ParentNode();
    krys_debug_assert(parent);

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

    movedNode.InvalidateStyle(style::Validity::SubtreeInvalid, style::InvalidationMode::InsertedIntoAncestor);
    movedNode.OnMove(isSubtreeRoot, oldAncestor);
  }

  void ExtensibilityHooks::NodeRemoved(Node &removedNode, bool isSubtreeRoot,
                                       ContainerNode &oldAncestor) noexcept
  {
    // The removed subtree's renderers go with it.
    if (auto *element = DynamicDowncast<Element>(removedNode); element && isSubtreeRoot)
    {
      RenderTreeUpdater::TearDownRenderers(*element);
    }

    removedNode.ClearNodeFlag(NodeFlags::IsConnected);

    if (auto *element = DynamicDowncast<Element>(removedNode))
    {
      element->NodeDocument().UserActionElements().ClearAllForElement(*element);
    }

    // Shadow roots and the nodes in their trees stay in their shadow trees; the removed node and the rest of
    // its light tree leave the shadow tree they were in, if any.
    if (!Is<ShadowRoot>(TreeQueries::Root(removedNode)))
    {
      removedNode.ClearNodeFlag(NodeFlags::IsInShadowTree);
    }

    removedNode.OnRemove(isSubtreeRoot, oldAncestor);

    // The removed node loses its parent's tree reference (which may delete it).
    if (isSubtreeRoot)
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
