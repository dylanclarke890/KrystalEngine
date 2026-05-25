#pragma once

#include "Krystal.HTML/Infra/StringAlgorithms.hpp"
#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Span.hpp"

namespace Krys::HTML
{
  /// @see https://dom.spec.whatwg.org/#ordered-sets
  class OrderedSet
  {
  public:
    /// @see https://dom.spec.whatwg.org/#concept-ordered-set-parser
    KRYS_NODISCARD static List<DOMString> Parser(DOMStringView input) noexcept
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
    KRYS_NODISCARD static inline DOMString Serializer(Span<DOMString> tokens) noexcept
    {
      return StringAlgorithms::Concatenate(tokens, u8" ");
    }
  };
}