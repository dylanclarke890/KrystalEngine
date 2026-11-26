#include "Krystal.Web/Css/Rules/Rule.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Web/Css/CSSOM/CSSContainerRule.hpp"
#include "Krystal.Web/Css/CSSOM/CSSCounterStyleRule.hpp"
#include "Krystal.Web/Css/CSSOM/CSSFontFaceRule.hpp"
#include "Krystal.Web/Css/CSSOM/CSSFontFeatureValuesRule.hpp"
#include "Krystal.Web/Css/CSSOM/CSSFontPaletteValuesRule.hpp"
#include "Krystal.Web/Css/CSSOM/CSSFunctionDeclarations.hpp"
#include "Krystal.Web/Css/CSSOM/CSSFunctionRule.hpp"
#include "Krystal.Web/Css/CSSOM/CSSImportRule.hpp"
#include "Krystal.Web/Css/CSSOM/CSSKeyframesRule.hpp"
#include "Krystal.Web/Css/CSSOM/CSSLayerBlockRule.hpp"
#include "Krystal.Web/Css/CSSOM/CSSLayerStatementRule.hpp"
#include "Krystal.Web/Css/CSSOM/CSSMediaRule.hpp"
#include "Krystal.Web/Css/CSSOM/CSSNamespaceRule.hpp"
#include "Krystal.Web/Css/CSSOM/CSSNestedDeclarations.hpp"
#include "Krystal.Web/Css/CSSOM/CSSPositionTryRule.hpp"
#include "Krystal.Web/Css/CSSOM/CSSPropertyRule.hpp"
#include "Krystal.Web/Css/CSSOM/CSSScopeRule.hpp"
#include "Krystal.Web/Css/CSSOM/CSSStartingStyleRule.hpp"
#include "Krystal.Web/Css/CSSOM/CSSStyleRule.hpp"
#include "Krystal.Web/Css/CSSOM/CSSSupportsRule.hpp"
#include "Krystal.Web/Css/Properties/MutableStyleProperties.hpp"
#include "Krystal.Web/Css/Properties/StyleProperties.hpp"
#include "Krystal.Web/Css/Rules/CounterStyleRule.hpp"
#include "Krystal.Web/Css/Rules/FunctionRule.hpp"
#include "Krystal.Web/Css/Rules/ImportRule.hpp"
#include "Krystal.Web/Css/Rules/KeyframeRule.hpp"
#include "Krystal.Web/Css/Rules/KeyframesRule.hpp"
#include "Krystal.Web/Css/Rules/PositionTryRule.hpp"
#include "Krystal.Web/Css/StyleSheets/StyleSheetContents.hpp"
#include <functional>
#include <memory>

namespace krys::web::css
{
  namespace
  {
    /// @brief @p properties, first replaced by a mutable copy if they aren't mutable.
    KRYS_NODISCARD MutableStyleProperties &EnsureMutable(Ref<StyleProperties> &properties) noexcept
    {
      if (!Is<MutableStyleProperties>(*properties))
      {
        properties = properties->MutableCopy();
      }
      return Downcast<MutableStyleProperties>(*properties);
    }

    /// @brief Copies of @p rules.
    KRYS_NODISCARD List<Ref<Rule>> CopyRules(const List<Ref<Rule>> &rules) noexcept
    {
      List<Ref<Rule>> copies;
      copies.reserve(rules.size());
      for (const auto &rule : rules)
      {
        copies.push_back(rule->Copy());
      }
      return copies;
    }
  }

#pragma region Rule

