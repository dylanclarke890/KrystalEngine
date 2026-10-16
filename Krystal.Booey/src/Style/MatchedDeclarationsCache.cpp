#include "Krystal.Booey/Style/MatchedDeclarationsCache.hpp"
#include "Krystal.Booey/CSS/CSSFontSelector.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
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
      : _owner(WeakPtr<const Resolver>::WithRef(owner))
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
    if (++_additionsSinceLastSweep >= additionsBetweenSweeps && !_isSweepQueued)
    {
      // FIXME: Upstream sweeps a minute later, on a timer. There are no timers, so the sweep is a task.
      _isSweepQueued = true;
      // The cache lives as long as its owner.
      html::EventLoop::Current().QueueTask(html::TaskSource::DOMManipulation,
                                           [owner = _owner, cache = this]
                                           {
                                             if (owner)
                                             {
                                               cache->Sweep();
                                             }
                                           });
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
    _isSweepQueued = false;
  }
}
