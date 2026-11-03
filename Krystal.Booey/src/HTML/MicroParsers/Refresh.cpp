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

    // (Step 1 is the caller's.)
    // 2. Let position point at the first code point of input.
    // 3. Skip ASCII whitespace within input given position.
    skipWhitespace();

    // 4. Let time be 0.
    // 5. Collect a sequence of code points that are ASCII digits from input given position, and let
    //    timeString be the result.
    auto numberStart = position;
    while (position < input.size() && text::IsASCIIDigit(input[position]))
    {
      ++position;
    }

    uint64 time = 0;
    auto timeString = input.substr(numberStart, position - numberStart);

    // 6. If timeString is the empty string:
    if (timeString.empty())
    {
      // 6a. If the code point in input pointed to by position is not U+002E (.), then return.
      if (position >= input.size() || input[position] != u8'.')
      {
        return null;
      }
    }
    // 7. Otherwise, set time to the result of parsing timeString using the rules for parsing non-negative
    //    integers.
    else
    {
      auto number = Numbers::ParseNonNegativeInteger(timeString);
      if (!number.Success())
      {
        return null;
      }
      time = number.Value;
    }

    // 8. Collect a sequence of code points that are ASCII digits and U+002E FULL STOP characters (.) from
    // input
    //    given position. Ignore any collected characters.
    while (position < input.size() && (text::IsASCIIDigit(input[position]) || input[position] == u8'.'))
    {
      ++position;
    }

    // 9. Let urlRecord be document's URL.
    // (An empty URL, which the caller takes as the document's.)
    RefreshDirective directive;
    directive.Delay = static_cast<double>(time);

    // 10. If position is not past the end of input:
    if (position >= input.size())
    {
      return directive;
    }

    // 10a. If the code point in input pointed to by position is not U+003B (;), U+002C (,), or ASCII
    //      whitespace, then return.
    if (input[position] != u8';' && input[position] != u8',' && !text::IsASCIIWhitespace(input[position]))
    {
      return null;
    }

    // 10b. Skip ASCII whitespace within input given position.
    skipWhitespace();

    // 10c. If the code point in input pointed to by position is U+003B (;) or U+002C (,), then advance
    //      position to the next code point.
    if (position < input.size() && (input[position] == u8';' || input[position] == u8','))
    {
      ++position;
    }

    // 10d. Skip ASCII whitespace within input given position.
    skipWhitespace();

    // 11. If position is not past the end of input:
    if (position >= input.size())
    {
      return directive;
    }

    // 11a. Let urlString be the substring of input from the code point at position to the end of the string.
    // 11b. If the code point in input pointed to by position is U+0055 (U) or U+0075 (u), then advance
    //      position to the next code point. Otherwise, jump to the step labeled skip quotes.
    // 11c. If the code point in input pointed to by position is U+0052 (R) or U+0072 (r), then advance
    //      position to the next code point. Otherwise, jump to the step labeled parse.
    // 11d. If the code point in input pointed to by position is U+004C (L) or U+006C (l), then advance
    //      position to the next code point. Otherwise, jump to the step labeled parse.
    // 11e. Skip ASCII whitespace within input given position.
    // 11f. If the code point in input pointed to by position is U+003D (=), then advance position to the next
    //      code point. Otherwise, jump to the step labeled parse.
    // 11g. Skip ASCII whitespace within input given position.
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

    // 11h. Skip quotes: If the code point in input pointed to by position is U+0027 (') or U+0022 ("), then
    // let
    //      quote be that code point, and advance position to the next code point. Otherwise, let quote be the
    //      empty string.
    // 11i. Set urlString to the substring of input from the code point at position to the end of the string.
    // 11j. If quote is not the empty string, and there is a code point in urlString equal to quote, then
    //      truncate urlString at that code point, so that it and all subsequent code points are removed.
    // (Steps 11k to 13 are the caller's.)
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
