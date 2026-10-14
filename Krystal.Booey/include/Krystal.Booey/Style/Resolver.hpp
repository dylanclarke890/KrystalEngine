#pragma once

#include "Krystal.Booey/CSS/Queries/MediaQueryEvaluator.hpp"
#include "Krystal.Booey/CSS/Rules/KeyframesRule.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Style/ScopeRuleSets.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
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
}

namespace krys::boo::dom
{
  class Document;
  class Element;
}

namespace krys::boo::style
{
  using KeyframesRuleMap = HashMap<css::CSSOMStringAtom, RefPtr<css::KeyframesRule>>;

  /// @brief Resolves the style of the elements of a document or of shadow trees from the rule sets of their
  /// style sheets.
  // FIXME: Only the rule set half is ported: style resolution (styleForElement, styleForPseudoElement,
  // styleForKeyframe, styleForPage, ...) comes with the builder and the tree resolver.
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

  private:
    WeakPtr<dom::Document> _document;
    const ScopeType _scopeType;

    ScopeRuleSets _ruleSets;

    KeyframesRuleMap _keyframesRuleMap;

    css::mq::MediaQueryEvaluator _mediaQueryEvaluator;
    UniquePtr<RenderStyle> _rootDefaultStyle;

    bool _isSharedBetweenShadowTrees {false};

    Resolver(dom::Document &document, ScopeType scopeType) noexcept;

  public:
    ~Resolver() noexcept;

    KRYS_NODISCARD static Ref<Resolver> Create(dom::Document &document, ScopeType scopeType) noexcept;

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

    KRYS_NODISCARD bool HasSelectorForId(const css::CSSOMStringAtom &idValue) const noexcept;
    KRYS_NODISCARD bool HasSelectorForAttribute(const dom::Element &element,
                                                const css::CSSOMStringAtom &attributeName) const noexcept;

    KRYS_NODISCARD bool HasViewportDependentMediaQueries() const noexcept;
    KRYS_NODISCARD Maybe<DynamicMediaQueryEvaluationChanges> EvaluateDynamicMediaQueries() noexcept;

    KRYS_NODISCARD static KeyframesRuleMap &UserAgentKeyframes() noexcept;
    static void AddUserAgentKeyframeStyle(Ref<css::KeyframesRule> &&rule) noexcept;
    void AddKeyframeStyle(Ref<css::KeyframesRule> &&rule) noexcept;

    KRYS_NODISCARD RefPtr<css::ViewTransitionRule> GetViewTransitionRule() const noexcept;

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
  };
}
