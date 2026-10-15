#include "Krystal.Booey/Style/Values/Masking/MaskBorderSlice.hpp"

#include "Krystal.Booey/CSS/Values/BorderImageSliceValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  // MARK: - Conversion

  KRYS_NODISCARD static MaskBorderSliceValue ConvertMaskBorderSliceValue(BuilderState &state,
                                                                         const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return MaskBorderSliceValue {0_css_number};
    }

    if (primitiveValue->IsNumber())
    {
      return ConvertCSSValueToStyle<MaskBorderSliceValue::Number>(state, *primitiveValue);
    }
    return ConvertCSSValueToStyle<MaskBorderSliceValue::Percentage>(state, *primitiveValue);
  }

  MaskBorderSlice CSSValueConversion<MaskBorderSlice>::operator()(BuilderState &state,
                                                                  const css::Value &value) noexcept
  {
    if (auto *sliceValue = DynamicDowncast<css::BorderImageSliceValue>(value))
    {
      auto &slices = sliceValue->Slices();
      return MaskBorderSlice {
        ConvertMaskBorderSliceValue(state, slices.Top()),
        ConvertMaskBorderSliceValue(state, slices.Right()),
        ConvertMaskBorderSliceValue(state, slices.Bottom()),
        ConvertMaskBorderSliceValue(state, slices.Left()),
        sliceValue->Fill() ? Maybe<css::keywords::Fill> {css::keywords::Fill {}} : std::nullopt,
      };
    }

    // Values coming from CSS Typed OM may not have been converted to a css::BorderImageSliceValue.
    return ConvertMaskBorderSliceValue(state, value);
  }

  Ref<css::Value> CSSValueCreation<MaskBorderSlice>::operator()(css::ValuePool &pool,
                                                                const RenderStyle &style,
                                                                const MaskBorderSlice &value) noexcept
  {
    return css::BorderImageSliceValue::Create(
      {
        CreateCSSValue(pool, style, value.Values.Top()),
        CreateCSSValue(pool, style, value.Values.Right()),
        CreateCSSValue(pool, style, value.Values.Bottom()),
        CreateCSSValue(pool, style, value.Values.Left()),
      },
      value.Fill.has_value());
  }

  // MARK: - Blending

  inline bool Blending<MaskBorderSliceValue>::CanBlend(const MaskBorderSliceValue &a,
                                                       const MaskBorderSliceValue &b) noexcept
  {
    return a.HasSameType(b);
  }

  inline bool Blending<MaskBorderSliceValue>::RequiresInterpolationForAccumulativeIteration(
    const MaskBorderSliceValue &a, const MaskBorderSliceValue &b) noexcept
  {
    return !a.HasSameType(b);
  }

  inline MaskBorderSliceValue Blending<MaskBorderSliceValue>::Blend(const MaskBorderSliceValue &a,
                                                                    const MaskBorderSliceValue &b,
                                                                    const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }

    return std::visit(krys::CreateVisitor([&]<typename T>(const T &a, const T &b) -> MaskBorderSliceValue
                                          { return style::Blend(a, b, context); },
                                          [&](const auto &, const auto &) -> MaskBorderSliceValue
                                          { krys_unreachable(); }),
                      a._value, b._value);
  }

  bool Blending<MaskBorderSlice>::CanBlend(const MaskBorderSlice &a, const MaskBorderSlice &b) noexcept
  {
    if (a.Fill != b.Fill)
    {
      return false;
    }

    return style::CanBlend(a.Values.Top(), b.Values.Top())
           && style::CanBlend(a.Values.Right(), b.Values.Right())
           && style::CanBlend(a.Values.Bottom(), b.Values.Bottom())
           && style::CanBlend(a.Values.Left(), b.Values.Left());
  }

  bool Blending<MaskBorderSlice>::RequiresInterpolationForAccumulativeIteration(
    const MaskBorderSlice &a, const MaskBorderSlice &b) noexcept
  {
    return style::RequiresInterpolationForAccumulativeIteration(a.Values.Top(), b.Values.Top())
           && style::RequiresInterpolationForAccumulativeIteration(a.Values.Right(), b.Values.Right())
           && style::RequiresInterpolationForAccumulativeIteration(a.Values.Bottom(), b.Values.Bottom())
           && style::RequiresInterpolationForAccumulativeIteration(a.Values.Left(), b.Values.Left());
  }

  MaskBorderSlice Blending<MaskBorderSlice>::Blend(const MaskBorderSlice &a, const MaskBorderSlice &b,
                                                   const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }

    return MaskBorderSlice {
      style::Blend(a.Values.Top(), b.Values.Top(), context),
      style::Blend(a.Values.Right(), b.Values.Right(), context),
      style::Blend(a.Values.Bottom(), b.Values.Bottom(), context),
      style::Blend(a.Values.Left(), b.Values.Left(), context),
      (!context.Progress || !context.IsDiscrete ? a : b).Fill,
    };
  }
}
