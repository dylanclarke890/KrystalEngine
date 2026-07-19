#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class CSSRuleType : uint8
  {
    Unknown,
    Style,
    Charset,
    Import,
    Media,
    FontFace,
    Page,
    Keyframes,
    Keyframe,
    Margin,
    Namespace,
    CounterStyle,
    Supports,
    FontFeatureValues,
    Viewport,
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
    InternalBaseAppearance, // For UA stylesheets only.
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::CSSRuleType, 29uz);