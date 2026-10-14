#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo
{
  /// @brief A property that can be animated: a property id, or a custom property's name.
  using AnimatableCSSProperty = Variant<css::PropertyId, css::CSSOMStringAtom>;
}
