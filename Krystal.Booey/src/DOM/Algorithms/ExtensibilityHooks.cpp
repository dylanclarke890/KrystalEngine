/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Peter Kelly (pmk@post.com)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 *           (C) 2007 David Smith (catfish.man@gmail.com)
 * Copyright (C) 2004-2025 Apple Inc. All rights reserved.
 *           (C) 2007 Eric Seidel (eric@webkit.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Krystal.Booey/DOM/Algorithms/ExtensibilityHooks.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Internals/IdTargetObserverRegistry.hpp"
#include "Krystal.Booey/DOM/Internals/UserActionElementSet.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLMapElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"
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

    /// @brief Forgets the directionality @p element kept from the parent it left.
    void ResetEffectiveTextDirectionAfterRemoval(Element &element) noexcept
    {
      if (element.UsesEffectiveTextDirection()) KRYS_UNLIKELY
      {
        if (!element.HasValidTextDirectionState())
        {
          if (auto *parent = TreeQueries::ParentOrShadowHostElement(element);
              !(parent && parent->UsesEffectiveTextDirection()))
          {
            element.SetUsesEffectiveTextDirection(false);
          }
        }
      }
    }

    /// @brief Updates the directionality of @p element as it joins its parent.
    void UpdateEffectiveTextDirectionAfterInsertion(Element &element) noexcept
    {
      // A slot's directionality is updated once it's inserted, from its assigned nodes.
      if (!Is<html::HTMLSlotElement>(element))
      {
        element.UpdateEffectiveTextDirectionIfNeeded();
      }
    }

    void UpdateSlotEffectiveTextDirectionAfterInsertion(Node &node) noexcept
    {
      if (auto *slot = DynamicDowncast<html::HTMLSlotElement>(node);
          slot && slot->SelfOrPrecedingNodesAffectDirAuto())
      {
        slot->UpdateEffectiveTextDirection();
      }
    }
  }

  namespace
  {
    KRYS_NODISCARD bool IsInTreeScope(const Node &node) noexcept
    {
      return node.IsConnected() || node.IsInShadowTree();
    }

    /// @brief Calls @p onId with each element of the subtree that has an ID, and @p onMapName with each name
    /// a map in it goes by.
    template <typename TOnId, typename TOnMapName>
    void ForEachTreeScopeKeyInSubtree(Node &subtreeRoot, TOnId onId, TOnMapName onMapName) noexcept
    {
      // The nodes in shadow trees below stay in their own scopes.
      for (auto &node : InclusiveDescendantRange(subtreeRoot))
      {
        auto *element = DynamicDowncast<Element>(node);
        if (!element)
        {
          continue;
        }

        auto id = element->Id();
        if (!id.empty())
        {
          onId(*element, DOMStringAtom(id));
        }

        if (auto *map = DynamicDowncast<html::HTMLMapElement>(*element))
        {
          if (!id.empty())
          {
            onMapName(*map, DOMStringAtom(id));
          }
          if (auto name = map->Name(); !name.empty())
          {
            onMapName(*map, DOMStringAtom(name));
          }
        }
      }
    }
  }

  void ExtensibilityHooks::SubtreeInserted(Node &node, ContainerNode &parent) noexcept
  {
    parent.InvalidateLiveCollectionCachesForChildChange(node);

    if (IsInTreeScope(parent))
    {
      auto &scope = parent.TreeScope();
      ForEachTreeScopeKeyInSubtree(
        node, [&](Element &element, DOMStringAtom id) { scope.AddElementById(id, element); },
        [&](html::HTMLMapElement &map, DOMStringAtom name) { scope.AddImageMap(name, map); });
    }
  }

  void ExtensibilityHooks::SubtreeRemoved(Node &node, ContainerNode &parent) noexcept
  {
    parent.InvalidateLiveCollectionCachesForChildChange(node);

    if (IsInTreeScope(parent))
    {
      auto &scope = parent.TreeScope();
      ForEachTreeScopeKeyInSubtree(
        node, [&](Element &element, DOMStringAtom id) { scope.RemoveElementById(id, element); },
        [&](html::HTMLMapElement &map, DOMStringAtom name) { scope.RemoveImageMap(name, map); });
    }
  }

  void ExtensibilityHooks::SubtreeMoved(Node &node, ContainerNode &oldParent,
                                        ContainerNode &newParent) noexcept
  {
    oldParent.InvalidateLiveCollectionCachesForChildChange(node);
    newParent.InvalidateLiveCollectionCachesForChildChange(node);

    auto *oldScope = IsInTreeScope(oldParent) ? &oldParent.TreeScope() : nullptr;
    auto *newScope = IsInTreeScope(newParent) ? &newParent.TreeScope() : nullptr;
    if (oldScope == newScope)
    {
      if (oldScope)
      {
        ForEachTreeScopeKeyInSubtree(
          node, [&](Element &, DOMStringAtom id) { oldScope->ElementsWithIdChangedOrder(id); },
          [&](html::HTMLMapElement &, DOMStringAtom name) { oldScope->ImageMapsChangedOrder(name); });
      }
      return;
    }

    if (oldScope)
    {
      SubtreeRemoved(node, oldParent);
    }
    if (newScope)
    {
      SubtreeInserted(node, newParent);
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
      insertedNode._containingShadowRoot = parent->_containingShadowRoot;
    }

    insertedNode.InvalidateStyle(style::Validity::SubtreeInvalid,
                                 style::InvalidationMode::InsertedIntoAncestor);
    if (parent->IsConnected() || parent->IsInShadowTree())
    {
      NotifyIdTargetObservers(insertedNode, insertedNode.TreeScope());
    }
    if (auto *element = DynamicDowncast<Element>(insertedNode))
    {
      UpdateEffectiveTextDirectionAfterInsertion(*element);
    }
    insertedNode.OnInsert();
    UpdateSlotEffectiveTextDirectionAfterInsertion(insertedNode);
  }

  void ExtensibilityHooks::NodePostConnection(Node &connectedNode) noexcept
  {
    connectedNode.OnPostConnection();
  }

  void ExtensibilityHooks::NodeChildrenChanged(ContainerNode &node,
                                               bool onlyReplacedElementsInserted) noexcept
  {
    node.OnChildrenChanged();

    if (node.NodeDocument().IsDirAttributeDirty()) KRYS_UNLIKELY
    {
      // Inserting a replaced Element (image, canvas, input, etc) should be treated as a neutral character.
      if (auto *element = DynamicDowncast<Element>(node);
          element && element->SelfOrPrecedingNodesAffectDirAuto() && !onlyReplacedElementsInserted)
      {
        element->UpdateEffectiveTextDirection();
      }
    }
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
    movedNode._containingShadowRoot = parent->_containingShadowRoot;

    movedNode.InvalidateStyle(style::Validity::SubtreeInvalid, style::InvalidationMode::InsertedIntoAncestor);
    if (auto &newScope = movedNode.TreeScope(); &newScope != &oldScope)
    {
      NotifyIdTargetObservers(movedNode, oldScope);
      NotifyIdTargetObservers(movedNode, newScope);
    }
    if (auto *element = DynamicDowncast<Element>(movedNode))
    {
      ResetEffectiveTextDirectionAfterRemoval(*element);
      UpdateEffectiveTextDirectionAfterInsertion(*element);
    }
    movedNode.OnMove(isSubtreeRoot, oldAncestor);
    UpdateSlotEffectiveTextDirectionAfterInsertion(movedNode);
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
      ResetEffectiveTextDirectionAfterRemoval(*element);

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
      removedNode._containingShadowRoot = nullptr;
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
