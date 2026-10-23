#include "Krystal.Booey/Style/ElementRuleCollector.hpp"
#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Properties/ImmutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/StylePropertiesInlines.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryEvaluator.hpp"
#include "Krystal.Booey/CSS/Rules/KeyframeRule.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/StyledElement.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/Style/ContainerQueryEvaluator.hpp"
#include "Krystal.Booey/Style/DeclarationOrigin.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/ScopeRuleSets.hpp"
#include "Krystal.Booey/Style/SelectorMatchingState.hpp"
#include "Krystal.Booey/Style/UserAgentStyle.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include <algorithm>

namespace krys::boo::style
{
  namespace
  {
    KRYS_NODISCARD const css::StyleProperties &LeftToRightDeclaration() noexcept
    {
      static auto &declaration = []() -> const css::StyleProperties &
      {
        auto properties = css::MutableStyleProperties::Create();
        properties->SetProperty(css::PropertyId::Direction, css::ValueId::Ltr);
        return *properties.release();
      }();
      return declaration;
    }

    KRYS_NODISCARD const css::StyleProperties &RightToLeftDeclaration() noexcept
    {
      static auto &declaration = []() -> const css::StyleProperties &
      {
        auto properties = css::MutableStyleProperties::Create();
        properties->SetProperty(css::PropertyId::Direction, css::ValueId::Rtl);
        return *properties.release();
      }();
      return declaration;
    }

    /// @brief The directionality of @p element if its `dir` attribute is in the auto state.
    /// @see https://html.spec.whatwg.org/#the-directionality
    KRYS_NODISCARD Maybe<TextDirection>
      ComputeTextDirectionIfDirIsAuto(const html::HTMLElement &element) noexcept
    {
      auto dir = element.GetAttribute(u8"dir");
      if (!dir || !text::IsASCIICaselessEqual(dom::DOMStringView(*dir), dom::DOMStringView(u8"auto")))
      {
        return {};
      }
      // FIXME: The auto directionality (the first strong character of the element's text), once there is bidi
      // data. Until then it is always the fallback, 'ltr', as for :dir().
      return TextDirection::LTR;
    }
  }

  struct MatchRequest
  {
    const style::RuleSet &RuleSet;
    ScopeOrdinal StyleScopeOrdinal;
    bool MatchingPartPseudoElementRules {false};

    explicit MatchRequest(const style::RuleSet &ruleSet,
                          ScopeOrdinal styleScopeOrdinal = ScopeOrdinal::Element) noexcept
        : RuleSet(ruleSet), StyleScopeOrdinal(styleScopeOrdinal)
    {
    }
  };

  ElementRuleCollector::ElementRuleCollector(const dom::Element &element, const ScopeRuleSets &ruleSets,
                                             SelectorMatchingState *selectorMatchingState,
                                             css::SelectorChecker::Mode mode) noexcept
      : _element(ShareRef(element)), _authorStyle(ShareRef(ruleSets.AuthorStyle())),
        _userStyle(ShareRefPtr(ruleSets.UserStyle())),
        _userAgentMediaQueryStyle(ShareRefPtr(ruleSets.UserAgentMediaQueryStyle())),
        _dynamicViewTransitionsStyle(ShareRefPtr(ruleSets.DynamicViewTransitionsStyle())),
        _selectorMatchingState(selectorMatchingState), _mode(mode),
        _result(MatchResult::Create(css::IsLink(element)))
  {
    krys_debug_assert(
      !_selectorMatchingState
      || _selectorMatchingState->SelectorFilter.ParentStackIsConsistent(element.ParentNode()));
  }

  ElementRuleCollector::ElementRuleCollector(const dom::Element &element, const RuleSet &authorStyle,
                                             SelectorMatchingState *selectorMatchingState,
                                             css::SelectorChecker::Mode mode) noexcept
      : _element(ShareRef(element)), _authorStyle(ShareRef(authorStyle)),
        _selectorMatchingState(selectorMatchingState), _mode(mode),
        _result(MatchResult::Create(css::IsLink(element)))
  {
    krys_debug_assert(
      !_selectorMatchingState
      || _selectorMatchingState->SelectorFilter.ParentStackIsConsistent(element.ParentNode()));
  }

  ElementRuleCollector::~ElementRuleCollector() noexcept = default;

  void ElementRuleCollector::SetMedium(const css::mq::MediaQueryEvaluator &medium) noexcept
  {
    _isPrintStyle = medium.IsPrintMedia();
  }

  const MatchResult &ElementRuleCollector::GetMatchResult() const noexcept
  {
    krys_debug_assert(_mode == css::SelectorChecker::Mode::ResolvingStyle);
    return *_result;
  }

  Ref<MatchResult> ElementRuleCollector::ReleaseMatchResult() noexcept
  {
    return krys::move(_result);
  }

  const List<RefPtr<const css::StyleRule>> &ElementRuleCollector::MatchedRuleList() const noexcept
  {
    krys_debug_assert(_mode == css::SelectorChecker::Mode::CollectingRules);
    return _matchedRuleList;
  }

