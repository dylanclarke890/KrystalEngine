#pragma once

#include "Krystal.HTML/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace Krys::HTML
{
  // <steps-easing-function> = steps( <integer>, <steps-easing-function-position>? )
  // <steps-easing-function-position> = jump-start | jump-end | jump-none | jump-both | start | end
  //
  // with range constraints, this is:
  //
  // <steps-easing-function> = steps( <integer [1,∞]>, jump-start )
  //                         | steps( <integer [1,∞]>, jump-end )
  //                         | steps( <integer [1,∞]>, jump-both )
  //                         | steps( <integer [1,∞]>, start )
  //                         | steps( <integer [1,∞]>, end )
  //                         | steps( <integer [2,∞]>, jump-none )
  // https://drafts.csswg.org/css-easing-2/#funcdef-steps
  struct StepsEasingParameters
  {
    enum class ShouldSerializeKeyword : bool
    {
      No,
      Yes
    };

    template <typename T, typename Keyword, auto shouldSerializeKeyword = ShouldSerializeKeyword::Yes>
    struct Kind
    {
      constexpr static Keyword keyword = Keyword {};
      T steps;

      constexpr bool operator==(const Kind &) const noexcept = default;
    };

    using JumpStart = Kind<Integer<CSSRange {1, CSSRange::Inf}>, Keywords::JumpStart>;
    using JumpEnd = Kind<Integer<CSSRange {1, CSSRange::Inf}>, Keywords::JumpEnd, ShouldSerializeKeyword::No>;
    using JumpBoth = Kind<Integer<CSSRange {1, CSSRange::Inf}>, Keywords::JumpBoth>;
    using Start = Kind<Integer<CSSRange {1, CSSRange::Inf}>, Keywords::Start>;
    using End = Kind<Integer<CSSRange {1, CSSRange::Inf}>, Keywords::End, ShouldSerializeKeyword::No>;
    using JumpNone = Kind<Integer<CSSRange {2, CSSRange::Inf}>, Keywords::JumpNone>;

    Variant<JumpStart, JumpEnd, JumpBoth, Start, End, JumpNone> value;

    constexpr bool operator==(const StepsEasingParameters &) const noexcept = default;
  };

  using StepsEasingFunction = FunctionNotation<CSSValueId::Steps, StepsEasingParameters>;

  DEFINE_TYPE_WRAPPER_GET(StepsEasingParameters, value);

  template <size_t I, typename T, typename K, auto shouldSerializeKeyword>
  KRYS_NODISCARD constexpr const auto &
    get(const StepsEasingParameters::Kind<T, K, shouldSerializeKeyword> &value) noexcept
  {
    return value.steps;
  }
}

DEFINE_TUPLE_LIKE_CONFORMANCE(Krys::HTML::StepsEasingParameters, 1)

namespace std
{
  template <typename T, typename K, auto shouldSerializeKeyword>
  class tuple_size<Krys::HTML::StepsEasingParameters::Kind<T, K, shouldSerializeKeyword>>
      : public std::integral_constant<size_t, 1>
  {
  };

  template <size_t I, typename T, typename K, auto shouldSerializeKeyword>
  class tuple_element<I, Krys::HTML::StepsEasingParameters::Kind<T, K, shouldSerializeKeyword>>
  {
  public:
    using type = T;
  };

}

template <typename T, typename K, auto shouldSerializeKeyword>
constexpr bool
  Krys::HTML::TreatAsTupleLike<Krys::HTML::StepsEasingParameters::Kind<T, K, shouldSerializeKeyword>> = true;
