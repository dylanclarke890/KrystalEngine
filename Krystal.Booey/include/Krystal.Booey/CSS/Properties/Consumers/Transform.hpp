#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class CSSValue;
  struct CSSPropertyParserState;
  struct CSSTransform;
  struct ParserContext;

  namespace CSSPropertyParserHelpers
  {
    // MARK: <rotate3d()> consuming (CSSValue)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeRotate3dFunction(TokenRange &tokens,
                                                            CSSPropertyParserState &state) noexcept;
    // MARK: <translate()> consuming (CSSValue)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeTranslateFunction(TokenRange &tokens,
                                                             CSSPropertyParserState &state) noexcept;
    // MARK: <translate3d()> consuming (CSSValue)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeTranslate3dFunction(TokenRange &tokens,
                                                               CSSPropertyParserState &state) noexcept;

    // MARK: <'translate'> consuming (CSSValue)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeTranslate(TokenRange &tokens,
                                                     CSSPropertyParserState &state) noexcept;
    // MARK: <'scale'> consuming (CSSValue)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeScale(TokenRange &tokens,
                                                 CSSPropertyParserState &state) noexcept;
    // MARK: <'rotate'> consuming (CSSValue)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeRotate(TokenRange &tokens,
                                                  CSSPropertyParserState &state) noexcept;

    // MARK: <'transform'> parsing (raw)
    KRYS_NODISCARD Maybe<CSSTransform> ParseTransformRaw(const CSSOMString &str,
                                                         const ParserContext &context);
  }
}