#pragma once

#include "Krystal.Booey/CSS/Parser/AtRuleId.hpp"
#include "Krystal.Booey/CSS/Parser/ParserEnum.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/ParsedPropertyList.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Properties/Types/IsImportant.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/Rules/RuleType.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::css
{
  class CounterStyleRule;
  class FunctionRule;
  class ImmutableStyleProperties;
  class ImportRule;
  class KeyframeRule;
  class KeyframesRule;
  class MutableStyleProperties;
  class PositionTryRule;
  class StyleSheetContents;
  class ViewTransitionRule;
  struct ParserContext;

  /// @brief Parses style sheets, rules and declaration lists into the internal rule model.
  /// @see https://drafts.csswg.org/css-syntax/#parsing
  class Parser
  {
    KRYS_NON_COPYABLE(Parser);

  public:
    enum class ParseResult : uint8
    {
      Changed,
      Unchanged,
      Error
    };

    enum class AllowedRules : uint8
    {
      // As per css-syntax, css-cascade and css-namespaces, @charset rules
      // must come first, followed by @import then @namespace.
      // AllowImportRules actually means we allow @import and any rules that
      // may follow it, i.e. @namespace rules and regular rules.
      // AllowCharsetRules and AllowNamespaceRules behave similarly.
      CharsetRules,
      LayerStatementRules,
      ImportRules,
      NamespaceRules,
      RegularRules,
      KeyframeRules,
      FontFeatureValuesRules,
      ConditionalGroupRules,
      NoRules, // For parsing at-rules inside declaration lists (without nesting support)
    };

  private:
    struct NestingContext
    {
      // FIXME: Can we build StylePropertySets directly?
      // FIXME: Investigate using a smaller inline buffer
      ParsedPropertyList ParsedProperties;
      List<Ref<Rule>> ParsedRules;
    };

    enum class RuleList : uint8
    {
      TopLevel,
      Regular,
      Keyframes,
      FontFeatureValues,
    };

    enum class BlockAllowedRule : uint8
    {
      QualifiedRules,
      Declarations,
      AtRules,
    };

    // FIXME: we could unify all those into a single stack data structure.
    // https://bugs.webkit.org/show_bug.cgi?id=265566
    uint32 _ruleListNestingLevel {0};
    SmallList<NestedParserContextType, 16> _ancestorRuleTypeStack;
    SmallList<NestingContext, 16> _nestingContextStack {NestingContext {}};

    const ParserContext &_context;

    RefPtr<StyleSheetContents> _styleSheet;
    UniquePtr<Tokenizer> _tokenizer;

  public:
    Parser(const ParserContext &context, CSSOMStringView string, StyleSheetContents *styleSheet = nullptr,
           NestedContext nestedContext = {}) noexcept;
    ~Parser() noexcept;

    KRYS_NODISCARD static ParseResult ParseValue(MutableStyleProperties &declaration, PropertyId property,
                                                 CSSOMStringView string, IsImportant important,
                                                 const ParserContext &context) noexcept;
    KRYS_NODISCARD static ParseResult ParseCustomPropertyValue(MutableStyleProperties &declaration,
                                                               const CSSOMStringAtom &propertyName,
                                                               CSSOMStringView string, IsImportant important,
                                                               const ParserContext &context) noexcept;
    // FIXME: ParseInlineStyleDeclaration(const CSSOMString &, const dom::Element &), once Element has a
    // document with a parser context and quirks mode.
    static bool ParseDeclarationList(MutableStyleProperties &declaration, CSSOMStringView string,
                                     const ParserContext &context) noexcept;
    KRYS_NODISCARD static RefPtr<Rule> ParseRule(CSSOMStringView string, const ParserContext &context,
                                                 StyleSheetContents *styleSheet, AllowedRules allowedRules,
                                                 NestedContext nestedContext = {}) noexcept;
    KRYS_NODISCARD static RefPtr<KeyframeRule> ParseKeyframeRule(CSSOMStringView string,
                                                                 const ParserContext &context) noexcept;
    static void ParseStyleSheet(CSSOMStringView string, const ParserContext &context,
                                StyleSheetContents &styleSheet) noexcept;
    KRYS_NODISCARD static SelectorList ParsePageSelector(TokenRange tokens,
                                                         StyleSheetContents *styleSheet) noexcept;

    KRYS_NODISCARD bool SupportsDeclaration(TokenRange &tokens) noexcept;

    KRYS_NODISCARD const ParserContext &Context() const noexcept
    {
      return _context;
    }

    // This function updates the range it's given.
    KRYS_NODISCARD RefPtr<Rule> ConsumeAtRule(TokenRange &tokens, AllowedRules allowedRules) noexcept;

    KRYS_NODISCARD static IsImportant ConsumeTrailingImportantAndWhitespace(TokenRange &tokens) noexcept;

    KRYS_NODISCARD static RefPtr<NestedDeclarationsRule>
      ParseNestedDeclarations(const ParserContext &context, CSSOMStringView string) noexcept;

    KRYS_NODISCARD Tokenizer *GetTokenizer() const noexcept
    {
      return _tokenizer.get();
    }

  private:
    Parser(const ParserContext &context, StyleSheetContents *styleSheet) noexcept;

    // Returns whether the first encountered rule was valid
    template <typename T>
    bool ConsumeRuleList(TokenRange tokens, RuleList ruleListType, KRYS_NOESCAPE const T &callback) noexcept;

    // This function updates the range it's given.
    KRYS_NODISCARD RefPtr<Rule> ConsumeQualifiedRule(TokenRange &tokens, AllowedRules allowedRules) noexcept;

    // This function is used for all the nested group rules (@media, @layer, @supports, @scope,
    // @container,..etc) It handles potentially orphaned declarations (in the context of style nesting)
    // https://drafts.csswg.org/css-nesting/#conditionals
    KRYS_NODISCARD List<Ref<Rule>> ConsumeNestedGroupRules(TokenRange block) noexcept;

    KRYS_NODISCARD static RefPtr<CharsetRule> ConsumeCharsetRule(TokenRange prelude) noexcept;
    KRYS_NODISCARD RefPtr<ImportRule> ConsumeImportRule(TokenRange prelude) noexcept;
    KRYS_NODISCARD RefPtr<NamespaceRule> ConsumeNamespaceRule(TokenRange prelude) noexcept;
    KRYS_NODISCARD RefPtr<MediaRule> ConsumeMediaRule(TokenRange prelude, TokenRange block) noexcept;
    KRYS_NODISCARD RefPtr<SupportsRule> ConsumeSupportsRule(TokenRange prelude, TokenRange block) noexcept;
    KRYS_NODISCARD RefPtr<FontFaceRule> ConsumeFontFaceRule(TokenRange prelude, TokenRange block) noexcept;
    KRYS_NODISCARD RefPtr<FontFeatureValuesBlockRule>
      ConsumeFontFeatureValuesRuleBlock(AtRuleId id, TokenRange prelude, TokenRange block) noexcept;
    KRYS_NODISCARD RefPtr<FontFeatureValuesRule> ConsumeFontFeatureValuesRule(TokenRange prelude,
                                                                              TokenRange block) noexcept;
    KRYS_NODISCARD RefPtr<FontPaletteValuesRule> ConsumeFontPaletteValuesRule(TokenRange prelude,
                                                                              TokenRange block) noexcept;
    KRYS_NODISCARD RefPtr<KeyframesRule> ConsumeKeyframesRule(TokenRange prelude, TokenRange block) noexcept;
    KRYS_NODISCARD RefPtr<PageRule> ConsumePageRule(TokenRange prelude, TokenRange block) noexcept;
    KRYS_NODISCARD RefPtr<CounterStyleRule> ConsumeCounterStyleRule(TokenRange prelude,
                                                                    TokenRange block) noexcept;
    KRYS_NODISCARD RefPtr<LayerRule> ConsumeLayerRule(TokenRange prelude, Maybe<TokenRange> block) noexcept;
    KRYS_NODISCARD RefPtr<ContainerRule> ConsumeContainerRule(TokenRange prelude, TokenRange block) noexcept;
    KRYS_NODISCARD RefPtr<PropertyRule> ConsumePropertyRule(TokenRange prelude, TokenRange block) noexcept;
    KRYS_NODISCARD RefPtr<ScopeRule> ConsumeScopeRule(TokenRange prelude, TokenRange block) noexcept;
    KRYS_NODISCARD RefPtr<StartingStyleRule> ConsumeStartingStyleRule(TokenRange prelude,
                                                                      TokenRange block) noexcept;
    KRYS_NODISCARD RefPtr<ViewTransitionRule> ConsumeViewTransitionRule(TokenRange prelude,
                                                                        TokenRange block) noexcept;
    KRYS_NODISCARD RefPtr<PositionTryRule> ConsumePositionTryRule(TokenRange prelude,
                                                                  TokenRange block) noexcept;
    KRYS_NODISCARD RefPtr<FunctionRule> ConsumeFunctionRule(TokenRange prelude, TokenRange block) noexcept;

    KRYS_NODISCARD RefPtr<KeyframeRule> ConsumeKeyframeStyleRule(TokenRange prelude,
                                                                 TokenRange block) noexcept;
    KRYS_NODISCARD RefPtr<Rule> ConsumeStyleRule(TokenRange prelude, TokenRange block) noexcept;
    KRYS_NODISCARD ParsedPropertyList ConsumeDeclarationListInNewNestingContext(TokenRange tokens,
                                                                                RuleType ruleType) noexcept;
    KRYS_NODISCARD List<Ref<Rule>> ConsumeDeclarationRuleListInNewNestingContext(TokenRange tokens,
                                                                                 RuleType ruleType) noexcept;

    // FIXME: We should return value for all those functions instead of using class member attributes.
    void ConsumeBlockContent(TokenRange tokens, RuleType ruleType,
                             EnumSet<BlockAllowedRule> blockAllowedRules) noexcept;
    void ConsumeDeclarationList(TokenRange tokens, RuleType ruleType) noexcept;
    void ConsumeDeclarationRuleList(TokenRange tokens, RuleType ruleType) noexcept;
    void ConsumeStyleBlock(TokenRange tokens, RuleType ruleType) noexcept;
    bool ConsumeDeclaration(TokenRange tokens, RuleType ruleType) noexcept;
    void ConsumeDeclarationValue(TokenRange tokens, PropertyId property, IsImportant important,
                                 RuleType ruleType) noexcept;
    void ConsumeCustomPropertyValue(TokenRange tokens, const CSSOMStringAtom &propertyName,
                                    IsImportant important) noexcept;

    KRYS_NODISCARD TokenRange ConsumeBlockCheckingForEditability(TokenRange &tokens) noexcept;

    KRYS_NODISCARD Ref<Rule> CreateNestedDeclarationsRule() noexcept;

    void RunInNewNestingContext(auto &&run) noexcept
    {
      _nestingContextStack.push_back(NestingContext {});
      run();
      _nestingContextStack.pop_back();
    }

    KRYS_NODISCARD NestingContext &TopContext() noexcept
    {
      krys_debug_assert(!_nestingContextStack.empty());
      return _nestingContextStack.back();
    }

    KRYS_NODISCARD bool IsStyleNestedContext() const noexcept
    {
      return !_ancestorRuleTypeStack.empty()
             && _ancestorRuleTypeStack.back() != NestedParserContextType::Function;
    }

    KRYS_NODISCARD bool IsFunctionNestedContext() const noexcept
    {
      return !_ancestorRuleTypeStack.empty()
             && _ancestorRuleTypeStack.back() == NestedParserContextType::Function;
    }

    KRYS_NODISCARD bool HasStyleRuleAncestor() const noexcept
    {
      for (auto type : _ancestorRuleTypeStack)
      {
        if (type == NestedParserContextType::Style)
        {
          return true;
        }
      }

      return false;
    }

    KRYS_NODISCARD NestedContext LastAncestorRuleType() const noexcept
    {
      if (!_ancestorRuleTypeStack.empty())
      {
        return _ancestorRuleTypeStack.back();
      }

      return null;
    }
  };
}
