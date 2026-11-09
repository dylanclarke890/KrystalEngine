/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Selectors/SelectorFilter.hpp"
#include "Krystal.Booey/Style/ContainerQueryEvaluator.hpp"
#include "Krystal.Booey/Style/Invalidation/HasSelectorFilter.hpp"
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
