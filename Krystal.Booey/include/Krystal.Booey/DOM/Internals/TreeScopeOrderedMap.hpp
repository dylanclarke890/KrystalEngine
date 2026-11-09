/*
 * Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo::dom
{
  class ContainerNode;
  class Element;

  /// @brief The elements of a tree scope under a key, such as their ID. The first in tree order, and all of
  /// them in tree order, are found by walking the tree only when more than one has the key and they aren't
  /// known already.
  /// @note The elements aren't kept alive: each is removed as it leaves the tree or stops having the key.
  class TreeScopeOrderedMap
  {
    KRYS_NON_COPY_MOVABLE(TreeScopeOrderedMap);

    struct Entry
    {
      /// @brief The first element with the key, in tree order; null if that isn't known.
      Element *First {nullptr};
      uint32 Count {0};
      /// @brief The elements with the key, in tree order; empty if that isn't known.
      List<Element *> Ordered;
    };

  private:
    mutable HashMap<DOMStringAtom, Entry> _entries;

  public:
    /// @brief Whether @p element has @p key.
    using MatchFunction = bool (*)(const Element &element, DOMStringView key) noexcept;

    TreeScopeOrderedMap() noexcept = default;

    void Add(DOMStringAtom key, Element &element) noexcept;
    void Remove(DOMStringAtom key, Element &element) noexcept;

    /// @brief Forgets the order of the elements with @p key, as it's changed.
    void OrderChanged(DOMStringAtom key) noexcept;

    /// @brief How many elements have @p key.
    KRYS_NODISCARD uint32 Count(DOMStringAtom key) const noexcept;

    /// @brief The first element in tree order with @p key, or null if there's none.
    /// @param root The root of the tree scope.
    KRYS_NODISCARD Element *First(DOMStringAtom key, const ContainerNode &root,
                                  MatchFunction matches) const noexcept;

    /// @brief The elements with @p key, in tree order.
    /// @param root The root of the tree scope.
    /// @note The elements are valid until the map or the tree next changes.
    KRYS_NODISCARD Span<Element *const> All(DOMStringAtom key, const ContainerNode &root,
                                            MatchFunction matches) const noexcept;
  };
}
