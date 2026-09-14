#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::css
{
  enum class ValueType : uint8
  {
    Invalid = 0,
    AppleColorFilter,
    Attr,
    BackgroundRepeat,
    BorderImageSlice,
    BorderImageWidth,
    BoxShadowProperty,
    Color,
    ColorScheme,
    Counter,
    CursorImage,
    Gradient,
    GridAutoRepeat,
    GridIntegerRepeat,
    GridLine,
    GridLineNames,
    GridTemplateAreas,
    EasingFunction,
    Filter,
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
    Url,
    ValueList,
    ValuePair,
    View,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::css::ValueType, 39uz);
