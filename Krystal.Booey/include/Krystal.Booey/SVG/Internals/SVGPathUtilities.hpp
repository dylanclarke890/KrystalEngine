#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Graphics/FloatPoint.hpp"
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
