#include "Krystal.Booey/Style/ScopeRuleSets.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Parser/KeywordLookup.hpp"
#include "Krystal.Booey/CSS/Queries/ContainerQuery.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryEvaluator.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Style/DeclarationOrigin.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/RuleSetBuilder.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Core/Assert.hpp"
#include <algorithm>
#include <ranges>

namespace krys::boo::style
{
  namespace
  {
    template <typename TRules>
    KRYS_NODISCARD RefPtr<RuleSet> MakeRuleSet(const TRules &rules) noexcept
    {
      size_t size = rules.size();
      if (!size)
      {
        return nullptr;
      }
      auto ruleSet = RuleSet::Create();
      for (size_t i = 0; i < size; ++i)
      {
        ruleSet->AddRule(*rules[i].StyleRule, rules[i].SelectorIndex, rules[i].SelectorListIndex);
      }
      ruleSet->ShrinkToFit();
      return ruleSet;
    }

    template <typename TKey, typename TRuleFeatureVector, typename TRuleSetMap, typename TFeatureMap>
    const List<InvalidationRuleSet> *EnsureInvalidationRuleSets(const TKey &key, TRuleSetMap &ruleSetMap,
                                                                const TFeatureMap &ruleFeatures) noexcept
    {
      if (auto it = ruleSetMap.find(key); it != ruleSetMap.end())
      {
        return it->second.get();
      }

      auto &invalidationRuleSets = ruleSetMap[key];

      auto featuresIt = ruleFeatures.find(key);
      if (featuresIt == ruleFeatures.end())
      {
        return nullptr;
      }
      auto &features = *featuresIt->second;

      struct Builder
      {
        RefPtr<RuleSet> RuleSet;
        List<const css::SelectorList *> InvalidationSelectors;
        style::MatchElement MatchElement;
        style::IsNegation IsNegation;
      };
      List<Builder> builders;

      for (auto &feature : features)
      {
        auto it = std::ranges::find_if(builders,
                                       [&](const Builder &builder)
                                       {
                                         return builder.MatchElement == feature.MatchElement
                                                && builder.IsNegation == feature.IsNegation;
                                       });
        if (it == builders.end())
        {
          builders.push_back(Builder {
            .RuleSet = RuleSet::Create(),
            .InvalidationSelectors = {},
            .MatchElement = feature.MatchElement,
            .IsNegation = feature.IsNegation,
          });
          it = builders.end() - 1;
        }
        auto &builder = *it;

        builder.RuleSet->AddRule(*feature.StyleRule, feature.SelectorIndex, feature.SelectorListIndex);

        if constexpr (std::is_same_v<typename TRuleFeatureVector::value_type,
                                     RuleFeatureWithInvalidationSelector>)
        {
          auto alreadyContains = [&](const css::SelectorList &invalidationSelector)
          {
            constexpr auto maximumSearchCount = 8;
            auto count = 0;
            for (auto *existing : builder.InvalidationSelectors | std::views::reverse)
            {
              if (++count > maximumSearchCount)
              {
                break;
              }
              if (invalidationSelector == *existing)
              {
                return true;
              }
            }
            return false;
          };

          if (!alreadyContains(feature.InvalidationSelector))
          {
            builder.InvalidationSelectors.push_back(&feature.InvalidationSelector);
          }
        }
      }

      invalidationRuleSets = CreateUnique<List<InvalidationRuleSet>>();
      for (auto &builder : builders)
      {
        builder.RuleSet->ShrinkToFit();
        invalidationRuleSets->push_back(InvalidationRuleSet {
          .RuleSet = krys::move(builder.RuleSet),
          .InvalidationSelectors = css::SelectorList::Join(builder.InvalidationSelectors),
          .MatchElement = builder.MatchElement,
          .IsNegation = builder.IsNegation,
        });
      }
      return invalidationRuleSets.get();
    }
  }

  ScopeRuleSets::ScopeRuleSets(Resolver &styleResolver) noexcept
      : _authorStyle(RuleSet::Create()), _styleResolver(styleResolver)
  {
  }

  ScopeRuleSets::~ScopeRuleSets() noexcept
  {
    krys_assert(!_isInvalidatingStyleWithRuleSets);
  }

