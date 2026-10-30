#include "Krystal.Booey/Style/RuleSetBuilder.hpp"
#include "Krystal.Booey/CSS/Counter/CounterStyleRegistry.hpp"
#include "Krystal.Booey/CSS/CSSFontSelector.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryEvaluator.hpp"
#include "Krystal.Booey/CSS/Rules/CounterStyleRule.hpp"
#include "Krystal.Booey/CSS/Rules/ImportRule.hpp"
#include "Krystal.Booey/CSS/Rules/KeyframesRule.hpp"
#include "Krystal.Booey/CSS/Rules/PositionTryRule.hpp"
#include "Krystal.Booey/CSS/Rules/ViewTransitionRule.hpp"
#include "Krystal.Booey/CSS/Selectors/MutableSelector.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParser.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Style/CustomPropertyRegistry.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include <algorithm>
#include <random>
#include <utility>

namespace krys::boo::style
{
  namespace
  {
    /// @brief A random number, to give anonymous cascade layers names no author layer can have.
    KRYS_NODISCARD uint64 RandomNumber() noexcept
    {
      static std::mt19937_64 engine {std::random_device {}()};
      return engine();
    }
  }

  RuleSetBuilder::RuleSetBuilder(RuleSet &ruleSet, const css::mq::MediaQueryEvaluator &evaluator,
                                 Resolver *resolver, ShrinkToFit shrinkToFit) noexcept
      : _ruleSet(ShareRefPtr(&ruleSet)), _mediaQueryCollector({.Evaluator = evaluator}), _resolver(resolver),
        _shrinkToFit(shrinkToFit)
  {
  }

  RuleSetBuilder::RuleSetBuilder(const css::mq::MediaQueryEvaluator &evaluator) noexcept
      : _mediaQueryCollector({.Evaluator = evaluator, .CollectDynamic = true})
  {
  }

  RuleSetBuilder::~RuleSetBuilder() noexcept
  {
    if (!_ruleSet)
    {
      return;
    }

    UpdateCascadeLayerPriorities();
    UpdateDynamicMediaQueries();
    AddMutatingRulesToResolver();

    if (_shrinkToFit == ShrinkToFit::Enable)
    {
      _ruleSet->ShrinkToFit();
    }
  }

  void RuleSetBuilder::AddRulesFromSheet(const css::StyleSheetContents &sheet,
                                         const css::mq::MediaQueryList &sheetQuery) noexcept
  {
    auto canUseDynamicMediaQueryEvaluation = [&]
    {
      if (!_resolver)
      {
        return false;
      }

      RuleSetBuilder dynamicEvaluationScanner(_mediaQueryCollector.Evaluator);
      if (dynamicEvaluationScanner._mediaQueryCollector.PushAndEvaluate(sheetQuery))
      {
        dynamicEvaluationScanner.AddRulesFromSheetContents(sheet);
      }
      dynamicEvaluationScanner._mediaQueryCollector.Pop(sheetQuery);

      return !dynamicEvaluationScanner._requiresStaticMediaQueryEvaluation;
    };

    _mediaQueryCollector.CollectDynamic = canUseDynamicMediaQueryEvaluation();

    if (_mediaQueryCollector.PushAndEvaluate(sheetQuery))
    {
      AddRulesFromSheetContents(sheet);
    }
    _mediaQueryCollector.Pop(sheetQuery);
  }

  void RuleSetBuilder::AddChildRules(const List<Ref<css::Rule>> &rules) noexcept
  {
    for (auto &rule : rules)
    {
      if (_requiresStaticMediaQueryEvaluation)
      {
        return;
      }
      AddChildRule(rule);
    }
  }

