#pragma once

#include "Krystal.Booey/Animation/WebAnimationTypes.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo
{
  /// @brief The name of an animatable property: the property's name, or the custom property's name.
  KRYS_NODISCARD inline css::CSSOMStringAtom
    AnimatablePropertyAsString(const AnimatableCSSProperty &property) noexcept
  {
    return krys::SwitchOn(
      property, [](css::PropertyId propertyId) { return css::CSSOMStringAtom {css::ToString(propertyId)}; },
      [](const css::CSSOMStringAtom &customProperty) { return customProperty; });
  }
}