  inline void ElementRuleCollector::AddMatchedRule(const RuleData &ruleData, uint32 specificity,
                                                   uint32 scopingRootDistance,
                                                   const MatchRequest &matchRequest) noexcept
  {
    auto cascadeLayerPriority = matchRequest.RuleSet.CascadeLayerPriorityFor(ruleData);
    _matchedRules.push_back(
      {&ruleData, specificity, scopingRootDistance, matchRequest.StyleScopeOrdinal, cascadeLayerPriority});
  }

  void ElementRuleCollector::ClearMatchedRules() noexcept
  {
    _matchedRules.clear();
    _matchedRuleTransferIndex = 0;
  }

  inline void ElementRuleCollector::AddElementStyleProperties(const css::StyleProperties *propertySet,
                                                              CascadeLayerPriority priority,
                                                              IsCacheable isCacheable,
                                                              FromStyleAttribute fromStyleAttribute) noexcept
  {
    if (!propertySet || propertySet->empty())
    {
      return;
    }

    auto matchedProperty = MatchedProperties {ShareRef(*propertySet)};
    matchedProperty.CascadeLayerPriority = priority;
    matchedProperty.FromStyleAttribute = fromStyleAttribute;
    matchedProperty.IsCacheable = isCacheable;
    AddMatchedProperties(krys::move(matchedProperty), DeclarationOrigin::Author);
  }

  bool ElementRuleCollector::IsFirstMatchModeAndHasMatchedAnyRules() const noexcept
  {
    return _mode == css::SelectorChecker::Mode::StyleInvalidation && !_matchedRules.empty();
  }

  void ElementRuleCollector::CollectMatchingRules(DeclarationOrigin origin) noexcept
  {
    switch (origin)
    {
      case DeclarationOrigin::Author:
      {
        MatchRequest matchRequest(*_authorStyle);
        CollectMatchingRules(matchRequest);
        if (IsFirstMatchModeAndHasMatchedAnyRules())
        {
          return;
        }
        break;
      }

      case DeclarationOrigin::User:
        if (_userStyle)
        {
          MatchRequest matchRequest(*_userStyle);
          CollectMatchingRules(matchRequest);
          if (IsFirstMatchModeAndHasMatchedAnyRules())
          {
            return;
          }
        }
        break;

      case DeclarationOrigin::UserAgent: krys_debug_assert(false); return;
    }

    auto *parent = Element().ParentElement();
    if (parent && dom::TreeQueries::GetShadowRoot(*parent))
    {
      MatchSlottedPseudoElementRules(origin);
      if (IsFirstMatchModeAndHasMatchedAnyRules())
      {
        return;
      }
    }

    if (dom::TreeQueries::GetShadowRoot(Element()))
    {
      MatchHostPseudoClassRules(origin);
      if (IsFirstMatchModeAndHasMatchedAnyRules())
      {
        return;
      }
    }

    if (dom::TreeQueries::ContainingShadowRoot(Element()))
    {
      MatchUserAgentPartRules(origin);
      if (IsFirstMatchModeAndHasMatchedAnyRules())
      {
        return;
      }
      MatchPartPseudoElementRules(origin);
    }
  }

  void ElementRuleCollector::CollectMatchingUserAgentPartRules(const MatchRequest &matchRequest) noexcept
  {
    krys_debug_assert(Element().IsInUserAgentShadowTree());

    auto &rules = matchRequest.RuleSet;
    // FIXME: ::cue rules for WebVTT elements, with media.
    if (auto part = Element().UserAgentPart(); !part.View().empty())
    {
      CollectMatchingRulesForList(rules.UserAgentPartRules(part), matchRequest);
    }
  }