  void RuleSetBuilder::AddChildRule(Ref<css::Rule> rule) noexcept
  {
    switch (rule->Type())
    {
      case css::RuleType::StyleWithNesting:
        if (_ruleSet)
        {
          AddStyleRule(static_cast<css::StyleRuleWithNesting &>(*rule));
        }
        return;

      case css::RuleType::Style:
        if (_ruleSet)
        {
          AddStyleRule(static_cast<const css::StyleRule &>(*rule));
        }
        return;

      case css::RuleType::NestedDeclarations:
        if (_ruleSet)
        {
          AddStyleRule(static_cast<css::NestedDeclarationsRule &>(*rule));
        }
        return;

      case css::RuleType::Scope:
      {
        auto &scopeRule = static_cast<css::ScopeRule &>(*rule);
        auto previousScopeIdentifier = _currentScopeIdentifier;
        if (_ruleSet)
        {
          _ruleSet->_scopeRules.push_back(
            {.ScopeRule = ShareRef<const css::ScopeRule>(scopeRule), .Parent = previousScopeIdentifier});
          _currentScopeIdentifier = static_cast<RuleSet::ScopeRuleIdentifier>(_ruleSet->_scopeRules.size());
        }

        // https://drafts.csswg.org/css-nesting/#nesting-at-scope
        // For the purposes of the style rules in its body and its own <scope-end> selector,
        // the @scope rule is treated as an ancestor style rule, matching the elements matched by its
        // <scope-start> selector.
        if (!scopeRule.OriginalScopeStart().IsEmpty() && scopeRule.ScopeStart().IsEmpty())
        {
          const css::SelectorList *parentResolvedSelectorList = nullptr;
          if (!_selectorListStack.empty())
          {
            parentResolvedSelectorList = _selectorListStack.back();
          }
          scopeRule.SetScopeStart(css::SelectorParser::ResolveNestingParent(scopeRule.OriginalScopeStart(),
                                                                            parentResolvedSelectorList));
        }
        if (!scopeRule.OriginalScopeEnd().IsEmpty() && scopeRule.ScopeEnd().IsEmpty())
        {
          scopeRule.SetScopeEnd(
            css::SelectorParser::ResolveNestingParent(scopeRule.OriginalScopeEnd(), &scopeRule.ScopeStart()));
        }

        const auto &scopeStart = scopeRule.ScopeStart();
        // If <scope-start> is empty, it doesn't create a nesting context (the nesting selector might
        // eventually be replaced by :scope)
        if (!scopeStart.IsEmpty())
        {
          _selectorListStack.push_back(&scopeStart);
        }
        _ancestorStack.push_back(css::NestedParserContextType::Scope);
        AddChildRules(scopeRule.ChildRules());
        _ancestorStack.pop_back();
        if (!scopeStart.IsEmpty())
        {
          _selectorListStack.pop_back();
        }

        if (_ruleSet)
        {
          _currentScopeIdentifier = previousScopeIdentifier;
        }
        return;
      }

      case css::RuleType::Page:
        if (_ruleSet)
        {
          _ruleSet->AddPageRule(static_cast<css::PageRule &>(*rule));
        }
        return;

      case css::RuleType::Media:
      {
        auto &mediaRule = static_cast<css::MediaRule &>(*rule);
        if (_mediaQueryCollector.PushAndEvaluate(mediaRule.MediaQueries()))
        {
          AddChildRules(mediaRule.ChildRules());
        }
        _mediaQueryCollector.Pop(mediaRule.MediaQueries());
        return;
      }

      case css::RuleType::Container:
      {
        auto &containerRule = static_cast<css::ContainerRule &>(*rule);
        auto previousContainerQueryIdentifier = _currentContainerQueryIdentifier;
        if (_ruleSet)
        {
          _ruleSet->_containerQueries.push_back(
            {.ContainerRule = ShareRef<const css::ContainerRule>(containerRule),
             .Parent = previousContainerQueryIdentifier});
          _currentContainerQueryIdentifier =
            static_cast<RuleSet::ContainerQueryIdentifier>(_ruleSet->_containerQueries.size());
        }
        AddChildRules(containerRule.ChildRules());
        if (_ruleSet)
        {
          _currentContainerQueryIdentifier = previousContainerQueryIdentifier;
        }
        return;
      }

      case css::RuleType::LayerBlock:
      case css::RuleType::LayerStatement:
      {
        DisallowDynamicMediaQueryEvaluationIfNeeded();

        auto &layerRule = static_cast<css::LayerRule &>(*rule);
        if (layerRule.IsStatement())
        {
          // Statement syntax just registers the layers.
          RegisterLayers(layerRule.NameList());
          return;
        }
        // Block syntax.
        PushCascadeLayer(layerRule.Name());
        AddChildRules(layerRule.ChildRules());
        PopCascadeLayer(layerRule.Name());
        return;
      }

      case css::RuleType::StartingStyle:
      {
        auto previousUsedRuleTypes =
          std::exchange(_usedRuleTypes, _usedRuleTypes | UsedRuleType::StartingStyle);
        AddChildRules(static_cast<css::StartingStyleRule &>(*rule).ChildRules());
        _usedRuleTypes = previousUsedRuleTypes;
        return;
      }

      case css::RuleType::CounterStyle:
      case css::RuleType::FontFace:
      case css::RuleType::FontPaletteValues:
      case css::RuleType::FontFeatureValues:
      case css::RuleType::Keyframes:
      case css::RuleType::Property:
      case css::RuleType::ViewTransition:
      case css::RuleType::PositionTry:
        DisallowDynamicMediaQueryEvaluationIfNeeded();
        if (_resolver)
        {
          _collectedResolverMutatingRules.push_back(
            {.Rule = krys::move(rule), .LayerIdentifier = _currentCascadeLayerIdentifier});
        }
        return;

      case css::RuleType::Supports:
      {
        auto &supportsRule = static_cast<css::SupportsRule &>(*rule);
        if (supportsRule.ConditionIsSupported())
        {
          AddChildRules(supportsRule.ChildRules());
        }
        return;
      }

      case css::RuleType::Import:
      case css::RuleType::Margin:
      case css::RuleType::Namespace:
      case css::RuleType::FontFeatureValuesBlock:
      case css::RuleType::Function:
      case css::RuleType::FunctionDeclarations:   return;

      case css::RuleType::Charset:
      case css::RuleType::Keyframe:               krys_debug_assert(false); return;
    }
  }

