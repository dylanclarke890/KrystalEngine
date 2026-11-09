/*
 * (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004-2023 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

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
    PaintImage,
    CrossFade,
    FilterImage,
    Gradient,

    // Other non-list classes.
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

    // Classes that contain lists, which derive from ValueContainingList.
    ValueList,
    Function,
    GridAutoRepeat,
    GridIntegerRepeat,
    ImageSet,
    Subgrid,
    TransformList,
    // Do not append classes here unless they derive from ValueContainingList.
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::css::ValueType, 59uz);
