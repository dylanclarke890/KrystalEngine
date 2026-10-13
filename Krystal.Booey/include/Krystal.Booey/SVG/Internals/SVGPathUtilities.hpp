#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathConsumer.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::svg
{
  class SVGPathByteStream;

  // FIXME: The Path, SVGPathSegList, blending, accumulation, traversal and absolute-coordinate helpers aren't
  // ported yet; they need the platform Path and the SVG DOM.

  // SVGPathByteStream <-> String
  KRYS_NODISCARD bool BuildSVGPathByteStreamFromString(dom::DOMStringView d, SVGPathByteStream &result,
                                                       PathParsingMode parsingMode) noexcept;
  KRYS_NODISCARD bool BuildStringFromByteStream(const SVGPathByteStream &stream, dom::DOMString &result,
                                                PathParsingMode parsingMode,
                                                bool checkForInitialMoveTo = true) noexcept;
}
