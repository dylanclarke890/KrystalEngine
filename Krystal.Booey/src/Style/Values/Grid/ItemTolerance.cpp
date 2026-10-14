#include "Krystal.Booey/Style/Values/Grid/ItemTolerance.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  // MARK: - Conversion

  ItemTolerance CSSValueConversion<ItemTolerance>::operator()(BuilderState &state,
                                                              const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Normal:   return css::keywords::Normal {};
        case css::ValueId::Infinite: return css::keywords::Infinite {};
        default:                     break;
      }

      // If not a keyword, it must be a <length-percentage [0,∞]>
      return ConvertCSSValueToStyle<LengthPercentage<css::NonNegativeUnzoomed>>(state, *primitiveValue);
    }

    state.SetCurrentPropertyInvalidAtComputedValueTime();
    return css::keywords::Normal {};
  }

  // MARK: - Blending

  bool Blending<ItemTolerance>::CanBlend(const ItemTolerance &a, const ItemTolerance &b) noexcept
  {
    // Can only blend if both are length-percentage values
    return !a.IsNormal() && !a.IsInfinite() && !b.IsNormal() && !b.IsInfinite();
  }

  ItemTolerance Blending<ItemTolerance>::Blend(const ItemTolerance &a, const ItemTolerance &b,
                                               const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }

    krys_debug_assert(!a.IsNormal());
    krys_debug_assert(!a.IsInfinite());
    krys_debug_assert(!b.IsNormal());
    krys_debug_assert(!b.IsInfinite());

    // Both must be length-percentage values
    return a.SwitchOn(
      [&](const LengthPercentage<css::NonNegativeUnzoomed> &aValue)
      {
        return b.SwitchOn([&](const LengthPercentage<css::NonNegativeUnzoomed> &bValue) -> ItemTolerance
                          { return krys::boo::style::Blend(aValue, bValue, context); },
                          [&](auto) -> ItemTolerance
                          {
                            krys_debug_assert_msg(false, "Unreachable");
                            return a;
                          });
      },
      [&](auto) -> ItemTolerance
      {
        krys_debug_assert_msg(false, "Unreachable");
        return a;
      });
  }
}