  void ElementRuleCollector::CollectMatchingRules(const MatchRequest &matchRequest) noexcept
  {
    krys_debug_assert_msg(
      !(_mode == css::SelectorChecker::Mode::StyleInvalidation && _pseudoElementRequest),
      "When in StyleInvalidation or SharingRules, SelectorChecker does not try to match the pseudo ID. While "
      "ElementRuleCollector supports matching a particular pseudoId in this case, this would indicate a "
      "error at "
      "the call site since matching a particular element should be unnecessary.");

    auto &element = Element();
    auto *shadowRoot = dom::TreeQueries::ContainingShadowRoot(element);
    if (shadowRoot && shadowRoot->Mode() == dom::ShadowRootMode::UserAgent)
    {
      CollectMatchingUserAgentPartRules(matchRequest);
    }

    bool isHTML = Is<html::HTMLElement>(element) && Is<dom::HTMLDocument>(element.NodeDocument());

    // We need to collect the rules for id, class, tag, and everything else into a buffer and
    // then sort the buffer.
    auto id = element.IdForStyleResolution();
    if (!id.empty())
    {
      CollectMatchingRulesForList(matchRequest.RuleSet.IdRules(css::CSSOMStringAtom(id)), matchRequest);
    }
    for (auto &className : element.ClassNames())
    {
      CollectMatchingRulesForList(matchRequest.RuleSet.ClassRules(className), matchRequest);
    }
    if (element.HasAttributes() && matchRequest.RuleSet.HasAttributeRules())
    {
      SmallList<const RuleSet::RuleDataVector *, 4> ruleVectors;
      for (auto &attribute : element.AttributeList())
      {
        if (auto *rules = matchRequest.RuleSet.AttributeRules(attribute->LocalName(), isHTML))
        {
          ruleVectors.push_back(rules);
        }
      }
      for (auto *rules : ruleVectors)
      {
        CollectMatchingRulesForList(rules, matchRequest);
      }
    }
    if (_pseudoElementRequest && _pseudoElementRequest->NameArgument() != css::CSSOMStringAtom::Null())
    {
      CollectMatchingRulesForList(
        matchRequest.RuleSet.NamedPseudoElementRules(_pseudoElementRequest->NameArgument()), matchRequest);
    }
    if (css::IsLink(element))
    {
      CollectMatchingRulesForList(matchRequest.RuleSet.LinkPseudoClassRules(), matchRequest);
    }
    if (css::MatchesFocusPseudoClass(element))
    {
      CollectMatchingRulesForList(matchRequest.RuleSet.FocusPseudoClassRules(), matchRequest);
    }
    if (css::MatchesFocusVisiblePseudoClass(element))
    {
      CollectMatchingRulesForList(matchRequest.RuleSet.FocusVisiblePseudoClassRules(), matchRequest);
    }
    if (&element == element.NodeDocument().DocumentElement().get())
    {
      CollectMatchingRulesForList(matchRequest.RuleSet.RootElementRules(), matchRequest);
    }
    CollectMatchingRulesForList(matchRequest.RuleSet.TagRules(element.LocalName(), isHTML), matchRequest);
    CollectMatchingRulesForList(matchRequest.RuleSet.UniversalRules(), matchRequest);
  }

  List<MatchedProperties> &ElementRuleCollector::DeclarationsForOrigin(DeclarationOrigin origin) noexcept
  {
    switch (origin)
    {
      case DeclarationOrigin::UserAgent: return _result->UserAgentDeclarations;
      case DeclarationOrigin::User:      return _result->UserDeclarations;
      case DeclarationOrigin::Author:    return _result->AuthorDeclarations;
    }
    krys_debug_assert(false);
    return _result->AuthorDeclarations;
  }

  void ElementRuleCollector::SortAndTransferMatchedRules(DeclarationOrigin origin) noexcept
  {
    if (_matchedRules.empty())
    {
      return;
    }

    SortMatchedRules();

    TransferMatchedRules(origin);
  }

  void ElementRuleCollector::TransferMatchedRules(DeclarationOrigin origin,
                                                  Maybe<ScopeOrdinal> fromScope) noexcept
  {
    if (_mode != css::SelectorChecker::Mode::CollectingRules)
    {
      DeclarationsForOrigin(origin).reserve(_matchedRules.size());
    }

    for (; _matchedRuleTransferIndex < _matchedRules.size(); ++_matchedRuleTransferIndex)
    {
      auto &matchedRule = _matchedRules[_matchedRuleTransferIndex];
      if (fromScope && matchedRule.StyleScopeOrdinal < *fromScope)
      {
        break;
      }

      if (_mode == css::SelectorChecker::Mode::CollectingRules)
      {
        _matchedRuleList.push_back(ShareRefPtr(&matchedRule.RuleData->StyleRule()));
        continue;
      }

      AddMatchedProperties(
        {
          ShareRef(matchedRule.RuleData->StyleRule().Properties()),
          static_cast<uint8>(matchedRule.RuleData->LinkMatchType()),
          matchedRule.RuleData->GetPropertyAllowlist(),
          matchedRule.StyleScopeOrdinal,
          FromStyleAttribute::No,
          matchedRule.CascadeLayerPriority,
          matchedRule.RuleData->UsedRuleTypes(),
        },
        origin);
    }
  }

  void ElementRuleCollector::MatchAuthorRules() noexcept
  {
    ClearMatchedRules();

    CollectMatchingRules(DeclarationOrigin::Author);

    SortAndTransferMatchedRules(DeclarationOrigin::Author);
  }

  bool ElementRuleCollector::MatchesAnyAuthorRules() noexcept
  {
    ClearMatchedRules();

    CollectMatchingRules(DeclarationOrigin::Author);

    return !_matchedRules.empty();
  }

  void ElementRuleCollector::MatchUserAgentPartRules(DeclarationOrigin origin) noexcept
  {
    krys_debug_assert(Element().IsInShadowTree());
    auto *shadowRoot = dom::TreeQueries::ContainingShadowRoot(Element());
    if (!shadowRoot || shadowRoot->Mode() != dom::ShadowRootMode::UserAgent)
    {
      return;
    }

    // Look up user agent parts also from the host scope style as they are web-exposed.
    auto *hostRules =
      Scope::ForNode(*shadowRoot->Host()).GetResolver().RuleSets().StyleForDeclarationOrigin(origin);
    if (!hostRules)
    {
      return;
    }

    MatchRequest hostRequest {*hostRules, ScopeOrdinal::ContainingHost};
    CollectMatchingUserAgentPartRules(hostRequest);
  }

