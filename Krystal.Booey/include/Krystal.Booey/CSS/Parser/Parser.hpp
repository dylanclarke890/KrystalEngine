#pragma once

#include "Krystal.Booey/CSS/Parser/AllowedBlockRules.hpp"
#include "Krystal.Booey/CSS/Parser/AllowedRules.hpp"
#include "Krystal.Booey/CSS/Parser/Context/NestedParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/Context/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/Token.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/CSSInternalStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Rules/RuleType.hpp"
#include "Krystal.Booey/URL/Url.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class CSSRule;
  class CSSCharsetRule;
  class CSSImportRule;
  class CSSNamespaceRule;
  class CSSStyleRule;
  class CSSStyleSheet;
  class CSSStyleDeclaration;
  class StyleSheetContents;

  class Parser
  {
  private:
    InputStream _inputStream;
    const ParserContext _context;
    Tokenizer _tokenizer;
    size_t _ruleListNestingLevel;
    List<NestedParserContext> _nestedContextStack;
    List<NestedParserContextType> _ancestorRuleTypeStack;
    RefPtr<StyleSheetContents> _stylesheet;

  public:
    Parser(CSSOMString &&input, const ParserContext &context, StyleSheetContents *stylesheet) noexcept;

#pragma region Parser Entry Points - https://drafts.csswg.org/css-syntax/#parser-entry-points

    /// @see https://drafts.csswg.org/css-syntax/#parse-stylesheet
    KRYS_NODISCARD static RefPtr<CSSStyleSheet> ParseStylesheet(CSSOMString &&input,
                                                                const ParserContext &context,
                                                                StyleSheetContents &stylesheet) noexcept;

    /// @see https://drafts.csswg.org/css-syntax/#parse-a-rule
    KRYS_NODISCARD static RefPtr<CSSRule> ParseRule(CSSOMString &&input, const ParserContext &context,
                                                    AllowedRules allowedRules) noexcept;

#pragma endregion

  private:
    /// @brief Pumps the tokenizer until the input stream is exhausted. Returns true if the tokenizer has
    /// consumed at least one token.
    bool PumpTokenizer() noexcept;

    /// @brief Gets the range of tokens produced by the tokenizer.
    /// @note The tokens are only valid as long as the tokenizer is alive. The tokenizer owns the tokens and
    /// will free them when it is destroyed.
    KRYS_NODISCARD TokenRange Tokens() const noexcept;

    /// @brief Computes the next allowed rules based on the current allowed rules and the reference rule. For
    /// example, if the current allowed rules are `Charset` and the reference rule is an `@import` rule, the
    /// next allowed rules will be `Import` (`Charset` is not allowed after an `@import` rule).
    KRYS_NODISCARD static AllowedRules ComputeNextAllowedRules(AllowedRules current, CSSRule *ref) noexcept;

    KRYS_NODISCARD static Ref<CSSInternalStyleProperties>
      CreateInternalStyleProperties(ParsedPropertyList &properties) noexcept;

#pragma region Parser Algorithms - https://drafts.csswg.org/css-syntax/#parser-algorithms

    /// @see https://drafts.csswg.org/css-syntax/#consume-a-stylesheets-contents
    KRYS_NODISCARD List<Ref<CSSRule>> ConsumeStyleSheetContents(TokenRange &input) noexcept;

    /// @see https://drafts.csswg.org/css-syntax/#consume-an-at-rule
    KRYS_NODISCARD RefPtr<CSSRule> ConsumeAtRule(TokenRange &tokens, AllowedRules allowedRules,
                                                 bool nested = false) noexcept;

    /// @see https://drafts.csswg.org/css-syntax/#consume-a-qualified-rule
    KRYS_NODISCARD RefPtr<CSSRule> ConsumeQualifiedRule(TokenRange &tokens, AllowedRules allowedRules,
                                                        bool nested = false) noexcept;

    /// @see https://drafts.csswg.org/css-syntax/#consume-a-blocks-contents
    void ConsumeBlockContents(TokenRange tokens, AllowedBlockRules allowedBlockRules,
                              RuleType ruleType) noexcept;

    /// @see https://drafts.csswg.org/css-syntax-3/#consume-a-declaration
    bool ConsumeDeclaration(TokenRange &tokens, RuleType ruleType) noexcept;

    IsImportant ConsumeTrailingImportantAndWhitespace(TokenRange &tokens);

    /// @see https://drafts.csswg.org/css-syntax/#consume-the-remnants-of-a-bad-declaration
    void ConsumeBadDeclaration(TokenRange &tokens, bool nested) noexcept;

    /// @see https://drafts.csswg.org/css-syntax-3/#typedef-declaration-list
    void ConsumeDeclarationList(TokenRange block, RuleType ruleType) noexcept;

    /// @see https://drafts.csswg.org/css-syntax-3/#typedef-qualified-rule-list
    void ConsumeQualifiedRuleList(TokenRange block, RuleType ruleType) noexcept;

    /// @see https://drafts.csswg.org/css-syntax-3/#typedef-at-rule-list
    void ConsumeAtRuleList(TokenRange block, RuleType ruleType) noexcept;

    /// @see https://drafts.csswg.org/css-syntax-3/#typedef-declaration-rule-list
    void ConsumeDeclarationRuleList(TokenRange block, RuleType ruleType) noexcept;

    /// @see https://drafts.csswg.org/css-syntax-3/#typedef-rule-list
    void ConsumeRuleList(TokenRange block, RuleType ruleType) noexcept;

    /// @see https://drafts.csswg.org/css-syntax-3/#typedef-block-contents
    void ConsumeStyleBlock(TokenRange block) noexcept;

#pragma endregion

#pragma region At-Rule Consumers

    KRYS_NODISCARD RefPtr<CSSCharsetRule> ConsumeCharsetRule(TokenRange prelude) noexcept;

    KRYS_NODISCARD RefPtr<CSSImportRule> ConsumeImportRule(TokenRange prelude) noexcept;

    KRYS_NODISCARD RefPtr<CSSNamespaceRule> ConsumeNamespaceRule(TokenRange prelude) noexcept;

#pragma endregion

#pragma region Qualified Rule Consumers

    KRYS_NODISCARD RefPtr<CSSStyleRule> ConsumeStyleRule(TokenRange prelude, TokenRange block) noexcept;

    KRYS_NODISCARD RefPtr<CSSStyleRule> ConsumeKeyframeStyleRule(TokenRange prelude,
                                                                 TokenRange block) noexcept;

#pragma endregion

#pragma region Nested Parsing Context

    void RunInNewNestedParsingContext(auto &&run) noexcept
    {
      _nestedContextStack.push_back(NestedParserContext {});
      run();
      _nestedContextStack.pop_back();
    }

    KRYS_NODISCARD NestedParserContext &CurrentNestedContext() noexcept
    {
      assert(!_nestedContextStack.empty());
      return _nestedContextStack.back();
    }

    KRYS_NODISCARD bool IsStyleNestedParsingContext() const noexcept
    {
      return !_ancestorRuleTypeStack.empty()
             && _ancestorRuleTypeStack.back() != NestedParserContextType::Function;
    }

    KRYS_NODISCARD bool IsFunctionNestedParsingContext() const noexcept
    {
      return !_ancestorRuleTypeStack.empty()
             && _ancestorRuleTypeStack.back() == NestedParserContextType::Function;
    }

    KRYS_NODISCARD bool HasStyleRuleAncestor() const noexcept
    {
      return std::ranges::any_of(_ancestorRuleTypeStack, [](NestedParserContextType type)
                                 { return type == NestedParserContextType::Style; });
    }

    KRYS_NODISCARD Maybe<NestedParserContextType> CurrentAncestorRuleType() const noexcept
    {
      if (_ancestorRuleTypeStack.empty())
      {
        return null;
      }

      return _ancestorRuleTypeStack.back();
    }

#pragma endregion
  };
}