  template <typename TVisitor>
  constexpr decltype(auto) Rule::VisitDerived(TVisitor &&visitor) noexcept
  {
    const auto visit = [&]<typename TRule>() -> decltype(auto)
    {
      return std::invoke(std::forward<TVisitor>(visitor), static_cast<TRule &>(*this));
    };

    switch (Type())
    {
      case RuleType::Style:                  return visit.template operator()<StyleRule>();
      case RuleType::StyleWithNesting:       return visit.template operator()<StyleRuleWithNesting>();
      case RuleType::NestedDeclarations:     return visit.template operator()<NestedDeclarationsRule>();
      case RuleType::FontFace:               return visit.template operator()<FontFaceRule>();
      case RuleType::FontFeatureValues:      return visit.template operator()<FontFeatureValuesRule>();
      case RuleType::FontFeatureValuesBlock: return visit.template operator()<FontFeatureValuesBlockRule>();
      case RuleType::FontPaletteValues:      return visit.template operator()<FontPaletteValuesRule>();
      case RuleType::Media:                  return visit.template operator()<MediaRule>();
      case RuleType::Supports:               return visit.template operator()<SupportsRule>();
      case RuleType::Import:                 return visit.template operator()<ImportRule>();
      case RuleType::Keyframes:              return visit.template operator()<KeyframesRule>();
      case RuleType::Namespace:              return visit.template operator()<NamespaceRule>();
      case RuleType::Keyframe:               return visit.template operator()<KeyframeRule>();
      case RuleType::Charset:                return visit.template operator()<CharsetRule>();
      case RuleType::CounterStyle:           return visit.template operator()<CounterStyleRule>();
      case RuleType::LayerBlock:
      case RuleType::LayerStatement:         return visit.template operator()<LayerRule>();
      case RuleType::Container:              return visit.template operator()<ContainerRule>();
      case RuleType::Property:               return visit.template operator()<PropertyRule>();
      case RuleType::Scope:                  return visit.template operator()<ScopeRule>();
      case RuleType::StartingStyle:          return visit.template operator()<StartingStyleRule>();
      case RuleType::PositionTry:            return visit.template operator()<PositionTryRule>();
      case RuleType::Function:               return visit.template operator()<FunctionRule>();
      case RuleType::FunctionDeclarations:   return visit.template operator()<FunctionDeclarationsRule>();
      // No margin rules are parsed (there's no paged media); the type only fills the CSSOM's numbering.
      case RuleType::Margin:                 break;
    }
    krys_unreachable();
  }

  template <typename TVisitor>
  constexpr decltype(auto) Rule::VisitDerived(TVisitor &&visitor) const noexcept
  {
    return const_cast<Rule &>(*this).VisitDerived(
      [&](auto &rule) -> decltype(auto)
      { return std::invoke(std::forward<TVisitor>(visitor), std::as_const(rule)); });
  }

  void Rule::operator delete(Rule *rule, std::destroying_delete_t) noexcept
  {
    rule->VisitDerived(
      []<typename TRule>(TRule &derived)
      {
        std::destroy_at(&derived);
        ::operator delete(&derived);
      });
  }

  Ref<Rule> Rule::Copy() const noexcept
  {
    return VisitDerived(
      []<typename TRule>(const TRule &rule) -> Ref<Rule>
      {
        // A class without its own Copy would find this one and recurse forever.
        static_assert(!SameType<decltype(&TRule::Copy), decltype(&Rule::Copy)>);
        return rule.Copy();
      });
  }

  Ref<CSSRule> Rule::CreateCSSOMWrapper(CSSStyleSheet &parentSheet) noexcept
  {
    return CreateCSSOMWrapper(&parentSheet, nullptr);
  }

  Ref<CSSRule> Rule::CreateCSSOMWrapper(CSSGroupingRule &parentRule) noexcept
  {
    return CreateCSSOMWrapper(nullptr, &parentRule);
  }

  Ref<CSSRule> Rule::CreateCSSOMWrapper(CSSStyleRule &parentRule) noexcept
  {
    return CreateCSSOMWrapper(nullptr, &parentRule);
  }

  Ref<CSSRule> Rule::CreateCSSOMWrapper() noexcept
  {
    return CreateCSSOMWrapper(nullptr, nullptr);
  }

