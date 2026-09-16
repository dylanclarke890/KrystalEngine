#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include <cassert>

namespace krys::boo::infra
{
  /// @see https://infra.spec.whatwg.org/#string-algorithms
  class StringAlgorithms
  {
  public:
    using position_variable = dom::DOMStringView::const_iterator;

    /// @see https://infra.spec.whatwg.org/#strip-leading-and-trailing-ascii-whitespace
    KRYS_NODISCARD static dom::DOMStringView
      StripLeadingAndTrailingWhitespace(dom::DOMStringView input) noexcept
    {
      auto start = input.begin();
      auto end = input.end();

      while (start != end && IsASCIIWhitespace(*start))
      {
        ++start;
      }

      while (end != start && IsASCIIWhitespace(*(end - 1)))
      {
        --end;
      }

      return dom::DOMStringView(start, end);
    }

    /// @see https://infra.spec.whatwg.org/#strip-and-collapse-ascii-whitespace
    KRYS_NODISCARD static dom::DOMString StripAndCollapseASCIIWhitespace(dom::DOMStringView input) noexcept
    {
      size_t writeIndex = 0;
      bool inWhitespace = false;

      dom::DOMString result = dom::DOMString(input);

      size_t startIndex = 0;
      while (startIndex < result.length() && IsASCIIWhitespace(result[startIndex]))
      {
        startIndex++;
      }

      for (size_t i = startIndex; i < result.length(); ++i)
      {
        if (IsASCIIWhitespace(result[i]))
        {
          if (!inWhitespace)
          {
            result[writeIndex++] = ' ';
            inWhitespace = true;
          }
        }
        else
        {
          result[writeIndex++] = result[i];
          inWhitespace = false;
        }
      }

      if (writeIndex > 0 && IsASCIIWhitespace(result[writeIndex - 1]))
      {
        writeIndex--;
      }

      result.resize(writeIndex);
      return result;
    }

    /// @see https://infra.spec.whatwg.org/#collect-a-sequence-of-code-points
    template <typename TFunc>
    KRYS_NODISCARD static dom::DOMStringView CollectCodePointSequence(dom::DOMStringView input,
                                                                 position_variable &position,
                                                                 TFunc &&condition) noexcept
    {
      position_variable start = position;
      AdvancePositionWhile(input, position, condition);
      return dom::DOMStringView(start, position);
    }

    /// @brief Advances 'position' until 'condition' returns false for the current code point.
    template <typename TFunc>
    static void AdvancePositionWhile(dom::DOMStringView input, position_variable &position,
                                     TFunc &&condition) noexcept
    {
      // TODO(fix): STRINGS - we're not iterating over code points properly here. we need a way of just
      // iterating over code points in a UTF-8 string instead of having to
      while (position != input.end() && condition(*position))
      {
        ++position;
      }
    }

    /// @see https://infra.spec.whatwg.org/#ascii-whitespace
    KRYS_NODISCARD static bool IsASCIIWhitespace(char32 codePoint) noexcept
    {
      return ::krys::text::IsASCIIWhitespace(codePoint);
    }

    /// @see https://infra.spec.whatwg.org/#skip-ascii-whitespace
    static void SkipWhitespace(dom::DOMStringView input, position_variable &position) noexcept
    {
      AdvancePositionWhile(input, position, IsASCIIWhitespace);
    }

    /// @see https://infra.spec.whatwg.org/#strictly-split
    KRYS_NODISCARD static List<dom::DOMString> StrictlySplit(dom::DOMStringView input, char32 delimiter) noexcept
    {
      List<dom::DOMString> tokens;
      auto position = input.begin();

      tokens.emplace_back(CollectCodePointSequence(input, position, [delimiter](char32 current)
                                                   { return current != delimiter; }));

      while (position != input.end())
      {
        assert(*position == delimiter);
        ++position;
        tokens.emplace_back(CollectCodePointSequence(input, position, [delimiter](char32 current)
                                                     { return current != delimiter; }));
      }

      return tokens;
    }

    /// @see https://infra.spec.whatwg.org/#split-on-ascii-whitespace
    KRYS_NODISCARD static List<dom::DOMString> SplitOnWhitespace(dom::DOMStringView input)
    {
      List<dom::DOMString> tokens;
      auto position = input.begin();

      SkipWhitespace(input, position);
      while (position != input.end())
      {
        tokens.emplace_back(CollectCodePointSequence(input, position, [](char32 current)
                                                     { return !IsASCIIWhitespace(current); }));
        SkipWhitespace(input, position);
      }

      return tokens;
    }

    /// @see https://infra.spec.whatwg.org/#split-on-commas
    KRYS_NODISCARD static List<dom::DOMString> SplitOnComma(dom::DOMStringView input)
    {
      List<dom::DOMString> tokens;
      const char comma = ',';
      auto position = input.begin();

      while (position != input.end())
      {
        auto token =
          CollectCodePointSequence(input, position, [comma](char32 current) { return current != comma; });
        tokens.emplace_back(StripLeadingAndTrailingWhitespace(token));

        if (position != input.end())
        {
          assert(*position == comma);
          ++position;
        }
      }

      return tokens;
    }

    /// @see https://infra.spec.whatwg.org/#string-concatenate
    KRYS_NODISCARD static dom::DOMString Concatenate(Span<dom::DOMString> tokens,
                                                     Maybe<dom::DOMString> separator) noexcept
    {
      if (tokens.empty())
      {
        return u8"";
      }

      auto sep = separator.value_or(u8"");
      dom::DOMString result;
      for (size_t i = 0; i < tokens.size() - 1; i++)
      {
        result += tokens[i];
        result += sep;
      }

      result += tokens.back();

      return result;
    }

    /// @see https://infra.spec.whatwg.org/#ascii-case-insensitive
    KRYS_NODISCARD static bool ASCIICaseInsensitiveMatch(dom::DOMStringView a, dom::DOMStringView b) noexcept
    {
      if (a.length() != b.length())
      {
        return false;
      }

      for (size_t i = 0; i < a.length(); i++)
      {
        if (krys::text::ToASCIILowerUnchecked(a[i]) != krys::text::ToASCIILowerUnchecked(b[i]))
        {
          return false;
        }
      }

      return true;
    }
  };
}