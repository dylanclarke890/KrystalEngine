/*
 * Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/DOM/Internals/TreeScopeOrderedMap.hpp"
#include "Krystal.Booey/DOM/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Nodes/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"

namespace krys::boo::dom
{
  void TreeScopeOrderedMap::Add(DOMStringAtom key, Element &element) noexcept
  {
    auto &entry = _entries[key];
    // Where it comes among the others isn't known yet.
    entry.First = entry.Count ? nullptr : &element;
    entry.Ordered.clear();
    ++entry.Count;
  }

  void TreeScopeOrderedMap::Remove(DOMStringAtom key, Element &element) noexcept
  {
    auto it = _entries.find(key);
    krys_debug_assert(it != _entries.end());
    if (it == _entries.end())
    {
      return;
    }

    if (!--it->second.Count)
    {
      _entries.erase(it);
    }
    else
    {
      if (it->second.First == &element)
      {
        it->second.First = nullptr;
      }
      it->second.Ordered.clear();
    }
  }

  void TreeScopeOrderedMap::OrderChanged(DOMStringAtom key) noexcept
  {
    if (auto it = _entries.find(key); it != _entries.end() && it->second.Count > 1)
    {
      it->second.First = nullptr;
      it->second.Ordered.clear();
    }
  }

  uint32 TreeScopeOrderedMap::Count(DOMStringAtom key) const noexcept
  {
    auto it = _entries.find(key);
    return it != _entries.end() ? it->second.Count : 0;
  }

  Element *TreeScopeOrderedMap::First(DOMStringAtom key, const ContainerNode &root,
                                      MatchFunction matches) const noexcept
  {
    auto it = _entries.find(key);
    if (it == _entries.end())
    {
      return nullptr;
    }

    auto &entry = it->second;
    if (!entry.First)
    {
      for (auto &node : DescendantRange(const_cast<ContainerNode &>(root)))
      {
        if (auto *element = DynamicDowncast<Element>(node); element && matches(*element, key.View()))
        {
          entry.First = element;
          break;
        }
      }
      krys_debug_assert(entry.First);
    }
    return entry.First;
  }

  Span<Element *const> TreeScopeOrderedMap::All(DOMStringAtom key, const ContainerNode &root,
                                                MatchFunction matches) const noexcept
  {
    auto it = _entries.find(key);
    if (it == _entries.end())
    {
      return {};
    }

    auto &entry = it->second;
    if (entry.Ordered.empty())
    {
      entry.Ordered.reserve(entry.Count);
      for (auto &node : DescendantRange(const_cast<ContainerNode &>(root)))
      {
        if (auto *element = DynamicDowncast<Element>(node); element && matches(*element, key.View()))
        {
          entry.Ordered.push_back(element);
        }
      }
      krys_debug_assert(entry.Ordered.size() == entry.Count);
    }
    return entry.Ordered;
  }
}
