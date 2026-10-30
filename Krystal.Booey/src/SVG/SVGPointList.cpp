#include "Krystal.Booey/SVG/SVGPointList.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::svg
{
  bool SVGPointList::Parse(dom::DOMStringView value) noexcept
  {
    ClearItems();

    auto buffer = value;
    SkipOptionalSVGSpaces(buffer);

    bool delimParsed = false;
    while (!buffer.empty())
    {
      delimParsed = false;
      auto xPos = ParseNumber(buffer);
      if (!xPos)
      {
        return false;
      }

      auto yPos = ParseNumber(buffer, SuffixSkippingPolicy::DontSkip);
      if (!yPos)
      {
        return false;
      }

      SkipOptionalSVGSpaces(buffer);

      if (!buffer.empty() && buffer.front() == u8',')
      {
        buffer.remove_prefix(1);
        delimParsed = true;
      }

      SkipOptionalSVGSpaces(buffer);

      Append(SVGPoint::Create({*xPos, *yPos}));
    }

    // FIXME: Should this ClearItems() on failure like SVGTransformList does?
    return !delimParsed;
  }

  dom::DOMString SVGPointList::ValueAsString() const noexcept
  {
    StringBuilder builder;
    for (const auto &point : _items)
    {
      if (builder.Length())
      {
        builder.Append(u8' ');
      }

      builder.Append(point->X(), u8' ', point->Y());
    }
    return krys::move(builder).ToString();
  }
}
