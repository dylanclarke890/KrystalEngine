#include "Krystal.Booey/Style/Values/Masking/Clip.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/RectValue.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/CSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  Clip CSSValueConversion<Clip>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    if (css::IsValueId(value, css::ValueId::Auto))
    {
      return css::keywords::Auto {};
    }

    const auto *rectValue = RequiredDowncast<css::RectValue>(state, value);
    if (!rectValue)
    {
      return css::keywords::Auto {};
    }

    const auto *primitiveValueTop = RequiredDowncast<css::PrimitiveValue>(state, rectValue->GetRect().Top());
    if (!primitiveValueTop)
    {
      return css::keywords::Auto {};
    }
    const auto *primitiveValueRight =
      RequiredDowncast<css::PrimitiveValue>(state, rectValue->GetRect().Right());
    if (!primitiveValueRight)
    {
      return css::keywords::Auto {};
    }
    const auto *primitiveValueBottom =
      RequiredDowncast<css::PrimitiveValue>(state, rectValue->GetRect().Bottom());
    if (!primitiveValueBottom)
    {
      return css::keywords::Auto {};
    }
    const auto *primitiveValueLeft =
      RequiredDowncast<css::PrimitiveValue>(state, rectValue->GetRect().Left());
    if (!primitiveValueLeft)
    {
      return css::keywords::Auto {};
    }

    auto convertEdge = [&](const css::PrimitiveValue &primitiveValue) -> ClipEdge
    {
      if (css::IsValueId(primitiveValue, css::ValueId::Auto))
      {
        return css::keywords::Auto {};
      }
      return ConvertCSSValueToStyle<Length<>>(state, primitiveValue);
    };

    return ClipRect {
      convertEdge(*primitiveValueTop),
      convertEdge(*primitiveValueRight),
      convertEdge(*primitiveValueBottom),
      convertEdge(*primitiveValueLeft),
    };
  }

  Ref<css::Value> CSSValueCreation<ClipRect>::operator()(css::ValuePool &pool, const RenderStyle &style,
                                                         const ClipRect &clipRect) noexcept
  {
    return css::RectValue::Create({
      CreateCSSValue(pool, style, clipRect.Value->Top()),
      CreateCSSValue(pool, style, clipRect.Value->Right()),
      CreateCSSValue(pool, style, clipRect.Value->Bottom()),
      CreateCSSValue(pool, style, clipRect.Value->Left()),
    });
  }

  // MARK: - Blending

  template <>
  struct Blending<ClipEdge>
  {
    KRYS_NODISCARD bool CanBlend(const ClipEdge &, const ClipEdge &) noexcept;
    KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(const ClipEdge &,
                                                                      const ClipEdge &) noexcept;
    KRYS_NODISCARD ClipEdge Blend(const ClipEdge &, const ClipEdge &, const BlendingContext &) noexcept;
  };

  template <>
  struct Blending<ClipRect>
  {
    KRYS_NODISCARD bool CanBlend(const ClipRect &, const ClipRect &) noexcept;
    KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(const ClipRect &,
                                                                      const ClipRect &) noexcept;
    KRYS_NODISCARD ClipRect Blend(const ClipRect &, const ClipRect &, const BlendingContext &) noexcept;
  };

  bool Blending<ClipEdge>::CanBlend(const ClipEdge &a, const ClipEdge &b) noexcept
  {
    return a.IsAuto() == b.IsAuto();
  }

  bool Blending<ClipEdge>::RequiresInterpolationForAccumulativeIteration(const ClipEdge &a,
                                                                         const ClipEdge &b) noexcept
  {
    return a.IsAuto() != b.IsAuto();
  }

  ClipEdge Blending<ClipEdge>::Blend(const ClipEdge &a, const ClipEdge &b,
                                     const BlendingContext &context) noexcept
  {
    if (a.IsAuto() || b.IsAuto())
    {
      return context.Progress < 0.5 ? a : b;
    }

    krys_debug_assert(CanBlend(a, b));
    return style::Blend(*a.TryValue(), *b.TryValue(), context);
  }

  bool Blending<ClipRect>::CanBlend(const ClipRect &a, const ClipRect &b) noexcept
  {
    return style::CanBlend(a.Value->Top(), b.Value->Top())
           && style::CanBlend(a.Value->Right(), b.Value->Right())
           && style::CanBlend(a.Value->Bottom(), b.Value->Bottom())
           && style::CanBlend(a.Value->Left(), b.Value->Left());
  }

  bool Blending<ClipRect>::RequiresInterpolationForAccumulativeIteration(const ClipRect &a,
                                                                         const ClipRect &b) noexcept
  {
    return style::RequiresInterpolationForAccumulativeIteration(a.Value->Top(), b.Value->Top())
           && style::RequiresInterpolationForAccumulativeIteration(a.Value->Right(), b.Value->Right())
           && style::RequiresInterpolationForAccumulativeIteration(a.Value->Bottom(), b.Value->Bottom())
           && style::RequiresInterpolationForAccumulativeIteration(a.Value->Left(), b.Value->Left());
  }

  ClipRect Blending<ClipRect>::Blend(const ClipRect &a, const ClipRect &b,
                                     const BlendingContext &context) noexcept
  {
    return ClipRect {
      style::Blend(a.Value->Top(), b.Value->Top(), context),
      style::Blend(a.Value->Right(), b.Value->Right(), context),
      style::Blend(a.Value->Bottom(), b.Value->Bottom(), context),
      style::Blend(a.Value->Left(), b.Value->Left(), context),
    };
  }

  bool Blending<Clip>::CanBlend(const Clip &a, const Clip &b) noexcept
  {
    return !a.IsAuto() && !b.IsAuto() && style::CanBlend(*a._value, *b._value);
  }

  bool Blending<Clip>::RequiresInterpolationForAccumulativeIteration(const Clip &a, const Clip &b) noexcept
  {
    return style::RequiresInterpolationForAccumulativeIteration(
      a._value.value_or(ClipRect {css::keywords::Auto {}}),
      b._value.value_or(ClipRect {css::keywords::Auto {}}));
  }

  Clip Blending<Clip>::Blend(const Clip &a, const Clip &b, const BlendingContext &context) noexcept
  {
    return style::Blend(a._value.value_or(ClipRect {css::keywords::Auto {}}),
                        b._value.value_or(ClipRect {css::keywords::Auto {}}), context);
  }
}
