#pragma once

#include "Krystal.Booey/CSS/Queries/ContainerQuery.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Booey/CSS/Rules/PositionTryRule.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/Rules/ViewTransitionRule.hpp"
#include "Krystal.Booey/Style/RuleData.hpp"
#include "Krystal.Booey/Style/RuleFeature.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <algorithm>
#include <limits>

namespace krys::boo::css::mq
{
  class MediaQueryEvaluator;
}

namespace krys::boo::style
{
  class RuleSet;

  using CascadeLayerPriority = uint16;

  struct RuleSetAndNegation
  {
    RefPtr<const style::RuleSet> RuleSet;
    style::IsNegation IsNegation {style::IsNegation::No};
  };

  using InvalidationRuleSetVector = SmallList<RuleSetAndNegation, 1>;

  struct DynamicMediaQueryEvaluationChanges
  {
    enum class Type : uint8
    {
      InvalidateStyle,
      ResetStyle
    };

    Type ChangeType;
    InvalidationRuleSetVector InvalidationRuleSets {};

    void Append(DynamicMediaQueryEvaluationChanges &&other) noexcept
    {
      ChangeType = std::max(ChangeType, other.ChangeType);
      if (ChangeType == Type::ResetStyle)
      {
        InvalidationRuleSets.clear();
      }
      else
      {
        for (auto &ruleSet : other.InvalidationRuleSets)
        {
          InvalidationRuleSets.push_back(krys::move(ruleSet));
        }
      }
    };
  };

  /// @brief The style rules of a set of style sheets, bucketed by the rightmost compound selector of each
  /// complex selector so matching only looks at rules that can match an element.
  class RuleSet : public RefCounted<RuleSet>
  {
    friend class RuleSetBuilder;
    KRYS_NON_COPYABLE(RuleSet);

  public:
    using RuleDataVector = SmallList<RuleData, 1>;
    using AtomRuleMap = HashMap<css::CSSOMStringAtom, UniquePtr<RuleDataVector>>;

    static constexpr auto CascadeLayerPriorityForPresentationalHints =
      std::numeric_limits<CascadeLayerPriority>::min();
    static constexpr auto CascadeLayerPriorityForUnlayered = std::numeric_limits<CascadeLayerPriority>::max();

  private:
    using CascadeLayerIdentifier = uint32;
    using ContainerQueryIdentifier = uint32;
    using ScopeRuleIdentifier = uint32;

    struct ResolverMutatingRule
    {
      Ref<css::Rule> Rule;
      CascadeLayerIdentifier LayerIdentifier;
    };

    struct CollectedMediaQueryChanges
    {
      bool RequiredFullReset {false};
      List<size_t> ChangedQueryIndexes {};
      List<List<Ref<const css::StyleRule>> *> AffectedRules {};
    };

    struct CascadeLayer
    {
      css::CascadeLayerName ResolvedName;
      CascadeLayerIdentifier ParentIdentifier;
      CascadeLayerPriority Priority {0};
    };

    struct ScopeAndParent
    {
      Ref<const css::ScopeRule> ScopeRule;
      ScopeRuleIdentifier Parent;
    };

    struct ContainerQueryAndParent
    {
      Ref<const css::ContainerRule> ContainerRule;
      ContainerQueryIdentifier Parent;
    };

    struct DynamicMediaQueryRules
    {
      List<css::mq::MediaQueryList> MediaQueries;
      List<size_t> AffectedRulePositions;
      List<Ref<const css::StyleRule>> AffectedRules;
      bool RequiresFullReset {false};
      bool Result {true};

      void ShrinkToFit() noexcept
      {
        MediaQueries.shrink_to_fit();
        AffectedRulePositions.shrink_to_fit();
        AffectedRules.shrink_to_fit();
      }
    };

    struct IndexListHash
    {
      KRYS_NODISCARD size_t operator()(const List<size_t> &indexes) const noexcept
      {
        size_t seed = indexes.size();
        for (auto index : indexes)
        {
          seed = Hash::Combine(seed, index);
        }
        return seed;
      }
    };

    AtomRuleMap _idRules;
    AtomRuleMap _classRules;
    AtomRuleMap _attributeLocalNameRules;
    AtomRuleMap _attributeLowercaseLocalNameRules;
    AtomRuleMap _tagLocalNameRules;
    AtomRuleMap _tagLowercaseLocalNameRules;
    AtomRuleMap _userAgentPartRules;
    AtomRuleMap _namedPseudoElementRules;
    RuleDataVector _linkPseudoClassRules;
    RuleDataVector _cuePseudoRules;
    RuleDataVector _hostPseudoClassRules;
    RuleDataVector _slottedPseudoElementRules;
    RuleDataVector _partPseudoElementRules;
    RuleDataVector _focusPseudoClassRules;
    RuleDataVector _focusVisiblePseudoClassRules;
    RuleDataVector _rootElementRules;
    RuleDataVector _universalRules;
    List<css::PageRule *> _pageRules;
    RefPtr<css::ViewTransitionRule> _viewTransitionRule;
    RuleFeatureSet _features;
    List<DynamicMediaQueryRules> _dynamicMediaQueryRules;
    HashMap<List<size_t>, Ref<const RuleSet>, IndexListHash> _mediaQueryInvalidationRuleSetCache;
    uint32 _ruleCount {0};

