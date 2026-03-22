#pragma once

#include "Krystal.HTML/Algorithms/StringAlgorithms.hpp"
#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  /// @see https://dom.spec.whatwg.org/#concept-ordered-set-parser
  KRYS_NODISCARD inline List<DOMString> OrderedSetParser(const DOMString &input) noexcept
  {
    auto inputTokens = StringAlgorithms::SplitOnWhitespace(input);

    List<DOMString> tokens;
    for (auto &token : inputTokens)
    {
      if (std::ranges::find(tokens, token) == tokens.end())
      {
        tokens.emplace_back(std::move(token));
      }
    }

    return tokens;
  }

  /// @see https://dom.spec.whatwg.org/#concept-ordered-set-serializer
  KRYS_NODISCARD inline DOMString OrderedSetSerializer(const List<DOMString> &tokens) noexcept
  {
    return StringAlgorithms::Concatenate(tokens, u8" ");
  }
}