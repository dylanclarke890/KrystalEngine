#include "Krystal.Booey/Style/Values/Masking/MaskBorderOutset.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/QuadValue.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  // MARK: - Conversion

  KRYS_NODISCARD static MaskBorderOutsetValue ConvertMaskBorderOutsetValue(BuilderState &state,
                                                                           const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return MaskBorderOutsetValue {0_css_number};
    }

    if (primitiveValue->IsNumber())
    {
      return ConvertCSSValueToStyle<MaskBorderOutsetValue::Number>(state, *primitiveValue);
    }
    return ConvertCSSValueToStyle<MaskBorderOutsetValue::Length>(state, *primitiveValue);
  }

  MaskBorderOutset CSSValueConversion<MaskBorderOutset>::operator()(BuilderState &state,
                                                                    const css::Value &value) noexcept
  {
    if (auto *quadValue = DynamicDowncast<css::QuadValue>(value))
    {
      auto &quad = quadValue->GetQuad();
      return MaskBorderOutset {
        ConvertMaskBorderOutsetValue(state, quad.Top()),
        ConvertMaskBorderOutsetValue(state, quad.Right()),
        ConvertMaskBorderOutsetValue(state, quad.Bottom()),
        ConvertMaskBorderOutsetValue(state, quad.Left()),
      };
    }

    // Values coming from CSS Typed OM may not have been converted to a Quad.
    return ConvertMaskBorderOutsetValue(state, value);
  }

  Ref<css::Value> CSSValueCreation<MaskBorderOutset>::operator()(css::ValuePool &pool,
                                                                 const RenderStyle &style,
                                                                 const MaskBorderOutset &value) noexcept
  {
    return css::QuadValue::Create({
      CreateCSSValue(pool, style, value.Values.Top()),
      CreateCSSValue(pool, style, value.Values.Right()),
      CreateCSSValue(pool, style, value.Values.Bottom()),
      CreateCSSValue(pool, style, value.Values.Left()),
    });
  }

  // MARK: - Blending

  inline bool Blending<MaskBorderOutsetValue>::CanBlend(const MaskBorderOutsetValue &a,
                                                        const MaskBorderOutsetValue &b) noexcept
  {
    return a.HasSameType(b);
  }

  inline bool Blending<MaskBorderOutsetValue>::RequiresInterpolationForAccumulativeIteration(
    const MaskBorderOutsetValue &a, const MaskBorderOutsetValue &b) noexcept
  {
    return !a.HasSameType(b);
  }

  inline MaskBorderOutsetValue Blending<MaskBorderOutsetValue>::Blend(const MaskBorderOutsetValue &a,
                                                                      const MaskBorderOutsetValue &b,
                                                                      const BlendingContext &context) noexcept
  {
    if (!a.HasSameType(b))
    {
      return MaskBorderOutsetValue {0_css_px};
    }

    if (!context.Progress && context.IsReplace())
    {
      return a;
    }
    if (context.Progress == 1 && context.IsReplace())
    {
      return b;
    }

    return std::visit(krys::CreateVisitor([&]<typename T>(const T &a, const T &b) -> MaskBorderOutsetValue
                                          { return style::Blend(a, b, context); },
                                          [&](const auto &, const auto &) -> MaskBorderOutsetValue
                                          { krys_unreachable(); }),
                      a._value, b._value);
  }

  bool Blending<MaskBorderOutset>::CanBlend(const MaskBorderOutset &a, const MaskBorderOutset &b) noexcept
  {
    return style::CanBlend(a.Values.Top(), b.Values.Top())
           && style::CanBlend(a.Values.Right(), b.Values.Right())
           && style::CanBlend(a.Values.Bottom(), b.Values.Bottom())
           && style::CanBlend(a.Values.Left(), b.Values.Left());
  }

  bool Blending<MaskBorderOutset>::RequiresInterpolationForAccumulativeIteration(
    const MaskBorderOutset &a, const MaskBorderOutset &b) noexcept
  {
    return style::RequiresInterpolationForAccumulativeIteration(a.Values.Top(), b.Values.Top())
           && style::RequiresInterpolationForAccumulativeIteration(a.Values.Right(), b.Values.Right())
           && style::RequiresInterpolationForAccumulativeIteration(a.Values.Bottom(), b.Values.Bottom())
           && style::RequiresInterpolationForAccumulativeIteration(a.Values.Left(), b.Values.Left());
  }

  MaskBorderOutset Blending<MaskBorderOutset>::Blend(const MaskBorderOutset &a, const MaskBorderOutset &b,
                                                     const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }

    return MaskBorderOutset {
      style::Blend(a.Values.Top(), b.Values.Top(), context),
      style::Blend(a.Values.Right(), b.Values.Right(), context),
      style::Blend(a.Values.Bottom(), b.Values.Bottom(), context),
      style::Blend(a.Values.Left(), b.Values.Left(), context),
    };
  }
}
