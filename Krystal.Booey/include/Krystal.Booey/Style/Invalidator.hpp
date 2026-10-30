#pragma once

#include "Krystal.Booey/Style/RuleFeature.hpp"
#include "Krystal.Booey/Style/RuleSet.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class StyleSheetContents;
}

namespace krys::boo::css::mq
{
  class MediaQueryEvaluator;
}

namespace krys::boo::dom
{
  class Document;
  class Element;
  class ShadowRoot;
}

namespace krys::boo::style
{
  class Scope;
  struct InvalidationRuleSet;
  struct SelectorMatchingState;

  /// @brief Invalidates the style of the elements that a set of rules may match, after a change to the rules
  /// (new style sheets, media query changes) or to the elements (class, id, attribute, pseudo-class and child
  /// changes).
  class Invalidator
  {
    KRYS_NON_COPY_MOVABLE(Invalidator);

  public:
    using MatchElementRuleSets = HashMap<MatchElement, InvalidationRuleSetVector>;

  private:
    struct RuleInformation
    {
      bool HasSlottedPseudoElementRules {false};
      bool HasHostPseudoClassRules {false};
      bool HasHostPseudoClassRulesMatchingInShadowTree {false};
      bool HasUserAgentPartRules {false};
      bool HasCuePseudoElementRules {false};
      bool HasPartPseudoElementRules {false};
    };

    enum class CheckDescendants : bool
    {
      No,
      Yes
    };

    RefPtr<RuleSet> _ownedRuleSet;
    const InvalidationRuleSetVector _ruleSets;

    RuleInformation _ruleInformation;

    bool _dirtiesAllStyle {false};

  public:
    Invalidator(const List<Ref<css::StyleSheetContents>> &sheets,
                const css::mq::MediaQueryEvaluator &mediaQueryEvaluator) noexcept;
    explicit Invalidator(const InvalidationRuleSetVector &ruleSets) noexcept;

    ~Invalidator() noexcept;

    KRYS_NODISCARD bool DirtiesAllStyle() const noexcept
    {
      return _dirtiesAllStyle;
    }

    void InvalidateStyle(dom::Document &document) noexcept;
    void InvalidateStyle(Scope &scope) noexcept;
    void InvalidateStyle(dom::ShadowRoot &shadowRoot) noexcept;
    void InvalidateStyle(dom::Element &element) noexcept;

    static void InvalidateShadowParts(dom::ShadowRoot &shadowRoot) noexcept;

    static void AddToMatchElementRuleSets(MatchElementRuleSets &matchElementRuleSets,
                                          const InvalidationRuleSet &invalidationRuleSet) noexcept;
    static void
      AddToMatchElementRuleSetsRespectingNegation(MatchElementRuleSets &matchElementRuleSets,
                                                  const InvalidationRuleSet &invalidationRuleSet) noexcept;
    static void InvalidateWithMatchElementRuleSets(dom::Element &element,
                                                   const MatchElementRuleSets &matchElementRuleSets) noexcept;
    static void InvalidateAllStyle(Scope &scope) noexcept;
    static void InvalidateHostAndSlottedStyleIfNeeded(dom::ShadowRoot &shadowRoot) noexcept;
    static void InvalidateWithScopeBreakingHasPseudoClassRuleSet(dom::Element &element,
                                                                 const RuleSet *ruleSet) noexcept;

  private:
    CheckDescendants InvalidateIfNeeded(dom::Element &element,
                                        SelectorMatchingState *selectorMatchingState) noexcept;
    void InvalidateStyleForTree(dom::Element &root, SelectorMatchingState *selectorMatchingState) noexcept;
    void InvalidateStyleForDescendants(dom::Element &root,
                                       SelectorMatchingState *selectorMatchingState) noexcept;
    void InvalidateInShadowTreeIfNeeded(dom::Element &element) noexcept;
    void InvalidateUserAgentParts(dom::ShadowRoot &shadowRoot) noexcept;
    void InvalidateStyleWithMatchElement(dom::Element &element, MatchElement matchElement) noexcept;

    KRYS_NODISCARD RuleInformation CollectRuleInformation() const noexcept;
  };
}
