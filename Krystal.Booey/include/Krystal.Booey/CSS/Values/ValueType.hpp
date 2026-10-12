#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::css
{
  /// @note The order matters: Value::IsImageGeneratorValue() and Value::ContainsValueList() check ranges.
  enum class ValueType : uint8
  {
    Primitive,

    // Image classes.
    Image,
    ImageSetOption,
    CursorImage,
    // Image generator classes.
    Canvas,
    PaintImage,
    NamedImage,
    CrossFade,
    FilterImage,
    Gradient,

    // Other non-list classes.
    AppleColorFilter,
    Attr,
    BackgroundRepeat,
    BasicShape,
    BorderImageSlice,
    BorderImageWidth,
    BoxShadowProperty,
    Color,
    ColorScheme,
    Counter,
    CustomProperty,
    DynamicRangeLimit,
    EasingFunction,
    Filter,
    Font,
    FontFaceSrcLocal,
    FontFaceSrcResource,
    FontFeature,
    FontStyleRange,
    FontStyleWithAngle,
    FontVariation,
    GridLineNames,
    GridLine,
    GridTemplateAreas,
    OffsetRotate,
    Path,
    PendingSubstitution,
    Position,
    PositionX,
    PositionY,
    Quad,
    Ratio,
    Ray,
    Rect,
    Reflect,
    Scroll,
    TextShadowProperty,
    Url,
    UnicodeRange,
    ValuePair,
    VariableReference,
    View,

    // Classes that contain lists, which derive from ValueWithSmallList.
    ValueList,
    Function,
    GridAutoRepeat,
    GridIntegerRepeat,
    ImageSet,
    Subgrid,
    TransformList,
    // Do not append classes here unless they derive from ValueWithSmallList.
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::css::ValueType, 59uz);