  void ElementRuleCollector::MatchHostPseudoClassRules(DeclarationOrigin origin) noexcept
  {
    auto *shadowRoot = dom::TreeQueries::GetShadowRoot(Element());
    krys_debug_assert(shadowRoot);

    auto *shadowRules = shadowRoot->StyleScope().GetResolver().RuleSets().StyleForDeclarationOrigin(origin);
    if (!shadowRules)
    {
      return;
    }

    auto collect = [&](const RuleSet::RuleDataVector &rules)
    {
      if (rules.empty())
      {
        return;
      }

      MatchRequest hostMatchRequest {*shadowRules, ScopeOrdinal::Shadow};
      CollectMatchingRulesForList(&rules, hostMatchRequest);
    };

    if (shadowRules->HasHostOrScopePseudoClassRulesInUniversalBucket())
    {
      if (auto *universalRules = shadowRules->UniversalRules())
      {
        collect(*universalRules);
      }
    }

    collect(shadowRules->HostPseudoClassRules());
  }

  void ElementRuleCollector::MatchSlottedPseudoElementRules(DeclarationOrigin origin) noexcept
  {
    auto *slot = dom::SlotAlgorithms::GetAssignedSlot(Element());
    auto styleScopeOrdinal = ScopeOrdinal::FirstSlot;

    for (; slot; slot = dom::SlotAlgorithms::GetAssignedSlot(*slot), ++styleScopeOrdinal)
    {
      auto &styleScope = Scope::ForNode(*slot);
      if (!styleScope.GetResolver().RuleSets().IsAuthorStyleDefined())
      {
        continue;
      }

      auto *scopeRules = styleScope.GetResolver().RuleSets().StyleForDeclarationOrigin(origin);
      if (!scopeRules)
      {
        continue;
      }

      MatchRequest scopeMatchRequest(*scopeRules, styleScopeOrdinal);
      CollectMatchingRulesForList(&scopeRules->SlottedPseudoElementRules(), scopeMatchRequest);

      if (styleScopeOrdinal == ScopeOrdinal::SlotLimit)
      {
        break;
      }
    }
  }

  void ElementRuleCollector::MatchPartPseudoElementRules(DeclarationOrigin origin) noexcept
  {
    if (!dom::TreeQueries::ContainingShadowRoot(Element()))
    {
      return;
    }

    bool isUserAgentPart =
      dom::TreeQueries::ContainingShadowRoot(Element())->Mode() == dom::ShadowRootMode::UserAgent
      && Element().UserAgentPart() != dom::DOMStringAtom::Null();

    auto &partMatchingElement = isUserAgentPart ? *dom::TreeQueries::ShadowHost(Element()) : Element();
    // An element without a part attribute has no part names.
    if (!partMatchingElement.HasAttribute(u8"part")
        || !dom::TreeQueries::ContainingShadowRoot(partMatchingElement))
    {
      return;
    }

    MatchPartPseudoElementRulesForScope(partMatchingElement, origin);
  }

  void ElementRuleCollector::MatchPartPseudoElementRulesForScope(const dom::Element &partMatchingElement,
                                                                 DeclarationOrigin origin) noexcept
  {
    auto *element = &partMatchingElement;
    auto styleScopeOrdinal = ScopeOrdinal::Element;

    for (; element; element = dom::TreeQueries::ShadowHost(*element), --styleScopeOrdinal)
    {
      auto &styleScope = Scope::ForNode(const_cast<dom::Element &>(*element));
      if (!styleScope.GetResolver().RuleSets().IsAuthorStyleDefined())
      {
        continue;
      }

      auto *hostRules = styleScope.GetResolver().RuleSets().StyleForDeclarationOrigin(origin);
      if (!hostRules)
      {
        continue;
      }

      MatchRequest scopeMatchRequest(*hostRules, styleScopeOrdinal);
      scopeMatchRequest.MatchingPartPseudoElementRules = true;

      CollectMatchingRulesForList(&hostRules->PartPseudoElementRules(), scopeMatchRequest);

      // Element may only be exposed to styling from enclosing scopes via exportparts attributes.
      // FIXME: The shadow root's part mappings (the host's exportparts attribute); until then no shadow root
      // has any, so parts are never exposed beyond the host's scope.
      if (element != &partMatchingElement)
      {
        break;
      }

      if (styleScopeOrdinal == ScopeOrdinal::ContainingHostLimit)
      {
        break;
      }
    }
  }

  void ElementRuleCollector::MatchUserRules() noexcept
  {
    ClearMatchedRules();

    CollectMatchingRules(DeclarationOrigin::User);

    SortAndTransferMatchedRules(DeclarationOrigin::User);
  }

