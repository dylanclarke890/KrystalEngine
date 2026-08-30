#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class CSSValueType : uint8
  {
    Invalid = 0,
    Attr,
    BackgroundRepeat,
    BorderImageSlice,
    BorderImageWidth,
    BoxShadowProperty,
    Color,
    ColorScheme,
    Counter,
    CursorImage,
    GridAutoRepeat,
    GridIntegerRepeat,
    GridLine,
    GridLineNames,
    GridTemplateAreas,
    Function,
    OffsetRotate,
    Position,
    PositionX,
    PositionY,
    Primitive,
    Quad,
    Ratio,
    Ray,
    Rect,
    Reflect,
    Scroll,
    Subgrid,
    TextShadowProperty,
    TransformList,
    UnicodeRange,
    URL,
    ValueList,
    ValuePair,
    View,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::CSSValueType, 35uz);
