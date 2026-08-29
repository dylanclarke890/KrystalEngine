#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  enum class AllowedURLModifiers
  {
    None = 0,
    CrossOrigin = 1 << 0,
    Integrity = 1 << 1,
    ReferrerPolicy = 1 << 2,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::HTML::CSSPropertyParserHelpers::AllowedURLModifiers, 4uz)

namespace Krys::HTML
{
  class CSSValue;
  class CSSTokenRange;

  struct CSSPropertyParserState;
  struct CSSURL;

  namespace CSSPropertyParserHelpers
  {
    // MARK: <url>
    // https://drafts.csswg.org/css-values/#urls

    KRYS_NODISCARD Maybe<CSSURL> ConsumeURLRaw(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                               AllowedURLModifiers allowed) noexcept;

    KRYS_NODISCARD RefPtr<CSSValue> ConsumeURL(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                               AllowedURLModifiers allowed) noexcept;
  }
}