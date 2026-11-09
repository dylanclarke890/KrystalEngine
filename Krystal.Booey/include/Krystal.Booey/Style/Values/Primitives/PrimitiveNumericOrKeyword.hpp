/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/PrimitiveNumericOrKeyword.hpp"
#include "Krystal.Booey/Style/Values/Primitives/PrimitiveNumeric.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/CompactVariant.hpp"
#include <algorithm>

namespace krys::boo::style
{
  namespace detail
  {
    /// @brief A numeric's own alternatives followed by the keywords, so that one index covers them all.
    template <typename TRepresentation, typename... Ks>
    struct AppendAlternatives;

    template <typename... As, typename... Ks>
    struct AppendAlternatives<CompactVariant<As...>, Ks...>
    {
      using Type = CompactVariant<As..., Ks...>;
    };
  }

  template <DimensionPercentageNumeric N, css::PrimitiveKeyword... Ks>
  class PrimitiveNumericOrKeyword
  {
  public:
    using CSS = typename N::CSS;
    using Raw = typename N::Raw;
    using Calc = typename N::Calc;
    using Dimension = typename N::Dimension;
    using Keywords = css::PrimitiveKeywordList<Ks...>;
    using Representation = typename detail::AppendAlternatives<typename N::Representation, Ks...>::Type;

  private:
    Representation _value;

  public:
    template <typename U>
    requires SameType<remove_cvref_t<U>, N>
    PrimitiveNumericOrKeyword(U &&value) noexcept : _value {FromNumeric(std::forward<U>(value))}
    {
    }

    PrimitiveNumericOrKeyword(css::ValidKeywordForList<Keywords> auto keyword) noexcept : _value {keyword}
    {
    }

    // "Variant-Like" operators.
    template <typename U>
    KRYS_NODISCARD constexpr bool HoldsAlternative() const noexcept
    {
      return krys::HoldsAlternative<U>(_value);
    }

    template <typename... F>
    constexpr decltype(auto) SwitchOn(F &&...functors) const noexcept
    {
      return krys::SwitchOn(_value, std::forward<F>(functors)...);
    }

    KRYS_NODISCARD constexpr bool IsZero() const noexcept
    {
      return SwitchOn([]<css::HasIsZero U>(const U &value) { return value.IsZero(); },
                      [](const auto &) { return false; });
    }

    KRYS_NODISCARD bool operator==(const PrimitiveNumericOrKeyword &) const noexcept = default;

  private:
    /// @brief The representation of a numeric: its active alternative.
    template <typename U>
    KRYS_NODISCARD static Representation FromNumeric(U &&value) noexcept
    {
      return krys::SwitchOn(std::forward<U>(value), []<typename A>(A &&alternative) -> Representation
                            { return Representation(std::forward<A>(alternative)); });
    }
  };

  // MARK: CSS -> Style

  template <typename N, typename... Ks>
  struct ToStyleMapping<css::PrimitiveNumericOrKeyword<N, Ks...>>
  {
    using type = PrimitiveNumericOrKeyword<StyleType<N>, Ks...>;
  };

  // MARK: Style -> CSS

  template <typename N, typename... Ks>
  struct ToCSSMapping<PrimitiveNumericOrKeyword<N, Ks...>>
  {
    using type = css::PrimitiveNumericOrKeyword<CSSType<N>, Ks...>;
  };
}

namespace krys::boo::css
{
  template <typename N, typename... Ks>
  constexpr auto TreatAsVariantLike<style::PrimitiveNumericOrKeyword<N, Ks...>> = true;
}
