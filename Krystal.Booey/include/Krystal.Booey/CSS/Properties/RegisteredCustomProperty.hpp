#pragma once

#include "Krystal.Booey/CSS/Parser/CustomPropertySyntax.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/VariableData.hpp"
#include "Krystal.Booey/Style/CustomProperty.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief A custom property registered with `@property` or `CSS.registerProperty()`.
  struct RegisteredCustomProperty
  {
    CSSOMStringAtom Name;
    CustomPropertySyntax Syntax;
    bool Inherits {false};
    RefPtr<const style::CustomProperty> InitialValue;
    // The initial value's tokens, when it depends on the viewport's size (and is reparsed when that changes).
    RefPtr<const VariableData> InitialValueTokensForViewportUnits;
  };
}
