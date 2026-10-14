#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/Rules/CounterStyleRule.hpp"
#include "Krystal.Booey/CSS/Rules/FunctionRule.hpp"
#include "Krystal.Booey/CSS/Rules/ImportRule.hpp"
#include "Krystal.Booey/CSS/Rules/KeyframeRule.hpp"
#include "Krystal.Booey/CSS/Rules/KeyframesRule.hpp"
#include "Krystal.Booey/CSS/Rules/PositionTryRule.hpp"
#include "Krystal.Booey/CSS/Rules/ViewTransitionRule.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Core/Assert.hpp"
#include <functional>
#include <memory>

namespace krys::boo::css
{
  namespace
  {
    KRYS_NODISCARD MutableStyleProperties &EnsureMutable(Ref<StyleProperties> &properties) noexcept
    {
      if (!Is<MutableStyleProperties>(*properties))
      {
        properties = properties->MutableCopy();
      }
      return Downcast<MutableStyleProperties>(*properties);
    }
  }

#pragma region Rule

  template <typename Visitor>
  constexpr decltype(auto) Rule::VisitDerived(Visitor &&visitor) noexcept
  {
    switch (Type())
    {
      case RuleType::Style:
      {
        return std::invoke(std::forward<Visitor>(visitor), static_cast<StyleRule &>(*this));
      }
      case RuleType::StyleWithNesting:
      {
        return std::invoke(std::forward<Visitor>(visitor), static_cast<StyleRuleWithNesting &>(*this));
      }
      case RuleType::NestedDeclarations:
      {
        return std::invoke(std::forward<Visitor>(visitor), static_cast<NestedDeclarationsRule &>(*this));
      }
      case RuleType::Page:
      {
        return std::invoke(std::forward<Visitor>(visitor), static_cast<PageRule &>(*this));
      }
      case RuleType::FontFace:
      {
        return std::invoke(std::forward<Visitor>(visitor), static_cast<FontFaceRule &>(*this));
      }
      case RuleType::FontFeatureValues:
      {
        return std::invoke(std::forward<Visitor>(visitor), static_cast<FontFeatureValuesRule &>(*this));
      }
      case RuleType::FontFeatureValuesBlock:
      {
        return std::invoke(std::forward<Visitor>(visitor), static_cast<FontFeatureValuesBlockRule &>(*this));
      }
      case RuleType::FontPaletteValues:
      {
        return std::invoke(std::forward<Visitor>(visitor), static_cast<FontPaletteValuesRule &>(*this));
      }
      case RuleType::Media:
      {
        return std::invoke(std::forward<Visitor>(visitor), static_cast<MediaRule &>(*this));
      }
      case RuleType::Supports:
      {
        return std::invoke(std::forward<Visitor>(visitor), static_cast<SupportsRule &>(*this));
      }
      case RuleType::Import:
      {
        return std::invoke(std::forward<Visitor>(visitor), static_cast<ImportRule &>(*this));
      }
      case RuleType::Keyframes:
      {
        return std::invoke(std::forward<Visitor>(visitor), static_cast<KeyframesRule &>(*this));
      }
      case RuleType::Namespace:
      {
        return std::invoke(std::forward<Visitor>(visitor), static_cast<NamespaceRule &>(*this));
      }
      case RuleType::Keyframe:
      {
        return std::invoke(std::forward<Visitor>(visitor), static_cast<KeyframeRule &>(*this));
      }
      case RuleType::Charset:
      {
        return std::invoke(std::forward<Visitor>(visitor), static_cast<CharsetRule &>(*this));
      }
      case RuleType::CounterStyle:
      {
        return std::invoke(std::forward<Visitor>(visitor), static_cast<CounterStyleRule &>(*this));
      }
      case RuleType::LayerBlock:
      case RuleType::LayerStatement:
      {
        return std::invoke(std::forward<Visitor>(visitor), static_cast<LayerRule &>(*this));
      }
      case RuleType::Container:
      {
        return std::invoke(std::forward<Visitor>(visitor), static_cast<ContainerRule &>(*this));
      }
      case RuleType::Property:
      {
        return std::invoke(std::forward<Visitor>(visitor), static_cast<PropertyRule &>(*this));
      }
      case RuleType::Scope:
      {
        return std::invoke(std::forward<Visitor>(visitor), static_cast<ScopeRule &>(*this));
      }
      case RuleType::StartingStyle:
      {
        return std::invoke(std::forward<Visitor>(visitor), static_cast<StartingStyleRule &>(*this));
      }
      case RuleType::ViewTransition:
      {
        return std::invoke(std::forward<Visitor>(visitor), static_cast<ViewTransitionRule &>(*this));
      }
      case RuleType::PositionTry:
      {
        return std::invoke(std::forward<Visitor>(visitor), static_cast<PositionTryRule &>(*this));
      }
      case RuleType::Function:
      {
        return std::invoke(std::forward<Visitor>(visitor), static_cast<FunctionRule &>(*this));
      }
      case RuleType::FunctionDeclarations:
      {
        return std::invoke(std::forward<Visitor>(visitor), static_cast<FunctionDeclarationsRule &>(*this));
      }
      case RuleType::Margin:
      {
        break;
      }
    }
    krys_unreachable();
  }

