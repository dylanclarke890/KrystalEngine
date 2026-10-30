#pragma once

#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"

namespace krys::boo::style
{
  // <'perspective'> = none | <length [0,∞]>
  // https://drafts.csswg.org/css-transforms-2/#propdef-perspective
  struct Perspective : css::ValueOrKeyword<Length<css::NonNegative, float>, css::keywords::None>
  {
    using Base::Base;
    using Length = typename Base::Value;

    KRYS_NODISCARD float UsedPerspective() const noexcept
    {
      return std::max(1.0f, TryValue().value_or(1.0f).ResolveZoom(style::ZoomNeeded {}));
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return IsKeyword();
    }
    KRYS_NODISCARD bool IsLength() const noexcept
    {
      return IsValue();
    }
  };
  static_assert(sizeof(Perspective) == sizeof(float));

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<Perspective>
  {
    KRYS_NODISCARD Perspective operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<Perspective>
  {
    KRYS_NODISCARD bool CanBlend(const Perspective &, const Perspective &) noexcept;
    KRYS_NODISCARD Perspective Blend(const Perspective &, const Perspective &,
                                     const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::Perspective)
