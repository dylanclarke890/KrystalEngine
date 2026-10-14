#include "Krystal.Booey/CSS/CSSOM/CSSStyleRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSRuleList.hpp"
#include "Krystal.Booey/CSS/CSSStyleProperties.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Parser/Parser.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParser.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Style/RuleData.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo::css
{
  namespace
  {
    // NOTE: The selector list has to fit into RuleData. <http://webkit.org/b/118369>
    constexpr uint32 MaximumSelectorComponentCount = style::RuleData::MaximumSelectorComponentCount;

    using SelectorTextCache = HashMap<const CSSStyleRule *, CSSOMString>;

    KRYS_NODISCARD SelectorTextCache &GetSelectorTextCache() noexcept
    {
      static MainThreadNeverDestroyed<SelectorTextCache> cache;
      return cache;
    }

    KRYS_NODISCARD const List<Ref<Rule>> &EmptyRuleList() noexcept
    {
      static MainThreadNeverDestroyed<List<Ref<Rule>>> rules;
      return rules;
    }
  }

  CSSStyleRule::CSSStyleRule(StyleRule &styleRule, CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _styleRule(ShareRef(styleRule))
  {
  }

  CSSStyleRule::CSSStyleRule(StyleRuleWithNesting &styleRule, CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _styleRule(ShareRef(styleRule)),
        _childRuleCSSOMWrappers(styleRule.NestedRules().size())
  {
  }

  CSSStyleRule::~CSSStyleRule() noexcept
  {
    if (_propertiesCSSOMWrapper)
    {
      _propertiesCSSOMWrapper->ClearParentRule();
    }

    if (HasCachedSelectorText())
    {
      GetSelectorTextCache().erase(this);
      SetHasCachedSelectorText(false);
    }
  }

  Ref<CSSStyleRule> CSSStyleRule::Create(StyleRule &rule, CSSStyleSheet *sheet) noexcept
  {
    return AdoptRef(*new CSSStyleRule(rule, sheet));
  }

  Ref<CSSStyleRule> CSSStyleRule::Create(StyleRuleWithNesting &rule, CSSStyleSheet *sheet) noexcept
  {
    return AdoptRef(*new CSSStyleRule(rule, sheet));
  }

  CSSStyleProperties &CSSStyleRule::Style() noexcept
  {
    if (!_propertiesCSSOMWrapper)
    {
      _propertiesCSSOMWrapper = StyleRuleCSSStyleProperties::Create(_styleRule->MutableProperties(), *this);
    }
    return *_propertiesCSSOMWrapper;
  }

  CSSOMString CSSStyleRule::GenerateSelectorText() const noexcept
  {
    if (auto *styleRule = DynamicDowncast<StyleRuleWithNesting>(_styleRule.get()))
    {
      return styleRule->OriginalSelectorList().SelectorsText();
    }
    return _styleRule->GetSelectorList().SelectorsText();
  }

  CSSOMString CSSStyleRule::SelectorText() const noexcept
  {
    auto &cache = GetSelectorTextCache();
    if (HasCachedSelectorText())
    {
      krys_debug_assert(cache.contains(this));
      return cache.at(this);
    }

    krys_debug_assert(!cache.contains(this));
    auto text = GenerateSelectorText();
    cache[this] = text;
    SetHasCachedSelectorText(true);
    return text;
  }

  void CSSStyleRule::SetSelectorText(const CSSOMString &selectorText) noexcept
  {
    // FIXME: getMatchedCSSRules can return CSSStyleRules that are missing parent stylesheet pointer while
    // referencing StyleRules that are part of stylesheet. Disallow mutations in this case.
    auto *sheet = ParentStyleSheet();
    if (!sheet)
    {
      return;
    }

    auto selectorList = SelectorParser::ParseSelectorList(
      selectorText, SelectorParserContext(GetParserContext()), &sheet->Contents(), GetNestedContext());
    if (!selectorList)
    {
      return;
    }

    // NOTE: The selector list has to fit into RuleData. <http://webkit.org/b/118369>
    if (selectorList->ComponentCount() > MaximumSelectorComponentCount)
    {
      return;
    }

    CSSStyleSheet::RuleMutationScope mutationScope(this);

    if (auto *styleRule = DynamicDowncast<StyleRuleWithNesting>(_styleRule.get()))
    {
      styleRule->WrapperAdoptOriginalSelectorList(krys::move(*selectorList));
    }
    else
    {
      _styleRule->WrapperAdoptSelectorList(krys::move(*selectorList));
    }

    if (HasCachedSelectorText())
    {
      GetSelectorTextCache().erase(this);
      SetHasCachedSelectorText(false);
    }
  }

  const List<Ref<Rule>> &CSSStyleRule::NestedRules() const noexcept
  {
    if (auto *styleRule = DynamicDowncast<StyleRuleWithNesting>(_styleRule.get()))
    {
      return styleRule->NestedRules();
    }
    return EmptyRuleList();
  }

  // https://w3c.github.io/csswg-drafts/cssom-1/#serialize-a-css-rule
  CSSOMString CSSStyleRule::CssText() const noexcept
  {
    auto declarations = _styleRule->Properties().AsText(DefaultSerialisationContext());
    CSSOMString rules;
    CssTextForRules(rules);
    return CssTextInternal(declarations, rules);
  }

  void CSSStyleRule::CssTextForRules(CSSOMString &rules) const noexcept
  {
    for (uint32 index = 0; index < Length(); ++index)
    {
      auto ruleText = Item(index)->CssText();
      if (!ruleText.empty())
      {
        rules.append(u8"\n  ").append(ruleText);
      }
    }
  }

  CSSOMString CSSStyleRule::CssText(const SerialisationContext &context) const noexcept
  {
    auto declarations = _styleRule->Properties().AsText(context);
    CSSOMString rules;
    CssTextForRulesWithReplacementURLs(rules, context);
    return CssTextInternal(declarations, rules);
  }

  void CSSStyleRule::CssTextForRulesWithReplacementURLs(CSSOMString &rules,
                                                        const SerialisationContext &context) const noexcept
  {
    for (uint32 index = 0; index < Length(); index++)
    {
      rules.append(u8"\n  ").append(Item(index)->CssText(context));
    }
  }

  CSSOMString CSSStyleRule::CssTextInternal(const CSSOMString &declarations,
                                            const CSSOMString &rules) const noexcept
  {
    CSSOMString builder;
    builder.append(SelectorText()).append(u8" {");

    if (declarations.empty() && rules.empty())
    {
      builder.append(u8" }");
      return builder;
    }

    if (rules.empty())
    {
      builder.push_back(u8' ');
      builder.append(declarations);
      builder.append(u8" }");
      return builder;
    }

    if (declarations.empty())
    {
      builder.append(rules);
      builder.append(u8"\n}");
      return builder;
    }

    builder.append(u8"\n  ");
    builder.append(declarations);
    builder.append(rules);
    builder.append(u8"\n}");
    return builder;
  }

  // FIXME: share all methods below with CSSGroupingRule.

  void CSSStyleRule::Reattach(Rule &rule) noexcept
  {
    if (auto *styleRuleWithNesting = DynamicDowncast<StyleRuleWithNesting>(&rule))
    {
      _styleRule = ShareRef(*styleRuleWithNesting);
    }
    else
    {
      _styleRule = ShareRef(Downcast<StyleRule>(rule));
    }

    if (_propertiesCSSOMWrapper)
    {
      _propertiesCSSOMWrapper->Reattach(_styleRule->MutableProperties());
    }
  }

  dom::ExceptionOr<uint32> CSSStyleRule::InsertRule(const CSSOMString &ruleString, uint32 index) noexcept
  {
    krys_debug_assert(_childRuleCSSOMWrappers.size() == NestedRules().size());

    if (index > NestedRules().size())
    {
      return dom::ExceptionCode::IndexSizeError;
    }

    auto *styleSheet = ParentStyleSheet();
    auto newRule =
      Parser::ParseRule(ruleString, GetParserContext(), styleSheet ? &styleSheet->Contents() : nullptr,
                        Parser::AllowedRules::ImportRules, NestedParserContextType::Style);
    if (!newRule)
    {
      newRule = Parser::ParseNestedDeclarations(GetParserContext(), ruleString);
      if (!newRule)
      {
        return dom::ExceptionCode::SyntaxError;
      }
    }

    // We only accepts style rule, nested group rule (@media,...) or nested declarations inside style rules.
    if (!newRule->IsStyleRule() && !newRule->IsGroupRule() && !newRule->IsNestedDeclarationsRule())
    {
      return dom::ExceptionCode::HierarchyRequestError;
    }

    CSSStyleSheet::RuleMutationScope mutationScope(this);
    if (!_styleRule->IsStyleRuleWithNesting())
    {
      // Call the parent rule (or parent stylesheet if top-level or nothing if it's an orphaned rule) to
      // transform the current StyleRule to StyleRuleWithNesting.
      RefPtr<StyleRuleWithNesting> styleRuleWithNesting;
      if (auto *parent = ParentRule())
      {
        styleRuleWithNesting = parent->PrepareChildStyleRuleForNesting(*_styleRule);
      }
      else if (auto *parentSheet = ParentStyleSheet())
      {
        styleRuleWithNesting = parentSheet->PrepareChildStyleRuleForNesting(*_styleRule);
      }
      else
      {
        styleRuleWithNesting = StyleRuleWithNesting::Create(krys::move(*_styleRule));
      }
      krys_debug_assert(styleRuleWithNesting);
      _styleRule = Ref<StyleRule>(krys::move(styleRuleWithNesting));
    }

    auto &nestedRules = Downcast<StyleRuleWithNesting>(*_styleRule).NestedRules();
    nestedRules.insert(nestedRules.begin() + index, Ref<Rule>(krys::move(newRule)));
    _childRuleCSSOMWrappers.insert(_childRuleCSSOMWrappers.begin() + index, RefPtr<CSSRule>());
    return index;
  }

  dom::ExceptionOr<void> CSSStyleRule::DeleteRule(uint32 index) noexcept
  {
    krys_debug_assert(_childRuleCSSOMWrappers.size() == NestedRules().size());

    if (index >= NestedRules().size())
    {
      // IndexSizeError: Raised if the specified index does not correspond to a
      // rule in the media rule list.
      return dom::ExceptionCode::IndexSizeError;
    }

    auto &rules = Downcast<StyleRuleWithNesting>(*_styleRule).NestedRules();

    CSSStyleSheet::RuleMutationScope mutationScope(this);
    rules.erase(rules.begin() + index);

    if (_childRuleCSSOMWrappers[index])
    {
      _childRuleCSSOMWrappers[index]->SetParentRule(nullptr);
    }
    _childRuleCSSOMWrappers.erase(_childRuleCSSOMWrappers.begin() + index);

    return {};
  }

  uint32 CSSStyleRule::Length() const noexcept
  {
    return static_cast<uint32>(NestedRules().size());
  }

  CSSRule *CSSStyleRule::Item(uint32 index) const noexcept
  {
    if (index >= Length())
    {
      return nullptr;
    }

    krys_debug_assert(_childRuleCSSOMWrappers.size() == NestedRules().size());
    auto &rule = _childRuleCSSOMWrappers[index];
    if (!rule)
    {
      rule = NestedRules()[index]->CreateCSSOMWrapper(const_cast<CSSStyleRule &>(*this));
    }
    return rule.get();
  }

  CSSRuleList &CSSStyleRule::CssRules() const noexcept
  {
    if (!_ruleListCSSOMWrapper)
    {
      _ruleListCSSOMWrapper = CreateUnique<LiveCSSRuleList<CSSStyleRule>>(const_cast<CSSStyleRule &>(*this));
    }
    return *_ruleListCSSOMWrapper;
  }

  void CSSStyleRule::GetChildStyleSheets(HashSet<RefPtr<CSSStyleSheet>> &childStyleSheets) noexcept
  {
    for (uint32 index = 0; index < Length(); ++index)
    {
      Item(index)->GetChildStyleSheets(childStyleSheets);
    }
  }
}
