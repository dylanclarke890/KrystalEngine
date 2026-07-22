#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class CSSRuleType : uint8
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
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::CSSRuleType, 17uz);