#pragma once

#include "Krystal.Booey/CSS/Selectors/SelectorFilter.hpp"
#include "Krystal.Booey/Style/ContainerQueryEvaluator.hpp"
#include "Krystal.Booey/Style/HasSelectorFilter.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::style
{
  using HasPseudoClassCacheKey = Pair<const dom::Element *, const css::Selector *>;
  using HasPseudoClassFilterKey = Pair<const dom::Element *, uint8>;

  enum class HasPseudoClassMatch : uint8
  {
    None,
    Matches,
    Fails,
    FailsSubtree
  };

  struct HasPseudoClassKeyHash
  {
    template <typename T, typename U>
    KRYS_NODISCARD size_t operator()(const Pair<T, U> &pair) const noexcept
    {
      return Hash::Combine(pair.first, pair.second);
    }
  };

  /// @brief Selector matching state that lives for a style resolution: the ancestor filter, the container
  /// query evaluation state and the `:has()` match cache and filters.
  struct SelectorMatchingState
  {
    css::SelectorFilter SelectorFilter;
    style::ContainerQueryEvaluationState ContainerQueryEvaluationState;
    HashMap<HasPseudoClassCacheKey, HasPseudoClassMatch, HasPseudoClassKeyHash> HasPseudoClassMatchCache;
    HashMap<HasPseudoClassFilterKey, UniquePtr<HasSelectorFilter>, HasPseudoClassKeyHash>
      HasPseudoClassSelectorFilters;
  };

  KRYS_NODISCARD inline HasPseudoClassCacheKey
    MakeHasPseudoClassCacheKey(const dom::Element &element, const css::Selector &selector) noexcept
  {
    return {&element, &selector};
  }

  KRYS_NODISCARD inline HasPseudoClassFilterKey
    MakeHasPseudoClassFilterKey(const dom::Element &element, HasSelectorFilter::Type type) noexcept
  {
    return {&element, static_cast<uint8>(type)};
  }
}