  RuleSet *ScopeRuleSets::UserAgentMediaQueryStyle() const noexcept
  {
    UpdateUserAgentMediaQueryStyleIfNeeded();
    return _userAgentMediaQueryStyle.get();
  }

  void ScopeRuleSets::UpdateUserAgentMediaQueryStyleIfNeeded() const noexcept
  {
    if (!UserAgentStyle::MediaQueryStyleSheet)
    {
      return;
    }

    auto ruleCount = UserAgentStyle::MediaQueryStyleSheet->RuleCount();
    if (_userAgentMediaQueryStyle && ruleCount == _userAgentMediaQueryRuleCountOnUpdate)
    {
      return;
    }
    _userAgentMediaQueryRuleCountOnUpdate = ruleCount;

    // Media queries on user agent sheet need to evaluated in document context. They behave like author sheets
    // in this respect.
    auto &mediaQueryEvaluator = _styleResolver.MediaQueryEvaluator();
    _userAgentMediaQueryStyle = RuleSet::Create();
    RuleSetBuilder builder(*_userAgentMediaQueryStyle, mediaQueryEvaluator, &_styleResolver);
    builder.AddRulesFromSheet(*UserAgentStyle::MediaQueryStyleSheet);
  }

  RuleSet *ScopeRuleSets::DynamicViewTransitionsStyle() const noexcept
  {
    return _dynamicViewTransitionsStyle.get();
  }

  RuleSet *ScopeRuleSets::UserStyle() const noexcept
  {
    if (_usesSharedUserStyle)
    {
      return _styleResolver.Document().StyleScope().GetResolver().RuleSets().UserStyle();
    }
    return _userStyle.get();
  }

  RuleSet *ScopeRuleSets::StyleForDeclarationOrigin(DeclarationOrigin origin) noexcept
  {
    switch (origin)
    {
      case DeclarationOrigin::Author:    return _authorStyle.get();
      case DeclarationOrigin::User:      return UserStyle();
      case DeclarationOrigin::UserAgent: return UserAgentMediaQueryStyle();
    }
    krys_debug_assert(false);
    return nullptr;
  }

  void ScopeRuleSets::InitializeUserStyle() noexcept
  {
    auto userStyle = RuleSet::Create();

    // FIXME: Add the rules of the page user style sheet and the injected and document user style sheets, once
    // there are user style sheets (ExtensionStyleSheets).

    if (userStyle->RuleCount() > 0 || !userStyle->PageRules().empty())
    {
      _userStyle = krys::move(userStyle);
    }
  }

  void ScopeRuleSets::ResetAuthorStyle() noexcept
  {
    _isAuthorStyleDefined = true;
    _authorStyle = RuleSet::Create();
  }

  void ScopeRuleSets::ResetUserAgentMediaQueryStyle() noexcept
  {
    _userAgentMediaQueryStyle = nullptr;
  }

  bool ScopeRuleSets::HasViewportDependentMediaQueries() const noexcept
  {
    if (_authorStyle->HasViewportDependentMediaQueries())
    {
      return true;
    }
    if (_userStyle && _userStyle->HasViewportDependentMediaQueries())
    {
      return true;
    }
    if (_userAgentMediaQueryStyle && _userAgentMediaQueryStyle->HasViewportDependentMediaQueries())
    {
      return true;
    }

    return false;
  }

  bool ScopeRuleSets::HasContainerQueries() const noexcept
  {
    if (_authorStyle->HasContainerQueries())
    {
      return true;
    }
    if (_userStyle && _userStyle->HasContainerQueries())
    {
      return true;
    }
    if (_userAgentMediaQueryStyle && _userAgentMediaQueryStyle->HasContainerQueries())
    {
      return true;
    }

    return false;
  }

  bool ScopeRuleSets::HasScopeRules() const noexcept
  {
    if (_authorStyle->HasScopeRules())
    {
      return true;
    }
    if (_userStyle && _userStyle->HasScopeRules())
    {
      return true;
    }
    if (_userAgentMediaQueryStyle && _userAgentMediaQueryStyle->HasScopeRules())
    {
      return true;
    }

    return false;
  }

