#pragma once

#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  class CSSTokenRange;
  class CSSValue;
  struct CSSPropertyParserState;
  struct CSSTransform;
  struct CSSParserContext;

  namespace CSSPropertyParserHelpers
  {
    // MARK: <rotate3d()> consuming (CSSValue)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeRotate3dFunction(CSSTokenRange &tokens,
                                                            CSSPropertyParserState &state) noexcept;
    // MARK: <translate()> consuming (CSSValue)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeTranslateFunction(CSSTokenRange &tokens,
                                                             CSSPropertyParserState &state) noexcept;
    // MARK: <translate3d()> consuming (CSSValue)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeTranslate3dFunction(CSSTokenRange &tokens,
                                                               CSSPropertyParserState &state) noexcept;

    // MARK: <'translate'> consuming (CSSValue)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeTranslate(CSSTokenRange &tokens,
                                                     CSSPropertyParserState &state) noexcept;
    // MARK: <'scale'> consuming (CSSValue)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeScale(CSSTokenRange &tokens,
                                                 CSSPropertyParserState &state) noexcept;
    // MARK: <'rotate'> consuming (CSSValue)
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeRotate(CSSTokenRange &tokens,
                                                  CSSPropertyParserState &state) noexcept;

    // MARK: <'transform'> parsing (raw)
    KRYS_NODISCARD Maybe<CSSTransform> ParseTransformRaw(const CSSOMString &str,
                                                         const CSSParserContext &context);
  }
}