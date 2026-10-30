#pragma once

#include "Krystal.Booey/CSS/Selectors/SelectorChecker.hpp"
#include "Krystal.Booey/Style/MatchResult.hpp"
#include "Krystal.Booey/Style/PropertyAllowlist.hpp"
#include "Krystal.Booey/Style/PseudoElementRequest.hpp"
#include "Krystal.Booey/Style/Relations.hpp"
#include "Krystal.Booey/Style/RuleSet.hpp"
#include "Krystal.Booey/Style/ScopeOrdinal.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include <limits>
#include <utility>

namespace krys::boo::css
{
  class KeyframeRule;
  class SelectorList;
  class StyleRule;
}

namespace krys::boo::css::mq
{
  class MediaQueryEvaluator;
}

namespace krys::boo::dom
{
  class ContainerNode;
  class Element;
}

namespace krys::boo::style
{
  class ScopeRuleSets;
  struct MatchRequest;
  struct SelectorMatchingState;
  enum class DeclarationOrigin : uint8;

  struct MatchedRule
  {
    const style::RuleData *RuleData {nullptr};
    uint32 Specificity {0};
    uint32 ScopingRootDistance {0};
    ScopeOrdinal StyleScopeOrdinal;
    style::CascadeLayerPriority CascadeLayerPriority;
  };

  /// @brief Collects the rules that match an element (or one of its pseudo-elements) from the rule sets that
  /// can style it, and turns them into declaration blocks per origin, in cascade order.
  class ElementRuleCollector
  {
    KRYS_NON_COPYABLE(ElementRuleCollector);

    struct ScopingRootWithDistance
    {
      RefPtr<const dom::ContainerNode> ScopingRoot;
      uint32 Distance {std::numeric_limits<uint32>::max()};
      bool MatchesVisited {false};
    };

  private:
    const Ref<const dom::Element> _element;
    Ref<const RuleSet> _authorStyle;
    RefPtr<const RuleSet> _userStyle;
    RefPtr<const RuleSet> _userAgentMediaQueryStyle;
    RefPtr<const RuleSet> _dynamicViewTransitionsStyle;
    SelectorMatchingState *_selectorMatchingState;

    bool _shouldIncludeEmptyRules {false};
    bool _isPrintStyle {false};
    Maybe<PseudoElementRequest> _pseudoElementRequest {};
    const css::SelectorChecker::Mode _mode {css::SelectorChecker::Mode::ResolvingStyle};

    SmallList<MatchedRule, 64> _matchedRules;
    size_t _matchedRuleTransferIndex {0};

    // Output.
    List<RefPtr<const css::StyleRule>> _matchedRuleList;
    Ref<MatchResult> _result;
    Relations _styleRelations;
    EnumSet<PseudoElementType> _matchedPseudoElements;

  public:
    ElementRuleCollector(
      const dom::Element &element, const ScopeRuleSets &ruleSets,
      SelectorMatchingState *selectorMatchingState,
      css::SelectorChecker::Mode mode = css::SelectorChecker::Mode::ResolvingStyle) noexcept;
    ElementRuleCollector(
      const dom::Element &element, const RuleSet &authorStyle, SelectorMatchingState *selectorMatchingState,
      css::SelectorChecker::Mode mode = css::SelectorChecker::Mode::ResolvingStyle) noexcept;
    ~ElementRuleCollector() noexcept;

    void SetIncludeEmptyRules(bool value) noexcept
    {
      _shouldIncludeEmptyRules = value;
    }

    void MatchAllRules(bool matchAuthorAndUserStyles, bool includeSMILProperties) noexcept;
    void MatchUARules() noexcept;
    void MatchAuthorRules() noexcept;
    void MatchUserRules() noexcept;

    KRYS_NODISCARD bool MatchesAnyAuthorRules() noexcept;

    void SetPseudoElementRequest(const Maybe<PseudoElementRequest> &request) noexcept
    {
      _pseudoElementRequest = request;
    }

    void SetMedium(const css::mq::MediaQueryEvaluator &medium) noexcept;

    KRYS_NODISCARD const MatchResult &GetMatchResult() const noexcept;
    KRYS_NODISCARD Ref<MatchResult> ReleaseMatchResult() noexcept;

    KRYS_NODISCARD const List<RefPtr<const css::StyleRule>> &MatchedRuleList() const noexcept;

    void ClearMatchedRules() noexcept;

    KRYS_NODISCARD EnumSet<PseudoElementType> MatchedPseudoElements() const noexcept
    {
      return _matchedPseudoElements;
    }

    KRYS_NODISCARD const Relations &StyleRelations() const noexcept
    {
      return _styleRelations;
    }

    void AddAuthorKeyframeRules(const css::KeyframeRule &keyframe) noexcept;

  private:
    void AddElementStyleProperties(const css::StyleProperties *propertySet, CascadeLayerPriority priority,
                                   IsCacheable isCacheable = IsCacheable::Yes,
                                   FromStyleAttribute fromStyleAttribute = FromStyleAttribute::No) noexcept;

    void MatchUARules(const RuleSet &rules) noexcept;

    void AddElementInlineStyleProperties(bool includeSMILProperties) noexcept;

    void MatchUserAgentPartRules(DeclarationOrigin origin) noexcept;
    void MatchHostPseudoClassRules(DeclarationOrigin origin) noexcept;
    void MatchSlottedPseudoElementRules(DeclarationOrigin origin) noexcept;
    void MatchPartPseudoElementRules(DeclarationOrigin origin) noexcept;
    void MatchPartPseudoElementRulesForScope(const dom::Element &partMatchingElement,
                                             DeclarationOrigin origin) noexcept;

    void CollectMatchingRules(DeclarationOrigin origin) noexcept;
    void CollectMatchingRules(const MatchRequest &matchRequest) noexcept;
    void CollectMatchingUserAgentPartRules(const MatchRequest &matchRequest) noexcept;
    void CollectMatchingRulesForList(const RuleSet::RuleDataVector *rules,
                                     const MatchRequest &matchRequest) noexcept;
    KRYS_NODISCARD bool IsFirstMatchModeAndHasMatchedAnyRules() const noexcept;

    KRYS_NODISCARD bool RuleMatches(const RuleData &ruleData, uint32 &specificity,
                                    ScopeOrdinal styleScopeOrdinal,
                                    const Maybe<ScopingRootWithDistance> &scopingRoot = {}) noexcept;
    KRYS_NODISCARD bool ContainerQueriesMatch(const RuleData &ruleData,
                                              const MatchRequest &matchRequest) const noexcept;
    KRYS_NODISCARD std::pair<bool, Maybe<List<ScopingRootWithDistance>>>
      ScopeRulesMatch(const RuleData &ruleData, const MatchRequest &matchRequest) const noexcept;

    void SortMatchedRules() noexcept;

    KRYS_NODISCARD List<MatchedProperties> &DeclarationsForOrigin(DeclarationOrigin origin) noexcept;
    void SortAndTransferMatchedRules(DeclarationOrigin origin) noexcept;
    void TransferMatchedRules(DeclarationOrigin origin, Maybe<ScopeOrdinal> fromScope = {}) noexcept;

    void AddMatchedRule(const RuleData &ruleData, uint32 specificity, uint32 scopingRootDistance,
                        const MatchRequest &matchRequest) noexcept;
    void AddMatchedProperties(MatchedProperties &&matchedProperties, DeclarationOrigin origin) noexcept;

    KRYS_NODISCARD const dom::Element &Element() const noexcept
    {
      return *_element;
    }
  };
}