  void RuleSetBuilder::AddRulesFromSheetContents(const css::StyleSheetContents &sheet) noexcept
  {
    for (auto &rule : sheet.LayerRulesBeforeImportRules())
    {
      RegisterLayers(rule->NameList());
    }

    for (auto &rule : sheet.ImportRules())
    {
      if (!rule->StyleSheet())
      {
        continue;
      }

      if (!rule->SupportsMatches())
      {
        continue;
      }

      if (_mediaQueryCollector.PushAndEvaluate(rule->MediaQueries()))
      {
        auto &cascadeLayerName = rule->GetCascadeLayerName();
        if (cascadeLayerName)
        {
          DisallowDynamicMediaQueryEvaluationIfNeeded();
          PushCascadeLayer(*cascadeLayerName);
        }

        AddRulesFromSheetContents(*rule->StyleSheet());

        if (cascadeLayerName)
        {
          PopCascadeLayer(*cascadeLayerName);
        }
      }
      _mediaQueryCollector.Pop(rule->MediaQueries());
    }

    AddChildRules(sheet.ChildRules());
  }

  void RuleSetBuilder::ResolveSelectorListWithNesting(css::StyleRuleWithNesting &rule) noexcept
  {
    const css::SelectorList *parentResolvedSelectorList = nullptr;
    bool parentIsScopeRule = false;
    if (!_selectorListStack.empty())
    {
      parentResolvedSelectorList = _selectorListStack.back();
      parentIsScopeRule = _ancestorStack.back() == css::NestedParserContextType::Scope;
    }

    auto resolvedSelectorList = css::SelectorParser::ResolveNestingParent(
      rule.OriginalSelectorList(), parentResolvedSelectorList, parentIsScopeRule);
    rule.WrapperAdoptSelectorList(krys::move(resolvedSelectorList));
  }