  Ref<CSSRule> Rule::CreateCSSOMWrapper(CSSStyleSheet *parentSheet, CSSRule *parentRule) noexcept
  {
    auto wrapper = VisitDerived(
      [&]<typename TRule>(TRule &rule) -> Ref<CSSRule>
      {
        if constexpr (SameType<TRule, StyleRule> || SameType<TRule, StyleRuleWithNesting>)
        {
          return CSSStyleRule::Create(rule, parentSheet);
        }
        else if constexpr (SameType<TRule, NestedDeclarationsRule>)
        {
          return CSSNestedDeclarations::Create(rule, parentSheet);
        }
        else if constexpr (SameType<TRule, FontFaceRule>)
        {
          return CSSFontFaceRule::Create(rule, parentSheet);
        }
        else if constexpr (SameType<TRule, FontFeatureValuesRule>)
        {
          return CSSFontFeatureValuesRule::Create(rule, parentSheet);
        }
        else if constexpr (SameType<TRule, FontFeatureValuesBlockRule>)
        {
          return CSSFontFeatureValuesBlockRule::Create(rule, parentSheet);
        }
        else if constexpr (SameType<TRule, FontPaletteValuesRule>)
        {
          return CSSFontPaletteValuesRule::Create(rule, parentSheet);
        }
        else if constexpr (SameType<TRule, MediaRule>)
        {
          return CSSMediaRule::Create(rule, parentSheet);
        }
        else if constexpr (SameType<TRule, SupportsRule>)
        {
          return CSSSupportsRule::Create(rule, parentSheet);
        }
        else if constexpr (SameType<TRule, ImportRule>)
        {
          return CSSImportRule::Create(rule, parentSheet);
        }
        else if constexpr (SameType<TRule, KeyframesRule>)
        {
          return CSSKeyframesRule::Create(rule, parentSheet);
        }
        else if constexpr (SameType<TRule, NamespaceRule>)
        {
          return CSSNamespaceRule::Create(rule, parentSheet);
        }
        else if constexpr (SameType<TRule, CounterStyleRule>)
        {
          return CSSCounterStyleRule::Create(rule, parentSheet);
        }
        else if constexpr (SameType<TRule, LayerRule>)
        {
          if (rule.IsStatement())
          {
            return CSSLayerStatementRule::Create(rule, parentSheet);
          }
          return CSSLayerBlockRule::Create(rule, parentSheet);
        }
        else if constexpr (SameType<TRule, ContainerRule>)
        {
          return CSSContainerRule::Create(rule, parentSheet);
        }
        else if constexpr (SameType<TRule, PropertyRule>)
        {
          return CSSPropertyRule::Create(rule, parentSheet);
        }
        else if constexpr (SameType<TRule, ScopeRule>)
        {
          return CSSScopeRule::Create(rule, parentSheet);
        }
        else if constexpr (SameType<TRule, StartingStyleRule>)
        {
          return CSSStartingStyleRule::Create(rule, parentSheet);
        }
        else if constexpr (SameType<TRule, PositionTryRule>)
        {
          return CSSPositionTryRule::Create(rule, parentSheet);
        }
        else if constexpr (SameType<TRule, FunctionRule>)
        {
          return CSSFunctionRule::Create(rule, parentSheet);
        }
        else if constexpr (SameType<TRule, FunctionDeclarationsRule>)
        {
          return CSSFunctionDeclarations::Create(rule, parentSheet);
        }
        else
        {
          // `@charset` rules and keyframes (which `CSSKeyframesRule` wraps) have no wrapper of their own.
          static_assert(SameType<TRule, CharsetRule> || SameType<TRule, KeyframeRule>);
          krys_unreachable();
        }
      });

    if (parentRule)
    {
      wrapper->SetParentRule(parentRule);
    }
    return wrapper;
  }

  void Rule::InvalidateResolvedSelectorListRecursively() noexcept
  {
    VisitDerived(
      []<typename TRule>(TRule &rule)
      {
        if constexpr (SameType<TRule, StyleRuleWithNesting>)
        {
          rule.AdoptSelectorList({});
          for (auto &child : rule.NestedRules())
          {
            child->InvalidateResolvedSelectorListRecursively();
          }
        }
        else if constexpr (SameType<TRule, NestedDeclarationsRule>)
        {
          rule.AdoptSelectorList({});
        }
        else if constexpr (BaseOf<GroupRule, TRule>)
        {
          if constexpr (SameType<TRule, ScopeRule>)
          {
            rule.SetScopeStart({});
            rule.SetScopeEnd({});
          }
          for (auto &child : rule.ChildRules())
          {
            child->InvalidateResolvedSelectorListRecursively();
          }
        }
      });
  }

#pragma endregion

#pragma region StyleRule

  StyleRule::StyleRule(Ref<StyleProperties> properties, SelectorList selectors) noexcept
      : Rule(RuleType::Style), _properties(krys::move(properties)), _selectorList(krys::move(selectors))
  {
  }

  StyleRule::StyleRule(const StyleRule &other) noexcept
      : Rule(other), _isSplitRule(other._isSplitRule), _isLastRuleInSplitRule(other._isLastRuleInSplitRule),
        _properties(other.Properties().MutableCopy()), _selectorList(other._selectorList)
  {
  }

  StyleRule::~StyleRule() noexcept = default;

