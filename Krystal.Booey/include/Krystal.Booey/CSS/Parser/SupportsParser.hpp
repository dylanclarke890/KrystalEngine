#pragma once

#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Parser/TokenType.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::css
{
  class Parser;
  struct ParserContext;

  /// @brief Evaluates `@supports` conditions and `CSS.supports()`.
  /// @see https://drafts.csswg.org/css-conditional-3/#at-supports
  class SupportsParser
  {
  public:
    enum SupportsResult : uint8
    {
      Unsupported = false,
      Supported = true,
      Invalid
    };

    enum class ParsingMode : bool
    {
      ForAtRuleSupports,
      AllowBareDeclarationAndGeneralEnclosed,
    };

  private:
    Parser &_parser;

    explicit SupportsParser(Parser &parser) noexcept : _parser(parser)
    {
    }

  public:
    KRYS_NODISCARD static SupportsResult SupportsCondition(TokenRange tokens, Parser &parser,
                                                           ParsingMode mode) noexcept;
    KRYS_NODISCARD static SupportsResult
      SupportsCondition(CSSOMStringView condition, const ParserContext &context, ParsingMode mode) noexcept;

  private:
    KRYS_NODISCARD SupportsResult ConsumeCondition(TokenRange tokens) noexcept;
    KRYS_NODISCARD SupportsResult ConsumeNegation(TokenRange tokens) noexcept;
    KRYS_NODISCARD SupportsResult ConsumeSupportsFunction(TokenRange &tokens) noexcept;
    KRYS_NODISCARD SupportsResult ConsumeSupportsFeatureOrGeneralEnclosed(TokenRange &tokens) noexcept;
    // https://drafts.csswg.org/css-conditional-4/#typedef-supports-selector-fn
    // <supports-selector-fn> = selector( <complex-selector> );
    KRYS_NODISCARD SupportsResult ConsumeSupportsSelectorFunction(TokenRange &tokens) noexcept;

    // https://drafts.csswg.org/css-conditional-5/#typedef-supports-font-format-fn
    KRYS_NODISCARD SupportsResult ConsumeSupportsFontFormatFunction(TokenRange &tokens) noexcept;
    // https://drafts.csswg.org/css-conditional-5/#typedef-supports-font-tech-fn
    KRYS_NODISCARD SupportsResult ConsumeSupportsFontTechFunction(TokenRange &tokens) noexcept;

    KRYS_NODISCARD SupportsResult ConsumeConditionInParenthesis(TokenRange &tokens,
                                                                TokenType startTokenType) noexcept;
  };
}
