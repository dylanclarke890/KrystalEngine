/*
 * Copyright (C) 2008-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2008 David Smith <catfish.man@gmail.com>
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
 *
 */

#pragma once

#include "Krystal.Booey/DOM/MutationObservers/MutationObserver.hpp"
#include "Krystal.Booey/DOM/MutationObservers/RegisteredObserver.hpp"
#include "Krystal.Booey/DOM/MutationObservers/TransientRegisteredObserver.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class ContainerNode;
  class HTMLCollection;
  class LiveCollection;
  class Node;
  class NodeList;

  /// @brief The data a node needs only some of the time, made when it's first needed; an element's is an
  /// `ElementRareData`.
  class NodeRareData
  {
  private:
    WeakPtr<NodeList> _childNodeList;
    WeakPtr<HTMLCollection> _children;
    WeakPtr<NodeList> _labelsNodeList;
    /// @brief The live collections rooted at the node that have cached their items. They hold the node, and
    /// remove themselves as they forget the items or are destroyed.
    List<LiveCollection *> _liveCollectionCaches;

    /// @see https://dom.spec.whatwg.org/#registered-observer-list
    List<Ref<RegisteredObserver>> _registeredObservers;

    /// @brief We track transient registered observers separately despite the spec having them in the same
    /// list as it makes it easier to inspect them later.
    /// @see https://dom.spec.whatwg.org/#registered-observer-list
    List<Ref<TransientRegisteredObserver>> _transientRegisteredObservers;

    bool _hasEverPaintedImages {false};

  public:
    virtual ~NodeRareData() noexcept;

    KRYS_NODISCARD Ref<NodeList> ChildNodes(Node &node) noexcept;

    KRYS_NODISCARD bool HasEverPaintedImages() const noexcept
    {
      return _hasEverPaintedImages;
    }

    void SetHasEverPaintedImages(bool hasEverPaintedImages) noexcept
    {
      _hasEverPaintedImages = hasEverPaintedImages;
    }

    KRYS_NODISCARD Ref<HTMLCollection> Children(ContainerNode &parent) noexcept;

    /// @brief The node's labels list, if it has made one; held weakly, as the list holds the node.
    KRYS_NODISCARD WeakPtr<NodeList> &LabelsNodeList() noexcept;

    void AddLiveCollectionCache(LiveCollection &collection) noexcept;
    void RemoveLiveCollectionCache(LiveCollection &collection) noexcept;

    /// @brief Clears the caches of the live collections rooted at the node.
    void InvalidateLiveCollectionCaches() noexcept;

    /// @brief Clears what those collections cached that depends on the attribute called @p localName.
    void InvalidateLiveCollectionCachesForAttribute(DOMStringAtom localName) noexcept;

    /// @brief Clears the cache of the node's `childNodes`, the only collection to hold nodes but elements.
    void InvalidateChildNodeListCache() noexcept;

    KRYS_NODISCARD List<Ref<RegisteredObserver>> &RegisteredObservers() noexcept;

    KRYS_NODISCARD List<Ref<TransientRegisteredObserver>> &TransientRegisteredObservers() noexcept;
  };
}