  Ref<StyleRule> StyleRule::Create(Ref<StyleProperties> properties, SelectorList selectors) noexcept
  {
    return AdoptRef(*new StyleRule(krys::move(properties), krys::move(selectors)));
  }

  Ref<StyleRule> StyleRule::Copy() const noexcept
  {
    return AdoptRef(*new StyleRule(*this));
  }

  MutableStyleProperties &StyleRule::MutableProperties() noexcept
  {
    return EnsureMutable(_properties);
  }

  void StyleRule::WrapperAdoptSelectorList(SelectorList selectors) noexcept
  {
    AdoptSelectorList(krys::move(selectors));
  }

  uint32 StyleRule::AverageSizeInBytes() noexcept
  {
    return static_cast<uint32>(sizeof(StyleRule) + sizeof(Selector) + StyleProperties::AverageSizeInBytes()
                               + sizeof(List<Ref<Rule>>));
  }

  Ref<StyleRule> StyleRule::CreateForSplitting(const List<const Selector *> &selectors,
                                               Ref<StyleProperties> properties) noexcept
  {
    krys_assert(!selectors.empty());
    auto selectorArray = CreateUniqueArray<Selector>(selectors.size());
    for (size_t i = 0uz; i < selectors.size(); ++i)
    {
      new (NotNullTag {}, &selectorArray[i]) Selector(*selectors[i]);
    }
    selectorArray[selectors.size() - 1].SetLastInSelectorList();
    auto styleRule = Create(krys::move(properties), SelectorList(krys::move(selectorArray)));
    styleRule->MarkAsSplitRule();
    return styleRule;
  }

  List<Ref<StyleRule>>
    StyleRule::SplitIntoMultipleRulesWithMaximumSelectorComponentCount(uint32 maxCount) const noexcept
  {
    krys_debug_assert(GetSelectorList().ComponentCount() > maxCount);

    List<Ref<StyleRule>> rules;
    // The pieces share the declarations.
    const auto split = [&](const List<const Selector *> &components)
    {
      rules.push_back(CreateForSplitting(components, ShareRef(const_cast<StyleProperties &>(Properties()))));
    };

    List<const Selector *> pendingComponents;
    for (const auto &complexSelector : GetSelectorList())
    {
      List<const Selector *> components;
      for (const auto *component = &complexSelector; component;
           component = component->PrecedingComplexSelectorComponent())
      {
        components.push_back(component);
      }
      // Complex selectors are kept whole, so a piece only exceeds the limit if one complex selector does.
      if (!pendingComponents.empty() && pendingComponents.size() + components.size() > maxCount)
      {
        split(pendingComponents);
        pendingComponents.clear();
      }
      pendingComponents.insert(pendingComponents.end(), components.begin(), components.end());
    }
    if (!pendingComponents.empty())
    {
      split(pendingComponents);
    }

    if (!rules.empty())
    {
      rules.back()->MarkAsLastRuleInSplitRule();
    }
    return rules;
  }

#pragma endregion

#pragma region StyleRuleWithNesting

  StyleRuleWithNesting::StyleRuleWithNesting(Ref<StyleProperties> properties, SelectorList selectors,
                                             List<Ref<Rule>> nestedRules) noexcept
      // The selectors are resolved when a rule set is built from the sheet.
      : StyleRule(krys::move(properties), {}), _nestedRules(krys::move(nestedRules)),
        _originalSelectorList(krys::move(selectors))
  {
    SetType(RuleType::StyleWithNesting);
  }

  StyleRuleWithNesting::StyleRuleWithNesting(StyleRule &&rule) noexcept
      : StyleRule(krys::move(rule)), _originalSelectorList(GetSelectorList())
  {
    SetType(RuleType::StyleWithNesting);
  }

  StyleRuleWithNesting::StyleRuleWithNesting(const StyleRuleWithNesting &other) noexcept
      : StyleRule(other), _nestedRules(CopyRules(other._nestedRules)),
        _originalSelectorList(other._originalSelectorList)
  {
  }

  StyleRuleWithNesting::~StyleRuleWithNesting() noexcept = default;

  Ref<StyleRuleWithNesting> StyleRuleWithNesting::Create(Ref<StyleProperties> properties,
                                                         SelectorList selectors,
                                                         List<Ref<Rule>> nestedRules) noexcept
  {
    return AdoptRef(
      *new StyleRuleWithNesting(krys::move(properties), krys::move(selectors), krys::move(nestedRules)));
  }

