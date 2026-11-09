/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/DOM/Internals/NodeRareData.hpp"
#include "Krystal.Booey/DOM/Abort/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Collections/HTMLCollection.hpp"
#include "Krystal.Booey/DOM/Collections/LiveHTMLCollection.hpp"
#include "Krystal.Booey/DOM/Collections/LiveNodeList.hpp"
#include "Krystal.Booey/DOM/MutationObservers/MutationObserver.hpp"
#include "Krystal.Booey/DOM/MutationObservers/TransientRegisteredObserver.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/Node.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"

namespace krys::boo::dom
{
  NodeRareData::~NodeRareData() noexcept = default;

  Ref<NodeList> NodeRareData::ChildNodes(Node &node) noexcept
  {
    if (auto childNodes = _childNodeList.lock())
    {
      return ShareRef(*childNodes);
    }

    auto childNodes = CreateRef<LiveNodeList>(
      node, [](const Node &) { return true; }, LiveCollectionInvalidation::TreeOnly,
      LiveCollectionTraversal::Children);
    _childNodeList = CreateWeakPtr<NodeList>(childNodes.get());

    return childNodes;
  }

  Ref<HTMLCollection> NodeRareData::Children(ContainerNode &parent) noexcept
  {
    if (auto children = _children.lock())
    {
      return ShareRef(*children.get());
    }

    auto children = CreateRef<LiveHTMLCollection>(
      parent, [](const Element &) { return true; }, LiveCollectionInvalidation::TreeOnly,
      LiveCollectionTraversal::Children);
    _children = CreateWeakPtr(children.get());

    return children;
  }

  WeakPtr<NodeList> &NodeRareData::LabelsNodeList() noexcept
  {
    return _labelsNodeList;
  }

  void NodeRareData::AddLiveCollectionCache(LiveCollection &collection) noexcept
  {
    _liveCollectionCaches.push_back(&collection);
  }

  void NodeRareData::RemoveLiveCollectionCache(LiveCollection &collection) noexcept
  {
    auto it = std::ranges::find(_liveCollectionCaches, &collection);
    krys_debug_assert(it != _liveCollectionCaches.end());
    *it = _liveCollectionCaches.back();
    _liveCollectionCaches.pop_back();
  }

  void NodeRareData::InvalidateLiveCollectionCaches() noexcept
  {
    // Last first: each removes itself, which moves the last into its place.
    for (auto i = _liveCollectionCaches.size(); i-- > 0;)
    {
      _liveCollectionCaches[i]->InvalidateCache();
    }
  }

  void NodeRareData::InvalidateLiveCollectionCachesForAttribute(DOMStringAtom localName) noexcept
  {
    // Last first: each that forgets its items removes itself, which moves the last into its place.
    for (auto i = _liveCollectionCaches.size(); i-- > 0;)
    {
      _liveCollectionCaches[i]->InvalidateCacheForAttribute(localName);
    }
  }

  void NodeRareData::InvalidateChildNodeListCache() noexcept
  {
    if (auto *childNodes = _childNodeList.get())
    {
      Downcast<LiveNodeList>(*childNodes).InvalidateCache();
    }
  }

  List<Ref<RegisteredObserver>> &NodeRareData::RegisteredObservers() noexcept
  {
    return _registeredObservers;
  }

  List<Ref<TransientRegisteredObserver>> &NodeRareData::TransientRegisteredObservers() noexcept
  {
    return _transientRegisteredObservers;
  }
}
