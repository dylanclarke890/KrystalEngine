#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::css
{
  /// @brief The class of a `Value`, which its operations switch on to reach the class's own version.
  enum class ValueType : uint8
  {
    Attr,
    BackgroundRepeat,
    BasicShape,
    BorderImageSlice,
    BorderImageWidth,
    BoxShadowProperty,
    Color,
    ColorScheme,
    Counter,
    CrossFade,
    CursorImage,
    CustomProperty,
    DynamicRangeLimit,
    EasingFunction,
    Filter,
    FilterImage,
    FontFaceSrcLocal,
    FontFaceSrcResource,
    FontFeature,
    FontStyleRange,
    FontStyleWithAngle,
    FontVariation,
    Function,
    Gradient,
    GridAutoRepeat,
    GridIntegerRepeat,
    GridLine,
    GridLineNames,
    GridTemplateAreas,
    Image,
    ImageSet,
    ImageSetOption,
    OffsetRotate,
    PaintImage,
    Path,
    PendingSubstitution,
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
    VariableReference,
    View,
  };

  consteval ValueType MaxEnumValue(ValueType) noexcept
  {
    return ValueType::View;
  }
}
