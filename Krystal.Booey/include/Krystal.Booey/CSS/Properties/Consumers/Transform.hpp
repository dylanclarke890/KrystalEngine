#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class Value;
  struct PropertyParserState;
  struct CSSTransform;
  struct ParserContext;

  namespace PropertyParserHelpers
  {
    // MARK: <rotate3d()> consuming (Value)
    KRYS_NODISCARD RefPtr<Value> ConsumeRotate3dFunction(TokenRange &tokens,
                                                            PropertyParserState &state) noexcept;
    // MARK: <translate()> consuming (Value)
    KRYS_NODISCARD RefPtr<Value> ConsumeTranslateFunction(TokenRange &tokens,
                                                             PropertyParserState &state) noexcept;
    // MARK: <translate3d()> consuming (Value)
    KRYS_NODISCARD RefPtr<Value> ConsumeTranslate3dFunction(TokenRange &tokens,
                                                               PropertyParserState &state) noexcept;

    // MARK: <'translate'> consuming (Value)
    KRYS_NODISCARD RefPtr<Value> ConsumeTranslate(TokenRange &tokens,
                                                     PropertyParserState &state) noexcept;
    // MARK: <'scale'> consuming (Value)
    KRYS_NODISCARD RefPtr<Value> ConsumeScale(TokenRange &tokens,
                                                 PropertyParserState &state) noexcept;
    // MARK: <'rotate'> consuming (Value)
    KRYS_NODISCARD RefPtr<Value> ConsumeRotate(TokenRange &tokens,
                                                  PropertyParserState &state) noexcept;

    // MARK: <'transform'> parsing (raw)
    KRYS_NODISCARD Maybe<CSSTransform> ParseTransformRaw(const CSSOMString &str,
                                                         const ParserContext &context);
  }
}