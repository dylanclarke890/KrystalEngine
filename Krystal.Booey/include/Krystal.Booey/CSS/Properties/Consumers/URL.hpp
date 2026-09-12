#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css::CSSPropertyParserHelpers
{
  enum class AllowedURLModifiers
  {
    None = 0,
    CrossOrigin = 1 << 0,
    Integrity = 1 << 1,
    ReferrerPolicy = 1 << 2,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::css::CSSPropertyParserHelpers::AllowedURLModifiers, 4uz)

namespace krys::boo::css
{
  class CSSValue;
  class TokenRange;

  struct CSSPropertyParserState;
  struct CSSURL;

  namespace CSSPropertyParserHelpers
  {
    // MARK: <url>
    // https://drafts.csswg.org/css-values/#urls

    KRYS_NODISCARD Maybe<CSSURL> ConsumeURLRaw(TokenRange &tokens, CSSPropertyParserState &state,
                                               AllowedURLModifiers allowed) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue> ConsumeURL(TokenRange &tokens, CSSPropertyParserState &state,
                                               AllowedURLModifiers allowed) noexcept;
  }
}