  void RuleSetBuilder::AddStyleRuleWithSelectorList(const css::SelectorList &selectorList,
                                                    const css::StyleRule &rule) noexcept
  {
    // Empty selector list are only valid in forgiving selector list inside some functional pseudo-class.
    // It should not happen here.
    krys_debug_assert(!selectorList.IsEmpty());

    uint32 selectorListIndex = 0;
    for (Maybe<size_t> selectorIndex = 0; selectorIndex;
         selectorIndex = selectorList.IndexOfNextSelectorAfter(*selectorIndex))
    {
      RuleData ruleData(rule, static_cast<uint32>(*selectorIndex), selectorListIndex, _ruleSet->RuleCount(),
                        _usedRuleTypes);
      _mediaQueryCollector.AddRuleIfNeeded(ruleData);
      _ruleSet->AddRule(krys::move(ruleData), _currentCascadeLayerIdentifier,
                        _currentContainerQueryIdentifier, _currentScopeIdentifier,
                        &_featureCollectionContext);
      ++selectorListIndex;
    }
  }

  void RuleSetBuilder::AddStyleRule(css::StyleRuleWithNesting &rule) noexcept
  {
    krys_debug_assert(!rule.OriginalSelectorList().IsEmpty());

    if (rule.GetSelectorList().IsEmpty())
    {
      ResolveSelectorListWithNesting(rule);
    }

    const auto &selectorList = rule.GetSelectorList();
    AddStyleRuleWithSelectorList(selectorList, rule);

    // Process nested rules
    _selectorListStack.push_back(&selectorList);
    _ancestorStack.push_back(css::NestedParserContextType::Style);
    for (auto &nestedRule : rule.NestedRules())
    {
      AddChildRule(nestedRule);
    }
    _ancestorStack.pop_back();
    _selectorListStack.pop_back();
  }

  void RuleSetBuilder::AddStyleRule(const css::StyleRule &rule) noexcept
  {
    AddStyleRuleWithSelectorList(rule.GetSelectorList(), rule);
  }

  void RuleSetBuilder::AddStyleRule(css::NestedDeclarationsRule &rule) noexcept
  {
    auto whereScopeSelector = []
    {
      auto scopeSelector = CreateUnique<css::MutableSelector>();
      scopeSelector->SetMatch(css::SelectorMatch::PseudoClass);
      scopeSelector->SetPseudoClass(css::SelectorPseudoClass::Scope);

      css::MutableSelectorList scopeList;
      scopeList.push_back(krys::move(scopeSelector));

      auto whereSelector = CreateUnique<css::MutableSelector>();
      whereSelector->SetMatch(css::SelectorMatch::PseudoClass);
      whereSelector->SetPseudoClass(css::SelectorPseudoClass::Where);
      whereSelector->SetSubSelectors(CreateUnique<css::SelectorList>(krys::move(scopeList)));

      css::MutableSelectorList whereList;
      whereList.push_back(krys::move(whereSelector));
      return whereList;
    };

    auto selectorList = [&]
    {
      krys_debug_assert(!_ancestorStack.empty());

      if (_ancestorStack.back() == css::NestedParserContextType::Style)
      {
        krys_debug_assert(!_selectorListStack.empty());
        return css::SelectorList(*_selectorListStack.back());
      }

      if (_ancestorStack.back() == css::NestedParserContextType::Scope)
      {
        return css::SelectorList(whereScopeSelector());
      }

      krys_debug_assert(false);
      return css::SelectorList();
    };

    if (rule.GetSelectorList().IsEmpty())
    {
      rule.WrapperAdoptSelectorList(selectorList());
    }

    AddStyleRuleWithSelectorList(rule.GetSelectorList(), rule);
  }

  void RuleSetBuilder::DisallowDynamicMediaQueryEvaluationIfNeeded() noexcept
  {
    bool isScanningForDynamicEvaluation = !_ruleSet;
    if (isScanningForDynamicEvaluation && !_mediaQueryCollector.DynamicContextStack.empty())
    {
      _requiresStaticMediaQueryEvaluation = true;
    }
  }