    List<CascadeLayer> _cascadeLayers;
    // This is a side vector to hold layer identifiers without bloating RuleData.
    List<CascadeLayerIdentifier> _cascadeLayerIdentifierForRulePosition;

    List<ResolverMutatingRule> _resolverMutatingRulesInLayers;

    List<ContainerQueryAndParent> _containerQueries;
    List<ContainerQueryIdentifier> _containerQueryIdentifierForRulePosition;

    // @scope
    List<ScopeAndParent> _scopeRules;
    List<ScopeRuleIdentifier> _scopeRuleIdentifierForRulePosition;

    // @position-try
    HashMap<css::CSSOMStringAtom, RefPtr<const css::PositionTryRule>> _positionTryRules;

    bool _hasHostPseudoClassRulesMatchingInShadowTree {false};
    bool _hasViewportDependentMediaQueries {false};
    bool _hasHostOrScopePseudoClassRulesInUniversalBucket {false};

    RuleSet() noexcept;

  public:
    ~RuleSet() noexcept;

    KRYS_NODISCARD static Ref<RuleSet> Create() noexcept;

    void AddRule(const css::StyleRule &rule, uint32 selectorIndex, uint32 selectorListIndex) noexcept;
    void AddPageRule(css::PageRule &rule) noexcept;
    void SetViewTransitionRule(css::ViewTransitionRule &rule) noexcept;
    KRYS_NODISCARD RefPtr<css::ViewTransitionRule> GetViewTransitionRule() const noexcept;

    void AddToRuleSet(const css::CSSOMStringAtom &key, AtomRuleMap &map, const RuleData &ruleData) noexcept;
    void ShrinkToFit() noexcept;

    KRYS_NODISCARD bool HasViewportDependentMediaQueries() const noexcept
    {
      return _hasViewportDependentMediaQueries;
    }

    KRYS_NODISCARD Maybe<DynamicMediaQueryEvaluationChanges>
      EvaluateDynamicMediaQueryRules(const css::mq::MediaQueryEvaluator &evaluator) noexcept;

    KRYS_NODISCARD const RuleFeatureSet &Features() const noexcept
    {
      return _features;
    }

    KRYS_NODISCARD const RuleDataVector *IdRules(const css::CSSOMStringAtom &key) const noexcept
    {
      return Find(_idRules, key);
    }

    KRYS_NODISCARD const RuleDataVector *ClassRules(const css::CSSOMStringAtom &key) const noexcept
    {
      return Find(_classRules, key);
    }

    KRYS_NODISCARD const RuleDataVector *AttributeRules(const css::CSSOMStringAtom &key,
                                                        bool isHTMLName) const noexcept;
    KRYS_NODISCARD const RuleDataVector *TagRules(const css::CSSOMStringAtom &key,
                                                  bool isHTMLName) const noexcept;

    KRYS_NODISCARD const RuleDataVector *UserAgentPartRules(const css::CSSOMStringAtom &key) const noexcept
    {
      return Find(_userAgentPartRules, key);
    }

    KRYS_NODISCARD const RuleDataVector *LinkPseudoClassRules() const noexcept
    {
      return &_linkPseudoClassRules;
    }

    KRYS_NODISCARD const RuleDataVector *
      NamedPseudoElementRules(const css::CSSOMStringAtom &key) const noexcept
    {
      return Find(_namedPseudoElementRules, key);
    }

    KRYS_NODISCARD const RuleDataVector &CuePseudoRules() const noexcept
    {
      return _cuePseudoRules;
    }

    KRYS_NODISCARD const RuleDataVector &HostPseudoClassRules() const noexcept
    {
      return _hostPseudoClassRules;
    }

    KRYS_NODISCARD const RuleDataVector &SlottedPseudoElementRules() const noexcept
    {
      return _slottedPseudoElementRules;
    }

    KRYS_NODISCARD const RuleDataVector &PartPseudoElementRules() const noexcept
    {
      return _partPseudoElementRules;
    }

    KRYS_NODISCARD const RuleDataVector *FocusPseudoClassRules() const noexcept
    {
      return &_focusPseudoClassRules;
    }

    KRYS_NODISCARD const RuleDataVector *FocusVisiblePseudoClassRules() const noexcept
    {
      return &_focusVisiblePseudoClassRules;
    }

    KRYS_NODISCARD const RuleDataVector *RootElementRules() const noexcept
    {
      return &_rootElementRules;
    }

    KRYS_NODISCARD const RuleDataVector *UniversalRules() const noexcept
    {
      return &_universalRules;
    }

    KRYS_NODISCARD const List<css::PageRule *> &PageRules() const noexcept
    {
      return _pageRules;
    }

    KRYS_NODISCARD uint32 RuleCount() const noexcept
    {
      return _ruleCount;
    }

    KRYS_NODISCARD bool HasAttributeRules() const noexcept
    {
      return !_attributeLocalNameRules.empty();
    }

