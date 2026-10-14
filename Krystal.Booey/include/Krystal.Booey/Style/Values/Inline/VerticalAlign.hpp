#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"

namespace krys::boo::style
{
  struct VerticalAlignLength : LengthWrapperBase<LengthPercentage<>>
  {
    using Base::Base;
  };

  // <'vertical-align'> = baseline | sub | super | top | text-top | middle | bottom | text-bottom |
  // -webkit-baseline-middle | <length-percentage>
  // https://www.w3.org/TR/CSS22/visudet.html#propdef-vertical-align

  // FIXME: We implement the version of `vertical-align` from CSS 2. In CSS Inline 3, it has been changed to a
  // shorthand property with grammar `[ first | last] || <'alignment-baseline'> || <'baseline-shift'>`.
  // https://drafts.csswg.org/css-inline/#propdef-vertical-align

  struct VerticalAlign
  {
    using Length = VerticalAlignLength;

    VerticalAlign(css::keywords::Baseline keyword) noexcept : _value {keyword}
    {
    }
    VerticalAlign(css::keywords::Sub keyword) noexcept : _value {keyword}
    {
    }
    VerticalAlign(css::keywords::Super keyword) noexcept : _value {keyword}
    {
    }
    VerticalAlign(css::keywords::Top keyword) noexcept : _value {keyword}
    {
    }
    VerticalAlign(css::keywords::TextTop keyword) noexcept : _value {keyword}
    {
    }
    VerticalAlign(css::keywords::Middle keyword) noexcept : _value {keyword}
    {
    }
    VerticalAlign(css::keywords::Bottom keyword) noexcept : _value {keyword}
    {
    }
    VerticalAlign(css::keywords::TextBottom keyword) noexcept : _value {keyword}
    {
    }
    VerticalAlign(css::keywords::WebkitBaselineMiddle keyword) noexcept : _value {keyword}
    {
    }
    VerticalAlign(Length &&length) noexcept : _value {krys::move(length)}
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
    KRYS_NODISCARD bool IsTop() const noexcept
    {
      return krys::HoldsAlternative<css::keywords::Top>(_value);
    }
    KRYS_NODISCARD bool IsTextTop() const noexcept
    {
      return krys::HoldsAlternative<css::keywords::TextTop>(_value);
    }
    KRYS_NODISCARD bool IsMiddle() const noexcept
    {
      return krys::HoldsAlternative<css::keywords::Middle>(_value);
    }
    KRYS_NODISCARD bool IsBottom() const noexcept
    {
      return krys::HoldsAlternative<css::keywords::Bottom>(_value);
    }
    KRYS_NODISCARD bool IsTextBottom() const noexcept
    {
      return krys::HoldsAlternative<css::keywords::TextBottom>(_value);
    }
    KRYS_NODISCARD bool IsWebkitBaselineMiddle() const noexcept
    {
      return krys::HoldsAlternative<css::keywords::WebkitBaselineMiddle>(_value);
    }
    KRYS_NODISCARD bool IsLength() const noexcept
    {
      return krys::HoldsAlternative<Length>(_value);
    }
    KRYS_NODISCARD Maybe<Length> TryLength() const noexcept
    {
      return IsLength() ? Maybe(std::get<Length>(_value)) : null;
    }

    template <typename U>
    KRYS_NODISCARD bool HoldsAlternative() const noexcept
    {
      return krys::HoldsAlternative<U>(_value);
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(_value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const VerticalAlign &) const noexcept = default;

  private:
    friend struct Blending<VerticalAlign>;

    Variant<css::keywords::Baseline, css::keywords::Sub, css::keywords::Super, css::keywords::Top,
            css::keywords::TextTop, css::keywords::Middle, css::keywords::Bottom, css::keywords::TextBottom,
            css::keywords::WebkitBaselineMiddle, VerticalAlignLength>
      _value;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<VerticalAlign>
  {
    KRYS_NODISCARD VerticalAlign operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<VerticalAlign>
  {
    KRYS_NODISCARD bool CanBlend(const VerticalAlign &, const VerticalAlign &) noexcept;
    KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(const VerticalAlign &,
                                                                      const VerticalAlign &) noexcept;
    KRYS_NODISCARD VerticalAlign Blend(const VerticalAlign &, const VerticalAlign &,
                                       const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::VerticalAlignLength)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::VerticalAlign)
