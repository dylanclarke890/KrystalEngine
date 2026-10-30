#pragma once

#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"
#include "Krystal.Booey/Style/RuleFeature.hpp"
#include "Krystal.Booey/Style/RuleSet.hpp"
#include "Krystal.Booey/Style/UserAgentStyle.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::css
{
  class CSSStyleSheet;
}

namespace krys::boo::css::mq
{
  class MediaQueryEvaluator;
}

namespace krys::boo::style
{
  enum class DeclarationOrigin : uint8;
  class Resolver;

  struct InvalidationRuleSet
  {
    RefPtr<style::RuleSet> RuleSet;
    // Invalidation selectors are used for attribute selector and :has() invalidation.
    // For attributes selectors it contains the simple selectors for fast testing of whether an attribute
    // mutation may have an effect. For :has() it contains the complex argument selectors for testing if
    // adding or removing a node may affect :has() matching. Otherwise the list is empty.
    css::SelectorList InvalidationSelectors;
    style::MatchElement MatchElement;
    style::IsNegation IsNegation;
  };

  enum class SelectorsForStyleAttribute : uint8
  {
    None,
    SubjectPositionOnly,
    NonSubjectPosition
  };

  /// @brief The rule sets of a resolver: author, user and the document-dependent user agent rules, and the
  /// features and invalidation rule sets collected from them and the default style.
  class ScopeRuleSets
  {
    KRYS_NON_COPY_MOVABLE(ScopeRuleSets);

    template <typename T>
    using AtomMap = HashMap<css::CSSOMStringAtom, UniquePtr<T>>;
    template <typename T>
    using PseudoClassMap = HashMap<PseudoClassInvalidationKey, UniquePtr<T>, PseudoClassInvalidationKeyHash>;

  private:
    RefPtr<RuleSet> _authorStyle;
    mutable RefPtr<RuleSet> _userAgentMediaQueryStyle;
    mutable RefPtr<RuleSet> _dynamicViewTransitionsStyle;
    RefPtr<RuleSet> _userStyle;

    Resolver &_styleResolver;
    mutable RuleFeatureSet _features;
    mutable RefPtr<RuleSet> _scopeBreakingHasPseudoClassInvalidationRuleSet;
    mutable AtomMap<List<InvalidationRuleSet>> _idInvalidationRuleSets;
    mutable AtomMap<List<InvalidationRuleSet>> _classInvalidationRuleSets;
    mutable AtomMap<List<InvalidationRuleSet>> _attributeInvalidationRuleSets;
    mutable PseudoClassMap<List<InvalidationRuleSet>> _pseudoClassInvalidationRuleSets;
    mutable PseudoClassMap<List<InvalidationRuleSet>> _hasPseudoClassInvalidationRuleSets;

    mutable Maybe<HashSet<css::CSSOMStringAtom>> _customPropertyNamesInStyleContainerQueries;

    mutable Maybe<SelectorsForStyleAttribute> _cachedSelectorsForStyleAttribute;

    mutable uint32 _defaultStyleVersionOnFeatureCollection {0};
    mutable uint32 _userAgentMediaQueryRuleCountOnUpdate {0};

    bool _usesSharedUserStyle {false};
    bool _isAuthorStyleDefined {false};

    bool _isInvalidatingStyleWithRuleSets {false};

  public:
    explicit ScopeRuleSets(Resolver &styleResolver) noexcept;
    ~ScopeRuleSets() noexcept;

    KRYS_NODISCARD bool IsAuthorStyleDefined() const noexcept
    {
      return _isAuthorStyleDefined;
    }

    KRYS_NODISCARD RuleSet *UserAgentMediaQueryStyle() const noexcept;
    KRYS_NODISCARD RuleSet *DynamicViewTransitionsStyle() const noexcept;

    KRYS_NODISCARD RuleSet &AuthorStyle() const noexcept
    {
      return *_authorStyle;
    }

    KRYS_NODISCARD RuleSet *UserStyle() const noexcept;
    KRYS_NODISCARD RuleSet *StyleForDeclarationOrigin(DeclarationOrigin origin) noexcept;

