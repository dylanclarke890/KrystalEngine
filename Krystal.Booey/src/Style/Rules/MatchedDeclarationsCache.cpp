/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004-2005 Allan Sandfeld Jensen (kde@carewolf.com)
 * Copyright (C) 2006, 2007 Nicholas Shanks (webkit@nickshanks.com)
 * Copyright (C) 2005-2019 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Alexey Proskuryakov <ap@webkit.org>
 * Copyright (C) 2007, 2008 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2008, 2009 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
 * Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 * Copyright (C) Research In Motion Limited 2011. All rights reserved.
 * Copyright (C) 2012, 2013 Google Inc. All rights reserved.
 * Copyright (C) 2014 Igalia S.L.
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

#include "Krystal.Booey/Style/Rules/MatchedDeclarationsCache.hpp"
#include "Krystal.Booey/CSS/Font/CSSFontSelector.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"
#include "Krystal.Booey/Style/CustomProperty.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthResolution.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Hash.hpp"
#include <algorithm>

namespace krys::boo::style
{
  MatchedDeclarationsCache::MatchedDeclarationsCache(const Resolver &owner) noexcept
      : _owner(CreateWeakPtr(&owner)), _sweepTimer(*this, &MatchedDeclarationsCache::Sweep)
  {
  }

  MatchedDeclarationsCache::~MatchedDeclarationsCache() noexcept = default;

  bool MatchedDeclarationsCache::IsCacheable(const dom::Element &element, const RenderStyle &style,
                                             const RenderStyle &parentStyle) noexcept
  {
    // FIXME: Writing mode and direction properties modify state when applying to document element by calling
    // Document::setWritingMode/DirectionSetOnDocumentElement. We can't skip the applying by caching.
    if (&element == element.NodeDocument().DocumentElement().get())
    {
      return false;
    }
    // FIXME: Without the following early return we hit the final assert in
    // Element::resolvePseudoElementStyle(). Making matchedPseudoElements
    // PseudoElementIdentifier-aware might be a possible solution.
    if (style.PseudoElementNameArgument() != css::CSSOMStringAtom::Null())
    {
      return false;
    }
    // content:attr() value depends on the element it is being applied to.
    if (style.HasAttrContent() || (style.GetPseudoElementType() && parentStyle.HasAttrContent()))
    {
      return false;
    }
    if (style.GetZoom() != ComputedStyle::InitialZoom())
    {
      return false;
    }
    if (style.GetWritingMode().ComputedWritingMode() != ComputedStyle::InitialWritingMode()
        || style.GetWritingMode().ComputedTextDirection() != ComputedStyle::InitialDirection())
    {
      return false;
    }
    if (style.UsesContainerUnits())
    {
      return false;
    }
    if (style.UseTreeCountingFunctions())
    {
      return false;
    }
    if (style.UsesAnchorFunctions())
    {
      return false;
    }

    // Getting computed style after a font environment change but before full style resolution may involve
    // styles with non-current fonts. Avoid caching them.
    auto &fontSelector = element.NodeDocument().GetFontSelector();
    if (!style.GetFontCascade().IsCurrent(fontSelector))
    {
      return false;
    }
    if (!parentStyle.GetFontCascade().IsCurrent(fontSelector))
    {
      return false;
    }

    if (element.HasRandomCachingKeyMap())
    {
      return false;
    }

    // FIXME: counter-style: we might need to resolve cache like for fontSelector here (rdar://103018993).

    return true;
  }

  bool MatchedDeclarationsCache::Entry::IsUsableAfterHighPriorityProperties(
    const boo::RenderStyle &style) const noexcept
  {
    if (style.UsedZoom() != RenderStyle->UsedZoom())
    {
      return false;
    }

    if (style.GetColorScheme() != RenderStyle->GetColorScheme())
    {
      return false;
    }

    return EqualForLengthResolution(style, *RenderStyle);
  }

  uint32 MatchedDeclarationsCache::ComputeHash(const MatchResult &matchResult,
                                               const CustomPropertyData &inheritedCustomProperties) noexcept
  {
    if (matchResult.IsCompletelyNonCacheable)
    {
      return 0;
    }

    if (matchResult.UserAgentDeclarations.empty() && matchResult.UserDeclarations.empty())
    {
      bool allNonCacheable =
        std::ranges::all_of(matchResult.AuthorDeclarations, [](auto &matchedProperties)
                            { return matchedProperties.IsCacheable != IsCacheable::Yes; });
      // No point of caching if we are not applying any properties.
      if (allNonCacheable)
      {
        return 0;
      }
    }
    auto hash = krys::Hash::Combine(DefaultHash<MatchResult>::Hash(matchResult), &inheritedCustomProperties);
    // Zero means "not cacheable".
    return static_cast<uint32>(hash) ? static_cast<uint32>(hash) : 1;
  }