  void ElementRuleCollector::MatchUARules() noexcept
  {
    // First we match rules from the user agent sheet.
    auto *userAgentStyleSheet =
      _isPrintStyle ? UserAgentStyle::DefaultPrintStyle : UserAgentStyle::DefaultStyle;
    MatchUARules(*userAgentStyleSheet);

    // In quirks mode, we match rules from the quirks user agent sheet.
    if (Element().NodeDocument().InQuirksMode())
    {
      MatchUARules(*UserAgentStyle::DefaultQuirksStyle);
    }

    if (_userAgentMediaQueryStyle)
    {
      MatchUARules(*_userAgentMediaQueryStyle);
    }

    if (_dynamicViewTransitionsStyle)
    {
      MatchUARules(*_dynamicViewTransitionsStyle);
    }
  }

  void ElementRuleCollector::MatchUARules(const RuleSet &rules) noexcept
  {
    ClearMatchedRules();

    CollectMatchingRules(MatchRequest(rules));

    SortAndTransferMatchedRules(DeclarationOrigin::UserAgent);
  }

  inline bool ElementRuleCollector::RuleMatches(const RuleData &ruleData, uint32 &specificity,
                                                ScopeOrdinal styleScopeOrdinal,
                                                const Maybe<ScopingRootWithDistance> &scopingRoot) noexcept
  {
    // We know a sufficiently simple single part selector matches simply because we found it from the rule
    // hash when filtering the RuleSet. This is limited to HTML only so we don't need to check the namespace
    // (because of tag name match).
    auto matchBasedOnRuleHash = ruleData.GetMatchBasedOnRuleHash();
    if (matchBasedOnRuleHash != MatchBasedOnRuleHash::None && Is<html::HTMLElement>(Element()))
    {
      krys_debug_assert_msg(
        !_pseudoElementRequest,
        "If we match based on the rule hash while collecting for a particular pseudo element "
        "ID, we would add incorrect rules for that pseudo element ID. We should never end in "
        "RuleMatches() with a pseudo element if the ruleData cannot match any pseudo element.");

      switch (matchBasedOnRuleHash)
      {
        case MatchBasedOnRuleHash::None:      krys_debug_assert(false); break;
        case MatchBasedOnRuleHash::Universal: specificity = 0; break;
        case MatchBasedOnRuleHash::ClassA:
          specificity = ToUnderlying(css::SelectorSpecificityIncrement::ClassA);
          break;
        case MatchBasedOnRuleHash::ClassB:
          specificity = ToUnderlying(css::SelectorSpecificityIncrement::ClassB);
          break;
        case MatchBasedOnRuleHash::ClassC:
          specificity = ToUnderlying(css::SelectorSpecificityIncrement::ClassC);
          break;
      }
      return true;
    }

    css::SelectorChecker::CheckingContext context(_mode);
    if (_pseudoElementRequest)
    {
      auto &pseudoElementIdentifier = _pseudoElementRequest->Identifier();
      context.SetRequestedPseudoElement(pseudoElementIdentifier);
      // FIXME: The class list of the captured element for named view transition pseudo-elements
      // (IsNamedViewTransitionPseudoElement), once documents have an active view transition.
    }
    context.StyleScopeOrdinal = styleScopeOrdinal;
    context.SelectorMatchingState = _selectorMatchingState;
    if (scopingRoot)
    {
      context.Scope = scopingRoot->ScopingRoot;
      context.ScopingRootMatchesVisited = scopingRoot->MatchesVisited;
    }

    auto *selector = ruleData.Selector();
    // Slow path.
    css::SelectorChecker selectorChecker(Element().NodeDocument());
    bool selectorMatches = selectorChecker.Match(*selector, Element(), context);
    if (selectorMatches)
    {
      specificity = selector->ComputeSpecificity();
    }

    _matchedPseudoElements.Add(context.PublicPseudoElements);
    _styleRelations.append(context.StyleRelations.begin(), context.StyleRelations.end());

    return selectorMatches;
  }

  void ElementRuleCollector::CollectMatchingRulesForList(const RuleSet::RuleDataVector *rules,
                                                         const MatchRequest &matchRequest) noexcept
  {
    if (!rules)
    {
      return;
    }

    for (auto &ruleData : *rules)
    {
      if (!ruleData.IsEnabled()) KRYS_UNLIKELY
      {
        continue;
      }

      if (!ruleData.CanMatchPseudoElement() && _pseudoElementRequest)
      {
        continue;
      }

      if (_selectorMatchingState
          && _selectorMatchingState->SelectorFilter.FastRejectSelector(
            ruleData.DescendantSelectorIdentifierHashes()))
      {
        continue;
      }

      if (matchRequest.RuleSet.HasContainerQueries() && !ContainerQueriesMatch(ruleData, matchRequest))
      {
        continue;
      }

      Maybe<List<ScopingRootWithDistance>> scopingRoots;
      if (matchRequest.RuleSet.HasScopeRules())
      {
        auto [result, roots] = ScopeRulesMatch(ruleData, matchRequest);
        if (!result)
        {
          continue;
        }
        scopingRoots = krys::move(roots);
      }

      auto &rule = ruleData.StyleRule();

      // If the rule has no properties to apply, then ignore it in the non-debug mode.
      if (rule.Properties().empty() && !_shouldIncludeEmptyRules)
      {
        continue;
      }

      auto addRuleIfMatches = [&](const ScopingRootWithDistance &scopingRootWithDistance = {})
      {
        uint32 specificity {0};
        if (RuleMatches(ruleData, specificity, matchRequest.StyleScopeOrdinal, scopingRootWithDistance))
        {
          AddMatchedRule(ruleData, specificity, scopingRootWithDistance.Distance, matchRequest);
        }
      };

      if (scopingRoots)
      {
        for (auto &scopingRoot : *scopingRoots)
        {
          addRuleIfMatches(scopingRoot);
        }
        continue;
      }

      addRuleIfMatches();
      if (IsFirstMatchModeAndHasMatchedAnyRules())
      {
        return;
      }
    }
  }

