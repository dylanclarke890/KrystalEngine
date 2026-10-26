#include "Krystal.Booey/SVG/SVGStringList.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::svg
{
  bool SVGStringList::Parse(dom::DOMStringView data, char8 delimiter) noexcept
  {
    ClearItems();

    auto isSVGSpaceOrDelimiter = [delimiter](char8 c)
    {
      return text::IsASCIIWhitespace(c) || c == delimiter;
    };

    auto buffer = data;
    SkipOptionalSVGSpaces(buffer);
    while (!buffer.empty())
    {
      auto start = buffer;

      while (!buffer.empty() && !isSVGSpaceOrDelimiter(buffer.front()))
      {
        buffer.remove_prefix(1);
      }

      if (buffer.size() == start.size())
      {
        break;
      }

      _items.push_back(dom::DOMString {start.substr(0, start.size() - buffer.size())});
      SkipOptionalSVGSpacesOrDelimiter(buffer, static_cast<char>(delimiter));
    }

    // FIXME: Should this ClearItems() on failure like SVGTransformList does?
    return buffer.empty();
  }

  dom::DOMString SVGStringList::ValueAsString() const noexcept
  {
    StringBuilder builder;
    for (const auto &string : _items)
    {
      if (builder.Length())
      {
        builder.Append(u8' ');
      }

      builder.Append(string);
    }
    return krys::move(builder).ToString();
  }
}
