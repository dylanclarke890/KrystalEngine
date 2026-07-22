#pragma once

#include "Krystal.HTML/CSS/Enums/CSSRuleType.hpp"
#include "Krystal.HTML/CSS/Parser/CSSInputStream.hpp"
#include "Krystal.HTML/CSS/Parser/CSSToken.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenizer.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Parser/Enums/CSSAllowedBlockRules.hpp"
#include "Krystal.HTML/CSS/Parser/Enums/CSSAllowedRules.hpp"
#include "Krystal.HTML/CSS/Parser/Enums/NestedContextType.hpp"
#include "Krystal.HTML/CSS/Parser/Types/NestedParsingContext.hpp"
#include "Krystal.HTML/CSS/Properties/CSSInternalStyleProperties.hpp"
#include "Krystal.HTML/CSS/Properties/CSSProperty.hpp"
#include "Krystal.HTML/URL/Url.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSRule;
  class CSSCharsetRule;
  class CSSImportRule;
  class CSSNamespaceRule;
  class CSSStyleRule;
  class CSSStyleSheet;

  class CSSParser
  {
  private:
    CSSInputStream _inputStream;
    CSSTokenizer _tokenizer;
    size_t _ruleListNestingLevel;
    List<NestedParsingContext> _nestedContextStack;
    List<NestedContextType> _ancestorRuleTypeStack;

  public:
    CSSParser(utf8_string &&input) noexcept : _inputStream(Krys::Move(input)), _tokenizer(_inputStream)
    {
    }

#pragma region Parser Entry Points - https://drafts.csswg.org/css-syntax/#parser-entry-points

    /// @see https://drafts.csswg.org/css-syntax/#parse-stylesheet
    KRYS_NODISCARD static RefPtr<CSSStyleSheet> ParseStylesheet(utf8_string &&input) noexcept;

    /// @see https://drafts.csswg.org/css-syntax/#parse-a-rule
    KRYS_NODISCARD static RefPtr<CSSRule> ParseRule(utf8_string &&input,
                                                    CSSAllowedRules allowedRules) noexcept;

#pragma endregion

  private:
    /// @brief Pumps the tokenizer until the input stream is exhausted. Returns true if the tokenizer has
    /// consumed at least one token.
    bool PumpTokenizer() noexcept;

    /// @brief Gets the range of tokens produced by the tokenizer.
    /// @note The tokens are only valid as long as the tokenizer is alive. The tokenizer owns the tokens and
    /// will free them when it is destroyed.
    KRYS_NODISCARD CSSTokenRange TokenRange() const noexcept;

    /// @brief Computes the next allowed rules based on the current allowed rules and the reference rule. For
    /// example, if the current allowed rules are `Charset` and the reference rule is an `@import` rule, the
    /// next allowed rules will be `Import` (`Charset` is not allowed after an `@import` rule).
    KRYS_NODISCARD static CSSAllowedRules ComputeNextAllowedRules(CSSAllowedRules current,
                                                                  RawPtr<CSSRule> ref) noexcept;

    KRYS_NODISCARD static Ref<CSSInternalStyleProperties>
      CreateInternalStyleProperties(ParsedCSSPropertyList &properties) noexcept;

#pragma region Parser Algorithms - https://drafts.csswg.org/css-syntax/#parser-algorithms

    /// @see https://drafts.csswg.org/css-syntax/#consume-a-stylesheets-contents
    KRYS_NODISCARD List<Ref<CSSRule>> ConsumeStyleSheetContents(CSSTokenRange &input) noexcept;

    /// @see https://drafts.csswg.org/css-syntax/#consume-an-at-rule
    KRYS_NODISCARD RefPtr<CSSRule> ConsumeAtRule(CSSTokenRange &tokens, CSSAllowedRules allowedRules,
                                                 bool nested = false) noexcept;

    /// @see https://drafts.csswg.org/css-syntax/#consume-a-qualified-rule
    KRYS_NODISCARD RefPtr<CSSRule> ConsumeQualifiedRule(CSSTokenRange &tokens, CSSAllowedRules allowedRules,
                                                        bool nested = false) noexcept;

    /// @see https://drafts.csswg.org/css-syntax/#consume-a-blocks-contents
    void ConsumeBlockContents(CSSTokenRange tokens, CSSAllowedBlockRules allowedBlockRules,
                              CSSRuleType ruleType) noexcept;

    /// @see https://drafts.csswg.org/css-syntax-3/#consume-a-declaration
    bool ConsumeDeclaration(CSSTokenRange &tokens, CSSRuleType ruleType) noexcept;

    IsImportant ConsumeTrailingImportantAndWhitespace(CSSTokenRange &range);

    /// @see https://drafts.csswg.org/css-syntax/#consume-the-remnants-of-a-bad-declaration
    void ConsumeBadDeclaration(CSSTokenRange &tokens, bool nested) noexcept;

    /// @see https://drafts.csswg.org/css-syntax-3/#typedef-declaration-list
    void ConsumeDeclarationList(CSSTokenRange block, CSSRuleType ruleType) noexcept;

    /// @see https://drafts.csswg.org/css-syntax-3/#typedef-qualified-rule-list
    void ConsumeQualifiedRuleList(CSSTokenRange block, CSSRuleType ruleType) noexcept;

    /// @see https://drafts.csswg.org/css-syntax-3/#typedef-at-rule-list
    void ConsumeAtRuleList(CSSTokenRange block, CSSRuleType ruleType) noexcept;

    /// @see https://drafts.csswg.org/css-syntax-3/#typedef-declaration-rule-list
    void ConsumeDeclarationRuleList(CSSTokenRange block, CSSRuleType ruleType) noexcept;

    /// @see https://drafts.csswg.org/css-syntax-3/#typedef-rule-list
    void ConsumeRuleList(CSSTokenRange block, CSSRuleType ruleType) noexcept;

    /// @see https://drafts.csswg.org/css-syntax-3/#typedef-block-contents
    void ConsumeStyleBlock(CSSTokenRange block) noexcept;

#pragma endregion

#pragma region At-Rule Consumers

    KRYS_NODISCARD RefPtr<CSSCharsetRule> ConsumeCharsetRule(CSSTokenRange prelude) noexcept;

    KRYS_NODISCARD RefPtr<CSSImportRule> ConsumeImportRule(CSSTokenRange prelude) noexcept;

    KRYS_NODISCARD RefPtr<CSSNamespaceRule> ConsumeNamespaceRule(CSSTokenRange prelude) noexcept;

#pragma endregion

#pragma region Qualified Rule Consumers

    KRYS_NODISCARD RefPtr<CSSStyleRule> ConsumeStyleRule(CSSTokenRange prelude, CSSTokenRange block) noexcept;

    KRYS_NODISCARD RefPtr<CSSStyleRule> ConsumeKeyframeStyleRule(CSSTokenRange prelude,
                                                                 CSSTokenRange block) noexcept;

#pragma endregion

#pragma region Nested Parsing Context

    void RunInNewNestedParsingContext(auto &&run) noexcept
    {
      _nestedContextStack.push_back(NestedParsingContext {});
      run();
      _nestedContextStack.pop_back();
    }

    KRYS_NODISCARD NestedParsingContext &CurrentNestedContext() noexcept
    {
      assert(!_nestedContextStack.empty());
      return _nestedContextStack.back();
    }

    KRYS_NODISCARD bool IsStyleNestedParsingContext() const noexcept
    {
      return !_ancestorRuleTypeStack.empty() && _ancestorRuleTypeStack.back() != NestedContextType::Function;
    }

    KRYS_NODISCARD bool IsFunctionNestedParsingContext() const noexcept
    {
      return !_ancestorRuleTypeStack.empty() && _ancestorRuleTypeStack.back() == NestedContextType::Function;
    }

    KRYS_NODISCARD bool HasStyleRuleAncestor() const noexcept
    {
      return std::ranges::any_of(_ancestorRuleTypeStack,
                                 [](NestedContextType type) { return type == NestedContextType::Style; });
    }

    KRYS_NODISCARD Maybe<NestedContextType> CurrentAncestorRuleType() const noexcept
    {
      if (_ancestorRuleTypeStack.empty())
      {
        return {};
      }

      return _ancestorRuleTypeStack.back();
    }

#pragma endregion
  };
}