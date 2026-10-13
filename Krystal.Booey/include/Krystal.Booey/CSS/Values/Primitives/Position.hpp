#pragma once

#include "Krystal.Booey/CSS/Values/Aggregates.hpp"
#include "Krystal.Booey/CSS/Values/Concepts.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo::css
{
  // MARK: Two Component Types

  struct TwoComponentPositionHorizontal
  {
    Variant<keywords::Left, keywords::Right, keywords::Center, keywords::XStart, keywords::XEnd,
            LengthPercentage<>>
      Offset;

    KRYS_NODISCARD bool operator==(const TwoComponentPositionHorizontal &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(TwoComponentPositionHorizontal, Offset)

  struct TwoComponentPositionVertical
  {
    Variant<keywords::Top, keywords::Bottom, keywords::Center, keywords::YStart, keywords::YEnd,
            LengthPercentage<>>
      Offset;

    KRYS_NODISCARD bool operator==(const TwoComponentPositionVertical &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(TwoComponentPositionVertical, Offset)

  // MARK: Three Component Types

  struct ThreeComponentPositionHorizontal
  {
    Variant<keywords::Left, keywords::Right, keywords::Center, keywords::XStart, keywords::XEnd> Offset;

    KRYS_NODISCARD bool operator==(const ThreeComponentPositionHorizontal &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(ThreeComponentPositionHorizontal, Offset)

  struct ThreeComponentPositionVertical
  {
    Variant<keywords::Top, keywords::Bottom, keywords::Center, keywords::YStart, keywords::YEnd> Offset;

    KRYS_NODISCARD bool operator==(const ThreeComponentPositionVertical &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(ThreeComponentPositionVertical, Offset)

  // MARK: Four Component Types

  struct FourComponentPositionHorizontal
  {
    SpaceSeparatedTuple<Variant<keywords::Left, keywords::Right, keywords::XStart, keywords::XEnd>,
                        LengthPercentage<>>
      Offset;

    KRYS_NODISCARD bool operator==(const FourComponentPositionHorizontal &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(FourComponentPositionHorizontal, Offset)

  struct FourComponentPositionVertical
  {
    SpaceSeparatedTuple<Variant<keywords::Top, keywords::Bottom, keywords::YStart, keywords::YEnd>,
                        LengthPercentage<>>
      Offset;

    KRYS_NODISCARD bool operator==(const FourComponentPositionVertical &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(FourComponentPositionVertical, Offset)

  using TwoComponentPositionHorizontalVertical =
    SpaceSeparatedTuple<TwoComponentPositionHorizontal, TwoComponentPositionVertical>;

  using ThreeComponentPositionHorizontalVerticalLengthFirst =
    SpaceSeparatedTuple<FourComponentPositionHorizontal, ThreeComponentPositionVertical>;
  using ThreeComponentPositionHorizontalVerticalLengthSecond =
    SpaceSeparatedTuple<ThreeComponentPositionHorizontal, FourComponentPositionVertical>;

  using FourComponentPositionHorizontalVertical =
    SpaceSeparatedTuple<FourComponentPositionHorizontal, FourComponentPositionVertical>;

  /// @brief A `<position>`, e.g. `left 10px top`, in the two-, three- or four-component form it was written
  /// in.
  struct Position
  {
    using Kind =
      Variant<TwoComponentPositionHorizontalVertical, ThreeComponentPositionHorizontalVerticalLengthFirst,
              ThreeComponentPositionHorizontalVerticalLengthSecond, FourComponentPositionHorizontalVertical>;

    template <typename T>
    Position(T &&value) noexcept : Value {std::forward<T>(value)}
    {
    }

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(Value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const Position &) const noexcept = default;

    Kind Value;
  };
  DEFINE_TYPE_WRAPPER_GET(Position, Value)

  /// @brief The horizontal half of a `<position>` (e.g. `background-position-x`).
  struct PositionX
  {
    using Kind = Variant<TwoComponentPositionHorizontal, FourComponentPositionHorizontal>;

    template <typename T>
    PositionX(T &&value) noexcept : Value {std::forward<T>(value)}
    {
    }

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(Value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const PositionX &) const noexcept = default;

    Kind Value;
  };
  DEFINE_TYPE_WRAPPER_GET(PositionX, Value)

  /// @brief The vertical half of a `<position>` (e.g. `background-position-y`).
  struct PositionY
  {
    using Kind = Variant<TwoComponentPositionVertical, FourComponentPositionVertical>;

    template <typename T>
    PositionY(T &&value) noexcept : Value {std::forward<T>(value)}
    {
    }

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(Value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const PositionY &) const noexcept = default;

    Kind Value;
  };
  DEFINE_TYPE_WRAPPER_GET(PositionY, Value)

  /// @brief Whether `position` is `center center` (or `50% 50%`) in its two-component form.
  KRYS_NODISCARD bool IsCenterPosition(const Position &position) noexcept;

  /// @brief Splits a position into its horizontal and vertical halves.
  KRYS_NODISCARD Pair<PositionX, PositionY> Split(Position &&position) noexcept;
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
