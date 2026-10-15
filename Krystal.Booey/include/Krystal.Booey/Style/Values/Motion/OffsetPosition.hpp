#pragma once

#include "Krystal.Booey/Style/Values/Primitives/Position.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'offset-position'> = auto | normal | <position>
  // https://drafts.fxtf.org/motion/#propdef-offset-position
  struct OffsetPosition
  {
    OffsetPosition(css::keywords::Auto keyword) noexcept : _value {keyword}
    {
    }
    OffsetPosition(css::keywords::Normal keyword) noexcept : _value {keyword}
    {
    }
    OffsetPosition(Position &&position) noexcept : _value {krys::move(position)}
    {
    }
    OffsetPosition(const Position &position) noexcept : _value {position}
    {
    }

    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return HoldsAlternative<css::keywords::Auto>();
    }
    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsNormal() const noexcept
    {
      return HoldsAlternative<css::keywords::Normal>();
    }
    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsPosition() const noexcept
    {
      return HoldsAlternative<Position>();
    }
    KRYS_NODISCARD Maybe<Position> TryPosition() const noexcept
    {
      return IsPosition() ? Maybe(std::get<Position>(_value)) : std::nullopt;
    }

    template <typename>
    KRYS_NODISCARD bool HoldsAlternative() const noexcept;
    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...) const noexcept;

    KRYS_NODISCARD bool operator==(const OffsetPosition &) const noexcept = default;

  private:
    Variant<css::keywords::Auto, css::keywords::Normal, Position> _value;
  };

  template <typename T>
  bool OffsetPosition::HoldsAlternative() const noexcept
  {
    return std::holds_alternative<T>(_value);
  }

  template <typename... F>
  decltype(auto) OffsetPosition::SwitchOn(F &&...f) const noexcept
  {
    return krys::SwitchOn(_value, std::forward<F>(f)...);
  }

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<OffsetPosition>
  {
    KRYS_NODISCARD OffsetPosition operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<OffsetPosition>
  {
    KRYS_NODISCARD bool CanBlend(const OffsetPosition &, const OffsetPosition &) noexcept;
    KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(const OffsetPosition &,
                                                                      const OffsetPosition &) noexcept;
    KRYS_NODISCARD OffsetPosition Blend(const OffsetPosition &, const OffsetPosition &,
                                        const BlendingContext &) noexcept;
  };

  // MARK: - Evaluation
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::OffsetPosition)
