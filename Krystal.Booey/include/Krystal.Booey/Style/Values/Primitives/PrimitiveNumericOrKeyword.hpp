#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/PrimitiveNumericOrKeyword.hpp"
#include "Krystal.Booey/Style/Values/Primitives/PrimitiveNumeric.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/FlatteningVariant.hpp"
#include <algorithm>

namespace krys::boo::style
{
  template <Numeric N, css::PrimitiveKeyword... Ks>
  class PrimitiveNumericOrKeyword
  {
  public:
    using CSS = typename N::CSS;
    using Raw = typename N::Raw;
    using Calc = typename N::Calc;
    using Dimension = typename N::Dimension;
    using Keywords = css::PrimitiveKeywordList<Ks...>;
    using Representation = FlatteningCompactVariant<N, Ks...>;

  private:
    Representation _value;

  public:
    template <typename U>
    requires SameType<remove_cvref_t<U>, N>
    PrimitiveNumericOrKeyword(U &&value) noexcept : _value {std::forward<U>(value)}
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

namespace krys
{
  template <typename N, typename... Ks>
  struct FlatteningVariantTraits<boo::style::PrimitiveNumericOrKeyword<N, Ks...>>
  {
    using TypeList = typename FlatteningVariantTraits<
      typename boo::style::PrimitiveNumericOrKeyword<N, Ks...>::Representation>::TypeList;
  };
}

namespace krys::boo::css
{
  template <typename N, typename... Ks>
  constexpr auto TreatAsVariantLike<style::PrimitiveNumericOrKeyword<N, Ks...>> = true;
}
