/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 2004-2005 Allan Sandfeld Jensen (kde@carewolf.com)
 * Copyright (C) 2006, 2007 Nicholas Shanks (webkit@nickshanks.com)
 * Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Alexey Proskuryakov <ap@webkit.org>
 * Copyright (C) 2007, 2008 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2008, 2009 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
 * Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 * Copyright (C) Research In Motion Limited 2011. All rights reserved.
 * Copyright (C) 2012 Google Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/BloomFilter.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::dom
{
  class ContainerNode;
  class Element;
}

namespace krys::boo::css
{
  class Selector;

  /// @brief A bloom filter of the identifiers (tag names, IDs, classes and attribute names) of the ancestors
  /// of the element being styled, for fast rejection of descendant and child selectors.
  class SelectorFilter
  {
  public:
    using Hashes = Array<uint32, 4>;

    struct CollectedSelectorHashes
    {
      using HashVector = SmallList<uint32, 8>;
      HashVector Ids;
      HashVector Classes;
      HashVector Tags;
      HashVector Attributes;
    };

  private:
    enum class IncludeRightmost : bool
    {
      No,
      Yes
    };

    struct ParentStackFrame
    {
      const dom::Element *Element {nullptr};
      SmallList<uint32, 4> IdentifierHashes;
    };

    List<ParentStackFrame> _parentStack;

    // With 100 unique strings in the filter, 2^12 slot table has false positive rate of ~0.2%.
    static constexpr uint32 BloomFilterKeyBits = 12;
    CountingBloomFilter<BloomFilterKeyBits> _ancestorIdentifierFilter;

  public:
    void PushParent(const dom::Element *parent) noexcept;
    void PushParentInitializingIfNeeded(const dom::Element &parent) noexcept;
    void PopParent() noexcept;
    void PopParentsUntil(const dom::Element *parent) noexcept;

    KRYS_NODISCARD bool ParentStackIsEmpty() const noexcept
    {
      return _parentStack.empty();
    }

    KRYS_NODISCARD bool ParentStackIsConsistent(const dom::ContainerNode *parentNode) const noexcept;

    void ParentStackReserveInitialCapacity(size_t initialCapacity) noexcept
    {
      _parentStack.reserve(initialCapacity);
    }

    KRYS_NODISCARD bool FastRejectSelector(const Hashes &hashes) const noexcept;

    KRYS_NODISCARD static Hashes CollectHashes(const Selector &selector) noexcept;
    static void CollectElementIdentifierHashes(const dom::Element &element,
                                               SmallList<uint32, 4> &identifierHashes) noexcept;
    static void CollectSimpleSelectorHash(CollectedSelectorHashes &collectedHashes,
                                          const Selector &selector) noexcept;

  private:
    void InitializeParentStack(const dom::Element &parent) noexcept;

    static void CollectSelectorHashes(CollectedSelectorHashes &collectedHashes,
                                      const Selector &rightmostSelector,
                                      IncludeRightmost includeRightmost) noexcept;
    KRYS_NODISCARD static Hashes
      ChooseSelectorHashesForFilter(const CollectedSelectorHashes &collectedSelectorHashes) noexcept;
  };

  inline bool SelectorFilter::FastRejectSelector(const Hashes &hashes) const noexcept
  {
    for (auto hash : hashes)
    {
      if (!hash)
      {
        return false;
      }
      if (!_ancestorIdentifierFilter.MayContain(hash))
      {
        return true;
      }
    }
    return false;
  }
}
