#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"

namespace krys::boo
{
  class FontMetrics;

  namespace style
  {
    struct TextDecorationThicknessLength : LengthWrapperBase<LengthPercentage<css::AllUnzoomed>>
    {
      using Base::Base;
    };

    // <'text-decoration-thickness'> = auto | from-font | <length-percentage>
    // https://drafts.csswg.org/css-text-decor-4/#propdef-text-decoration-thickness
    struct TextDecorationThickness
    {
      TextDecorationThickness(css::keywords::Auto keyword) noexcept : _value {keyword}
      {
      }

      TextDecorationThickness(css::keywords::FromFont keyword) noexcept : _value {keyword}
      {
      }

      TextDecorationThickness(TextDecorationThicknessLength &&length) noexcept : _value {krys::move(length)}
      {
      }

      KRYS_NODISCARD bool IsAuto() const noexcept
      {
        return krys::HoldsAlternative<css::keywords::Auto>(_value);
      }
      KRYS_NODISCARD bool IsFromFont() const noexcept
      {
        return krys::HoldsAlternative<css::keywords::FromFont>(_value);
      }
      KRYS_NODISCARD bool IsLength() const noexcept
      {
        return krys::HoldsAlternative<TextDecorationThicknessLength>(_value);
      }

      KRYS_NODISCARD float Resolve(const RenderStyle &) const noexcept;

      template <typename... F>
      decltype(auto) SwitchOn(F &&...f) const noexcept
      {
        return krys::SwitchOn(_value, std::forward<F>(f)...);
      }

      KRYS_NODISCARD bool operator==(const TextDecorationThickness &) const noexcept = default;

    private:
      Variant<css::keywords::Auto, css::keywords::FromFont, TextDecorationThicknessLength> _value;
    };

    // MARK: - Conversion

    template <>
    struct CSSValueConversion<TextDecorationThickness>
    {
      KRYS_NODISCARD TextDecorationThickness operator()(BuilderState &, const css::Value &) noexcept;
    };

    // MARK: - Blending

    template <>
    struct Blending<TextDecorationThickness>
    {
      KRYS_NODISCARD bool CanBlend(const TextDecorationThickness &, const TextDecorationThickness &,
                                   const RenderStyle &, const RenderStyle &) noexcept;
      KRYS_NODISCARD TextDecorationThickness Blend(const TextDecorationThickness &,
                                                   const TextDecorationThickness &, const RenderStyle &,
                                                   const RenderStyle &, const BlendingContext &) noexcept;
    };

  }
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::TextDecorationThicknessLength)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::TextDecorationThickness)