  RefPtr<css::ViewTransitionRule> ScopeRuleSets::GetViewTransitionRule() const noexcept
  {
    if (auto viewTransitionRule = _authorStyle->GetViewTransitionRule())
    {
      return viewTransitionRule;
    }

    if (_userStyle && _userStyle->GetViewTransitionRule())
    {
      return _userStyle->GetViewTransitionRule();
    }

    if (_userAgentMediaQueryStyle && _userAgentMediaQueryStyle->GetViewTransitionRule())
    {
      return _userAgentMediaQueryStyle->GetViewTransitionRule();
    }

    return nullptr;
  }

  Maybe<DynamicMediaQueryEvaluationChanges>
    ScopeRuleSets::EvaluateDynamicMediaQueryRules(const css::mq::MediaQueryEvaluator &evaluator) noexcept
  {
    Maybe<DynamicMediaQueryEvaluationChanges> evaluationChanges;

    auto evaluate = [&](RuleSet *ruleSet)
    {
      if (!ruleSet)
      {
        return;
      }
      if (auto changes = ruleSet->EvaluateDynamicMediaQueryRules(evaluator))
      {
        if (evaluationChanges)
        {
          evaluationChanges->Append(krys::move(*changes));
        }
        else
        {
          evaluationChanges = krys::move(changes);
        }
      }
    };

    evaluate(&AuthorStyle());
    evaluate(UserStyle());
    evaluate(UserAgentMediaQueryStyle());

    return evaluationChanges;
  }

  void
    ScopeRuleSets::AppendAuthorStyleSheets(Span<const RefPtr<css::CSSStyleSheet>> styleSheets,
                                           const css::mq::MediaQueryEvaluator &mediaQueryEvaluator) noexcept
  {
    {
      RuleSetBuilder builder(*_authorStyle, mediaQueryEvaluator, &_styleResolver,
                             RuleSetBuilder::ShrinkToFit::Enable);

      RefPtr<css::CSSStyleSheet> previous;
      for (auto &cssSheet : styleSheets)
      {
        krys_debug_assert(!cssSheet->Disabled());

        // In some cases, we have many identical <style> tags. To avoid pathological behavior, we check the
        // one-previous <style> and skip adding a new one when the content is exact same to the previous one.
        if (previous)
        {
          if (&previous->Contents() == &cssSheet->Contents() && previous->MediaQueries().empty()
              && cssSheet->MediaQueries().empty())
          {
            continue;
          }
        }

        builder.AddRulesFromSheet(cssSheet->Contents(), cssSheet->MediaQueries());

        previous = cssSheet;
      }
    }

    CollectFeatures();
  }

  void ScopeRuleSets::CollectFeatures() const noexcept
  {
    krys_assert(!_isInvalidatingStyleWithRuleSets);

    _features.Clear();
    if (UserAgentStyle::DefaultStyle)
    {
      _features.Add(UserAgentStyle::DefaultStyle->Features());
    }
    _defaultStyleVersionOnFeatureCollection = UserAgentStyle::DefaultStyleVersion;

    if (auto *userAgentMediaQueryStyle = UserAgentMediaQueryStyle())
    {
      _features.Add(userAgentMediaQueryStyle->Features());
    }

    if (_authorStyle)
    {
      _features.Add(_authorStyle->Features());
    }
    if (auto *userStyle = UserStyle())
    {
      _features.Add(userStyle->Features());
    }

    _scopeBreakingHasPseudoClassInvalidationRuleSet = MakeRuleSet(_features.ScopeBreakingHasPseudoClassRules);

    _idInvalidationRuleSets.clear();
    _classInvalidationRuleSets.clear();
    _attributeInvalidationRuleSets.clear();
    _pseudoClassInvalidationRuleSets.clear();
    _hasPseudoClassInvalidationRuleSets.clear();

    _customPropertyNamesInStyleContainerQueries = null;
    _cachedSelectorsForStyleAttribute = null;

    _features.ShrinkToFit();
  }

