#pragma once

#include "Krystal.Booey/CSS/Queries/MediaQueryEvaluator.hpp"
#include "Krystal.Booey/CSS/Rules/KeyframesRule.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorEnums.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Style/MatchedDeclarationsCache.hpp"
#include "Krystal.Booey/Style/PropertyCascade.hpp"
#include "Krystal.Booey/Style/PseudoElementRequest.hpp"
#include "Krystal.Booey/Style/ScopeRuleSets.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class RenderStyle;
}

namespace krys::boo::css
{
  class CSSStyleSheet;
  class MutableStyleProperties;
  class StyleRule;
}

namespace krys::boo::dom
{
  class Document;
  class Element;
}

namespace krys::boo
{
  // MatchOnlyUserAgentRules is used in media queries, where relative units
  // are interpreted according to the document root element style, and styled only
  // from the User Agent Stylesheet rules.
  enum class RuleMatchingBehavior : uint8
  {
    MatchAllRules,
    MatchAllRulesExcludingSMIL,
    MatchOnlyUserAgentRules,
  };
}

namespace krys::boo
{
  class BlendingKeyframe;
  class BlendingKeyframes;
  class TimingFunction;
}

namespace krys::boo::style
{
  class BuilderState;
  struct BuilderContext;
  struct CachedMatchResult;
  struct ResolvedStyle;
  struct SelectorMatchingState;
  struct UnadjustedStyle;

  struct ResolutionContext
  {
    const RenderStyle *ParentStyle;
    const RenderStyle *ParentBoxStyle {nullptr};
    // This needs to be provided during style resolution when up-to-date document element style is not
    // available via DOM.
    const RenderStyle *DocumentElementStyle {nullptr};
    style::SelectorMatchingState *SelectorMatchingState {nullptr};
    // FIXME: `treeResolutionState` (the anchor-positioned states) comes with anchor positioning.

    bool IsSVGUseTreeRoot {false};
  };

  using KeyframesRuleMap = HashMap<css::CSSOMStringAtom, RefPtr<css::KeyframesRule>>;

  /// @brief Resolves the style of the elements of a document or of shadow trees from the rule sets of their
  /// style sheets.
  class Resolver : public RefCounted<Resolver>, public CanMakeWeakPtr<Resolver>
  {
    KRYS_NON_COPY_MOVABLE(Resolver);

  public:
    // Style resolvers are shared between shadow trees with identical styles. That's why we don't simply
    // provide a Style::Scope.
    enum class ScopeType : bool
    {
      Document,
      ShadowTree
    };

    // These methods will give back the set of rules that matched for a given element (or a pseudo-element).
    enum CSSRuleFilter : uint32
    {
      UAAndUserCSSRules = 1 << 1,
      AuthorCSSRules = 1 << 2,
      EmptyCSSRules = 1 << 3,
      AllButEmptyCSSRules = UAAndUserCSSRules | AuthorCSSRules,
      AllCSSRules = AllButEmptyCSSRules | EmptyCSSRules,
    };

  private:
    class State;

    WeakPtr<dom::Document> _document;
    const ScopeType _scopeType;

    ScopeRuleSets _ruleSets;

    KeyframesRuleMap _keyframesRuleMap;

    css::mq::MediaQueryEvaluator _mediaQueryEvaluator;
    UniquePtr<RenderStyle> _rootDefaultStyle;

    MatchedDeclarationsCache _matchedDeclarationsCache;

    bool _matchAuthorAndUserStyles {true};
    bool _isSharedBetweenShadowTrees {false};

    Resolver(dom::Document &document, ScopeType scopeType) noexcept;

  public:
    ~Resolver() noexcept;

    KRYS_NODISCARD static Ref<Resolver> Create(dom::Document &document, ScopeType scopeType) noexcept;

    KRYS_NODISCARD UnadjustedStyle
      UnadjustedStyleForElement(dom::Element &element, const ResolutionContext &context,
                                RuleMatchingBehavior = RuleMatchingBehavior::MatchAllRules) noexcept;
    KRYS_NODISCARD UnadjustedStyle UnadjustedStyleForCachedMatchResult(
      dom::Element &element, const ResolutionContext &context, CachedMatchResult &&cachedResult) noexcept;

    KRYS_NODISCARD ResolvedStyle
      StyleForElement(dom::Element &element, const ResolutionContext &context,
                      RuleMatchingBehavior = RuleMatchingBehavior::MatchAllRules) noexcept;

    KRYS_NODISCARD Maybe<ResolvedStyle>
      StyleForPseudoElement(dom::Element &element, const PseudoElementRequest &pseudoElementRequest,
                            const ResolutionContext &context) noexcept;

    KRYS_NODISCARD UniquePtr<RenderStyle> StyleForPage(int32 pageIndex) noexcept;

    KRYS_NODISCARD dom::Document &Document() noexcept;
    KRYS_NODISCARD const dom::Document &Document() const noexcept;

    KRYS_NODISCARD ScopeType GetScopeType() const noexcept
    {
      return _scopeType;
    }

    void AppendAuthorStyleSheets(Span<const RefPtr<css::CSSStyleSheet>> styleSheets) noexcept;