  Ref<StyleRuleWithNesting> StyleRuleWithNesting::Create(StyleRule &&rule) noexcept
  {
    return AdoptRef(*new StyleRuleWithNesting(krys::move(rule)));
  }

  Ref<StyleRuleWithNesting> StyleRuleWithNesting::Copy() const noexcept
  {
    return AdoptRef(*new StyleRuleWithNesting(*this));
  }

  void StyleRuleWithNesting::WrapperAdoptOriginalSelectorList(SelectorList selectors) noexcept
  {
    _originalSelectorList = krys::move(selectors);
    InvalidateResolvedSelectorListRecursively();
  }

#pragma endregion

#pragma region NestedDeclarationsRule

  NestedDeclarationsRule::NestedDeclarationsRule(Ref<StyleProperties> properties) noexcept
      : StyleRule(krys::move(properties), {})
  {
    SetType(RuleType::NestedDeclarations);
  }

  NestedDeclarationsRule::NestedDeclarationsRule(const NestedDeclarationsRule &other) noexcept = default;

  Ref<NestedDeclarationsRule> NestedDeclarationsRule::Create(Ref<StyleProperties> properties) noexcept
  {
    return AdoptRef(*new NestedDeclarationsRule(krys::move(properties)));
  }

  Ref<NestedDeclarationsRule> NestedDeclarationsRule::Copy() const noexcept
  {
    return AdoptRef(*new NestedDeclarationsRule(*this));
  }

#pragma endregion

#pragma region FontFaceRule

  FontFaceRule::FontFaceRule(Ref<StyleProperties> properties) noexcept
      : Rule(RuleType::FontFace), _properties(krys::move(properties))
  {
  }

  FontFaceRule::FontFaceRule(const FontFaceRule &other) noexcept
      : Rule(other), _properties(other._properties->MutableCopy())
  {
  }

  FontFaceRule::~FontFaceRule() noexcept = default;

  Ref<FontFaceRule> FontFaceRule::Create(Ref<StyleProperties> properties) noexcept
  {
    return AdoptRef(*new FontFaceRule(krys::move(properties)));
  }

  Ref<FontFaceRule> FontFaceRule::Copy() const noexcept
  {
    return AdoptRef(*new FontFaceRule(*this));
  }

  MutableStyleProperties &FontFaceRule::MutableProperties() noexcept
  {
    return EnsureMutable(_properties);
  }

#pragma endregion

#pragma region FontPaletteValuesRule

  FontPaletteValuesRule::FontPaletteValuesRule(
    CSSOMStringAtom name, List<CSSOMStringAtom> fontFamilies, Maybe<FontPaletteIndex> basePalette,
    List<web::FontPaletteValues::OverriddenColor> overrideColors) noexcept
      : Rule(RuleType::FontPaletteValues), _name(krys::move(name)), _fontFamilies(krys::move(fontFamilies)),
        _fontPaletteValues(basePalette, krys::move(overrideColors))
  {
  }

  Ref<FontPaletteValuesRule>
    FontPaletteValuesRule::Create(CSSOMStringAtom name, List<CSSOMStringAtom> fontFamilies,
                                  Maybe<FontPaletteIndex> basePalette,
                                  List<web::FontPaletteValues::OverriddenColor> overrideColors) noexcept
  {
    return AdoptRef(*new FontPaletteValuesRule(krys::move(name), krys::move(fontFamilies), basePalette,
                                               krys::move(overrideColors)));
  }

  Ref<FontPaletteValuesRule> FontPaletteValuesRule::Copy() const noexcept
  {
    return AdoptRef(*new FontPaletteValuesRule(*this));
  }

#pragma endregion

#pragma region FontFeatureValuesBlockRule

  FontFeatureValuesBlockRule::FontFeatureValuesBlockRule(web::FontFeatureValuesType type,
                                                         List<FontFeatureValuesTag> tags) noexcept
      : Rule(RuleType::FontFeatureValuesBlock), _type(type), _tags(krys::move(tags))
  {
  }

  Ref<FontFeatureValuesBlockRule> FontFeatureValuesBlockRule::Create(web::FontFeatureValuesType type,
                                                                     List<FontFeatureValuesTag> tags) noexcept
  {
    return AdoptRef(*new FontFeatureValuesBlockRule(type, krys::move(tags)));
  }

