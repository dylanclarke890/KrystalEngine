#pragma once

#include "Krystal.HTML/CSS/Parser/Types/ParsedCSSPropertyList.hpp"
#include "Krystal.HTML/CSS/Properties/CSSProperty.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Properties/Enums/CSSPropertyId.hpp"
#include "Krystal.HTML/CSS/Properties/Types/IsImplicit.hpp"
#include "Krystal.HTML/CSS/Properties/Types/IsImportant.hpp"

namespace Krys::HTML
{
  class CSSValue;

  /// @brief Represents the result of parsing a CSS property.
  struct CSSPropertyParserResult
  {
    ParsedCSSPropertyList &ParsedProperties;

    void AddProperty(CSSProperty &&property) noexcept;

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