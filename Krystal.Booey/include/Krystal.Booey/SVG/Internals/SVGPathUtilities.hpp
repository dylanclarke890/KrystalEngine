/*
 * Copyright (C) Research In Motion Limited 2010, 2012. All rights reserved.
 * Copyright (C) 2015-2019 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatPoint.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathConsumer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class Path;
}

namespace krys::boo::svg
{
  class SVGPathByteStream;

  // String/SVGPathByteStream -> Path
  KRYS_NODISCARD Path BuildPathFromString(dom::DOMStringView d) noexcept;
  KRYS_NODISCARD Path BuildPathFromByteStream(const SVGPathByteStream &stream) noexcept;

  // SVGPathByteStream <-> String
  KRYS_NODISCARD bool BuildSVGPathByteStreamFromString(dom::DOMStringView d, SVGPathByteStream &result,
                                                       PathParsingMode parsingMode) noexcept;
  KRYS_NODISCARD bool BuildStringFromByteStream(const SVGPathByteStream &stream, dom::DOMString &result,
                                                PathParsingMode parsingMode,
                                                bool checkForInitialMoveTo = true) noexcept;

  // SVGPathByteStream blending
  KRYS_NODISCARD bool CanBlendSVGPathByteStreams(const SVGPathByteStream &fromStream,
                                                 const SVGPathByteStream &toStream) noexcept;
  KRYS_NODISCARD bool BuildAnimatedSVGPathByteStream(const SVGPathByteStream &fromStream,
                                                     const SVGPathByteStream &toStream,
                                                     SVGPathByteStream &result, float progress) noexcept;
  KRYS_NODISCARD bool AddToSVGPathByteStream(SVGPathByteStream &streamToAppendTo,
                                             const SVGPathByteStream &byStream,
                                             uint32 repeatCount = 1) noexcept;

  KRYS_NODISCARD Maybe<SVGPathByteStream>
    ConvertSVGPathByteStreamToAbsoluteCoordinates(const SVGPathByteStream &stream) noexcept;

  // SVGPathByteStream traversal
  KRYS_NODISCARD float GetTotalLengthOfSVGPathByteStream(const SVGPathByteStream &) noexcept;
  KRYS_NODISCARD FloatPoint GetPointAtLengthOfSVGPathByteStream(const SVGPathByteStream &,
                                                                float length) noexcept;
}
