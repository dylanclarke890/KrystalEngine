#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::css
{
  enum class RuleType : uint8
  {
    Unknown,
    Charset,
    Import,
    Margin,
    Media,
    Namespace,
    NestedDeclarations,
    Page,
    Style,
    StyleWithNesting,
    FontFace,
    Keyframes,
    Keyframe,
    CounterStyle,
    Supports,
    FontFeatureValues,
    FontFeatureValuesBlock,
    Viewport,
    FunctionDeclarations,
    FontPaletteValues,
    Property,
    ViewTransition,
    PositionTry,
    Function,
    InternalBaseAppearance,
    Scope,
    StartingStyle,
    Container,
    LayerBlock,
    LayerStatement,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::css::RuleType, 22uz);