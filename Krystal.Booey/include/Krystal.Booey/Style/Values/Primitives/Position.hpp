#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/Position.hpp"
#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Viewport/ZoomPrimitives.hpp"

namespace krys::boo::style
{
  struct PositionX : LengthWrapperBase<LengthPercentage<>>
  {
    using Base::Base;
  };

  struct PositionY : LengthWrapperBase<LengthPercentage<>>
  {
    using Base::Base;
  };

  struct TwoComponentPositionHorizontal
  {
    PositionX Offset;

    KRYS_NODISCARD bool operator==(const TwoComponentPositionHorizontal &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(TwoComponentPositionHorizontal, Offset)

  struct TwoComponentPositionVertical
  {
    PositionY Offset;

    KRYS_NODISCARD bool operator==(const TwoComponentPositionVertical &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(TwoComponentPositionVertical, Offset)

  /// @brief A computed `<position>`: a horizontal and a vertical length-percentage.
  struct Position
  {
    PositionX X;
    PositionY Y;

    Position(PositionX &&x, PositionY &&y) noexcept : X {krys::move(x)}, Y {krys::move(y)}
    {
    }

    Position(const PositionX &x, const PositionY &y) noexcept : X {x}, Y {y}
    {
    }

    Position(TwoComponentPositionHorizontal &&x, TwoComponentPositionVertical &&y) noexcept
        : X {krys::move(x.Offset)}, Y {krys::move(y.Offset)}
    {
    }

    Position(const TwoComponentPositionHorizontal &x, const TwoComponentPositionVertical &y) noexcept
        : X {x.Offset}, Y {y.Offset}
    {
    }

    Position(FloatPoint point) noexcept : X {PositionX::Fixed {point.X()}}, Y {PositionY::Fixed {point.Y()}}
    {
    }

    Position(css::ValueLiteral<css::LengthUnit::Px> literalX,
             css::ValueLiteral<css::LengthUnit::Px> literalY) noexcept
        : X {literalX}, Y {literalY}
    {
    }

    Position(css::ValueLiteral<css::PercentageUnit::Percentage> literalX,
             css::ValueLiteral<css::PercentageUnit::Percentage> literalY) noexcept
        : X {literalX}, Y {literalY}
    {
    }

    KRYS_NODISCARD bool operator==(const Position &) const noexcept = default;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Position &position) noexcept
  {
    if constexpr (!I)
    {
      return position.X;
    }
    else if constexpr (I == 1)
    {
      return position.Y;
    }
  }

  // MARK: - Conversion

  template <>
  struct ToCSS<TwoComponentPositionHorizontal>
  {
    KRYS_NODISCARD css::TwoComponentPositionHorizontal operator()(const TwoComponentPositionHorizontal &,
                                                                  const RenderStyle &) noexcept;
  };

  template <>
  struct ToStyle<css::TwoComponentPositionHorizontal>
  {
    KRYS_NODISCARD TwoComponentPositionHorizontal operator()(const css::TwoComponentPositionHorizontal &,
                                                             const BuilderState &) noexcept;
  };

  template <>
  struct ToCSS<TwoComponentPositionVertical>
  {
    KRYS_NODISCARD css::TwoComponentPositionVertical operator()(const TwoComponentPositionVertical &,
                                                                const RenderStyle &) noexcept;
  };

  template <>
  struct ToStyle<css::TwoComponentPositionVertical>
  {
    KRYS_NODISCARD TwoComponentPositionVertical operator()(const css::TwoComponentPositionVertical &,
                                                           const BuilderState &) noexcept;
  };

  template <>
  struct ToCSS<Position>
  {
    KRYS_NODISCARD css::Position operator()(const Position &, const RenderStyle &) noexcept;
  };

  template <>
  struct ToStyle<css::Position>
  {
    KRYS_NODISCARD Position operator()(const css::Position &, const BuilderState &) noexcept;
  };

  template <>
  struct ToCSS<PositionX>
  {
    KRYS_NODISCARD css::PositionX operator()(const PositionX &, const RenderStyle &) noexcept;
  };

  template <>
  struct ToStyle<css::PositionX>
  {
    KRYS_NODISCARD PositionX operator()(const css::PositionX &, const BuilderState &) noexcept;
  };

  template <>
  struct ToCSS<PositionY>
  {
    KRYS_NODISCARD css::PositionY operator()(const PositionY &, const RenderStyle &) noexcept;
  };

  template <>
  struct ToStyle<css::PositionY>
  {
    KRYS_NODISCARD PositionY operator()(const css::PositionY &, const BuilderState &) noexcept;
  };

  template <>
  struct CSSValueConversion<Position>
  {
    KRYS_NODISCARD Position operator()(BuilderState &, const css::Value &) noexcept;
  };

  template <>
  struct CSSValueConversion<PositionX>
  {
    KRYS_NODISCARD PositionX operator()(BuilderState &, const css::Value &) noexcept;
  };

  template <>
  struct CSSValueConversion<PositionY>
  {
    KRYS_NODISCARD PositionY operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Evaluation

  template <>
  struct Evaluation<Position, FloatPoint>
  {
    KRYS_NODISCARD FloatPoint operator()(const Position &, FloatSize, ZoomNeeded) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::PositionX)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::PositionY)
DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::TwoComponentPositionHorizontal)
DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::TwoComponentPositionVertical)
DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::style::Position, 2)