  auto MatchedDeclarationsCache::Find(uint32 hash, const MatchResult &matchResult,
                                      const CustomPropertyData &inheritedCustomProperties,
                                      const RenderStyle &parentStyle) noexcept -> Maybe<Result>
  {
    if (!hash)
    {
      return {};
    }

    auto it = _entries.find(hash);
    if (it == _entries.end())
    {
      return {};
    }

    const Entry *partiallyMatchingEntry = nullptr;
    for (auto &entry : it->second)
    {
      if (!matchResult.CacheablePropertiesEqual(*entry.MatchResult))
      {
        continue;
      }

      if (&entry.ParentRenderStyle->InheritedCustomProperties() != &inheritedCustomProperties)
      {
        continue;
      }

      if (parentStyle.InheritedEqual(*entry.ParentRenderStyle))
      {
        return Result {.Entry = entry, .InheritedEqual = true};
      }
      partiallyMatchingEntry = &entry;
    }
    if (partiallyMatchingEntry)
    {
      return Result {.Entry = *partiallyMatchingEntry, .InheritedEqual = false};
    }
    return {};
  }

  void MatchedDeclarationsCache::Add(const RenderStyle &style, const RenderStyle &parentStyle, uint32 hash,
                                     const MatchResult &matchResult) noexcept
  {
    constexpr uint32 additionsBetweenSweeps = 100;
    if (++_additionsSinceLastSweep >= additionsBetweenSweeps && !_sweepTimer.IsActive())
    {
      constexpr auto sweepDelay = std::chrono::minutes(1);
      _sweepTimer.StartOneShot(sweepDelay);
    }

    krys_debug_assert(hash);
    // Note that we don't cache the original RenderStyle instance. It may be further modified.
    // The RenderStyle in the cache is really just a holder for the substructures and never used as-is.
    constexpr size_t maxEntriesPerHash = 4;
    auto [it, inserted] = _entries.try_emplace(hash);
    if (inserted)
    {
      it->second.reserve(maxEntriesPerHash);
    }
    if (it->second.size() < maxEntriesPerHash)
    {
      it->second.push_back(
        Entry {ShareRefPtr(&matchResult), RenderStyle::ClonePtr(style), RenderStyle::ClonePtr(parentStyle)});
    }
  }

  void MatchedDeclarationsCache::Remove(uint32 hash) noexcept
  {
    _entries.erase(hash);
  }

  void MatchedDeclarationsCache::Invalidate() noexcept
  {
    _entries.clear();
  }

  template <typename TCallback>
  void MatchedDeclarationsCache::RemoveAllMatching(const TCallback &matches) noexcept
  {
    for (auto &[key, bucket] : _entries)
    {
      std::erase_if(bucket, matches);
    }
    std::erase_if(_entries, [](auto &keyValue) { return keyValue.second.empty(); });
  }

  void MatchedDeclarationsCache::ClearEntriesAffectedByViewportUnits() noexcept
  {
    RemoveAllMatching([&](const Entry &entry) -> bool { return entry.RenderStyle->UsesViewportUnits(); });
  }

  void MatchedDeclarationsCache::Sweep() noexcept
  {
    // Look for cache entries containing a style declaration with a single ref and remove them.
    // This may happen when an element attribute mutation causes it to generate a new inlineStyle()
    // or presentationalHintStyle(), potentially leaving this cache with the last ref on the old one.
    auto hasOneRef = [](auto &declarations)
    {
      for (auto &matchedProperties : declarations)
      {
        if (matchedProperties.Properties->HasOneRef())
        {
          return true;
        }
      }
      return false;
    };

    RemoveAllMatching(
      [&](const Entry &entry) -> bool
      {
        auto &matchResult = entry.MatchResult;
        return hasOneRef(matchResult->UserAgentDeclarations) || hasOneRef(matchResult->UserDeclarations)
               || hasOneRef(matchResult->AuthorDeclarations);
      });

    _additionsSinceLastSweep = 0;
  }
}
