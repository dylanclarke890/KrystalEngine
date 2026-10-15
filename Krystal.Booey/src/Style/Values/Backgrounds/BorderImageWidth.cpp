#include "Krystal.Booey/Style/Values/Backgrounds/BorderImageWidth.hpp"

#include "Krystal.Booey/CSS/Values/BorderImageWidthValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  // MARK: - Conversion

  KRYS_NODISCARD static BorderImageWidthValue ConvertBorderImageWidthValue(BuilderState &state,
                                                                           const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return BorderImageWidthValue {1_css_number};
    }

    if (primitiveValue->ValueId() == css::ValueId::Auto)
    {
      return css::keywords::Auto {};
    }

    if (primitiveValue->IsNumber())
    {
      return ConvertCSSValueToStyle<BorderImageWidthValue::Number>(state, *primitiveValue);
    }
    return ConvertCSSValueToStyle<BorderImageWidthValue::LengthPercentage>(state, *primitiveValue);
  }

  BorderImageWidth CSSValueConversion<BorderImageWidth>::operator()(BuilderState &state,
                                                                    const css::Value &value) noexcept
  {
    if (auto *widthValue = DynamicDowncast<css::BorderImageWidthValue>(value))
    {
      auto &widths = widthValue->Widths();
      return BorderImageWidth {
        ConvertBorderImageWidthValue(state, widths.Top()),
        ConvertBorderImageWidthValue(state, widths.Right()),
        ConvertBorderImageWidthValue(state, widths.Bottom()),
        ConvertBorderImageWidthValue(state, widths.Left()),
        widthValue->OverridesBorderWidths(),
      };
    }

    // Values coming from CSS Typed OM may not have been converted to a css::BorderImageWidthValue.
    return ConvertBorderImageWidthValue(state, value);
  }

  Ref<css::Value> CSSValueCreation<BorderImageWidth>::operator()(css::ValuePool &pool,
                                                                 const RenderStyle &style,
                                                                 const BorderImageWidth &value) noexcept
  {
    return css::BorderImageWidthValue::Create(
      {
        CreateCSSValue(pool, style, value.Values.Top()),
        CreateCSSValue(pool, style, value.Values.Right()),
        CreateCSSValue(pool, style, value.Values.Bottom()),
        CreateCSSValue(pool, style, value.Values.Left()),
      },
      value.LegacyWebkitBorderImage);
  }

  // MARK: - Blending

  inline bool Blending<BorderImageWidthValue>::CanBlend(const BorderImageWidthValue &a,
                                                        const BorderImageWidthValue &b) noexcept
  {
    if (a.HasSameType(b))
    {
      return true;
    }

    return (a.IsLengthPercentage() || a.IsNumber()) && (b.IsLengthPercentage() || b.IsNumber())
           && a.IsNumber() == b.IsNumber();
  }

  inline bool Blending<BorderImageWidthValue>::RequiresInterpolationForAccumulativeIteration(
    const BorderImageWidthValue &a, const BorderImageWidthValue &b) noexcept
  {
    return a.IsCalculated() || b.IsCalculated() || !a.HasSameType(b);
  }

  inline BorderImageWidthValue Blending<BorderImageWidthValue>::Blend(const BorderImageWidthValue &a,
                                                                      const BorderImageWidthValue &b,
                                                                      const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }

    return std::visit(krys::CreateVisitor([&]<typename T>(const T &a, const T &b) -> BorderImageWidthValue
                                          { return style::Blend(a, b, context); },
                                          [&](const auto &, const auto &) -> BorderImageWidthValue
                                          { krys_unreachable(); }),
                      a._value, b._value);
  }

  bool Blending<BorderImageWidth>::CanBlend(const BorderImageWidth &a, const BorderImageWidth &b) noexcept
  {
    if (a.LegacyWebkitBorderImage != b.LegacyWebkitBorderImage)
    {
      return false;
    }

    return style::CanBlend(a.Values.Top(), b.Values.Top())
           && style::CanBlend(a.Values.Right(), b.Values.Right())
           && style::CanBlend(a.Values.Bottom(), b.Values.Bottom())
           && style::CanBlend(a.Values.Left(), b.Values.Left());
  }

  bool Blending<BorderImageWidth>::RequiresInterpolationForAccumulativeIteration(
    const BorderImageWidth &a, const BorderImageWidth &b) noexcept
  {
    return style::RequiresInterpolationForAccumulativeIteration(a.Values.Top(), b.Values.Top())
           && style::RequiresInterpolationForAccumulativeIteration(a.Values.Right(), b.Values.Right())
           && style::RequiresInterpolationForAccumulativeIteration(a.Values.Bottom(), b.Values.Bottom())
           && style::RequiresInterpolationForAccumulativeIteration(a.Values.Left(), b.Values.Left());
  }

  BorderImageWidth Blending<BorderImageWidth>::Blend(const BorderImageWidth &a, const BorderImageWidth &b,
                                                     const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }

    return BorderImageWidth {
      style::Blend(a.Values.Top(), b.Values.Top(), context),
      style::Blend(a.Values.Right(), b.Values.Right(), context),
      style::Blend(a.Values.Bottom(), b.Values.Bottom(), context),
      style::Blend(a.Values.Left(), b.Values.Left(), context),
      (!context.Progress || !context.IsDiscrete ? a : b).LegacyWebkitBorderImage,
    };
  }
}
