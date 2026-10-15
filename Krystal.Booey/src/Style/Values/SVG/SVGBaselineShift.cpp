#include "Krystal.Booey/Style/Values/SVG/SVGBaselineShift.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"

namespace krys::boo::style
{
  SVGBaselineShift CSSValueConversion<SVGBaselineShift>::operator()(BuilderState &state,
                                                                    const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::Baseline {};
    }

    if (primitiveValue->IsValueId())
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Baseline: return css::keywords::Baseline {};
        case css::ValueId::Sub:      return css::keywords::Sub {};
        case css::ValueId::Super:    return css::keywords::Super {};
        default:                     break;
      }

      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return css::keywords::Baseline {};
    }

    return ConvertCSSValueToStyle<SVGBaselineShiftLength>(state, *primitiveValue);
  }

  // MARK: - Blending

  bool Blending<SVGBaselineShift>::CanBlend(const SVGBaselineShift &a, const SVGBaselineShift &b) noexcept
  {
    return a._value.index() == b._value.index();
  }

  bool Blending<SVGBaselineShift>::RequiresInterpolationForAccumulativeIteration(
    const SVGBaselineShift &a, const SVGBaselineShift &b) noexcept
  {
    if (a._value.index() != b._value.index())
    {
      return true;
    }
    if (!a.IsLength())
    {
      return false;
    }
    return style::RequiresInterpolationForAccumulativeIteration(*a.TryLength(), *b.TryLength());
  }

  SVGBaselineShift Blending<SVGBaselineShift>::Blend(const SVGBaselineShift &a, const SVGBaselineShift &b,
                                                     const BlendingContext &context) noexcept
  {
    if (!a.IsLength() || !b.IsLength())
    {
      return context.Progress < 0.5 ? a : b;
    }

    krys_debug_assert(CanBlend(a, b));
    return style::Blend(*a.TryLength(), *b.TryLength(), context);
  }
}
