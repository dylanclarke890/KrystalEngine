#include "Krystal.Booey/SVG/Internals/SVGPathByteStream.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathUtilities.hpp"

namespace krys::boo::svg
{
  SVGPathByteStream::SVGPathByteStream(dom::DOMStringView string) noexcept
  {
    (void)BuildSVGPathByteStreamFromString(string, *this, PathParsingMode::UnalteredParsing);
  }

  const SVGPathByteStream &SVGPathByteStream::Empty() noexcept
  {
    static const SVGPathByteStream empty;
    return empty;
  }
}
