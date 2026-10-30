#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorChecker.hpp"
#include "Krystal.Booey/Style/PropertyAllowlist.hpp"
#include "Krystal.Booey/Style/RuleSet.hpp"
#include "Krystal.Booey/Style/ScopeOrdinal.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include <algorithm>

namespace krys::boo::style
{
  enum class FromStyleAttribute : bool
  {
    No,
    Yes
  };

  enum class IsCacheable : uint8
  {
    No,
    Partially,
    Yes
  };

  /// @brief A declaration block that applies to an element, with where it came from.
  struct MatchedProperties
  {
    Ref<const css::StyleProperties> Properties;
    uint8 LinkMatchType {css::SelectorChecker::MatchAll};
    PropertyAllowlist AllowlistType {PropertyAllowlist::None};
    ScopeOrdinal StyleScopeOrdinal {ScopeOrdinal::Element};
    style::FromStyleAttribute FromStyleAttribute {style::FromStyleAttribute::No};
    style::CascadeLayerPriority CascadeLayerPriority {RuleSet::CascadeLayerPriorityForUnlayered};
    EnumSet<UsedRuleType> UsedRuleTypes {};
    style::IsCacheable IsCacheable {style::IsCacheable::Yes};

    KRYS_NODISCARD friend bool operator==(const MatchedProperties &a, const MatchedProperties &b) noexcept
    {
      return a.Properties.get() == b.Properties.get() && a.LinkMatchType == b.LinkMatchType
             && a.AllowlistType == b.AllowlistType && a.StyleScopeOrdinal == b.StyleScopeOrdinal
             && a.FromStyleAttribute == b.FromStyleAttribute
             && a.CascadeLayerPriority == b.CascadeLayerPriority && a.UsedRuleTypes == b.UsedRuleTypes
             && a.IsCacheable == b.IsCacheable;
    }
  };

  /// @brief The declaration blocks that apply to an element, per origin, in cascade order.
  struct MatchResult : RefCounted<MatchResult>
  {
    bool IsForLink {false};
    bool IsCompletelyNonCacheable {false};
    EnumSet<UsedRuleType> UsedRuleTypes {};
    List<MatchedProperties> UserAgentDeclarations;
    List<MatchedProperties> UserDeclarations;
    List<MatchedProperties> AuthorDeclarations;
    SmallList<css::PropertyId, 4> NonCacheablePropertyIds;

    KRYS_NODISCARD static Ref<MatchResult> Create(bool isForLink = false) noexcept
    {
      return AdoptRef(*new MatchResult(isForLink));
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return UserAgentDeclarations.empty() && UserDeclarations.empty() && AuthorDeclarations.empty();
    }

    KRYS_NODISCARD bool CacheablePropertiesEqual(const MatchResult &other) const noexcept;

  private:
    explicit MatchResult(bool isForLink) noexcept : IsForLink(isForLink)
    {
    }
  };

  inline bool MatchResult::CacheablePropertiesEqual(const MatchResult &other) const noexcept
  {
    if (IsForLink != other.IsForLink || UsedRuleTypes != other.UsedRuleTypes)
    {
      return false;
    }

    // Only author style can be non-cacheable.
    if (UserAgentDeclarations != other.UserAgentDeclarations)
    {
      return false;
    }
    if (UserDeclarations != other.UserDeclarations)
    {
      return false;
    }

    // Currently the cached style contains also the non-cacheable property values from when the entry was made
    // so we can only allow styles that override the same exact properties. Content usually animates or varies
    // the same small set of properties so this doesn't make a significant difference.
    auto nonCacheableEqual =
      std::ranges::equal(NonCacheablePropertyIds, other.NonCacheablePropertyIds,
                         [](css::PropertyId idA, css::PropertyId idB)
                         {
                           // This would need to check the custom property names for
                           // equality.
                           if (idA == css::PropertyId::Custom || idB == css::PropertyId::Custom)
                           {
                             return false;
                           }
                           return idA == idB;
                         });
    if (!nonCacheableEqual)
    {
      return false;
    }

    return std::ranges::equal(AuthorDeclarations, other.AuthorDeclarations,
                              [](const MatchedProperties &propertiesA, const MatchedProperties &propertiesB)
                              {
                                if (propertiesA.IsCacheable == IsCacheable::Partially
                                    && propertiesB.IsCacheable == IsCacheable::Partially)
                                {
                                  return true;
                                }
                                return propertiesA == propertiesB;
                              });
  }
}

namespace krys
{
  template <>
  struct DefaultHash<boo::style::MatchedProperties>
  {
    constexpr static bool IsSafeToCompareToEmptyOrDeleted = false;

    KRYS_NODISCARD static size_t Hash(const boo::style::MatchedProperties &matchedProperties) noexcept
    {
      // Ignore non-cacheable properties when computing hash.
      if (matchedProperties.IsCacheable == boo::style::IsCacheable::Partially)
      {
        return 0;
      }
      krys_debug_assert(matchedProperties.IsCacheable == boo::style::IsCacheable::Yes);

      return krys::Hash::Combine(
        matchedProperties.Properties.get(), matchedProperties.LinkMatchType,
        ToUnderlying(matchedProperties.AllowlistType), ToUnderlying(matchedProperties.StyleScopeOrdinal),
        ToUnderlying(matchedProperties.FromStyleAttribute), matchedProperties.CascadeLayerPriority,
        matchedProperties.UsedRuleTypes.ToRaw());
    }

    KRYS_NODISCARD static bool Equal(const boo::style::MatchedProperties &a,
                                     const boo::style::MatchedProperties &b) noexcept
    {
      return a == b;
    }
  };

  /// @brief Hashes the cacheable part of a match result; results with equal hashes are compared with
  /// `CacheablePropertiesEqual`.
  template <>
  struct DefaultHash<boo::style::MatchResult>
  {
    constexpr static bool IsSafeToCompareToEmptyOrDeleted = false;

    KRYS_NODISCARD static size_t Hash(const boo::style::MatchResult &matchResult) noexcept
    {
      krys_debug_assert(!matchResult.IsCompletelyNonCacheable);

      size_t seed = krys::Hash::Combine(matchResult.IsForLink, matchResult.NonCacheablePropertyIds.size());
      for (auto id : matchResult.NonCacheablePropertyIds)
      {
        seed = krys::Hash::Combine(seed, ToUnderlying(id));
      }
      for (auto *declarations : {&matchResult.UserAgentDeclarations, &matchResult.UserDeclarations,
                                 &matchResult.AuthorDeclarations})
      {
        seed = krys::Hash::Combine(seed, declarations->size());
        for (auto &matchedProperties : *declarations)
        {
          seed =
            krys::Hash::Combine(seed, DefaultHash<boo::style::MatchedProperties>::Hash(matchedProperties));
        }
      }
      return seed;
    }

    KRYS_NODISCARD static bool Equal(const boo::style::MatchResult &a,
                                     const boo::style::MatchResult &b) noexcept
    {
      return a.CacheablePropertiesEqual(b);
    }
  };
}
