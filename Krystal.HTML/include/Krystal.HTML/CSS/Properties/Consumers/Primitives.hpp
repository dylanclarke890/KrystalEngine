#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;

  namespace CSSPropertyParserHelpers
  {
    // MARK: - Comma
    KRYS_NODISCARD bool ConsumeComma(CSSTokenRange &tokens) noexcept;

    // MARK: - Slash
    KRYS_NODISCARD bool ConsumeSlash(CSSTokenRange &tokens) noexcept;

    // MARK: - Function
    // NOTE: ConsumeFunction expects the range starts with a FunctionToken.
    KRYS_NODISCARD CSSTokenRange ConsumeFunction(CSSTokenRange &tokens) noexcept;

    KRYS_NODISCARD Maybe<CSSTokenRange> ConsumeArgument(CSSTokenRange &tokens, size_t index) noexcept;
  }
}