  void RuleSetBuilder::RegisterLayers(const List<css::CascadeLayerName> &names) noexcept
  {
    for (auto &name : names)
    {
      PushCascadeLayer(name);
      PopCascadeLayer(name);
    }
  }

  void RuleSetBuilder::PushCascadeLayer(const css::CascadeLayerName &name) noexcept
  {
    if (!_ruleSet)
    {
      return;
    }

    if (_cascadeLayerIdentifierMap.empty() && !_ruleSet->_cascadeLayers.empty())
    {
      // For incremental build, reconstruct the name->identifier map.
      RuleSet::CascadeLayerIdentifier identifier = 0;
      for (auto &layer : _ruleSet->_cascadeLayers)
      {
        _cascadeLayerIdentifierMap.try_emplace(layer.ResolvedName, ++identifier);
      }
    }

    auto nameResolvingAnonymous = [&]
    {
      if (name.empty())
      {
        // Make unique name for an anonymous layer.
        StringBuilder anonymousName;
        anonymousName.Append(u8"anon_", RandomNumber());
        return css::CascadeLayerName {anonymousName.ToAtomString()};
      }
      return name;
    };

    // For hierarchical names we register the containing layers individually first.
    for (auto &nameSegment : nameResolvingAnonymous())
    {
      _resolvedCascadeLayerName.push_back(nameSegment);
      auto it = _cascadeLayerIdentifierMap.find(_resolvedCascadeLayerName);
      if (it == _cascadeLayerIdentifierMap.end())
      {
        // Previously unseen layer.
        _ruleSet->_cascadeLayers.push_back(
          {.ResolvedName = _resolvedCascadeLayerName, .ParentIdentifier = _currentCascadeLayerIdentifier});
        it = _cascadeLayerIdentifierMap
               .emplace(_resolvedCascadeLayerName,
                        static_cast<RuleSet::CascadeLayerIdentifier>(_ruleSet->_cascadeLayers.size()))
               .first;
      }
      _currentCascadeLayerIdentifier = it->second;
    }
  }

  void RuleSetBuilder::PopCascadeLayer(const css::CascadeLayerName &name) noexcept
  {
    if (!_ruleSet)
    {
      return;
    }

    for (auto size = name.empty() ? 1 : name.size(); size--;)
    {
      _resolvedCascadeLayerName.pop_back();
      _currentCascadeLayerIdentifier =
        _ruleSet->CascadeLayerForIdentifier(_currentCascadeLayerIdentifier).ParentIdentifier;
    }
  }

  void RuleSetBuilder::UpdateCascadeLayerPriorities() noexcept
  {
    if (_cascadeLayerIdentifierMap.empty())
    {
      return;
    }

    auto compare = [&](auto a, auto b)
    {
      while (true)
      {
        // Identifiers are in parse order.
        auto aParent = _ruleSet->CascadeLayerForIdentifier(a).ParentIdentifier;
        auto bParent = _ruleSet->CascadeLayerForIdentifier(b).ParentIdentifier;

        // For sibling layers, the later layer in parse order has a higher priority.
        if (aParent == bParent)
        {
          return a < b;
        }

        // For nested layers, the parent layer has a higher priority.
        if (aParent == b)
        {
          return true;
        }
        if (a == bParent)
        {
          return false;
        }

        // Traverse to parent. Parent layer identifiers are always lower.
        if (aParent > bParent)
        {
          a = aParent;
        }
        else
        {
          b = bParent;
        }
      }
    };

    auto layerCount = _ruleSet->_cascadeLayers.size();

    List<RuleSet::CascadeLayerIdentifier> layersInPriorityOrder(layerCount);
    for (size_t i = 0; i < layerCount; ++i)
    {
      layersInPriorityOrder[i] = static_cast<RuleSet::CascadeLayerIdentifier>(i + 1);
    }

    std::ranges::sort(layersInPriorityOrder, compare);

    // Priorities matter only relative to each other, so assign them enforcing these constraints:
    // - Layers must get a priority greater than RuleSet::CascadeLayerPriorityForPresentationalHints.
    // - Layers must get a priority smaller than RuleSet::CascadeLayerPriorityForUnlayered.
    // - A layer must get at least the same priority as the previous one.
    // - A layer should get more priority than the previous one, but this may be impossible if there are too
    // many
    //   layers. In that case, the last layers will get the maximum priority for layers,
    //   RuleSet::CascadeLayerPriorityForUnlayered - 1.
    for (size_t i = 0; i < layerCount; ++i)
    {
      auto priority = std::min<size_t>(i + RuleSet::CascadeLayerPriorityForPresentationalHints + 1,
                                       RuleSet::CascadeLayerPriorityForUnlayered - 1);
      _ruleSet->CascadeLayerForIdentifier(layersInPriorityOrder[i]).Priority =
        static_cast<CascadeLayerPriority>(priority);
    }
  }