  template <typename Visitor>
  constexpr decltype(auto) Rule::VisitDerived(Visitor &&visitor) const noexcept
  {
    return const_cast<Rule &>(*this).VisitDerived(
      [&](auto &value) { return std::invoke(std::forward<Visitor>(visitor), std::as_const(value)); });
  }

  void Rule::operator delete(Rule *rule, std::destroying_delete_t) noexcept
  {
    rule->VisitDerived(
      []<typename TRule>(TRule &rule)
      {
        std::destroy_at(&rule);
        ::operator delete(&rule);
      });
  }

  Ref<Rule> Rule::Copy() const noexcept
  {
    return VisitDerived(
      []<typename TRule>(const TRule &rule) -> Ref<Rule>
      {
        // Check at compile time for a mistake where this function would call itself, leading to infinite
        // recursion. We can do this with the types of pointers to member functions because they include the
        // type of the class.
        static_assert(!SameType<decltype(&TRule::Copy), decltype(&Rule::Copy)>);
        return rule.Copy();
      });
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
        else if constexpr (SameType<TRule, ScopeRule>)
        {
          rule.SetScopeStart({});
          rule.SetScopeEnd({});
          for (auto &child : rule.ChildRules())
          {
            child->InvalidateResolvedSelectorListRecursively();
          }
        }
        else if constexpr (std::is_base_of_v<GroupRule, TRule>)
        {
          for (auto &child : rule.ChildRules())
          {
            child->InvalidateResolvedSelectorListRecursively();
          }
        }
      });
  }

#pragma endregion

