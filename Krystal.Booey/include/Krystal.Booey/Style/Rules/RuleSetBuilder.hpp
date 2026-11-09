/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2003-2021 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include "Krystal.Booey/CSS/Parser/ParserEnum.hpp"
#include "Krystal.Booey/CSS/Queries/GenericMediaQueryTypes.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Booey/Style/Rules/RuleSet.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::css
{
  class StyleSheetContents;
}

namespace krys::boo::style
{
  class Resolver;

  /// @brief Adds the style rules of style sheets to a rule set, evaluating media queries, resolving nesting
  /// and assigning cascade layers, container queries and scopes. Finishes the rule set when destroyed.
  class RuleSetBuilder
  {
    KRYS_NON_COPY_MOVABLE(RuleSetBuilder);

  public:
    enum class ShrinkToFit : bool
    {
      Enable,
      Disable
    };

  private:
    struct CascadeLayerNameHash
    {
      KRYS_NODISCARD size_t operator()(const css::CascadeLayerName &name) const noexcept
      {
        size_t seed = name.size();
        for (auto &segment : name)
        {
          seed ^= std::hash<css::CSSOMStringAtom> {}(segment) + 0x9e'37'79'b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
      }
    };

    struct MediaQueryCollector
    {
      struct DynamicContext
      {
        const css::mq::MediaQueryList &Queries;
        List<size_t> AffectedRulePositions {};
        HashSet<const css::StyleRule *> AffectedRules {};
        List<Ref<const css::StyleRule>> AffectedRulesInOrder {};
      };

      const css::mq::MediaQueryEvaluator &Evaluator;
      bool CollectDynamic {false};
      List<DynamicContext> DynamicContextStack {};
      List<RuleSet::DynamicMediaQueryRules> DynamicMediaQueryRules {};
      css::mq::MediaQueryDynamicDependency AllDynamicDependencies {};

      KRYS_NODISCARD bool PushAndEvaluate(const css::mq::MediaQueryList &mediaQueries) noexcept;
      void Pop(const css::mq::MediaQueryList &mediaQueries) noexcept;
      void AddRuleIfNeeded(const RuleData &ruleData) noexcept;
    };

    RefPtr<RuleSet> _ruleSet;
    MediaQueryCollector _mediaQueryCollector;
    Resolver *_resolver {nullptr};
    const ShrinkToFit _shrinkToFit {ShrinkToFit::Enable};

    css::CascadeLayerName _resolvedCascadeLayerName;
    HashMap<css::CascadeLayerName, RuleSet::CascadeLayerIdentifier, CascadeLayerNameHash>
      _cascadeLayerIdentifierMap;
    RuleSet::CascadeLayerIdentifier _currentCascadeLayerIdentifier {0};
    List<const css::SelectorList *> _selectorListStack;
    List<css::NestedParserContextType> _ancestorStack;

    RuleSet::ContainerQueryIdentifier _currentContainerQueryIdentifier {0};
    RuleSet::ScopeRuleIdentifier _currentScopeIdentifier {0};

    EnumSet<UsedRuleType> _usedRuleTypes {};

    List<RuleSet::ResolverMutatingRule> _collectedResolverMutatingRules;
    bool _requiresStaticMediaQueryEvaluation {false};

    RuleFeatureSet::CollectionContext _featureCollectionContext;

    explicit RuleSetBuilder(const css::mq::MediaQueryEvaluator &evaluator) noexcept;

  public:
    RuleSetBuilder(RuleSet &ruleSet, const css::mq::MediaQueryEvaluator &evaluator,
                   Resolver *resolver = nullptr, ShrinkToFit shrinkToFit = ShrinkToFit::Enable) noexcept;
    ~RuleSetBuilder() noexcept;

    void AddRulesFromSheet(const css::StyleSheetContents &sheet,
                           const css::mq::MediaQueryList &sheetQuery = {}) noexcept;

    void AddStyleRule(const css::StyleRule &rule) noexcept;

  private:
    void AddStyleRule(css::StyleRuleWithNesting &rule) noexcept;
    void AddStyleRule(css::NestedDeclarationsRule &rule) noexcept;
    void AddRulesFromSheetContents(const css::StyleSheetContents &sheet) noexcept;
    void AddChildRules(const List<Ref<css::Rule>> &rules) noexcept;
    void AddChildRule(Ref<css::Rule> rule) noexcept;
    void DisallowDynamicMediaQueryEvaluationIfNeeded() noexcept;

    void AddStyleRuleWithSelectorList(const css::SelectorList &selectorList,
                                      const css::StyleRule &rule) noexcept;

    void RegisterLayers(const List<css::CascadeLayerName> &names) noexcept;
    void PushCascadeLayer(const css::CascadeLayerName &name) noexcept;
    void PopCascadeLayer(const css::CascadeLayerName &name) noexcept;
    void UpdateCascadeLayerPriorities() noexcept;

    void AddMutatingRulesToResolver() noexcept;
    void UpdateDynamicMediaQueries() noexcept;

    void ResolveSelectorListWithNesting(css::StyleRuleWithNesting &rule) noexcept;
  };
}
