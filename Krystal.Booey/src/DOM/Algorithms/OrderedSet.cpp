#include "Krystal.Booey/DOM/Algorithms/OrderedSet.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include <ranges>

namespace krys::boo::dom
{
  List<DOMString> OrderedSet::Parser(DOMStringView input) noexcept
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

  DOMString OrderedSet::Serializer(Span<DOMString> tokens) noexcept
  {
    return StringAlgorithms::Concatenate(tokens, u8" ");
  }
}