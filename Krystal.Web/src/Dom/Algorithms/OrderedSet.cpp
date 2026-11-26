#include "Krystal.Web/Dom/Algorithms/OrderedSet.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Web/Infra/StringAlgorithms.hpp"
#include <ranges>

namespace krys::web::dom
{
  List<DOMString> OrderedSet::Parser(DOMStringView input) noexcept
  {
    // 1. Let inputTokens be the result of splitting input on ASCII whitespace.
    auto inputTokens = infra::StringAlgorithms::SplitOnWhitespace(input);

    // 2. Let tokens be a new ordered set.
    List<DOMString> tokens;

    // 3. For each token of inputTokens: append token to tokens.
    for (auto &token : inputTokens)
    {
      // Appending to a set does nothing if it already contains the item.
      if (std::ranges::find(tokens, token) == tokens.end())
      {
        tokens.emplace_back(krys::move(token));
      }
    }

    // 4. Return tokens.
    return tokens;
  }

  DOMString OrderedSet::Serialiser(Span<DOMString> tokens) noexcept
  {
    return infra::StringAlgorithms::Concatenate(tokens, u8" ");
  }
}