  const List<InvalidationRuleSet> *
    ScopeRuleSets::IdInvalidationRuleSets(const css::CSSOMStringAtom &id) const noexcept
  {
    return EnsureInvalidationRuleSets<css::CSSOMStringAtom, RuleFeatureVector>(id, _idInvalidationRuleSets,
                                                                               _features.IdRules);
  }

  const List<InvalidationRuleSet> *
    ScopeRuleSets::ClassInvalidationRuleSets(const css::CSSOMStringAtom &className) const noexcept
  {
    return EnsureInvalidationRuleSets<css::CSSOMStringAtom, RuleFeatureVector>(
      className, _classInvalidationRuleSets, _features.ClassRules);
  }

  const List<InvalidationRuleSet> *
    ScopeRuleSets::AttributeInvalidationRuleSets(const css::CSSOMStringAtom &attributeName) const noexcept
  {
    return EnsureInvalidationRuleSets<css::CSSOMStringAtom, List<RuleFeatureWithInvalidationSelector>>(
      attributeName, _attributeInvalidationRuleSets, _features.AttributeRules);
  }

  const List<InvalidationRuleSet> *ScopeRuleSets::PseudoClassInvalidationRuleSets(
    const PseudoClassInvalidationKey &pseudoClassKey) const noexcept
  {
    return EnsureInvalidationRuleSets<PseudoClassInvalidationKey, RuleFeatureVector>(
      pseudoClassKey, _pseudoClassInvalidationRuleSets, _features.PseudoClassRules);
  }

  const List<InvalidationRuleSet> *
    ScopeRuleSets::HasPseudoClassInvalidationRuleSets(const PseudoClassInvalidationKey &key) const noexcept
  {
    return EnsureInvalidationRuleSets<PseudoClassInvalidationKey, List<RuleFeatureWithInvalidationSelector>>(
      key, _hasPseudoClassInvalidationRuleSets, _features.HasPseudoClassRules);
  }

  const HashSet<css::CSSOMStringAtom> &
    ScopeRuleSets::CustomPropertyNamesInStyleContainerQueries() const noexcept
  {
    if (!_customPropertyNamesInStyleContainerQueries)
    {
      HashSet<css::CSSOMStringAtom> propertyNames;

      auto collectPropertyNames = [&](RuleSet *ruleSet)
      {
        if (!ruleSet)
        {
          return;
        }
        for (auto &query : ruleSet->ContainerQueryRules())
        {
          css::mq::TraverseFeatures(query->ContainerQuery().Condition,
                                    [&](const css::mq::Feature &containerFeature)
                                    {
                                      if (css::IsCustomPropertyName(containerFeature.Name.View()))
                                      {
                                        propertyNames.insert(containerFeature.Name);
                                      }
                                    });
        }
      };

      collectPropertyNames(&AuthorStyle());
      collectPropertyNames(UserStyle());

      _customPropertyNamesInStyleContainerQueries = krys::move(propertyNames);
    }
    return *_customPropertyNamesInStyleContainerQueries;
  }

  SelectorsForStyleAttribute ScopeRuleSets::GetSelectorsForStyleAttribute() const noexcept
  {
    auto compute = [&]
    {
      auto *ruleSets = AttributeInvalidationRuleSets(css::CSSOMStringAtom(u8"style"));
      if (!ruleSets)
      {
        return SelectorsForStyleAttribute::None;
      }
      for (auto &ruleSet : *ruleSets)
      {
        if (ruleSet.MatchElement != style::MatchElement::Subject)
        {
          return SelectorsForStyleAttribute::NonSubjectPosition;
        }
      }
      return SelectorsForStyleAttribute::SubjectPositionOnly;
    };

    if (!_cachedSelectorsForStyleAttribute)
    {
      _cachedSelectorsForStyleAttribute = compute();
    }

    return *_cachedSelectorsForStyleAttribute;
  }

  bool ScopeRuleSets::HasMatchingUserOrAuthorStyle(const Func<bool(RuleSet &)> &predicate) noexcept
  {
    if (_authorStyle && predicate(*_authorStyle))
    {
      return true;
    }

    if (auto *userStyle = UserStyle(); userStyle && predicate(*userStyle))
    {
      return true;
    }

    return false;
  }
}
