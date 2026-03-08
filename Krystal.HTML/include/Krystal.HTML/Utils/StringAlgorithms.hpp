#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include <cassert>

namespace Krys::HTML
{
  /// @see https://infra.spec.whatwg.org/#string-algorithms
  class StringAlgorithms
  {
    using position_variable = DOMString::const_iterator;

  public:
    /// @see https://infra.spec.whatwg.org/#strictly-split
    KRYS_NODISCARD static List<DOMString> StrictlySplit(const DOMString &input, char32 delimiter) noexcept
    {
      auto position = input.begin();

      List<DOMString> tokens;

      tokens.emplace_back(CollectCodePointSequence(input, position, [delimiter](char32 current)
                                                   { return current != delimiter; }));

      while (position != input.end())
      {
        ++position;
        tokens.emplace_back(CollectCodePointSequence(input, position, [delimiter](char32 current)
                                                     { return current != delimiter; }));
      }

      return tokens;
    }

    /// @see https://infra.spec.whatwg.org/#collect-a-sequence-of-code-points
    template <typename TFunc>
    KRYS_NODISCARD static DOMString
      CollectCodePointSequence(const DOMString &input, position_variable& position, TFunc &&condition) noexcept
    {
      DOMString result;
      // TODO(fix): we're not iterating over code points properly here.
      while (position != input.end() && condition(*position))
      {
        result.push_back(*position);
        ++position;
      }
      return result;
    }
  };
}