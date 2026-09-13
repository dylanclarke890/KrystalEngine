#pragma once

#include "Krystal.Booey/CSS/Parser/Context/ParsedPropertyList.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Properties/Types/IsImplicit.hpp"
#include "Krystal.Booey/CSS/Properties/Types/IsImportant.hpp"

namespace krys::boo::css
{
  class CSSValue;

  /// @brief Represents the result of parsing a CSS property.
  struct PropertyParserResult
  {
    ParsedPropertyList &ParsedProperties;

    void AddProperty(Property &&property) noexcept;

    // Bottleneck where the CSSValue is added to the CSSProperty vector.
    void AddProperty(PropertyParserState &state, PropertyId property, PropertyId currentShorthand,
                     RefPtr<CSSValue> &&value, IsImportant important,
                     IsImplicit implicit = IsImplicit(false)) noexcept;

#pragma region Utils

    void AddPropertyForCurrentShorthand(PropertyParserState &state, PropertyId property,
                                        RefPtr<CSSValue> &&value,
                                        IsImplicit implicit = IsImplicit(false)) noexcept;

    void AddPropertyForAllLonghandsOfShorthand(PropertyParserState &state, PropertyId shorthand,
                                               RefPtr<CSSValue> &&value, IsImportant important,
                                               IsImplicit implicit = IsImplicit(false)) noexcept;

    void AddPropertyForAllLonghandsOfCurrentShorthand(PropertyParserState &, RefPtr<CSSValue> &&value,
                                                      IsImplicit implicit = IsImplicit(false)) noexcept;

#pragma endregion
  };
}