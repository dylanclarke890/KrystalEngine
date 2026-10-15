#include "Krystal.Booey/SVG/Internals/SVGPathUtilities.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathAbsoluteConverter.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathBlender.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathBuilder.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathByteStream.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathByteStreamBuilder.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathByteStreamSource.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathParser.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathStringViewSource.hpp"

namespace krys::boo::svg
{
  Path BuildPathFromString(dom::DOMStringView d) noexcept
  {
    if (d.empty())
    {
      return {};
    }

    Path path;
    SVGPathBuilder builder(path);
    SVGPathStringViewSource source(d);
    (void)SVGPathParser::Parse(source, builder);
    return path;
  }

  Path BuildPathFromByteStream(const SVGPathByteStream &stream) noexcept
  {
    if (stream.IsEmpty())
    {
      return {};
    }

    // FIXME: Cache the built path on the stream (see `SVGPathByteStream`).
    Path path;
    SVGPathBuilder builder(path);
    SVGPathByteStreamSource source(stream);
    (void)SVGPathParser::Parse(source, builder);
    return path;
  }

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

  bool CanBlendSVGPathByteStreams(const SVGPathByteStream &fromStream,
                                  const SVGPathByteStream &toStream) noexcept
  {
    SVGPathByteStreamSource fromSource(fromStream);
    SVGPathByteStreamSource toSource(toStream);
    return SVGPathBlender::CanBlendPaths(fromSource, toSource);
  }

  bool BuildAnimatedSVGPathByteStream(const SVGPathByteStream &fromStream, const SVGPathByteStream &toStream,
                                      SVGPathByteStream &result, float progress) noexcept
  {
    krys_debug_assert(&toStream != &result);
    result.Clear();
    if (toStream.IsEmpty())
    {
      return true;
    }

    SVGPathByteStreamBuilder builder(result);

    SVGPathByteStreamSource fromSource(fromStream);
    SVGPathByteStreamSource toSource(toStream);
    return SVGPathBlender::BlendAnimatedPath(fromSource, toSource, builder, progress);
  }

  Maybe<SVGPathByteStream>
    ConvertSVGPathByteStreamToAbsoluteCoordinates(const SVGPathByteStream &stream) noexcept
  {
    SVGPathByteStream result;
    if (stream.IsEmpty())
    {
      return result;
    }

    SVGPathByteStreamBuilder builder(result);
    SVGPathAbsoluteConverter converter(builder);

    SVGPathByteStreamSource source(stream);

    if (!SVGPathParser::Parse(source, converter, PathParsingMode::UnalteredParsing, false))
    {
      return std::nullopt;
    }

    return result;
  }
}
