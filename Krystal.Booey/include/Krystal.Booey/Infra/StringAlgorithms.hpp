#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include <algorithm>

namespace krys::boo::infra
{
  /// @see https://infra.spec.whatwg.org/#strings
  class StringAlgorithms
  {
  public:
    using position_variable = dom::DOMStringView::const_iterator;

    /// @see https://infra.spec.whatwg.org/#ascii-string
    KRYS_NODISCARD static bool IsASCIIString(dom::DOMStringView input) noexcept
    {
      return std::ranges::all_of(input, [](char8 c) { return ::krys::text::IsASCII(c); });
    }

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

    /// @brief @p input with each CR LF pair, then each remaining CR, replaced by an LF.
    /// @see https://infra.spec.whatwg.org/#normalize-newlines
    KRYS_NODISCARD static dom::DOMString NormalizeNewlines(dom::DOMStringView input) noexcept
    {
      dom::DOMString result;
      result.reserve(input.size());
      for (size_t i = 0; i < input.size(); ++i)
      {
        if (input[i] == u8'\r')
        {
          result.push_back(u8'\n');
          if (i + 1 < input.size() && input[i + 1] == u8'\n')
          {
            ++i;
          }
          continue;
        }
        result.push_back(input[i]);
      }
      return result;
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

    /// @note @p condition is given UTF-8 code units, which is the same as being given code points when it
    /// tests only for ASCII code points (giving the same answer for every non-ASCII code unit).
    /// @see https://infra.spec.whatwg.org/#collect-a-sequence-of-code-points
    template <typename TFunc>
    KRYS_NODISCARD static dom::DOMStringView CollectCodePointSequence(dom::DOMStringView input,
                                                                      position_variable &position,
                                                                      TFunc &&condition) noexcept
    {
      // 1. Let result be the empty string.
      position_variable start = position;

      // 2. While position doesn’t point past the end of input and the code point at position within input
      // meets the
      //    condition condition:
      // 2a. Append that code point to the end of result.
      // 2b. Advance position by 1.
      AdvancePositionWhile(input, position, condition);

      // 3. Return result.
      return dom::DOMStringView(start, position);
    }

    /// @brief Advances @p position while @p condition returns true for the code unit at it.
    /// @note As for CollectCodePointSequence, @p condition should test only for ASCII code points.
    template <typename TFunc>
    static void AdvancePositionWhile(dom::DOMStringView input, position_variable &position,
                                     TFunc &&condition) noexcept
    {
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

    /// @param delimiter An ASCII code point.
    /// @see https://infra.spec.whatwg.org/#strictly-split
    KRYS_NODISCARD static List<dom::DOMString> StrictlySplit(dom::DOMStringView input,
                                                             char32 delimiter) noexcept
    {
      krys_debug_assert(::krys::text::IsASCII(delimiter));

      // 1. Let position be a position variable for input, initially pointing at the start of input.
      auto position = input.begin();

      // 2. Let tokens be a list of strings, initially empty.
      List<dom::DOMString> tokens;

      // 3. Let token be the result of collecting a sequence of code points that are not equal to delimiter
      // from
      //    input, given position.
      // 4. Append token to tokens.
      tokens.emplace_back(CollectCodePointSequence(input, position, [delimiter](char32 current)
                                                   { return current != delimiter; }));

      // 5. While position is not past the end of input:
      while (position != input.end())
      {
        // 5a. Assert: the code point at position within input is delimiter.
        krys_debug_assert(static_cast<char32>(*position) == delimiter);

        // 5b. Advance position by 1.
        ++position;

        // 5c. Let token be the result of collecting a sequence of code points that are not equal to delimiter
        // from
        //     input, given position.
        // 5d. Append token to tokens.
        tokens.emplace_back(CollectCodePointSequence(input, position, [delimiter](char32 current)
                                                     { return current != delimiter; }));
      }

      // 6. Return tokens.
      return tokens;
    }

    /// @see https://infra.spec.whatwg.org/#split-on-ascii-whitespace
    KRYS_NODISCARD static List<dom::DOMString> SplitOnWhitespace(dom::DOMStringView input)
    {
      // 1. Let position be a position variable for input, initially pointing at the start of input.
      auto position = input.begin();

      // 2. Let tokens be a list of strings, initially empty.
      List<dom::DOMString> tokens;

      // 3. Skip ASCII whitespace within input given position.
      SkipWhitespace(input, position);

      // 4. While position is not past the end of input:
      while (position != input.end())
      {
        // 4a. Let token be the result of collecting a sequence of code points that are not ASCII whitespace
        // from
        //     input, given position.
        // 4b. Append token to tokens.
        tokens.emplace_back(CollectCodePointSequence(input, position, [](char32 current)
                                                     { return !IsASCIIWhitespace(current); }));

        // 4c. Skip ASCII whitespace within input given position.
        SkipWhitespace(input, position);
      }

      // 5. Return tokens.
      return tokens;
    }

    /// @see https://infra.spec.whatwg.org/#split-on-commas
    KRYS_NODISCARD static List<dom::DOMString> SplitOnComma(dom::DOMStringView input)
    {
      // 1. Let position be a position variable for input, initially pointing at the start of input.
      auto position = input.begin();

      // 2. Let tokens be a list of strings, initially empty.
      List<dom::DOMString> tokens;

      // 3. While position is not past the end of input:
      while (position != input.end())
      {
        // 3a. Let token be the result of collecting a sequence of code points that are not U+002C (,) from
        // input,
        //     given position.
        auto token =
          CollectCodePointSequence(input, position, [](char32 current) { return current != u8','; });

        // 3b. Strip leading and trailing ASCII whitespace from token.
        // 3c. Append token to tokens.
        tokens.emplace_back(StripLeadingAndTrailingWhitespace(token));

        // 3d. If position is not past the end of input:
        if (position != input.end())
        {
          // 3d.i. Assert: the code point at position within input is U+002C (,).
          krys_debug_assert(*position == u8',');

          // 3d.ii. Advance position by 1.
          ++position;
        }
      }

      // 4. Return tokens.
      return tokens;
    }

    /// @see https://infra.spec.whatwg.org/#string-concatenate
    KRYS_NODISCARD static dom::DOMString Concatenate(Span<dom::DOMString> tokens,
                                                     Maybe<dom::DOMString> separator) noexcept
    {
      // 1. If list is empty, then return the empty string.
      if (tokens.empty())
      {
        return u8"";
      }

      // 2. If separator is not given, then set separator to the empty string.
      auto sep = separator.value_or(u8"");

      // 3. Return a string whose contents are list’s items, in order, separated from each other by separator.
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
        if (krys::text::ToASCIILower(a[i]) != krys::text::ToASCIILower(b[i]))
        {
          return false;
        }
      }

      return true;
    }

    /// @brief Whether @p a sorts before @p b by their UTF-16 code units, given their UTF-8 forms.
    /// @see https://infra.spec.whatwg.org/#code-unit-less-than
    KRYS_NODISCARD static bool CodeUnitLessThan(dom::DOMStringView a, dom::DOMStringView b) noexcept
    {
      // A UTF-8 string is a prefix of another exactly when its UTF-16 form is.
      auto [aIt, bIt] = std::ranges::mismatch(a, b);

      // 1. If b is a code unit prefix of a, then return false.
      if (bIt == b.end())
      {
        return false;
      }

      // 2. If a is a code unit prefix of b, then return true.
      if (aIt == a.end())
      {
        return true;
      }

      // 3. Let n be the smallest index such that the nth code unit of a is different from the nth code unit
      // of b.
      // 4. If the nth code unit of a is less than the nth code unit of b, then return true.
      // 5. Return false.
      // UTF-8 sorts by code point, which matches UTF-16 except that supplementary code points (surrogate
      // pairs in UTF-16) sort before U+E000 to U+FFFF.
      auto i = static_cast<size_t>(aIt - a.begin());
      while (i > 0 && (static_cast<uint8>(a[i]) & 0xC0) == 0x80)
      {
        --i;
      }
      auto aLead = static_cast<uint8>(a[i]);
      auto bLead = static_cast<uint8>(b[i]);
      auto isSupplementary = [](uint8 lead)
      {
        return lead >= 0xF0;
      };
      auto isUpperBMP = [](uint8 lead)
      {
        return lead == 0xEE || lead == 0xEF;
      };
      if (isSupplementary(aLead) && isUpperBMP(bLead))
      {
        return true;
      }
      if (isUpperBMP(aLead) && isSupplementary(bLead))
      {
        return false;
      }
      return static_cast<uint8>(*aIt) < static_cast<uint8>(*bIt);
    }
  };
}