  bool ElementRuleCollector::ContainerQueriesMatch(const RuleData &ruleData,
                                                   const MatchRequest &matchRequest) const noexcept
  {
    auto queries = matchRequest.RuleSet.ContainerQueriesFor(ruleData);

    if (queries.empty())
    {
      return true;
    }

    // Style bits indicating which pseudo-elements match are set during regular element matching. Container
    // queries need to be evaluate in the right mode.
    auto selectionMode = [&]
    {
      if (matchRequest.MatchingPartPseudoElementRules)
      {
        return ContainerQueryEvaluator::SelectionMode::PartPseudoElement;
      }
      if (ruleData.CanMatchPseudoElement()
          && !css::ComplexSelectorMatchesElementBackedPseudoElement(*ruleData.Selector()))
      {
        return ContainerQueryEvaluator::SelectionMode::PseudoElement;
      }
      return ContainerQueryEvaluator::SelectionMode::Element;
    }();

    auto *containerQueryEvaluationState =
      _selectorMatchingState ? &_selectorMatchingState->ContainerQueryEvaluationState : nullptr;

    // "Style rules defined on an element inside multiple nested container queries apply when all of the
    // wrapping container queries are true for that element."
    ContainerQueryEvaluator evaluator(Element(), selectionMode, matchRequest.StyleScopeOrdinal,
                                      containerQueryEvaluationState);
    for (auto *query : queries)
    {
      if (!evaluator.Evaluate(*query))
      {
        return false;
      }
    }
    return true;
  }

