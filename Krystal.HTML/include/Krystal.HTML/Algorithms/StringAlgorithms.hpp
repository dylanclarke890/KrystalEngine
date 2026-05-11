#pragma once

#include "Krystal.HTML/DOMString.hpp"
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
    using position_variable = DOMStringView::const_iterator;

  public:
    /// @see https://infra.spec.whatwg.org/#strip-leading-and-trailing-ascii-whitespace
    KRYS_NODISCARD static DOMString StripLeadingAndTrailingWhitespace(DOMStringView input) noexcept
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
      return DOMString(start, end);
    }

    /// @see https://infra.spec.whatwg.org/#collect-a-sequence-of-code-points
    template <typename TFunc>
    KRYS_NODISCARD static DOMString CollectCodePointSequence(DOMStringView input, position_variable &position,
                                                             TFunc &&condition) noexcept
    {
      DOMString result;
      // TODO(fix): we're not iterating over code points properly here. we need a way of just iterating over
      // code points in a UTF-8 string instead of having to
      while (position != input.end() && condition(*position))
      {
        result.push_back(*position);
        ++position;
      }
      return result;
    }

    /// @see https://infra.spec.whatwg.org/#ascii-whitespace
    KRYS_NODISCARD static bool IsASCIIWhitespace(char32 codePoint) noexcept
    {
      return ::Krys::Text::IsASCIIWhitespace(codePoint);
    }

    /// @see https://infra.spec.whatwg.org/#skip-ascii-whitespace
    static void SkipWhitespace(DOMStringView input, position_variable &position) noexcept
    {
      while (position != input.end() && IsASCIIWhitespace(*position))
      {
        ++position;
      }
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
      auto position = input.begin();

      while (position != input.end())
      {
        auto token = CollectCodePointSequence(input, position, [](char32 current) { return current != ','; });
        tokens.emplace_back(StripLeadingAndTrailingWhitespace(token));

        if (position != input.end())
        {
          assert(*position == ',');
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