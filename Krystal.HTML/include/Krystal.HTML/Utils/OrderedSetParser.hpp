#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.HTML/Utils/StringAlgorithms.hpp"
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
}