#pragma region StyleRule

  uint32 StyleRule::AverageSizeInBytes() noexcept
  {
    return static_cast<uint32>(sizeof(StyleRule) + sizeof(css::Selector)
                               + StyleProperties::AverageSizeInBytes() + sizeof(List<Ref<Rule>>));
  }

  StyleRule::StyleRule(Ref<StyleProperties> &&properties, bool hasDocumentSecurityOrigin,
                       SelectorList &&selectors) noexcept
      : Rule(RuleType::Style, hasDocumentSecurityOrigin), _properties(krys::move(properties)),
        _selectorList(krys::move(selectors))
  {
  }

  StyleRule::StyleRule(const StyleRule &other) noexcept
      : Rule(other), _isSplitRule(other._isSplitRule), _isLastRuleInSplitRule(other._isLastRuleInSplitRule),
        _properties(other.Properties().MutableCopy()), _selectorList(other._selectorList)
  {
  }

  StyleRule::~StyleRule() noexcept = default;

  Ref<StyleRule> StyleRule::Create(Ref<StyleProperties> &&properties, bool hasDocumentSecurityOrigin,
                                   SelectorList &&selectors) noexcept
  {
    return AdoptRef(*new StyleRule(krys::move(properties), hasDocumentSecurityOrigin, krys::move(selectors)));
  }

  Ref<StyleRule> StyleRule::Copy() const noexcept
  {
    return AdoptRef(*new StyleRule(*this));
  }

  Ref<const StyleProperties> StyleRule::ProtectedProperties() const noexcept
  {
    return _properties;
  }

  void StyleRule::SetProperties(Ref<StyleProperties> &&properties) noexcept
  {
    _properties = krys::move(properties);
  }

  MutableStyleProperties &StyleRule::MutableProperties() noexcept
  {
    return EnsureMutable(_properties);
  }

  void StyleRule::WrapperAdoptSelectorList(SelectorList &&selectors) noexcept
  {
    AdoptSelectorList(krys::move(selectors));
  }

  Ref<StyleRule> StyleRule::CreateForSplitting(const List<const css::Selector *> &selectors,
                                               Ref<StyleProperties> &&properties,
                                               bool hasDocumentSecurityOrigin) noexcept
  {
    krys_assert(!selectors.empty());
    auto selectorListArray = CreateUniqueArray<css::Selector>(selectors.size());
    for (size_t i = 0uz; i < selectors.size(); ++i)
    {
      new (NotNullTag {}, &selectorListArray[i]) css::Selector(*selectors[i]);
    }
    selectorListArray[selectors.size() - 1].SetLastInSelectorList();
    auto styleRule = StyleRule::Create(krys::move(properties), hasDocumentSecurityOrigin,
                                       SelectorList(krys::move(selectorListArray)));
    styleRule->MarkAsSplitRule();
    return styleRule;
  }

  List<Ref<StyleRule>>
    StyleRule::SplitIntoMultipleRulesWithMaximumSelectorComponentCount(uint32 maxCount) const noexcept
  {
    krys_debug_assert(GetSelectorList().ComponentCount() > maxCount);

    List<Ref<StyleRule>> rules;
    List<const css::Selector *> componentsSinceLastSplit;

    for (const auto &selector : GetSelectorList())
    {
      List<const css::Selector *> componentsInThisSelector;
      for (const auto *component = &selector; component;
           component = component->PrecedingComplexSelectorComponent())
      {
        componentsInThisSelector.push_back(component);
      }

      if (componentsInThisSelector.size() + componentsSinceLastSplit.size() > maxCount
          && !componentsSinceLastSplit.empty())
      {
        rules.push_back(
          CreateForSplitting(componentsSinceLastSplit,
                             Ref<StyleProperties>::WithRef(const_cast<StyleProperties &>(Properties())),
                             HasDocumentSecurityOrigin()));
        componentsSinceLastSplit.clear();
      }

      componentsSinceLastSplit.insert(componentsSinceLastSplit.end(), componentsInThisSelector.begin(),
                                      componentsInThisSelector.end());
    }

    if (!componentsSinceLastSplit.empty())
    {
      rules.push_back(CreateForSplitting(
        componentsSinceLastSplit, Ref<StyleProperties>::WithRef(const_cast<StyleProperties &>(Properties())),
        HasDocumentSecurityOrigin()));
    }

    if (!rules.empty())
    {
      rules.back()->MarkAsLastRuleInSplitRule();
    }

    return rules;
  }

#pragma endregion

#pragma region StyleRuleWithNesting

  StyleRuleWithNesting::StyleRuleWithNesting(Ref<StyleProperties> &&properties,
                                             bool hasDocumentSecurityOrigin, SelectorList &&selectors,
                                             List<Ref<Rule>> &&nestedRules) noexcept
      // Actual selectors will be resolved later, at RuleSetBuilder time.
      : StyleRule(krys::move(properties), hasDocumentSecurityOrigin, {}),
        _nestedRules(krys::move(nestedRules)), _originalSelectorList(krys::move(selectors))
  {
    SetType(RuleType::StyleWithNesting);
  }

  StyleRuleWithNesting::StyleRuleWithNesting(StyleRule &&rule) noexcept
      : StyleRule(krys::move(rule)), _originalSelectorList(GetSelectorList())
  {
    SetType(RuleType::StyleWithNesting);
  }

  StyleRuleWithNesting::StyleRuleWithNesting(const StyleRuleWithNesting &other) noexcept
      : StyleRule(other), _originalSelectorList(other._originalSelectorList)
  {
    _nestedRules.reserve(other._nestedRules.size());
    for (const auto &rule : other._nestedRules)
    {
      _nestedRules.push_back(rule->Copy());
    }
  }

  StyleRuleWithNesting::~StyleRuleWithNesting() noexcept = default;

  Ref<StyleRuleWithNesting> StyleRuleWithNesting::Create(Ref<StyleProperties> &&properties,
                                                         bool hasDocumentSecurityOrigin,
                                                         SelectorList &&selectors,
                                                         List<Ref<Rule>> &&nestedRules) noexcept
  {
    return AdoptRef(*new StyleRuleWithNesting(krys::move(properties), hasDocumentSecurityOrigin,
                                              krys::move(selectors), krys::move(nestedRules)));
  }

  Ref<StyleRuleWithNesting> StyleRuleWithNesting::Create(StyleRule &&rule) noexcept
  {
    return AdoptRef(*new StyleRuleWithNesting(krys::move(rule)));
  }

  Ref<StyleRuleWithNesting> StyleRuleWithNesting::Copy() const noexcept
  {
    return AdoptRef(*new StyleRuleWithNesting(*this));
  }

  void StyleRuleWithNesting::WrapperAdoptOriginalSelectorList(SelectorList &&selectors) noexcept
  {
    _originalSelectorList = krys::move(selectors);
    InvalidateResolvedSelectorListRecursively();
  }

