#pragma once

#include "Krystal.Booey/CSS/Values/Aggregates.hpp"
#include "Krystal.Booey/CSS/Values/Concepts.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
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

  struct Position
  {
    using Kind =
      Variant<TwoComponentPositionHorizontalVertical, ThreeComponentPositionHorizontalVerticalLengthFirst,
              ThreeComponentPositionHorizontalVerticalLengthSecond, FourComponentPositionHorizontalVertical>;

    template <typename T>
    Position(T &&value) noexcept : value {std::forward<T>(value)}
    {
    }

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const Position &) const noexcept = default;

    Kind value;
  };
  DEFINE_TYPE_WRAPPER_GET(Position, value)

  struct PositionX
  {
    using Kind = Variant<TwoComponentPositionHorizontal, FourComponentPositionHorizontal>;

    template <typename T>
    PositionX(T &&value) noexcept : value {std::forward<T>(value)}
    {
    }

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const PositionX &) const noexcept = default;

    Kind value;
  };

  DEFINE_TYPE_WRAPPER_GET(PositionX, value)

  struct PositionY
  {
    using Kind = Variant<TwoComponentPositionVertical, FourComponentPositionVertical>;

    template <typename T>
    PositionY(T &&value) noexcept : value {std::forward<T>(value)}
    {
    }

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const PositionY &) const noexcept = default;

    Kind value;
  };

  DEFINE_TYPE_WRAPPER_GET(PositionY, value)

  KRYS_NODISCARD bool IsCenterPosition(const Position &) noexcept;

  KRYS_NODISCARD Pair<PositionX, PositionY> Split(Position &&) noexcept;
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::TwoComponentPositionHorizontal, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::TwoComponentPositionVertical, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::ThreeComponentPositionHorizontal, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::ThreeComponentPositionVertical, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::FourComponentPositionHorizontal, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::FourComponentPositionVertical, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::Position, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::PositionX, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::PositionY, 1)