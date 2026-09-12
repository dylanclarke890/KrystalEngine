#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  class TokenRange;

  namespace CSSPropertyParserHelpers
  {
    // MARK: - Comma
    KRYS_NODISCARD bool ConsumeComma(TokenRange &tokens) noexcept;

    // MARK: - Slash
    KRYS_NODISCARD bool ConsumeSlash(TokenRange &tokens) noexcept;

    // MARK: - Function
    // NOTE: ConsumeFunction expects the range starts with a FunctionToken.
    KRYS_NODISCARD TokenRange ConsumeFunction(TokenRange &tokens) noexcept;

    KRYS_NODISCARD Maybe<TokenRange> ConsumeArgument(TokenRange &tokens, size_t index) noexcept;
  }
}