  std::pair<bool, Maybe<List<ElementRuleCollector::ScopingRootWithDistance>>>
    ElementRuleCollector::ScopeRulesMatch(const RuleData &ruleData,
                                          const MatchRequest &matchRequest) const noexcept
  {
    auto scopeRules = matchRequest.RuleSet.ScopeRulesFor(ruleData);

    if (scopeRules.empty())
    {
      return {true, {}};
    }

    css::SelectorChecker checker(Element().NodeDocument());
    css::SelectorChecker::CheckingContext context(_mode);
    context.StyleScopeOrdinal = matchRequest.StyleScopeOrdinal;

    List<ScopingRootWithDistance> scopingRoots;
    auto isWithinScope = [&](const css::ScopeRule &rule)
    {
      auto previousScopingRoots = krys::move(scopingRoots);
      // The last rule (=innermost @scope rule) determines the scoping roots
      scopingRoots.clear();

      auto findScopingRoots = [&](const css::SelectorList &selectorList)
      {
        uint32 distance = 0;
        const dom::Element *ancestor = &Element();
        bool shadowHostCrossed = false;
        while (ancestor && !shadowHostCrossed)
        {
          auto subContext = context;
          if (dom::TreeQueries::GetShadowRoot(*ancestor))
          {
            subContext.StyleScopeOrdinal = ScopeOrdinal::Shadow;
            shadowHostCrossed = true;
          }
          for (auto &selector : selectorList)
          {
            auto appendIfMatch = [&](const ScopingRootWithDistance *previousScopingRoot = nullptr)
            {
              if (previousScopingRoot)
              {
                subContext.Scope = previousScopingRoot->ScopingRoot;
              }
              // Reset visited flag for each scoping root evaluation
              subContext.ScopingRootMatchesVisited = false;
              subContext.IsEvaluatingScopingRoot = true;
              auto match = checker.Match(selector, *ancestor, subContext);
              if (match)
              {
                scopingRoots.push_back(
                  {ShareRefPtr(ancestor), distance, subContext.ScopingRootMatchesVisited});
              }
            };
            if (previousScopingRoots.empty())
            {
              appendIfMatch();
            }
            else
            {
              for (auto &previousScopingRoot : previousScopingRoots)
              {
                appendIfMatch(&previousScopingRoot);
              }
            }
          }

          ++distance;
          ancestor = dom::TreeQueries::ParentOrShadowHostElement(*ancestor);
        }
      };

      // An element is in scope if:
      //     It is an inclusive descendant of the scoping root, and
      //     It is not an inclusive descendant of a scoping limit.
      auto isWithinScopingRootsAndScopeEnd = [&](const css::SelectorList &scopeEnd)
      {
        auto match = [&](const dom::ContainerNode *scopingRoot, const css::Selector &selector)
        {
          auto subContext = context;
          subContext.Scope = ShareRefPtr(scopingRoot);
          const dom::Element *ancestor = &Element();
          while (ancestor)
          {
            if (checker.Match(selector, *ancestor, subContext))
            {
              return true;
            }
            if (ancestor == scopingRoot)
            {
              // The end of the scope can't be an ancestor of the start of the scope.
              return false;
            }
            ancestor = dom::TreeQueries::ParentOrShadowHostElement(*ancestor);
          }
          return false;
        };

        List<ScopingRootWithDistance> scopingRootsWithinScope;
        for (auto &scopingRootWithDistance : scopingRoots)
        {
          bool anyScopingLimitMatch = false;
          for (auto &selector : scopeEnd)
          {
            if (match(scopingRootWithDistance.ScopingRoot.get(), selector))
            {
              anyScopingLimitMatch = true;
              break;
            }
          }
          if (!anyScopingLimitMatch)
          {
            scopingRootsWithinScope.push_back(scopingRootWithDistance);
          }
        }
        return scopingRootsWithinScope;
      };

      auto &scopeStart = rule.ScopeStart();
      if (!scopeStart.IsEmpty())
      {
        findScopingRoots(scopeStart);
        if (scopingRoots.empty())
        {
          return false;
        }
      }
      else
      {
        // The scoping root is the parent element of the owner node of the stylesheet where the @scope rule is
        // defined. (If no such element exists, then the scoping root is the root of the containing node tree.
        auto styleSheetContents = rule.GetStyleSheetContents();
        if (!styleSheetContents)
        {
          return false;
        }

        auto *rootStyleSheet = styleSheetContents->RootStyleSheet();
        krys_debug_assert(rootStyleSheet);
        if (!rootStyleSheet)
        {
          return false;
        }

        auto appendImplicitScopingRoot = [&](const css::CSSStyleSheet &client)
        {
          auto addScopingRootWithDistance = [&](const dom::Element &scopingRoot)
          {
            const dom::Element *ancestor = &Element();
            uint32 distance = 0;
            while (ancestor)
            {
              if (ancestor == &scopingRoot)
              {
                break;
              }
              ancestor = ancestor->ParentElement();
              ++distance;
            }
            scopingRoots.push_back({ShareRefPtr(&scopingRoot), distance});
          };

          if (auto *owner = client.OwnerNode())
          {
            // Regular stylesheet with owner node (e.g., <style> element).
            // Find the parent node of the <style>.
            auto *implicitParentNode = dom::TreeQueries::ParentOrShadowHostElement(*owner);
            if (!implicitParentNode)
            {
              return;
            }
            addScopingRootWithDistance(*implicitParentNode);
          }
          else
          {
            // Constructed stylesheet without owner node.
            // Check if it's adopted by shadow roots and use the shadow host as scoping root.
            for (auto &adoptingTreeScope : client.AdoptingTreeScopes())
            {
              if (auto *shadowRoot = DynamicDowncast<dom::ShadowRoot>(adoptingTreeScope.get()))
              {
                auto *shadowHost = shadowRoot->Host();
                if (shadowHost && &shadowHost->NodeDocument() == &Element().NodeDocument())
                {
                  addScopingRootWithDistance(*shadowHost);
                }
              }
            }
          }
        };

        // Each client might act as a scoping root.
        for (auto *client : rootStyleSheet->Clients())
        {
          appendImplicitScopingRoot(*client);
        }
      }

      auto &scopeEnd = rule.ScopeEnd();
      if (!scopeEnd.IsEmpty())
      {
        auto scopingRootsWithinScope = isWithinScopingRootsAndScopeEnd(scopeEnd);
        if (scopingRootsWithinScope.empty())
        {
          return false;
        }
        scopingRoots = krys::move(scopingRootsWithinScope);
      }
      // element is in the @scope donut
      return true;
    };

    // We need to respect each nested @scope to collect this rule
    for (auto &rule : scopeRules)
    {
      if (!isWithinScope(*rule))
      {
        return {false, {}};
      }
    }

    return {true, krys::move(scopingRoots)};
  }

  namespace
  {
    KRYS_NODISCARD bool CompareRules(const MatchedRule &r1, const MatchedRule &r2) noexcept
    {
      // For normal properties the earlier scope wins. This may be reversed by !important which is handled
      // when resolving cascade.
      if (r1.StyleScopeOrdinal != r2.StyleScopeOrdinal)
      {
        return r1.StyleScopeOrdinal > r2.StyleScopeOrdinal;
      }

      if (r1.CascadeLayerPriority != r2.CascadeLayerPriority)
      {
        return r1.CascadeLayerPriority < r2.CascadeLayerPriority;
      }

      if (r1.Specificity != r2.Specificity)
      {
        return r1.Specificity < r2.Specificity;
      }

      // Rule with the smallest distance has priority.
      if (r1.ScopingRootDistance != r2.ScopingRootDistance)
      {
        return r2.ScopingRootDistance < r1.ScopingRootDistance;
      }

      return r1.RuleData->Position() < r2.RuleData->Position();
    }
  }

