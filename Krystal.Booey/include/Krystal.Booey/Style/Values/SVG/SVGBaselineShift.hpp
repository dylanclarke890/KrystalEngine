#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"

namespace krys::boo::style
{
  struct SVGBaselineShiftLength : LengthWrapperBase<LengthPercentage<>>
  {
    using Base::Base;
  };

  // <'baseline-shift'> = baseline | sub | super | <length-percentage>
  struct SVGBaselineShift
  {
    using Length = SVGBaselineShiftLength;

    SVGBaselineShift(css::keywords::Baseline keyword) noexcept : _value {keyword}
    {
    }

    SVGBaselineShift(css::keywords::Sub keyword) noexcept : _value {keyword}
    {
    }

    SVGBaselineShift(css::keywords::Super keyword) noexcept : _value {keyword}
    {
    }

    SVGBaselineShift(Length &&length) noexcept : _value {krys::move(length)}
    {
    }

    KRYS_NODISCARD bool IsBaseline() const noexcept
    {
      return krys::HoldsAlternative<css::keywords::Baseline>(_value);
    }
    KRYS_NODISCARD bool IsSub() const noexcept
    {
      return krys::HoldsAlternative<css::keywords::Sub>(_value);
    }
    KRYS_NODISCARD bool IsSuper() const noexcept
    {
      return krys::HoldsAlternative<css::keywords::Super>(_value);
    }
    KRYS_NODISCARD bool IsLength() const noexcept
    {
      return krys::HoldsAlternative<Length>(_value);
    }
    KRYS_NODISCARD Maybe<Length> TryLength() const noexcept
    {
      return IsLength() ? Maybe(std::get<Length>(_value)) : std::nullopt;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(_value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const SVGBaselineShift &) const noexcept = default;

  private:
    friend struct Blending<SVGBaselineShift>;

    Variant<css::keywords::Baseline, css::keywords::Sub, css::keywords::Super, SVGBaselineShiftLength> _value;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<SVGBaselineShift>
  {
    KRYS_NODISCARD SVGBaselineShift operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<SVGBaselineShift>
  {
    KRYS_NODISCARD bool CanBlend(const SVGBaselineShift &, const SVGBaselineShift &) noexcept;
    KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(const SVGBaselineShift &,
                                                                      const SVGBaselineShift &) noexcept;
    KRYS_NODISCARD SVGBaselineShift Blend(const SVGBaselineShift &, const SVGBaselineShift &,
                                          const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::SVGBaselineShiftLength)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::SVGBaselineShift)
