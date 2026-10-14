#include "Krystal.Booey/Style/Values/Text/TabSize.hpp"

#include "Krystal.Booey/Graphics/TabSize.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  TabSize CSSValueConversion<TabSize>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return 8_css_number;
    }

    if (primitiveValue->IsNumber())
    {
      return ConvertCSSValueToStyle<TabSize::Spaces>(state, *primitiveValue);
    }
    return ConvertCSSValueToStyle<TabSize::Length>(state, *primitiveValue);
  }

  // MARK: - Blending

  bool Blending<TabSize>::CanBlend(const TabSize &a, const TabSize &b) noexcept
  {
    return a.HasSameType(b);
  }

  TabSize Blending<TabSize>::Blend(const TabSize &a, const TabSize &b,
                                   const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }

    krys_debug_assert(CanBlend(a, b));
    if (a.IsSpaces())
    {
      krys_debug_assert(b.IsSpaces());
      return style::Blend(std::get<TabSize::Spaces>(a._value), std::get<TabSize::Spaces>(b._value), context);
    }
    else
    {
      krys_debug_assert(a.IsLength());
      krys_debug_assert(b.IsLength());
      return style::Blend(std::get<TabSize::Length>(a._value), std::get<TabSize::Length>(b._value), context);
    }
  }

  // MARK: - Platform

  boo::TabSize ToPlatform<TabSize>::operator()(const TabSize &value) noexcept
  {
    return krys::SwitchOn(
      value, [](const TabSize::Spaces &spaces) { return boo::TabSize {spaces.Value, SpaceValueType}; },
      [](const TabSize::Length &length)
      { return boo::TabSize {Evaluate<float>(length, ZoomNeeded {}), LengthValueType}; });
  }
}