  void ElementRuleCollector::SortMatchedRules() noexcept
  {
    std::ranges::sort(_matchedRules, CompareRules);
  }

  void ElementRuleCollector::MatchAllRules(bool matchAuthorAndUserStyles, bool includeSMILProperties) noexcept
  {
    MatchUARules();

    // Now we check user sheet rules.
    if (matchAuthorAndUserStyles)
    {
      MatchUserRules();
    }

    if (auto *styledElement = DynamicDowncast<dom::StyledElement>(Element()))
    {
      if (auto *presentationalHintStyle = styledElement->PresentationalHintStyle())
      {
        // https://html.spec.whatwg.org/#presentational-hints

        // Presentation attributes in SVG elements tend to be unique and not restyled often. Avoid bloating
        // the cache.
        // FIXME: Refcount is an imperfect proxy for sharing within a single document.
        static constexpr auto matchedDeclarationsCacheSharingThreshold = 4;
        bool allowFullCaching =
          !Is<svg::SVGElement>(*styledElement)
          || presentationalHintStyle->GetRefCount() > matchedDeclarationsCacheSharingThreshold;

        auto isCacheable = allowFullCaching ? IsCacheable::Yes : IsCacheable::Partially;
        AddElementStyleProperties(presentationalHintStyle,
                                  RuleSet::CascadeLayerPriorityForPresentationalHints, isCacheable);
      }

      // Tables and table cells share an additional presentation style that must be applied
      // after all attributes, since their style depends on the values of multiple attributes.
      AddElementStyleProperties(styledElement->AdditionalPresentationalHintStyle(),
                                RuleSet::CascadeLayerPriorityForPresentationalHints);

      if (auto *htmlElement = DynamicDowncast<html::HTMLElement>(*styledElement))
      {
        if (auto textDirection = ComputeTextDirectionIfDirIsAuto(*htmlElement))
        {
          auto &properties =
            *textDirection == TextDirection::LTR ? LeftToRightDeclaration() : RightToLeftDeclaration();
          AddMatchedProperties({ShareRef(properties)}, DeclarationOrigin::Author);
        }
      }
    }

    if (matchAuthorAndUserStyles)
    {
      ClearMatchedRules();

      CollectMatchingRules(DeclarationOrigin::Author);
      SortMatchedRules();

      TransferMatchedRules(DeclarationOrigin::Author, ScopeOrdinal::Element);

      // Inline style behaves as if it has higher specificity than any rule.
      AddElementInlineStyleProperties(includeSMILProperties);

      // Rules from the host scopes override inline style.
      TransferMatchedRules(DeclarationOrigin::Author);
    }
  }

  void ElementRuleCollector::AddElementInlineStyleProperties(bool includeSMILProperties) noexcept
  {
    auto *styledElement = DynamicDowncast<dom::StyledElement>(Element());
    if (!styledElement)
    {
      return;
    }

    if (auto *inlineStyle = styledElement->InlineStyle())
    {
      auto isInlineStyleCacheable = inlineStyle->IsMutable() ? IsCacheable::No : IsCacheable::Yes;
      AddElementStyleProperties(inlineStyle, RuleSet::CascadeLayerPriorityForUnlayered,
                                isInlineStyleCacheable, FromStyleAttribute::Yes);
    }

    // FIXME: SVG elements' animated SMIL style properties, once there is SMIL.
    (void)includeSMILProperties;
  }

  void ElementRuleCollector::AddMatchedProperties(MatchedProperties &&matchedProperties,
                                                  DeclarationOrigin origin) noexcept
  {
    auto &declarations = DeclarationsForOrigin(origin);
    if (!declarations.empty() && declarations.back() == matchedProperties)
    {
      // It might also be beneficial to overwrite the previous declaration (insteading of appending) if it
      // affects the same exact properties.
      return;
    }
    if (matchedProperties.UsedRuleTypes.Contains(UsedRuleType::StartingStyle))
    {
      _result->UsedRuleTypes.Add(UsedRuleType::StartingStyle);
    }

    if (matchedProperties.IsCacheable == IsCacheable::Partially && !_result->IsCompletelyNonCacheable)
    {
      for (auto property : *matchedProperties.Properties)
      {
        _result->NonCacheablePropertyIds.push_back(property.Id());
      }
    }
    if (matchedProperties.IsCacheable == IsCacheable::No)
    {
      _result->IsCompletelyNonCacheable = true;
      _result->NonCacheablePropertyIds.clear();
    }

    declarations.push_back(krys::move(matchedProperties));
  }

  void ElementRuleCollector::AddAuthorKeyframeRules(const css::KeyframeRule &keyframe) noexcept
  {
    krys_debug_assert(_result->AuthorDeclarations.empty());
    auto propertyAllowlist = _pseudoElementRequest
                               ? PropertyAllowlistForPseudoElement(_pseudoElementRequest->GetType())
                               : PropertyAllowlist::None;
    _result->AuthorDeclarations.push_back(
      {ShareRef(keyframe.Properties()), css::SelectorChecker::MatchAll, propertyAllowlist});
  }
}
