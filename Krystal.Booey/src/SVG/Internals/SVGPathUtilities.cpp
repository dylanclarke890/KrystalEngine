#include "Krystal.Booey/SVG/Internals/SVGPathUtilities.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathByteStream.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathByteStreamSource.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathParser.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathStringViewSource.hpp"

namespace krys::boo::svg
{
  bool BuildStringFromByteStream(const SVGPathByteStream &stream, dom::DOMString &result,
                                 PathParsingMode parsingMode, bool checkForInitialMoveTo) noexcept
  {
    if (stream.IsEmpty())
    {
      return true;
    }

    SVGPathByteStreamSource source(stream);
    return SVGPathParser::ParseToString(source, result, parsingMode, checkForInitialMoveTo);
  }

  bool BuildSVGPathByteStreamFromString(dom::DOMStringView d, SVGPathByteStream &result,
                                        PathParsingMode parsingMode) noexcept
  {
    result.Clear();
    if (d.empty())
    {
      return true;
    }

    SVGPathStringViewSource source(d);
    return SVGPathParser::ParseToByteStream(source, result, parsingMode);
  }
}
