#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::css
{
  /// @brief The type of a rule in the internal rule model.
  /// @note Values up to `FontFeatureValues` are the CSSOM `CSSRule.type` constants.
  /// @see https://drafts.csswg.org/cssom/#the-cssrule-interface
  enum class RuleType : uint8
  {
    Style = 1,
    Charset = 2,
    Import = 3,
    Media = 4,
    FontFace = 5,
    Page = 6,
    Keyframes = 7,
    Keyframe = 8,
    Margin = 9,
    Namespace = 10,
    CounterStyle = 11,
    Supports = 12,
    FontFeatureValues = 14,
    // Numbers above 14 are not exposed to the web.
    ViewTransition = 15,
    LayerBlock,
    LayerStatement,
    Container,
    FontPaletteValues,
    FontFeatureValuesBlock,
    Property,
    StyleWithNesting,
    Scope,
    StartingStyle,
    NestedDeclarations,
    PositionTry,
    Function,
    FunctionDeclarations,
  };

  constexpr auto FirstUnexposedRuleType = RuleType::ViewTransition;
}

KRYS_DEFINE_ENUM_TRAITS(krys::boo::css::RuleType, 27uz, false, false);
