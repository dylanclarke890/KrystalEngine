#pragma once

#include "Krystal.HTML/CSS/Values/CSSValueAggregates.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueConcepts.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"
#include "Krystal.Lib/Types/Pair.hpp"

namespace Krys::HTML
{
  struct TwoComponentPositionHorizontal
  {
    Variant<Keywords::Left, Keywords::Right, Keywords::Center, Keywords::XStart, Keywords::XEnd,
            LengthPercentage<>>
      offset;

    KRYS_NODISCARD bool operator==(const TwoComponentPositionHorizontal &) const noexcept = default;
  };

  DEFINE_TYPE_WRAPPER_GET(TwoComponentPositionHorizontal, offset);

  struct TwoComponentPositionVertical
  {
    Variant<Keywords::Top, Keywords::Bottom, Keywords::Center, Keywords::YStart, Keywords::YEnd,
            LengthPercentage<>>
      offset;

    KRYS_NODISCARD bool operator==(const TwoComponentPositionVertical &) const noexcept = default;
  };

  DEFINE_TYPE_WRAPPER_GET(TwoComponentPositionVertical, offset);

  struct ThreeComponentPositionHorizontal
  {
    Variant<Keywords::Left, Keywords::Right, Keywords::Center, Keywords::XStart, Keywords::XEnd> offset;

    KRYS_NODISCARD bool operator==(const ThreeComponentPositionHorizontal &) const noexcept = default;
  };

  DEFINE_TYPE_WRAPPER_GET(ThreeComponentPositionHorizontal, offset);

  struct ThreeComponentPositionVertical
  {
    Variant<Keywords::Top, Keywords::Bottom, Keywords::Center, Keywords::YStart, Keywords::YEnd> offset;

    KRYS_NODISCARD bool operator==(const ThreeComponentPositionVertical &) const noexcept = default;
  };

  DEFINE_TYPE_WRAPPER_GET(ThreeComponentPositionVertical, offset);

  struct FourComponentPositionHorizontal
  {
    SpaceSeparatedTuple<Variant<Keywords::Left, Keywords::Right, Keywords::XStart, Keywords::XEnd>,
                        LengthPercentage<>>
      offset;

    KRYS_NODISCARD bool operator==(const FourComponentPositionHorizontal &) const noexcept = default;
  };

  DEFINE_TYPE_WRAPPER_GET(FourComponentPositionHorizontal, offset);

  struct FourComponentPositionVertical
  {
    SpaceSeparatedTuple<Variant<Keywords::Top, Keywords::Bottom, Keywords::YStart, Keywords::YEnd>,
                        LengthPercentage<>>
      offset;

    KRYS_NODISCARD bool operator==(const FourComponentPositionVertical &) const noexcept = default;
  };

  DEFINE_TYPE_WRAPPER_GET(FourComponentPositionVertical, offset);

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
      return Krys::SwitchOn(value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const CSSPosition &) const noexcept = default;

    Kind value;
  };
  DEFINE_TYPE_WRAPPER_GET(CSSPosition, value);

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
      return Krys::SwitchOn(value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const CSSPositionX &) const noexcept = default;

    Kind value;
  };
  DEFINE_TYPE_WRAPPER_GET(CSSPositionX, value);

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
      return Krys::SwitchOn(value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const CSSPositionY &) const noexcept = default;

    Kind value;
  };
  DEFINE_TYPE_WRAPPER_GET(CSSPositionY, value);

  KRYS_NODISCARD bool IsCenterPosition(const CSSPosition &) noexcept;

  KRYS_NODISCARD Pair<CSSPositionX, CSSPositionY> Split(CSSPosition &&) noexcept;
}

DEFINE_TUPLE_LIKE_CONFORMANCE(Krys::HTML::TwoComponentPositionHorizontal, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(Krys::HTML::TwoComponentPositionVertical, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(Krys::HTML::ThreeComponentPositionHorizontal, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(Krys::HTML::ThreeComponentPositionVertical, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(Krys::HTML::FourComponentPositionHorizontal, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(Krys::HTML::FourComponentPositionVertical, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(Krys::HTML::CSSPosition, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(Krys::HTML::CSSPositionX, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(Krys::HTML::CSSPositionY, 1)