#pragma endregion

#pragma region NestedDeclarationsRule

  NestedDeclarationsRule::NestedDeclarationsRule(Ref<StyleProperties> &&properties) noexcept
      : StyleRule(krys::move(properties), false, {})
  {
    SetType(RuleType::NestedDeclarations);
  }

  NestedDeclarationsRule::NestedDeclarationsRule(const NestedDeclarationsRule &other) noexcept = default;

  Ref<NestedDeclarationsRule> NestedDeclarationsRule::Create(Ref<StyleProperties> &&properties) noexcept
  {
    return AdoptRef(*new NestedDeclarationsRule(krys::move(properties)));
  }

  Ref<NestedDeclarationsRule> NestedDeclarationsRule::Copy() const noexcept
  {
    return AdoptRef(*new NestedDeclarationsRule(*this));
  }

#pragma endregion

#pragma region FontFaceRule

  FontFaceRule::FontFaceRule(Ref<StyleProperties> &&properties) noexcept
      : Rule(RuleType::FontFace), _properties(krys::move(properties))
  {
  }

  FontFaceRule::FontFaceRule(const FontFaceRule &other) noexcept
      : Rule(other), _properties(other._properties->MutableCopy())
  {
  }

  FontFaceRule::~FontFaceRule() noexcept = default;

  Ref<FontFaceRule> FontFaceRule::Create(Ref<StyleProperties> &&properties) noexcept
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
    const CSSOMStringAtom &name, List<CSSOMStringAtom> &&fontFamilies, Maybe<FontPaletteIndex> basePalette,
    List<boo::FontPaletteValues::OverriddenColor> &&overrideColors) noexcept
      : Rule(RuleType::FontPaletteValues), _name(name), _fontFamilies(krys::move(fontFamilies)),
        _fontPaletteValues(basePalette, krys::move(overrideColors))
  {
  }

  Ref<FontPaletteValuesRule>
    FontPaletteValuesRule::Create(const CSSOMStringAtom &name, List<CSSOMStringAtom> &&fontFamilies,
                                  Maybe<FontPaletteIndex> basePalette,
                                  List<boo::FontPaletteValues::OverriddenColor> &&overrideColors) noexcept
  {
    return AdoptRef(
      *new FontPaletteValuesRule(name, krys::move(fontFamilies), basePalette, krys::move(overrideColors)));
  }

  Ref<FontPaletteValuesRule> FontPaletteValuesRule::Copy() const noexcept
  {
    return AdoptRef(*new FontPaletteValuesRule(*this));
  }

#pragma endregion

#pragma region FontFeatureValuesBlockRule

  FontFeatureValuesBlockRule::FontFeatureValuesBlockRule(boo::FontFeatureValuesType type,
                                                         const List<FontFeatureValuesTag> &tags) noexcept
      : Rule(RuleType::FontFeatureValuesBlock), _type(type), _tags(tags)
  {
  }

  Ref<FontFeatureValuesBlockRule>
    FontFeatureValuesBlockRule::Create(boo::FontFeatureValuesType type,
                                       const List<FontFeatureValuesTag> &tags) noexcept
  {
    return AdoptRef(*new FontFeatureValuesBlockRule(type, tags));
  }

  Ref<FontFeatureValuesBlockRule> FontFeatureValuesBlockRule::Copy() const noexcept
  {
    return AdoptRef(*new FontFeatureValuesBlockRule(*this));
  }

#pragma endregion

