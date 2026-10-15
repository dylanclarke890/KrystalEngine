#include "Krystal.Booey/Style/Values/Backgrounds/BorderImageSlice.hpp"

#include "Krystal.Booey/CSS/Values/BorderImageSliceValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
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

  KRYS_NODISCARD static BorderImageSliceValue ConvertBorderImageSliceValue(BuilderState &state,
                                                                           const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return BorderImageSliceValue {100_css_percentage};
    }

    if (primitiveValue->IsNumber())
    {
      return ConvertCSSValueToStyle<BorderImageSliceValue::Number>(state, *primitiveValue);
    }
    return ConvertCSSValueToStyle<BorderImageSliceValue::Percentage>(state, *primitiveValue);
  }

  BorderImageSlice CSSValueConversion<BorderImageSlice>::operator()(BuilderState &state,
                                                                    const css::Value &value) noexcept
  {
    if (auto *sliceValue = DynamicDowncast<css::BorderImageSliceValue>(value))
    {
      auto &slices = sliceValue->Slices();
      return BorderImageSlice {
        ConvertBorderImageSliceValue(state, slices.Top()),
        ConvertBorderImageSliceValue(state, slices.Right()),
        ConvertBorderImageSliceValue(state, slices.Bottom()),
        ConvertBorderImageSliceValue(state, slices.Left()),
        sliceValue->Fill() ? Maybe<css::keywords::Fill> {css::keywords::Fill {}} : std::nullopt,
      };
    }

    // Values coming from CSS Typed OM may not have been converted to a css::BorderImageSliceValue.
    return ConvertBorderImageSliceValue(state, value);
  }

  Ref<css::Value> CSSValueCreation<BorderImageSlice>::operator()(css::ValuePool &pool,
                                                                 const RenderStyle &style,
                                                                 const BorderImageSlice &value) noexcept
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

  inline bool Blending<BorderImageSliceValue>::CanBlend(const BorderImageSliceValue &a,
                                                        const BorderImageSliceValue &b) noexcept
  {
    return a.HasSameType(b);
  }

  inline bool Blending<BorderImageSliceValue>::RequiresInterpolationForAccumulativeIteration(
    const BorderImageSliceValue &a, const BorderImageSliceValue &b) noexcept
  {
    return !a.HasSameType(b);
  }

  inline BorderImageSliceValue Blending<BorderImageSliceValue>::Blend(const BorderImageSliceValue &a,
                                                                      const BorderImageSliceValue &b,
                                                                      const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }

    return std::visit(krys::CreateVisitor([&]<typename T>(const T &a, const T &b) -> BorderImageSliceValue
                                          { return style::Blend(a, b, context); },
                                          [&](const auto &, const auto &) -> BorderImageSliceValue
                                          { krys_unreachable(); }),
                      a._value, b._value);
  }

  bool Blending<BorderImageSlice>::CanBlend(const BorderImageSlice &a, const BorderImageSlice &b) noexcept
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

  bool Blending<BorderImageSlice>::RequiresInterpolationForAccumulativeIteration(
    const BorderImageSlice &a, const BorderImageSlice &b) noexcept
  {
    return style::RequiresInterpolationForAccumulativeIteration(a.Values.Top(), b.Values.Top())
           && style::RequiresInterpolationForAccumulativeIteration(a.Values.Right(), b.Values.Right())
           && style::RequiresInterpolationForAccumulativeIteration(a.Values.Bottom(), b.Values.Bottom())
           && style::RequiresInterpolationForAccumulativeIteration(a.Values.Left(), b.Values.Left());
  }

  BorderImageSlice Blending<BorderImageSlice>::Blend(const BorderImageSlice &a, const BorderImageSlice &b,
                                                     const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }

    return BorderImageSlice {
      style::Blend(a.Values.Top(), b.Values.Top(), context),
      style::Blend(a.Values.Right(), b.Values.Right(), context),
      style::Blend(a.Values.Bottom(), b.Values.Bottom(), context),
      style::Blend(a.Values.Left(), b.Values.Left(), context),
      (!context.Progress || !context.IsDiscrete ? a : b).Fill,
    };
  }
}
