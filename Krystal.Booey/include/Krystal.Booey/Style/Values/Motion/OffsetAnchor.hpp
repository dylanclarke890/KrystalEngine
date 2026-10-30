#pragma once

#include "Krystal.Booey/Style/Values/Primitives/Position.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'offset-anchor'> = auto | <position>
  // https://drafts.fxtf.org/motion/#propdef-offset-anchor
  struct OffsetAnchor
  {
    OffsetAnchor(css::keywords::Auto keyword) noexcept : _value {keyword}
    {
    }
    OffsetAnchor(Position &&position) noexcept : _value {krys::move(position)}
    {
    }
    OffsetAnchor(const Position &position) noexcept : _value {position}
    {
    }

    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return HoldsAlternative<css::keywords::Auto>();
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

    KRYS_NODISCARD bool operator==(const OffsetAnchor &) const noexcept = default;

  private:
    Variant<css::keywords::Auto, Position> _value;
  };

  template <typename T>
  bool OffsetAnchor::HoldsAlternative() const noexcept
  {
    return std::holds_alternative<T>(_value);
  }

  template <typename... F>
  decltype(auto) OffsetAnchor::SwitchOn(F &&...f) const noexcept
  {
    return krys::SwitchOn(_value, std::forward<F>(f)...);
  }

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<OffsetAnchor>
  {
    KRYS_NODISCARD OffsetAnchor operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<OffsetAnchor>
  {
    KRYS_NODISCARD bool CanBlend(const OffsetAnchor &, const OffsetAnchor &) noexcept;
    KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(const OffsetAnchor &,
                                                                      const OffsetAnchor &) noexcept;
    KRYS_NODISCARD OffsetAnchor Blend(const OffsetAnchor &, const OffsetAnchor &,
                                      const BlendingContext &) noexcept;
  };

  // MARK: - Evaluation
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::OffsetAnchor)