  void RuleSetBuilder::AddMutatingRulesToResolver() noexcept
  {
    if (!_resolver)
    {
      return;
    }

    auto compareLayers = [&](const auto &a, const auto &b)
    {
      auto aPriority = _ruleSet->CascadeLayerPriorityForIdentifier(a.LayerIdentifier);
      auto bPriority = _ruleSet->CascadeLayerPriorityForIdentifier(b.LayerIdentifier);
      return aPriority < bPriority;
    };

    // The order may change so we need to reprocess resolver mutating rules from earlier stylesheets.
    auto rulesToAdd = std::exchange(_ruleSet->_resolverMutatingRulesInLayers, {});
    for (auto &rule : _collectedResolverMutatingRules)
    {
      rulesToAdd.push_back(krys::move(rule));
    }
    _collectedResolverMutatingRules.clear();

    if (!_cascadeLayerIdentifierMap.empty())
    {
      std::ranges::stable_sort(rulesToAdd, compareLayers);
    }

    for (auto &collectedRule : rulesToAdd)
    {
      if (collectedRule.LayerIdentifier)
      {
        _ruleSet->_resolverMutatingRulesInLayers.push_back(collectedRule);
      }

      auto &rule = *collectedRule.Rule;
      if (auto *styleRuleFontFace = DynamicDowncast<css::FontFaceRule>(rule))
      {
        _resolver->Document().GetFontSelector().AddFontFaceRule(*styleRuleFontFace, false);
        _resolver->InvalidateMatchedDeclarationsCache();
        continue;
      }
      if (auto *styleRuleFontPaletteValues = DynamicDowncast<css::FontPaletteValuesRule>(rule))
      {
        _resolver->Document().GetFontSelector().AddFontPaletteValuesRule(*styleRuleFontPaletteValues);
        _resolver->InvalidateMatchedDeclarationsCache();
        continue;
      }
      if (auto *styleRuleFontFeatureValues = DynamicDowncast<css::FontFeatureValuesRule>(rule))
      {
        _resolver->Document().GetFontSelector().AddFontFeatureValuesRule(*styleRuleFontFeatureValues);
        _resolver->InvalidateMatchedDeclarationsCache();
        continue;
      }

      if (auto *styleRuleKeyframes = DynamicDowncast<css::KeyframesRule>(rule))
      {
        _resolver->AddKeyframeStyle(ShareRef(*styleRuleKeyframes));
        continue;
      }

      if (auto *styleRuleCounterStyle = DynamicDowncast<css::CounterStyleRule>(rule))
      {
        if (_resolver->GetScopeType() == Resolver::ScopeType::ShadowTree)
        {
          continue;
        }
        auto &registry = _resolver->Document().StyleScope().GetCounterStyleRegistry();
        registry.AddCounterStyle(styleRuleCounterStyle->Descriptors());
        continue;
      }

      if (auto *styleRuleProperty = DynamicDowncast<css::PropertyRule>(rule))
      {
        // "A @property is invalid if it occurs in a stylesheet inside of a shadow tree, and must be ignored."
        // https://drafts.css-houdini.org/css-properties-values-api/#at-property-rule
        if (_resolver->GetScopeType() == Resolver::ScopeType::ShadowTree)
        {
          continue;
        }
        auto &registry = _resolver->Document().StyleScope().GetCustomPropertyRegistry();
        registry.RegisterFromStylesheet(styleRuleProperty->GetDescriptor());
        continue;
      }

      if (auto *styleRuleViewTransition = DynamicDowncast<css::ViewTransitionRule>(rule))
      {
        _ruleSet->SetViewTransitionRule(*styleRuleViewTransition);
      }

      if (auto *positionTryRule = DynamicDowncast<css::PositionTryRule>(rule))
      {
        // "If multiple @position-try rules are declared with the same name, the last one in document order
        // wins." https://drafts.csswg.org/css-anchor-position-1/#fallback-rule
        _ruleSet->_positionTryRules.insert_or_assign(
          positionTryRule->Name(), ShareRefPtr<const css::PositionTryRule>(positionTryRule));
      }
    }
  }