    KRYS_NODISCARD bool HasUserAgentPartRules() const noexcept
    {
      return !_userAgentPartRules.empty();
    }

    KRYS_NODISCARD bool HasHostPseudoClassRulesMatchingInShadowTree() const noexcept
    {
      return _hasHostPseudoClassRulesMatchingInShadowTree;
    }

    KRYS_NODISCARD bool HasHostOrScopePseudoClassRulesInUniversalBucket() const noexcept
    {
      return _hasHostOrScopePseudoClassRulesInUniversalBucket;
    }

    KRYS_NODISCARD CascadeLayerPriority CascadeLayerPriorityFor(const RuleData &ruleData) const noexcept;

    KRYS_NODISCARD bool HasContainerQueries() const noexcept
    {
      return !_containerQueries.empty();
    }

    KRYS_NODISCARD List<const css::cq::ContainerQuery *>
      ContainerQueriesFor(const RuleData &ruleData) const noexcept;
    KRYS_NODISCARD List<Ref<const css::ContainerRule>> ContainerQueryRules() const noexcept;

    KRYS_NODISCARD bool HasScopeRules() const noexcept
    {
      return !_scopeRules.empty();
    }

    KRYS_NODISCARD List<Ref<const css::ScopeRule>> ScopeRulesFor(const RuleData &ruleData) const noexcept;

    KRYS_NODISCARD RefPtr<const css::PositionTryRule>
      PositionTryRuleForName(const css::CSSOMStringAtom &name) const noexcept;

  private:
    void AddRule(RuleData &&ruleData, CascadeLayerIdentifier cascadeLayerIdentifier,
                 ContainerQueryIdentifier containerQueryIdentifier, ScopeRuleIdentifier scopeRuleIdentifier,
                 RuleFeatureSet::CollectionContext *featureCollectionContext) noexcept;

    KRYS_NODISCARD CollectedMediaQueryChanges EvaluateDynamicMediaQueryRules(
      const css::mq::MediaQueryEvaluator &evaluator, size_t startIndex) noexcept;

    template <typename TFunction>
    void TraverseRuleDatas(TFunction &&function) noexcept;
    template <typename TFunction>
    void TraverseRuleDatas(TFunction &&function) const noexcept;

    KRYS_NODISCARD CascadeLayer &CascadeLayerForIdentifier(CascadeLayerIdentifier identifier) noexcept
    {
      return _cascadeLayers[identifier - 1];
    }

    KRYS_NODISCARD const CascadeLayer &
      CascadeLayerForIdentifier(CascadeLayerIdentifier identifier) const noexcept
    {
      return _cascadeLayers[identifier - 1];
    }

    KRYS_NODISCARD CascadeLayerPriority
      CascadeLayerPriorityForIdentifier(CascadeLayerIdentifier identifier) const noexcept;

    KRYS_NODISCARD static const RuleDataVector *Find(const AtomRuleMap &map,
                                                     const css::CSSOMStringAtom &key) noexcept
    {
      auto it = map.find(key);
      return it == map.end() ? nullptr : it->second.get();
    }
  };

  inline const RuleSet::RuleDataVector *RuleSet::AttributeRules(const css::CSSOMStringAtom &key,
                                                                bool isHTMLName) const noexcept
  {
    auto &rules = isHTMLName ? _attributeLowercaseLocalNameRules : _attributeLocalNameRules;
    return Find(rules, key);
  }

  inline const RuleSet::RuleDataVector *RuleSet::TagRules(const css::CSSOMStringAtom &key,
                                                          bool isHTMLName) const noexcept
  {
    auto &rules = isHTMLName ? _tagLowercaseLocalNameRules : _tagLocalNameRules;
    return Find(rules, key);
  }

  inline CascadeLayerPriority
    RuleSet::CascadeLayerPriorityForIdentifier(CascadeLayerIdentifier identifier) const noexcept
  {
    if (!identifier)
    {
      return CascadeLayerPriorityForUnlayered;
    }
    return CascadeLayerForIdentifier(identifier).Priority;
  }

  inline CascadeLayerPriority RuleSet::CascadeLayerPriorityFor(const RuleData &ruleData) const noexcept
  {
    if (_cascadeLayerIdentifierForRulePosition.size() <= ruleData.Position())
    {
      return CascadeLayerPriorityForUnlayered;
    }
    auto identifier = _cascadeLayerIdentifierForRulePosition[ruleData.Position()];
    return CascadeLayerPriorityForIdentifier(identifier);
  }

  inline List<const css::cq::ContainerQuery *>
    RuleSet::ContainerQueriesFor(const RuleData &ruleData) const noexcept
  {
    if (_containerQueryIdentifierForRulePosition.size() <= ruleData.Position())
    {
      return {};
    }

    List<const css::cq::ContainerQuery *> queries;

    auto identifier = _containerQueryIdentifierForRulePosition[ruleData.Position()];
    while (identifier)
    {
      auto &query = _containerQueries[identifier - 1];
      queries.push_back(&query.ContainerRule->ContainerQuery());
      identifier = query.Parent;
    };

    return queries;
  }
}
