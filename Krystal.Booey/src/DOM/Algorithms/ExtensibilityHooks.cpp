#include "Krystal.Booey/DOM/Algorithms/ExtensibilityHooks.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/IdTargetObserverRegistry.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/DOM/UserActionElementSet.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/PointerCaptureController.hpp"
#include "Krystal.Booey/Page/PointerLockController.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeUpdater.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"

namespace krys::boo::dom
{
  namespace
  {
    /// @brief Tells @p scope's observers of @p node's ID (if it's an element with one) that it joined or left
    /// the scope.
    void NotifyIdTargetObservers(Node &node, DocumentOrShadowRoot &scope) noexcept
    {
      auto *element = DynamicDowncast<Element>(node);
      if (!element)
      {
        return;
      }
      auto id = element->GetAttribute(u8"id");
      if (id && !id->empty())
      {
        scope.IdTargetObserverRegistry().NotifyObservers(*element, DOMStringAtom(*id));
      }
    }
  }

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
    if (parent->IsConnected() || parent->IsInShadowTree())
    {
      NotifyIdTargetObservers(insertedNode, insertedNode.TreeScope());
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
    if (Is<ShadowRoot>(movedNode))
    {
      movedNode.OnMove(isSubtreeRoot, oldAncestor);
      return;
    }

    auto *parent = movedNode.ParentNode();
    krys_debug_assert(parent);
    auto &oldScope = oldAncestor.TreeScope();

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
    if (auto &newScope = movedNode.TreeScope(); &newScope != &oldScope)
    {
      NotifyIdTargetObservers(movedNode, oldScope);
      NotifyIdTargetObservers(movedNode, newScope);
    }
    movedNode.OnMove(isSubtreeRoot, oldAncestor);
  }

  void ExtensibilityHooks::DestroyRenderTreeIfNeeded(Node &child) noexcept
  {
    if (auto *element = DynamicDowncast<Element>(child))
    {
      RenderTreeUpdater::TearDownRenderers(*element);
    }
    else if (auto *text = DynamicDowncast<Text>(child))
    {
      RenderTreeUpdater::TearDownRenderer(*text);
    }
  }

  void ExtensibilityHooks::NodeRemoved(Node &removedNode, bool isSubtreeRoot,
                                       ContainerNode &oldAncestor) noexcept
  {
    bool wasConnected = removedNode.IsConnected();
    bool wasInTreeScope = wasConnected || removedNode.IsInShadowTree();
    removedNode.ClearNodeFlag(NodeFlags::IsConnected);

    if (auto *element = DynamicDowncast<Element>(removedNode))
    {
      if (auto *page = element->NodeDocument().GetPage())
      {
        page->GetPointerLockController().ElementWasRemoved(*element);
        page->GetPointerCaptureController().ElementWasRemoved(*element);
      }
      style::Styleable::FromElement(*element).ElementWasRemoved();
      element->NodeDocument().UserActionElements().ClearAllForElement(*element);
      element->SetSavedLayerScrollPosition({});
      element->ClearBeforePseudoElement();
      element->ClearAfterPseudoElement();

      if (wasConnected)
      {
        if (element->IsInTopLayer()) KRYS_UNLIKELY
        {
          element->RemoveFromTopLayer();
        }

        auto &oldDocument = element->NodeDocument();
        if (element->LastRememberedLogicalWidth() || element->LastRememberedLogicalHeight())
        {
          // The disconnected element could be unobserved because of other properties, here we need to make
          // sure it is observed, so that deliver could be triggered and it would clear lastRememberedSize.
          oldDocument.ObserveForContainIntrinsicSize(*element);
          oldDocument.ResetObservationSizeForContainIntrinsicSize(*element);
        }
        if (oldDocument.CSSTarget() == element)
        {
          oldDocument.SetCSSTarget(nullptr);
        }
      }
    }

    // Shadow roots and the nodes in their trees stay in their shadow trees; the removed node and the rest of
    // its light tree leave the shadow tree they were in, if any.
    if (!Is<ShadowRoot>(TreeQueries::Root(removedNode)))
    {
      removedNode.ClearNodeFlag(NodeFlags::IsInShadowTree);
    }

    // Nodes in the shadow trees of the removed nodes stay in their trees.
    if (wasInTreeScope && !Is<ShadowRoot>(TreeQueries::Root(removedNode)))
    {
      NotifyIdTargetObservers(removedNode, oldAncestor.TreeScope());
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