  Ref<FontFeatureValuesBlockRule> FontFeatureValuesBlockRule::Copy() const noexcept
  {
    return AdoptRef(*new FontFeatureValuesBlockRule(*this));
  }

#pragma endregion

#pragma region FontFeatureValuesRule

  FontFeatureValuesRule::FontFeatureValuesRule(List<CSSOMStringAtom> fontFamilies,
                                               Ref<web::FontFeatureValues> value) noexcept
      : Rule(RuleType::FontFeatureValues), _fontFamilies(krys::move(fontFamilies)), _value(krys::move(value))
  {
  }

  Ref<FontFeatureValuesRule> FontFeatureValuesRule::Create(List<CSSOMStringAtom> fontFamilies,
                                                           Ref<web::FontFeatureValues> value) noexcept
  {
    return AdoptRef(*new FontFeatureValuesRule(krys::move(fontFamilies), krys::move(value)));
  }

  Ref<FontFeatureValuesRule> FontFeatureValuesRule::Copy() const noexcept
  {
    return AdoptRef(*new FontFeatureValuesRule(*this));
  }

#pragma endregion

#pragma region GroupRule

  GroupRule::GroupRule(RuleType type, List<Ref<Rule>> childRules) noexcept
      : Rule(type), _childRules(krys::move(childRules))
  {
  }

  GroupRule::GroupRule(const GroupRule &other) noexcept
      : Rule(other), _childRules(CopyRules(other._childRules))
  {
  }

  void GroupRule::WrapperInsertRule(uint32 index, Ref<Rule> rule) noexcept
  {
    _childRules.insert(_childRules.begin() + index, krys::move(rule));
  }

  void GroupRule::WrapperRemoveRule(uint32 index) noexcept
  {
    _childRules.erase(_childRules.begin() + index);
  }

#pragma endregion

#pragma region MediaRule

  MediaRule::MediaRule(mq::MediaQueryList mediaQueries, List<Ref<Rule>> childRules) noexcept
      : GroupRule(RuleType::Media, krys::move(childRules)), _mediaQueries(krys::move(mediaQueries))
  {
  }

  MediaRule::MediaRule(const MediaRule &other) noexcept = default;

  Ref<MediaRule> MediaRule::Create(mq::MediaQueryList mediaQueries, List<Ref<Rule>> childRules) noexcept
  {
    return AdoptRef(*new MediaRule(krys::move(mediaQueries), krys::move(childRules)));
  }

  Ref<MediaRule> MediaRule::Copy() const noexcept
  {
    return AdoptRef(*new MediaRule(*this));
  }

#pragma endregion

#pragma region SupportsRule

  SupportsRule::SupportsRule(CSSOMString conditionText, bool conditionIsSupported,
                             List<Ref<Rule>> rules) noexcept
      : GroupRule(RuleType::Supports, krys::move(rules)), _conditionText(krys::move(conditionText)),
        _conditionIsSupported(conditionIsSupported)
  {
  }

  Ref<SupportsRule> SupportsRule::Create(CSSOMString conditionText, bool conditionIsSupported,
                                         List<Ref<Rule>> rules) noexcept
  {
    return AdoptRef(*new SupportsRule(krys::move(conditionText), conditionIsSupported, krys::move(rules)));
  }

  Ref<SupportsRule> SupportsRule::Copy() const noexcept
  {
    return AdoptRef(*new SupportsRule(*this));
  }

#pragma endregion

#pragma region LayerRule

  LayerRule::LayerRule(List<CascadeLayerName> nameList) noexcept
      : GroupRule(RuleType::LayerStatement, {}), _nameVariant(krys::move(nameList))
  {
  }

  LayerRule::LayerRule(CascadeLayerName name, List<Ref<Rule>> rules) noexcept
      : GroupRule(RuleType::LayerBlock, krys::move(rules)), _nameVariant(krys::move(name))
  {
  }

  Ref<LayerRule> LayerRule::CreateStatement(List<CascadeLayerName> nameList) noexcept
  {
    return AdoptRef(*new LayerRule(krys::move(nameList)));
  }

  Ref<LayerRule> LayerRule::CreateBlock(CascadeLayerName name, List<Ref<Rule>> rules) noexcept
  {
    return AdoptRef(*new LayerRule(krys::move(name), krys::move(rules)));
  }

  Ref<LayerRule> LayerRule::Copy() const noexcept
  {
    return AdoptRef(*new LayerRule(*this));
  }

#pragma endregion

#pragma region ContainerRule

