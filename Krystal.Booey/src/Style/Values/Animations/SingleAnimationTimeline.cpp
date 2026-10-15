#include "Krystal.Booey/Style/Values/Animations/SingleAnimationTimeline.hpp"

#include "Krystal.Booey/CSS/PrimitiveValueMappings.hpp"
#include "Krystal.Booey/CSS/Values/ScrollValue.hpp"
#include "Krystal.Booey/CSS/Values/ViewValue.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/CSSValueConversion.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  SingleAnimationTimeline
    CSSValueConversion<SingleAnimationTimeline>::operator()(BuilderState &state,
                                                            const css::Value &value) noexcept
  {
    if (const auto *scrollValue = DynamicDowncast<css::ScrollValue>(value))
    {
      return ConvertCSSValueToStyle<ScrollFunction>(state, *scrollValue);
    }

    if (const auto *viewValue = DynamicDowncast<css::ViewValue>(value))
    {
      return ConvertCSSValueToStyle<ViewFunction>(state, *viewValue);
    }

    const auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::Auto {};
    }

    switch (primitiveValue->ValueId())
    {
      case css::ValueId::Auto: return css::keywords::Auto {};
      case css::ValueId::None: return css::keywords::None {};
      default:                 break;
    }

    return ConvertCSSValueToStyle<css::CustomIdentifier>(state, *primitiveValue);
  }
}
