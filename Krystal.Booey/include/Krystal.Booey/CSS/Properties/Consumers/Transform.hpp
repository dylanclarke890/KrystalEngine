#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::style
{
  struct Transform;
}

namespace krys::boo::css
{
  struct ParserContext;
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

    // MARK: <'transform'> parsing (raw)
    /// @brief Parses @p string as a whole `transform` value into its style value, or nothing if it doesn't
    /// parse or its lengths need a style to resolve (`em`, viewport units, ...).
    KRYS_NODISCARD Maybe<style::Transform> ParseTransformRaw(const CSSOMString &string,
                                                             const ParserContext &context) noexcept;
  }
}