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
    FontFace,
    Keyframes,
    Keyframe,
    CounterStyle,
    Supports,
    FontFeatureValues,
    Viewport,
    FunctionDeclarations,
    FontPaletteValues,
    Property,
    ViewTransition,
    PositionTry,
    Function,
    InternalBaseAppearance,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::css::RuleType, 22uz);