#pragma region FontFeatureValuesRule

  FontFeatureValuesRule::FontFeatureValuesRule(const List<CSSOMStringAtom> &fontFamilies,
                                               Ref<boo::FontFeatureValues> &&value) noexcept
      : Rule(RuleType::FontFeatureValues), _fontFamilies(fontFamilies), _value(krys::move(value))
  {
  }

  Ref<FontFeatureValuesRule> FontFeatureValuesRule::Create(const List<CSSOMStringAtom> &fontFamilies,
                                                           Ref<boo::FontFeatureValues> &&values) noexcept
  {
    return AdoptRef(*new FontFeatureValuesRule(fontFamilies, krys::move(values)));
  }

  Ref<FontFeatureValuesRule> FontFeatureValuesRule::Copy() const noexcept
  {
    return AdoptRef(*new FontFeatureValuesRule(*this));
  }

#pragma endregion

#pragma region PageRule

  PageRule::PageRule(Ref<StyleProperties> &&properties, SelectorList &&selectors) noexcept
      : Rule(RuleType::Page), _properties(krys::move(properties)), _selectorList(krys::move(selectors))
  {
  }

  PageRule::PageRule(const PageRule &other) noexcept
      : Rule(other), _properties(other._properties->MutableCopy()), _selectorList(other._selectorList)
  {
  }

  PageRule::~PageRule() noexcept = default;

  Ref<PageRule> PageRule::Create(Ref<StyleProperties> &&properties, SelectorList &&selectors) noexcept
  {
    return AdoptRef(*new PageRule(krys::move(properties), krys::move(selectors)));
  }

  Ref<PageRule> PageRule::Copy() const noexcept
  {
    return AdoptRef(*new PageRule(*this));
  }

  MutableStyleProperties &PageRule::MutableProperties() noexcept
  {
    return EnsureMutable(_properties);
  }

#pragma endregion

#pragma region GroupRule

  GroupRule::GroupRule(RuleType type, List<Ref<Rule>> &&childRules) noexcept
      : Rule(type), _childRules(krys::move(childRules))
  {
  }

  GroupRule::GroupRule(const GroupRule &other) noexcept : Rule(other)
  {
    _childRules.reserve(other._childRules.size());
    for (const auto &rule : other._childRules)
    {
      _childRules.push_back(rule->Copy());
    }
  }

  void GroupRule::WrapperInsertRule(uint32 index, Ref<Rule> &&rule) noexcept
  {
    _childRules.insert(_childRules.begin() + index, krys::move(rule));
  }

  void GroupRule::WrapperRemoveRule(uint32 index) noexcept
  {
    _childRules.erase(_childRules.begin() + index);
  }

#pragma endregion

#pragma region MediaRule

  MediaRule::MediaRule(mq::MediaQueryList &&mediaQueries, List<Ref<Rule>> &&childRules) noexcept
      : GroupRule(RuleType::Media, krys::move(childRules)), _mediaQueries(krys::move(mediaQueries))
  {
  }

  MediaRule::MediaRule(const MediaRule &other) noexcept : GroupRule(other), _mediaQueries(other._mediaQueries)
  {
  }

  Ref<MediaRule> MediaRule::Create(mq::MediaQueryList &&mediaQueries, List<Ref<Rule>> &&childRules) noexcept
  {
    return AdoptRef(*new MediaRule(krys::move(mediaQueries), krys::move(childRules)));
  }

  Ref<MediaRule> MediaRule::Copy() const noexcept
  {
    return AdoptRef(*new MediaRule(*this));
  }

#pragma endregion

#pragma region SupportsRule

  SupportsRule::SupportsRule(const CSSOMString &conditionText, bool conditionIsSupported,
                             List<Ref<Rule>> &&rules) noexcept
      : GroupRule(RuleType::Supports, krys::move(rules)), _conditionText(conditionText),
        _conditionIsSupported(conditionIsSupported)
  {
  }

  Ref<SupportsRule> SupportsRule::Create(const CSSOMString &conditionText, bool conditionIsSupported,
                                         List<Ref<Rule>> &&rules) noexcept
  {
    return AdoptRef(*new SupportsRule(conditionText, conditionIsSupported, krys::move(rules)));
  }

  Ref<SupportsRule> SupportsRule::Copy() const noexcept
  {
    return AdoptRef(*new SupportsRule(*this));
  }