    KRYS_NODISCARD const RuleFeatureSet &Features() const noexcept;

    KRYS_NODISCARD RuleSet *ScopeBreakingHasPseudoClassInvalidationRuleSet() const noexcept
    {
      return _scopeBreakingHasPseudoClassInvalidationRuleSet.get();
    }

    KRYS_NODISCARD const List<InvalidationRuleSet> *
      IdInvalidationRuleSets(const css::CSSOMStringAtom &id) const noexcept;
    KRYS_NODISCARD const List<InvalidationRuleSet> *
      ClassInvalidationRuleSets(const css::CSSOMStringAtom &className) const noexcept;
    KRYS_NODISCARD const List<InvalidationRuleSet> *
      AttributeInvalidationRuleSets(const css::CSSOMStringAtom &attributeName) const noexcept;
    KRYS_NODISCARD const List<InvalidationRuleSet> *
      PseudoClassInvalidationRuleSets(const PseudoClassInvalidationKey &pseudoClassKey) const noexcept;
    KRYS_NODISCARD const List<InvalidationRuleSet> *
      HasPseudoClassInvalidationRuleSets(const PseudoClassInvalidationKey &key) const noexcept;

    KRYS_NODISCARD const HashSet<css::CSSOMStringAtom> &
      CustomPropertyNamesInStyleContainerQueries() const noexcept;

    KRYS_NODISCARD SelectorsForStyleAttribute GetSelectorsForStyleAttribute() const noexcept;

    void SetUsesSharedUserStyle(bool b) noexcept
    {
      _usesSharedUserStyle = b;
    }

    void InitializeUserStyle() noexcept;

    void ResetAuthorStyle() noexcept;
    void AppendAuthorStyleSheets(Span<const RefPtr<css::CSSStyleSheet>> styleSheets,
                                 const css::mq::MediaQueryEvaluator &mediaQueryEvaluator) noexcept;

    void ResetUserAgentMediaQueryStyle() noexcept;

    KRYS_NODISCARD bool HasViewportDependentMediaQueries() const noexcept;
    KRYS_NODISCARD bool HasContainerQueries() const noexcept;
    KRYS_NODISCARD bool HasScopeRules() const noexcept;

    KRYS_NODISCARD RefPtr<css::ViewTransitionRule> GetViewTransitionRule() const noexcept;

    KRYS_NODISCARD Maybe<DynamicMediaQueryEvaluationChanges>
      EvaluateDynamicMediaQueryRules(const css::mq::MediaQueryEvaluator &evaluator) noexcept;

    // FIXME: There should be just the const version.
    KRYS_NODISCARD RuleFeatureSet &MutableFeatures() noexcept;

    void SetDynamicViewTransitionsStyle(RuleSet *ruleSet) noexcept
    {
      _dynamicViewTransitionsStyle = ShareRefPtr(ruleSet);
    }

    KRYS_NODISCARD bool &IsInvalidatingStyleWithRuleSets() noexcept
    {
      return _isInvalidatingStyleWithRuleSets;
    }

    KRYS_NODISCARD bool HasMatchingUserOrAuthorStyle(const Func<bool(RuleSet &)> &predicate) noexcept;

  private:
    void CollectFeatures() const noexcept;
    void UpdateUserAgentMediaQueryStyleIfNeeded() const noexcept;
  };

  inline const RuleFeatureSet &ScopeRuleSets::Features() const noexcept
  {
    if (_defaultStyleVersionOnFeatureCollection < UserAgentStyle::DefaultStyleVersion)
    {
      CollectFeatures();
    }
    return _features;
  }

  // FIXME: There should be just the const version.
  inline RuleFeatureSet &ScopeRuleSets::MutableFeatures() noexcept
  {
    if (_defaultStyleVersionOnFeatureCollection < UserAgentStyle::DefaultStyleVersion)
    {
      CollectFeatures();
    }
    return _features;
  }
}
