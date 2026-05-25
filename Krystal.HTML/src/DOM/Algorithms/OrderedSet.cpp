#include "Krystal.HTML/DOM/Algorithms/OrderedSet.hpp"
#include "Krystal.HTML/Infra/StringAlgorithms.hpp"
#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Span.hpp"
#include <ranges>

namespace Krys::HTML
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

  /// @see https://dom.spec.whatwg.org/#concept-ordered-set-serializer
  DOMString OrderedSet::Serializer(Span<DOMString> tokens) noexcept
  {
    return StringAlgorithms::Concatenate(tokens, u8" ");
  }
}