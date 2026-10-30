#include "Krystal.Booey/SVG/SVGNumberList.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::svg
{
  bool SVGNumberList::Parse(dom::DOMStringView value) noexcept
  {
    ClearItems();

    auto buffer = value;
    SkipOptionalSVGSpaces(buffer);
    while (!buffer.empty())
    {
      auto number = ParseNumber(buffer);
      if (!number)
      {
        break;
      }
      Append(SVGNumber::Create(*number));
    }

    // FIXME: Should this ClearItems() on failure like SVGTransformList does?
    return buffer.empty();
  }

  dom::DOMString SVGNumberList::ValueAsString() const noexcept
  {
    StringBuilder builder;
    for (const auto &number : _items)
    {
      if (builder.Length())
      {
        builder.Append(u8' ');
      }

      builder.Append(number->Value());
    }
    return krys::move(builder).ToString();
  }
}
