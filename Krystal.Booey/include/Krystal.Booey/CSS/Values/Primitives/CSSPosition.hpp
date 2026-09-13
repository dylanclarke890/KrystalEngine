#pragma once

#include "Krystal.Booey/CSS/Values/CSSValueAggregates.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueConcepts.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo::css
{
  struct TwoComponentPositionHorizontal
  {
    Variant<keywords::Left, keywords::Right, keywords::Center, keywords::XStart, keywords::XEnd,
            LengthPercentage<>>
      offset;

    KRYS_NODISCARD bool operator==(const TwoComponentPositionHorizontal &) const noexcept = default;
  };

  DEFINE_TYPE_WRAPPER_GET(TwoComponentPositionHorizontal, offset)

  struct TwoComponentPositionVertical
  {
    Variant<keywords::Top, keywords::Bottom, keywords::Center, keywords::YStart, keywords::YEnd,
            LengthPercentage<>>
      offset;

    KRYS_NODISCARD bool operator==(const TwoComponentPositionVertical &) const noexcept = default;
  };

  DEFINE_TYPE_WRAPPER_GET(TwoComponentPositionVertical, offset)

  struct ThreeComponentPositionHorizontal
  {
    Variant<keywords::Left, keywords::Right, keywords::Center, keywords::XStart, keywords::XEnd> offset;

    KRYS_NODISCARD bool operator==(const ThreeComponentPositionHorizontal &) const noexcept = default;
  };

  DEFINE_TYPE_WRAPPER_GET(ThreeComponentPositionHorizontal, offset)

  struct ThreeComponentPositionVertical
  {
    Variant<keywords::Top, keywords::Bottom, keywords::Center, keywords::YStart, keywords::YEnd> offset;

    KRYS_NODISCARD bool operator==(const ThreeComponentPositionVertical &) const noexcept = default;
  };

  DEFINE_TYPE_WRAPPER_GET(ThreeComponentPositionVertical, offset)

  struct FourComponentPositionHorizontal
  {
    SpaceSeparatedTuple<Variant<keywords::Left, keywords::Right, keywords::XStart, keywords::XEnd>,
                        LengthPercentage<>>
      offset;

    KRYS_NODISCARD bool operator==(const FourComponentPositionHorizontal &) const noexcept = default;
  };

  DEFINE_TYPE_WRAPPER_GET(FourComponentPositionHorizontal, offset)

  struct FourComponentPositionVertical
  {
    SpaceSeparatedTuple<Variant<keywords::Top, keywords::Bottom, keywords::YStart, keywords::YEnd>,
                        LengthPercentage<>>
      offset;

    KRYS_NODISCARD bool operator==(const FourComponentPositionVertical &) const noexcept = default;
  };

  DEFINE_TYPE_WRAPPER_GET(FourComponentPositionVertical, offset)

  using TwoComponentPositionHorizontalVertical =
    SpaceSeparatedTuple<TwoComponentPositionHorizontal, TwoComponentPositionVertical>;

  using ThreeComponentPositionHorizontalVerticalLengthFirst =
    SpaceSeparatedTuple<FourComponentPositionHorizontal, ThreeComponentPositionVertical>;

  using ThreeComponentPositionHorizontalVerticalLengthSecond =
    SpaceSeparatedTuple<ThreeComponentPositionHorizontal, FourComponentPositionVertical>;

  using FourComponentPositionHorizontalVertical =
    SpaceSeparatedTuple<FourComponentPositionHorizontal, FourComponentPositionVertical>;

  struct CSSPosition
  {
    using Kind =
      Variant<TwoComponentPositionHorizontalVertical, ThreeComponentPositionHorizontalVerticalLengthFirst,
              ThreeComponentPositionHorizontalVerticalLengthSecond, FourComponentPositionHorizontalVertical>;

    template <typename T>
    CSSPosition(T &&value) noexcept : value {std::forward<T>(value)}
    {
    }

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const CSSPosition &) const noexcept = default;

    Kind value;
  };
  DEFINE_TYPE_WRAPPER_GET(CSSPosition, value)

  struct CSSPositionX
  {
    using Kind = Variant<TwoComponentPositionHorizontal, FourComponentPositionHorizontal>;

    template <typename T>
    CSSPositionX(T &&value) noexcept : value {std::forward<T>(value)}
    {
    }

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const CSSPositionX &) const noexcept = default;

    Kind value;
  };

  DEFINE_TYPE_WRAPPER_GET(CSSPositionX, value)

  struct CSSPositionY
  {
    using Kind = Variant<TwoComponentPositionVertical, FourComponentPositionVertical>;

    template <typename T>
    CSSPositionY(T &&value) noexcept : value {std::forward<T>(value)}
    {
    }

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const CSSPositionY &) const noexcept = default;

    Kind value;
  };

  DEFINE_TYPE_WRAPPER_GET(CSSPositionY, value)

  KRYS_NODISCARD bool IsCenterPosition(const CSSPosition &) noexcept;

  KRYS_NODISCARD Pair<CSSPositionX, CSSPositionY> Split(CSSPosition &&) noexcept;
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::TwoComponentPositionHorizontal, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::TwoComponentPositionVertical, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::ThreeComponentPositionHorizontal, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::ThreeComponentPositionVertical, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::FourComponentPositionHorizontal, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::FourComponentPositionVertical, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::CSSPosition, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::CSSPositionX, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::CSSPositionY, 1)