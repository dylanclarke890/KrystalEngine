#include "Krystal.Booey/SVG/SVGLengthList.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Booey/SVG/SVGParsingError.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::svg
{
  bool SVGLengthList::Parse(dom::DOMStringView value) noexcept
  {
    ClearItems();

    auto buffer = value;
    SkipOptionalSVGSpaces(buffer);
    while (!buffer.empty())
    {
      auto start = buffer;
      while (!buffer.empty() && !IsSVGSpaceOrComma(buffer.front()))
      {
        buffer.remove_prefix(1);
      }

      if (buffer.size() == start.size())
      {
        break;
      }

      SVGParsingError parseError {SVGParsingError::None};
      auto length =
        SVGLengthValue::Construct(_lengthMode, start.substr(0, start.size() - buffer.size()), parseError);
      if (parseError != SVGParsingError::None)
      {
        break;
      }

      Append(SVGLength::Create(krys::move(length)));
      SkipOptionalSVGSpacesOrDelimiter(buffer);
    }

    // FIXME: Should this ClearItems() on failure like SVGTransformList does?
    return buffer.empty();
  }

  dom::DOMString SVGLengthList::ValueAsString() const noexcept
  {
    StringBuilder builder;
    for (const auto &length : _items)
    {
      if (builder.Length())
      {
        builder.Append(u8' ');
      }

      builder.Append(length->Value().ValueAsString());
    }
    return krys::move(builder).ToString();
  }
}