#pragma endregion

#pragma region LayerRule

  LayerRule::LayerRule(List<CascadeLayerName> &&nameList) noexcept
      : GroupRule(RuleType::LayerStatement, List<Ref<Rule>> {}), _nameVariant(krys::move(nameList))
  {
  }

  LayerRule::LayerRule(CascadeLayerName &&name, List<Ref<Rule>> &&rules) noexcept
      : GroupRule(RuleType::LayerBlock, krys::move(rules)), _nameVariant(krys::move(name))
  {
  }

  Ref<LayerRule> LayerRule::CreateStatement(List<CascadeLayerName> &&nameList) noexcept
  {
    return AdoptRef(*new LayerRule(krys::move(nameList)));
  }

  Ref<LayerRule> LayerRule::CreateBlock(CascadeLayerName &&name, List<Ref<Rule>> &&rules) noexcept
  {
    return AdoptRef(*new LayerRule(krys::move(name), krys::move(rules)));
  }

  Ref<LayerRule> LayerRule::Copy() const noexcept
  {
    return AdoptRef(*new LayerRule(*this));
  }

#pragma endregion

#pragma region ContainerRule

  ContainerRule::ContainerRule(cq::ContainerQuery &&query, List<Ref<Rule>> &&rules) noexcept
      : GroupRule(RuleType::Container, krys::move(rules)), _containerQuery(krys::move(query))
  {
  }

  Ref<ContainerRule> ContainerRule::Create(cq::ContainerQuery &&query, List<Ref<Rule>> &&rules) noexcept
  {
    return AdoptRef(*new ContainerRule(krys::move(query), krys::move(rules)));
  }

  Ref<ContainerRule> ContainerRule::Copy() const noexcept
  {
    return AdoptRef(*new ContainerRule(*this));
  }

#pragma endregion

#pragma region PropertyRule

  PropertyRule::PropertyRule(PropertyRule::Descriptor &&descriptor) noexcept
      : Rule(RuleType::Property), _descriptor(krys::move(descriptor))
  {
  }

  Ref<PropertyRule> PropertyRule::Create(PropertyRule::Descriptor &&descriptor) noexcept
  {
    return AdoptRef(*new PropertyRule(krys::move(descriptor)));
  }

  Ref<PropertyRule> PropertyRule::Copy() const noexcept
  {
    return AdoptRef(*new PropertyRule(*this));
  }

#pragma endregion

#pragma region ScopeRule

  ScopeRule::ScopeRule(SelectorList &&scopeStart, SelectorList &&scopeEnd, List<Ref<Rule>> &&rules) noexcept
      : GroupRule(RuleType::Scope, krys::move(rules)), _originalScopeStart(krys::move(scopeStart)),
        _originalScopeEnd(krys::move(scopeEnd))
  {
  }

  ScopeRule::ScopeRule(const ScopeRule &other) noexcept = default;

  ScopeRule::~ScopeRule() noexcept = default;

  Ref<ScopeRule> ScopeRule::Create(SelectorList &&scopeStart, SelectorList &&scopeEnd,
                                   List<Ref<Rule>> &&rules) noexcept
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
    _styleSheetOwner = WeakPtr<const StyleSheetContents>::WithRef(sheet);
  }

#pragma endregion

#pragma region StartingStyleRule

  StartingStyleRule::StartingStyleRule(List<Ref<Rule>> &&rules) noexcept
      : GroupRule(RuleType::StartingStyle, krys::move(rules))
  {
  }

  Ref<StartingStyleRule> StartingStyleRule::Create(List<Ref<Rule>> &&rules) noexcept
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

  NamespaceRule::NamespaceRule(const CSSOMStringAtom &prefix, const CSSOMStringAtom &uri) noexcept
      : Rule(RuleType::Namespace), _prefix(prefix), _uri(uri)
  {
  }

  Ref<NamespaceRule> NamespaceRule::Create(const CSSOMStringAtom &prefix, const CSSOMStringAtom &uri) noexcept
  {
    return AdoptRef(*new NamespaceRule(prefix, uri));
  }

  Ref<NamespaceRule> NamespaceRule::Copy() const noexcept
  {
    return AdoptRef(*new NamespaceRule(*this));
  }

#pragma endregion
}
