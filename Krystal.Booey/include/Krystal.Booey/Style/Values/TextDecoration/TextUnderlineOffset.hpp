#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"

namespace krys::boo::style
{
  // <'text-underline-offset'> = auto | <length-percentage>
  // https://drafts.csswg.org/css-text-decor-4/#propdef-text-underline-offset
  struct TextUnderlineOffset : LengthWrapperBase<LengthPercentage<css::AllUnzoomed>, css::keywords::Auto>
  {
    using Base::Base;

    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return HoldsAlternative<css::keywords::Auto>();
    }

    KRYS_NODISCARD float Resolve(const RenderStyle &, float autoValue = 0.0f) const noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<TextUnderlineOffset>
  {
    KRYS_NODISCARD bool CanBlend(const TextUnderlineOffset &, const TextUnderlineOffset &,
                                 const RenderStyle &, const RenderStyle &) noexcept;
    KRYS_NODISCARD TextUnderlineOffset Blend(const TextUnderlineOffset &, const TextUnderlineOffset &,
                                             const RenderStyle &, const RenderStyle &,
                                             const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::TextUnderlineOffset)
