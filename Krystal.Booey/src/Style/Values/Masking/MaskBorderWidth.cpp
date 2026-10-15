#include "Krystal.Booey/Style/Values/Masking/MaskBorderWidth.hpp"

#include "Krystal.Booey/CSS/Values/BorderImageWidthValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  // MARK: - Conversion

  KRYS_NODISCARD static MaskBorderWidthValue ConvertMaskBorderWidthValue(BuilderState &state,
                                                                         const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::Auto {};
    }

    if (primitiveValue->ValueId() == css::ValueId::Auto)
    {
      return css::keywords::Auto {};
    }

    if (primitiveValue->IsNumber())
    {
      return ConvertCSSValueToStyle<MaskBorderWidthValue::Number>(state, *primitiveValue);
    }
    return ConvertCSSValueToStyle<MaskBorderWidthValue::LengthPercentage>(state, *primitiveValue);
  }

  MaskBorderWidth CSSValueConversion<MaskBorderWidth>::operator()(BuilderState &state,
                                                                  const css::Value &value) noexcept
  {
    if (auto *widthValue = DynamicDowncast<css::BorderImageWidthValue>(value))
    {
      krys_debug_assert(!widthValue->OverridesBorderWidths());
      auto &widths = widthValue->Widths();
      return MaskBorderWidth {
        ConvertMaskBorderWidthValue(state, widths.Top()),
        ConvertMaskBorderWidthValue(state, widths.Right()),
        ConvertMaskBorderWidthValue(state, widths.Bottom()),
        ConvertMaskBorderWidthValue(state, widths.Left()),
      };
    }

    // Values coming from CSS Typed OM may not have been converted to a css::BorderImageWidthValue.
    return ConvertMaskBorderWidthValue(state, value);
  }

  Ref<css::Value> CSSValueCreation<MaskBorderWidth>::operator()(css::ValuePool &pool,
                                                                const RenderStyle &style,
                                                                const MaskBorderWidth &value) noexcept
  {
    return css::BorderImageWidthValue::Create(
      {
        CreateCSSValue(pool, style, value.Values.Top()),
        CreateCSSValue(pool, style, value.Values.Right()),
        CreateCSSValue(pool, style, value.Values.Bottom()),
        CreateCSSValue(pool, style, value.Values.Left()),
      },
      false);
  }

  // MARK: - Blending

  inline bool Blending<MaskBorderWidthValue>::CanBlend(const MaskBorderWidthValue &a,
                                                       const MaskBorderWidthValue &b) noexcept
  {
    if (a.HasSameType(b))
    {
      return true;
    }

    return (a.IsLengthPercentage() || a.IsNumber()) && (b.IsLengthPercentage() || b.IsNumber())
           && a.IsNumber() == b.IsNumber();
  }

  inline bool Blending<MaskBorderWidthValue>::RequiresInterpolationForAccumulativeIteration(
    const MaskBorderWidthValue &a, const MaskBorderWidthValue &b) noexcept
  {
    return a.IsCalculated() || b.IsCalculated() || !a.HasSameType(b);
  }

  inline MaskBorderWidthValue Blending<MaskBorderWidthValue>::Blend(const MaskBorderWidthValue &a,
                                                                    const MaskBorderWidthValue &b,
                                                                    const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }

    return std::visit(krys::CreateVisitor([&]<typename T>(const T &a, const T &b) -> MaskBorderWidthValue
                                          { return style::Blend(a, b, context); },
                                          [&](const auto &, const auto &) -> MaskBorderWidthValue
                                          { krys_unreachable(); }),
                      a._value, b._value);
  }

  bool Blending<MaskBorderWidth>::CanBlend(const MaskBorderWidth &a, const MaskBorderWidth &b) noexcept
  {
    return style::CanBlend(a.Values.Top(), b.Values.Top())
           && style::CanBlend(a.Values.Right(), b.Values.Right())
           && style::CanBlend(a.Values.Bottom(), b.Values.Bottom())
           && style::CanBlend(a.Values.Left(), b.Values.Left());
  }

  bool Blending<MaskBorderWidth>::RequiresInterpolationForAccumulativeIteration(
    const MaskBorderWidth &a, const MaskBorderWidth &b) noexcept
  {
    return style::RequiresInterpolationForAccumulativeIteration(a.Values.Top(), b.Values.Top())
           && style::RequiresInterpolationForAccumulativeIteration(a.Values.Right(), b.Values.Right())
           && style::RequiresInterpolationForAccumulativeIteration(a.Values.Bottom(), b.Values.Bottom())
           && style::RequiresInterpolationForAccumulativeIteration(a.Values.Left(), b.Values.Left());
  }

  MaskBorderWidth Blending<MaskBorderWidth>::Blend(const MaskBorderWidth &a, const MaskBorderWidth &b,
                                                   const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }

    return MaskBorderWidth {
      style::Blend(a.Values.Top(), b.Values.Top(), context),
      style::Blend(a.Values.Right(), b.Values.Right(), context),
      style::Blend(a.Values.Bottom(), b.Values.Bottom(), context),
      style::Blend(a.Values.Left(), b.Values.Left(), context),
    };
  }
}
