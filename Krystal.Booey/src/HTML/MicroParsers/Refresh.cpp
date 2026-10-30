#include "Krystal.Booey/HTML/MicroParsers/Refresh.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::html
{
  Maybe<RefreshDirective> ParseRefresh(dom::DOMStringView input) noexcept
  {
    size_t position = 0;
    auto skipWhitespace = [&]
    {
      while (position < input.size() && text::IsASCIIWhitespace(input[position]))
      {
        ++position;
      }
    };

    skipWhitespace();

    // The time: digits, then anything of digits and periods.
    auto numberStart = position;
    while (position < input.size() && text::IsASCIIDigit(input[position]))
    {
      ++position;
    }

    uint64 time = 0;
    auto timeString = input.substr(numberStart, position - numberStart);
    if (timeString.empty())
    {
      if (position >= input.size() || input[position] != u8'.')
      {
        return null;
      }
    }
    else
    {
      auto number = Numbers::ParseNonNegativeInteger(timeString);
      if (!number.Success())
      {
        return null;
      }
      time = number.Value;
    }

    while (position < input.size() && (text::IsASCIIDigit(input[position]) || input[position] == u8'.'))
    {
      ++position;
    }

    RefreshDirective directive;
    directive.Delay = static_cast<double>(time);
    if (position >= input.size())
    {
      return directive;
    }

    if (input[position] != u8';' && input[position] != u8',' && !text::IsASCIIWhitespace(input[position]))
    {
      return null;
    }

    skipWhitespace();
    if (position < input.size() && (input[position] == u8';' || input[position] == u8','))
    {
      ++position;
    }
    skipWhitespace();

    if (position >= input.size())
    {
      return directive;
    }

    // An optional "url=", in any case.
    if (input[position] == u8'U' || input[position] == u8'u')
    {
      auto urlStart = position;
      ++position;
      if (position >= input.size() || (input[position] != u8'R' && input[position] != u8'r'))
      {
        directive.URL = dom::DOMString(input.substr(urlStart));
        return directive;
      }
      ++position;
      if (position >= input.size() || (input[position] != u8'L' && input[position] != u8'l'))
      {
        directive.URL = dom::DOMString(input.substr(urlStart));
        return directive;
      }
      ++position;
      skipWhitespace();
      if (position >= input.size() || input[position] != u8'=')
      {
        directive.URL = dom::DOMString(input.substr(urlStart));
        return directive;
      }
      ++position;
      skipWhitespace();
    }

    // The URL, up to its closing quote if it's quoted.
    char8 quote = 0;
    if (position < input.size() && (input[position] == u8'\'' || input[position] == u8'"'))
    {
      quote = input[position++];
    }
    auto url = input.substr(position);
    if (quote)
    {
      if (auto end = url.find(quote); end != dom::DOMStringView::npos)
      {
        url = url.substr(0, end);
      }
    }
    directive.URL = dom::DOMString(url);
    return directive;
  }
}