  void RuleSetBuilder::UpdateDynamicMediaQueries() noexcept
  {
    if (HasFlag(_mediaQueryCollector.AllDynamicDependencies, css::mq::MediaQueryDynamicDependency::Viewport))
    {
      _ruleSet->_hasViewportDependentMediaQueries = true;
    }

    if (!_mediaQueryCollector.DynamicMediaQueryRules.empty())
    {
      auto firstNewIndex = _ruleSet->_dynamicMediaQueryRules.size();
      for (auto &rules : _mediaQueryCollector.DynamicMediaQueryRules)
      {
        _ruleSet->_dynamicMediaQueryRules.push_back(krys::move(rules));
      }
      _mediaQueryCollector.DynamicMediaQueryRules.clear();

      // Set the initial values.
      (void)_ruleSet->EvaluateDynamicMediaQueryRules(_mediaQueryCollector.Evaluator, firstNewIndex);
    }
  }

  bool
    RuleSetBuilder::MediaQueryCollector::PushAndEvaluate(const css::mq::MediaQueryList &mediaQueries) noexcept
  {
    if (mediaQueries.empty())
    {
      return true;
    }

    auto dynamicDependencies = Evaluator.CollectDynamicDependencies(mediaQueries);
    AllDynamicDependencies |= dynamicDependencies;

    if (dynamicDependencies != css::mq::MediaQueryDynamicDependency::None)
    {
      DynamicContextStack.push_back({.Queries = mediaQueries});
      if (CollectDynamic)
      {
        return true;
      }
    }
    return Evaluator.Evaluate(mediaQueries);
  }

  void RuleSetBuilder::MediaQueryCollector::Pop(const css::mq::MediaQueryList &mediaQueries) noexcept
  {
    if (DynamicContextStack.empty() || &DynamicContextStack.back().Queries != &mediaQueries)
    {
      return;
    }

    if (!DynamicContextStack.back().AffectedRulePositions.empty() || !CollectDynamic)
    {
      RuleSet::DynamicMediaQueryRules rules;
      for (auto &context : DynamicContextStack)
      {
        rules.MediaQueries.push_back(context.Queries);
      }

      if (CollectDynamic)
      {
        rules.AffectedRulePositions = DynamicContextStack.back().AffectedRulePositions;
        rules.AffectedRules = DynamicContextStack.back().AffectedRulesInOrder;
      }
      else
      {
        rules.RequiresFullReset = true;
      }

      DynamicMediaQueryRules.push_back(krys::move(rules));
    }

    DynamicContextStack.pop_back();
  }

  void RuleSetBuilder::MediaQueryCollector::AddRuleIfNeeded(const RuleData &ruleData) noexcept
  {
    if (DynamicContextStack.empty())
    {
      return;
    }

    auto &context = DynamicContextStack.back();
    context.AffectedRulePositions.push_back(ruleData.Position());
    if (context.AffectedRules.insert(&ruleData.StyleRule()).second)
    {
      context.AffectedRulesInOrder.push_back(ShareRef(ruleData.StyleRule()));
    }
  }
}