    KRYS_NODISCARD ScopeRuleSets &RuleSets() noexcept
    {
      return _ruleSets;
    }

    KRYS_NODISCARD const ScopeRuleSets &RuleSets() const noexcept
    {
      return _ruleSets;
    }

    KRYS_NODISCARD const css::mq::MediaQueryEvaluator &MediaQueryEvaluator() const noexcept
    {
      return _mediaQueryEvaluator;
    }

    KRYS_NODISCARD bool IsAnimationNameValid(const css::CSSOMString &name) const noexcept;

    /// @brief Fills @p list with the keyframes of the `@keyframes` rule it names, each with its style.
    /// @return Whether there's such a rule.
    bool KeyframeStylesForAnimation(dom::Element &element, const RenderStyle &elementStyle,
                                    const ResolutionContext &context, BlendingKeyframes &list,
                                    const TimingFunction *defaultTimingFunction) const noexcept;
    /// @brief The style of @p elementStyle with @p keyframe's declarations applied; adds the properties it
    /// animates to @p blendingKeyframe.
    KRYS_NODISCARD UniquePtr<RenderStyle> StyleForKeyframe(dom::Element &element,
                                                           const RenderStyle &elementStyle,
                                                           const ResolutionContext &context,
                                                           const css::KeyframeRule &keyframe,
                                                           BlendingKeyframe &blendingKeyframe) const noexcept;

    void SetViewTransitionStyles(css::SelectorPseudoElement element, const css::CSSOMStringAtom &name,
                                 Ref<css::MutableStyleProperties> properties) noexcept;

    KRYS_NODISCARD List<RefPtr<const css::StyleRule>>
      StyleRulesForElement(const dom::Element *element, uint32 rulesToInclude = AllButEmptyCSSRules) noexcept;
    KRYS_NODISCARD List<RefPtr<const css::StyleRule>>
      PseudoStyleRulesForElement(const dom::Element *element,
                                 const Maybe<PseudoElementRequest> &pseudoElementIdentifier,
                                 uint32 rulesToInclude = AllButEmptyCSSRules) noexcept;

    KRYS_NODISCARD bool HasSelectorForId(const css::CSSOMStringAtom &idValue) const noexcept;
    KRYS_NODISCARD bool HasSelectorForAttribute(const dom::Element &element,
                                                const css::CSSOMStringAtom &attributeName) const noexcept;

    KRYS_NODISCARD bool HasViewportDependentMediaQueries() const noexcept;
    KRYS_NODISCARD Maybe<DynamicMediaQueryEvaluationChanges> EvaluateDynamicMediaQueries() noexcept;

    KRYS_NODISCARD static KeyframesRuleMap &UserAgentKeyframes() noexcept;
    static void AddUserAgentKeyframeStyle(Ref<css::KeyframesRule> &&rule) noexcept;
    void AddKeyframeStyle(Ref<css::KeyframesRule> &&rule) noexcept;
    KRYS_NODISCARD List<Ref<css::KeyframeRule>>
      KeyframeRulesForName(const css::CSSOMStringAtom &animationName,
                           const TimingFunction *defaultTimingFunction) const noexcept;

    KRYS_NODISCARD RefPtr<css::ViewTransitionRule> GetViewTransitionRule() const noexcept;

    /// @brief A style with the initial values and the document's default font (`medium`), for @p element
    /// (which may be null).
    KRYS_NODISCARD static UniquePtr<RenderStyle> DefaultStyleForElement(const dom::Document &document,
                                                                        const dom::Element *element) noexcept;

    KRYS_NODISCARD bool UsesFirstLineRules() const noexcept
    {
      return _ruleSets.Features().UsesFirstLineRules;
    }

    KRYS_NODISCARD bool UsesFirstLetterRules() const noexcept
    {
      return _ruleSets.Features().UsesFirstLetterRules;
    }

    KRYS_NODISCARD bool UsesStartingStyleRules() const noexcept
    {
      return _ruleSets.Features().HasStartingStyleRules;
    }

    void InvalidateMatchedDeclarationsCache() noexcept;
    void ClearCachedDeclarationsAffectedByViewportUnits() noexcept;

    KRYS_NODISCARD bool IsSharedBetweenShadowTrees() const noexcept
    {
      return _isSharedBetweenShadowTrees;
    }

    void SetSharedBetweenShadowTrees() noexcept
    {
      _isSharedBetweenShadowTrees = true;
    }

    KRYS_NODISCARD const RenderStyle *RootDefaultStyle() const noexcept
    {
      return _rootDefaultStyle.get();
    }

  private:
    void Initialize() noexcept;

    KRYS_NODISCARD State InitializeStateAndStyle(const dom::Element &element,
                                                 const ResolutionContext &context,
                                                 UniquePtr<RenderStyle> &&initialStyle = {}) noexcept;
    KRYS_NODISCARD BuilderContext GetBuilderContext(State &state) const noexcept;

    void ApplyMatchedProperties(State &state, const MatchResult &matchResult,
                                PropertyCascade::IncludedProperties &&includedProperties) noexcept;
    void SetGlobalStateAfterApplyingProperties(const BuilderState &builderState) noexcept;
  };
}
