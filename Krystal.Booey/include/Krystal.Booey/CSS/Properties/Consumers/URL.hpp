#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  enum class AllowedURLModifiers
  {
    None = 0,
    CrossOrigin = 1 << 0,
    Integrity = 1 << 1,
    ReferrerPolicy = 1 << 2,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::css::PropertyParserHelpers::AllowedURLModifiers, 4uz)

namespace krys::boo::css
{
  class CSSValue;
  class TokenRange;

  struct PropertyParserState;
  struct CSSURL;

  namespace PropertyParserHelpers
  {
    // MARK: <url>
    // https://drafts.csswg.org/css-values/#urls

    KRYS_NODISCARD Maybe<CSSURL> ConsumeURLRaw(TokenRange &tokens, PropertyParserState &state,
                                               AllowedURLModifiers allowed) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue> ConsumeURL(TokenRange &tokens, PropertyParserState &state,
                                               AllowedURLModifiers allowed) noexcept;
  }
}