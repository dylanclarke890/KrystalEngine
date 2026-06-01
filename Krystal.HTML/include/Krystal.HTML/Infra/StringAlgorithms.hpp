#pragma once

#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include "Krystal.Text/ASCII.hpp"
#include <cassert>

namespace Krys::HTML
{
  /// @see https://infra.spec.whatwg.org/#string-algorithms
  class StringAlgorithms
  {
  public:
    using position_variable = DOMStringView::const_iterator;

    /// @see https://infra.spec.whatwg.org/#strip-leading-and-trailing-ascii-whitespace
    KRYS_NODISCARD static DOMStringView StripLeadingAndTrailingWhitespace(DOMStringView input) noexcept
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
      return DOMStringView(start, end);
    }

    /// @see https://infra.spec.whatwg.org/#strip-and-collapse-ascii-whitespace
    KRYS_NODISCARD static DOMString StripAndCollapseASCIIWhitespace(DOMStringView input) noexcept
    {
      size_t writeIndex = 0;
      bool inWhitespace = false;

      DOMString result = DOMString(input);

      size_t startIndex = 0;
      while (startIndex < result.length() && Krys::Text::IsASCIIWhitespace(result[startIndex]))
      {
        startIndex++;
      }

      for (size_t i = startIndex; i < result.length(); ++i)
      {
        if (Krys::Text::IsASCIIWhitespace(result[i]))
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

      if (writeIndex > 0 && Krys::Text::IsASCIIWhitespace(result[writeIndex - 1]))
      {
        writeIndex--;
      }

      result.resize(writeIndex);
      return result;
    }

    /// @see https://infra.spec.whatwg.org/#collect-a-sequence-of-code-points
    template <typename TFunc>
    KRYS_NODISCARD static DOMStringView CollectCodePointSequence(DOMStringView input, position_variable &position,
                                                             TFunc &&condition) noexcept
    {
      position_variable start = position;
      AdvancePositionWhile(input, position, condition);
      return DOMStringView(start, position);
    }

    /// @brief Advances 'position' until 'condition' returns false for the current code point.
    template <typename TFunc>
    static void AdvancePositionWhile(DOMStringView input, position_variable &position,
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
      return ::Krys::Text::IsASCIIWhitespace(codePoint);
    }

    /// @see https://infra.spec.whatwg.org/#skip-ascii-whitespace
    static void SkipWhitespace(DOMStringView input, position_variable &position) noexcept
    {
      AdvancePositionWhile(input, position, IsASCIIWhitespace);
    }

    /// @see https://infra.spec.whatwg.org/#strictly-split
    KRYS_NODISCARD static List<DOMString> StrictlySplit(DOMStringView input, char32 delimiter) noexcept
    {
      List<DOMString> tokens;
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
    KRYS_NODISCARD static List<DOMString> SplitOnWhitespace(DOMStringView input)
    {
      List<DOMString> tokens;
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
    KRYS_NODISCARD static List<DOMString> SplitOnComma(DOMStringView input)
    {
      List<DOMString> tokens;
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
    KRYS_NODISCARD static DOMString Concatenate(Span<DOMString> tokens, Maybe<DOMString> separator) noexcept
    {
      if (tokens.empty())
      {
        return u8"";
      }

      auto sep = separator.value_or(u8"");
      DOMString result;
      for (size_t i = 0; i < tokens.size() - 1; i++)
      {
        result += tokens[i];
        result += sep;
      }

      result += tokens.back();

      return result;
    }
  };
}