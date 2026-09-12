#pragma once

#include "Krystal.Booey/CSS/Parser/Context/ParsedPropertyList.hpp"
#include "Krystal.Booey/CSS/Properties/CSSPropertyId.hpp"
#include "Krystal.Booey/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Properties/Types/IsImplicit.hpp"
#include "Krystal.Booey/CSS/Properties/Types/IsImportant.hpp"

namespace krys::boo::css
{
  class CSSValue;

  /// @brief Represents the result of parsing a CSS property.
  struct CSSPropertyParserResult
  {
    ParsedPropertyList &ParsedProperties;

    void AddProperty(Property &&property) noexcept;

    // Bottleneck where the CSSValue is added to the CSSProperty vector.
    void AddProperty(CSSPropertyParserState &state, CSSPropertyId property, CSSPropertyId currentShorthand,
                     RefPtr<CSSValue> &&value, IsImportant important,
                     IsImplicit implicit = IsImplicit(false)) noexcept;

#pragma region Utils

    void AddPropertyForCurrentShorthand(CSSPropertyParserState &state, CSSPropertyId property,
                                        RefPtr<CSSValue> &&value,
                                        IsImplicit implicit = IsImplicit(false)) noexcept;

    void AddPropertyForAllLonghandsOfShorthand(CSSPropertyParserState &state, CSSPropertyId shorthand,
                                               RefPtr<CSSValue> &&value, IsImportant important,
                                               IsImplicit implicit = IsImplicit(false)) noexcept;

    void AddPropertyForAllLonghandsOfCurrentShorthand(CSSPropertyParserState &, RefPtr<CSSValue> &&value,
                                                      IsImplicit implicit = IsImplicit(false)) noexcept;

#pragma endregion
  };
}