  ContainerRule::ContainerRule(cq::ContainerQueryList queries, List<Ref<Rule>> rules) noexcept
      : GroupRule(RuleType::Container, krys::move(rules)), _containerQueries(krys::move(queries))
  {
    krys_debug_assert(!_containerQueries.empty());
  }

  Ref<ContainerRule> ContainerRule::Create(cq::ContainerQueryList queries, List<Ref<Rule>> rules) noexcept
  {
    return AdoptRef(*new ContainerRule(krys::move(queries), krys::move(rules)));
  }

  Ref<ContainerRule> ContainerRule::Copy() const noexcept
  {
    return AdoptRef(*new ContainerRule(*this));
  }

#pragma endregion

#pragma region PropertyRule

  PropertyRule::PropertyRule(Descriptor descriptor) noexcept
      : Rule(RuleType::Property), _descriptor(krys::move(descriptor))
  {
  }

  Ref<PropertyRule> PropertyRule::Create(Descriptor descriptor) noexcept
  {
    return AdoptRef(*new PropertyRule(krys::move(descriptor)));
  }

  Ref<PropertyRule> PropertyRule::Copy() const noexcept
  {
    return AdoptRef(*new PropertyRule(*this));
  }

#pragma endregion

#pragma region ScopeRule

  ScopeRule::ScopeRule(SelectorList scopeStart, SelectorList scopeEnd, List<Ref<Rule>> rules) noexcept
      : GroupRule(RuleType::Scope, krys::move(rules)), _originalScopeStart(krys::move(scopeStart)),
        _originalScopeEnd(krys::move(scopeEnd))
  {
  }

  ScopeRule::ScopeRule(const ScopeRule &other) noexcept = default;

  ScopeRule::~ScopeRule() noexcept = default;

  Ref<ScopeRule> ScopeRule::Create(SelectorList scopeStart, SelectorList scopeEnd,
                                   List<Ref<Rule>> rules) noexcept
  {
    return AdoptRef(*new ScopeRule(krys::move(scopeStart), krys::move(scopeEnd), krys::move(rules)));
  }

  Ref<ScopeRule> ScopeRule::Copy() const noexcept
  {
    return AdoptRef(*new ScopeRule(*this));
  }

  WeakPtr<const StyleSheetContents> ScopeRule::GetStyleSheetContents() const noexcept
  {
    return _styleSheetOwner;
  }

  void ScopeRule::SetStyleSheetContents(const StyleSheetContents &sheet) noexcept
  {
    _styleSheetOwner = CreateWeakPtr(&sheet);
  }

#pragma endregion

#pragma region StartingStyleRule

  StartingStyleRule::StartingStyleRule(List<Ref<Rule>> rules) noexcept
      : GroupRule(RuleType::StartingStyle, krys::move(rules))
  {
  }

  Ref<StartingStyleRule> StartingStyleRule::Create(List<Ref<Rule>> rules) noexcept
  {
    return AdoptRef(*new StartingStyleRule(krys::move(rules)));
  }

  Ref<StartingStyleRule> StartingStyleRule::Copy() const noexcept
  {
    return AdoptRef(*new StartingStyleRule(*this));
  }

#pragma endregion

#pragma region CharsetRule

  CharsetRule::CharsetRule() noexcept : Rule(RuleType::Charset)
  {
  }

  Ref<CharsetRule> CharsetRule::Create() noexcept
  {
    return AdoptRef(*new CharsetRule);
  }

  Ref<CharsetRule> CharsetRule::Copy() const noexcept
  {
    return AdoptRef(*new CharsetRule(*this));
  }

#pragma endregion

#pragma region NamespaceRule

  NamespaceRule::NamespaceRule(CSSOMStringAtom prefix, CSSOMStringAtom uri) noexcept
      : Rule(RuleType::Namespace), _prefix(krys::move(prefix)), _uri(krys::move(uri))
  {
  }

  Ref<NamespaceRule> NamespaceRule::Create(CSSOMStringAtom prefix, CSSOMStringAtom uri) noexcept
  {
    return AdoptRef(*new NamespaceRule(krys::move(prefix), krys::move(uri)));
  }

  Ref<NamespaceRule> NamespaceRule::Copy() const noexcept
  {
    return AdoptRef(*new NamespaceRule(*this));
  }

#pragma endregion
}
