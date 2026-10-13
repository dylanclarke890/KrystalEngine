#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class TokenRange;
  class Value;
  struct PropertyParserState;

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
    KRYS_NODISCARD RefPtr<Value> ConsumeTranslate(TokenRange &tokens, PropertyParserState &state) noexcept;
    // MARK: <'scale'> consuming (Value)
    KRYS_NODISCARD RefPtr<Value> ConsumeScale(TokenRange &tokens, PropertyParserState &state) noexcept;
    // MARK: <'rotate'> consuming (Value)
    KRYS_NODISCARD RefPtr<Value> ConsumeRotate(TokenRange &tokens, PropertyParserState &state) noexcept;

    // FIXME: Add `ParseTransformRaw` (<'transform'> parsing to a `style::Transform`) once `style::Transform`,
    // `style::BuilderState` and the generated `PropertyParsing::ConsumeTransform` are available.
  }
}