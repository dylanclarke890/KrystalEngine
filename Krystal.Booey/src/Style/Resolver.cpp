#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/Selectors/MutableSelector.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"
#include "Krystal.Booey/CSS/WebKitFontFamilyNames.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/VisitedLinkState.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/Adjuster.hpp"
#include "Krystal.Booey/Style/Builder.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/CustomPropertyRegistry.hpp"
#include "Krystal.Booey/Style/ElementRuleCollector.hpp"
#include "Krystal.Booey/Style/FontSizeFunctions.hpp"
#include "Krystal.Booey/Style/MatchResultCache.hpp"
#include "Krystal.Booey/Style/PageRuleCollector.hpp"
#include "Krystal.Booey/Style/ResolvedStyle.hpp"
#include "Krystal.Booey/Style/RuleSetBuilder.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/UserAgentStyle.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo::style
{
  class Resolver::State
  {
  private:
    const dom::Element *_element {};
    UniquePtr<RenderStyle> _style;
    const RenderStyle *_parentStyle {};
    UniquePtr<const RenderStyle> _ownedParentStyle;
    const RenderStyle *_rootElementStyle {};
    // FIXME: The tree resolution state (the anchor-positioned states) comes with anchor positioning.

  public:
    State() noexcept = default;

    State(const dom::Element &element, const RenderStyle *parentStyle,
          const RenderStyle *documentElementStyle) noexcept
        : _element(&element), _parentStyle(parentStyle)
    {
      krys_debug_assert(element.IsConnected());

      auto &document = element.NodeDocument();
      auto documentElement = document.DocumentElement();
      if (!documentElement || documentElement.get() == &element)
      {
        _rootElementStyle = document.InitialContainingBlockStyle();
      }
      else if (documentElementStyle)
      {
        _rootElementStyle = documentElementStyle;
      }
      else if (auto *documentElementRenderStyle = documentElement->GetRenderStyle())
      {
        _rootElementStyle = documentElementRenderStyle;
      }
      else
      {
        _rootElementStyle = document.InitialContainingBlockStyle();
      }
    }

    State(State &&) noexcept = default;
    State &operator=(State &&) noexcept = default;

    KRYS_NODISCARD const dom::Element *Element() const noexcept
    {
      return _element;
    }

    void SetStyle(UniquePtr<RenderStyle> style) noexcept
    {
      _style = krys::move(style);
    }

    KRYS_NODISCARD RenderStyle *Style() const noexcept
    {
      return _style.get();
    }

    KRYS_NODISCARD UniquePtr<RenderStyle> TakeStyle() noexcept
    {
      return krys::move(_style);
    }

    void SetParentStyle(UniquePtr<RenderStyle> parentStyle) noexcept
    {
      _ownedParentStyle = krys::move(parentStyle);
      _parentStyle = _ownedParentStyle.get();
    }

    KRYS_NODISCARD const RenderStyle *ParentStyle() const noexcept
    {
      return _parentStyle;
    }

    KRYS_NODISCARD const RenderStyle *RootElementStyle() const noexcept
    {
      return _rootElementStyle;
    }
  };

  Ref<Resolver> Resolver::Create(dom::Document &document, ScopeType scopeType) noexcept
  {
    return AdoptRef(*new Resolver(document, scopeType));
  }

  Resolver::Resolver(dom::Document &document, ScopeType scopeType) noexcept
      : _document(WeakPtr<dom::Document>::WithRef(document)), _scopeType(scopeType), _ruleSets(*this),
        _matchedDeclarationsCache(*this)
  {
    // FIXME: Settings: authorAndUserStylesEnabled (on by default).
    Initialize();
  }

  void Resolver::Initialize() noexcept
  {
    UserAgentStyle::InitDefaultStyleSheet();

    // construct document root element default style. this is needed
    // to evaluate media queries that contain relative constraints, like "screen and (max-width: 10em)"
    // This is here instead of constructor, because when constructor is run,
    // document doesn't have documentElement
    // NOTE: this assumes that element that gets passed to styleForElement -call
    // is always from the document that owns the style selector
    auto *view = Document().View();
    if (view)
    {
      _mediaQueryEvaluator = css::mq::MediaQueryEvaluator {view->MediaType()};
    }
    else
    {
      _mediaQueryEvaluator = css::mq::MediaQueryEvaluator {};
    }

    if (auto documentElement = Document().DocumentElement())
    {
      _rootDefaultStyle = StyleForElement(*documentElement, {Document().InitialContainingBlockStyle()},
                                          RuleMatchingBehavior::MatchOnlyUserAgentRules)
                            .Style;
      // FIXME: Upstream updates the root default style's font cascade with the document's font selector and
      // loads its primary font here; there are no fonts.
    }

    if (_rootDefaultStyle && view)
    {
      _mediaQueryEvaluator =
        css::mq::MediaQueryEvaluator {view->MediaType(), Document(), _rootDefaultStyle.get()};
    }

    _ruleSets.ResetAuthorStyle();
    _ruleSets.ResetUserAgentMediaQueryStyle();
  }

  Resolver::~Resolver() noexcept = default;

  dom::Document &Resolver::Document() noexcept
  {
    return *_document;
  }

  const dom::Document &Resolver::Document() const noexcept
  {
    return *_document;
  }

  void Resolver::AppendAuthorStyleSheets(Span<const RefPtr<css::CSSStyleSheet>> styleSheets) noexcept
  {
    _ruleSets.AppendAuthorStyleSheets(styleSheets, _mediaQueryEvaluator);

    // FIXME: Update the render view's font cascade with the document's font selector, once there are fonts.
  }

  KeyframesRuleMap &Resolver::UserAgentKeyframes() noexcept
  {
    static NeverDestroyed<KeyframesRuleMap> keyframes;
    return keyframes;
  }

  void Resolver::AddUserAgentKeyframeStyle(Ref<css::KeyframesRule> &&rule) noexcept
  {
    const auto animationName = rule->Name();
    UserAgentKeyframes().insert_or_assign(animationName, RefPtr<css::KeyframesRule> {krys::move(rule)});
  }

  void Resolver::AddKeyframeStyle(Ref<css::KeyframesRule> &&rule) noexcept
  {
    const auto animationName = rule->Name();
    _keyframesRuleMap.insert_or_assign(animationName, RefPtr<css::KeyframesRule> {krys::move(rule)});
    DocumentKeyframesRuleDidChange(Document(), css::CSSOMString(animationName.View()));
  }

  auto Resolver::InitializeStateAndStyle(const dom::Element &element, const ResolutionContext &context,
                                         UniquePtr<RenderStyle> &&initialStyle) noexcept -> State
  {
    auto state = State {element, context.ParentStyle, context.DocumentElementStyle};

    if (initialStyle)
    {
      state.SetStyle(krys::move(initialStyle));
    }
    else if (state.ParentStyle())
    {
      state.SetStyle(RenderStyle::CreatePtrWithRegisteredInitialValues(
        Document().StyleScope().GetCustomPropertyRegistry()));
      if (&element == Document().DocumentElement().get() && !context.IsSVGUseTreeRoot)
      {
        // Initial values for custom properties are inserted to the document element style. Don't overwrite
        // them.
        state.Style()->InheritIgnoringCustomPropertiesFrom(*state.ParentStyle());
      }
      else
      {
        state.Style()->InheritFrom(*state.ParentStyle());
      }
    }
    else
    {
      state.SetStyle(DefaultStyleForElement(Document(), &element));
      state.SetParentStyle(RenderStyle::ClonePtr(*state.Style()));
    }

    // BuilderState::useSVGZoomRulesForLength equivalent
    if (auto *svgElement = DynamicDowncast<svg::SVGElement>(element);
        svgElement && !(Is<svg::SVGSvgElement>(*svgElement) && element.ParentNode()))
    {
      state.Style()->SetUseSVGZoomRulesForLength(true);
    }

    auto linkState = Document().GetVisitedLinkState().DetermineLinkState(element);
    if (linkState != InsideLink::NotInside)
    {
      auto &style = *state.Style();
      style.SetIsLink(true);
      // FIXME: The inspector can force `:visited` (forcePseudoState); there's no inspector.
      style.SetInsideLink(linkState);
    }

    return state;
  }

  BuilderContext Resolver::GetBuilderContext(State &state) const noexcept
  {
    return BuilderContext {.Document = ShareRefPtr(&Document()),
                           .ParentStyle = state.ParentStyle(),
                           .RootElementStyle = state.RootElementStyle(),
                           .Element = ShareRefPtr(state.Element())};
  }

  UnadjustedStyle Resolver::UnadjustedStyleForElement(dom::Element &element, const ResolutionContext &context,
                                                      RuleMatchingBehavior matchingBehavior) noexcept
  {
    auto state = InitializeStateAndStyle(element, context);
    auto &style = *state.Style();

    UserAgentStyle::EnsureDefaultStyleSheetsForElement(element);

    ElementRuleCollector collector(element, _ruleSets, context.SelectorMatchingState);
    collector.SetMedium(_mediaQueryEvaluator);

    if (matchingBehavior == RuleMatchingBehavior::MatchOnlyUserAgentRules)
    {
      collector.MatchUARules();
    }
    else
    {
      collector.MatchAllRules(_matchAuthorAndUserStyles,
                              matchingBehavior != RuleMatchingBehavior::MatchAllRulesExcludingSMIL);
    }

    if (collector.MatchedPseudoElements())
    {
      style.SetHasPseudoStyles(collector.MatchedPseudoElements());
    }

    auto elementStyleRelations = CommitRelationsToRenderStyle(style, element, collector.StyleRelations());

    ApplyMatchedProperties(state, collector.GetMatchResult(), PropertyCascade::NormalProperties());

    return {.Style = state.TakeStyle(),
            .Relations = krys::move(elementStyleRelations),
            .MatchResult = collector.ReleaseMatchResult()};
  }

  ResolvedStyle Resolver::StyleForElement(dom::Element &element, const ResolutionContext &context,
                                          RuleMatchingBehavior matchingBehavior) noexcept
  {
    auto unadjustedStyle = UnadjustedStyleForElement(element, context, matchingBehavior);
    auto &parentStyle = context.ParentStyle ? *context.ParentStyle : RenderStyle::DefaultStyleSingleton();

    auto style = krys::move(unadjustedStyle.Style);

    Adjuster adjuster(Document(), parentStyle, context.ParentBoxStyle, &element);
    adjuster.Adjust(*style);

    return {.Style = krys::move(style),
            .Relations = krys::move(unadjustedStyle.Relations),
            .MatchResult = krys::move(unadjustedStyle.MatchResult)};
  }

  UnadjustedStyle Resolver::UnadjustedStyleForCachedMatchResult(dom::Element &element,
                                                                const ResolutionContext &context,
                                                                CachedMatchResult &&cachedResult) noexcept
  {
    auto &unadjustedStyle = cachedResult.UnadjustedStyle;

    if (cachedResult.ChangedProperties.IsEmpty())
    {
      // The cached result can be used as-is.
      return krys::move(unadjustedStyle);
    }

    bool applyPartially = !cachedResult.ChangedProperties.Ids.empty();

    auto state =
      InitializeStateAndStyle(element, context, applyPartially ? krys::move(unadjustedStyle.Style) : nullptr);
    if (!applyPartially)
    {
      state.Style()->CopyPseudoElementBitsFrom(*unadjustedStyle.Style);
      CopyRelations(*state.Style(), *unadjustedStyle.Style);
    }

    ApplyMatchedProperties(state, *unadjustedStyle.MatchResult, krys::move(cachedResult.ChangedProperties));

    return {.Style = state.TakeStyle(),
            .Relations = krys::move(unadjustedStyle.Relations),
            .MatchResult = unadjustedStyle.MatchResult};
  }

  bool Resolver::IsAnimationNameValid(const css::CSSOMString &name) const noexcept
  {
    auto atom = css::CSSOMStringAtom(name);
    return _keyframesRuleMap.contains(atom) || UserAgentKeyframes().contains(atom);
  }

  bool Resolver::HasSelectorForId(const css::CSSOMStringAtom &idValue) const noexcept
  {
    krys_debug_assert(!idValue.View().empty());
    return _ruleSets.Features().IdsInRules.contains(idValue);
  }

  bool Resolver::HasSelectorForAttribute(const dom::Element &element,
                                         const css::CSSOMStringAtom &attributeName) const noexcept
  {
    krys_debug_assert(!attributeName.View().empty());
    if (Is<html::HTMLElement>(element) && Is<dom::HTMLDocument>(element.NodeDocument()))
    {
      return _ruleSets.Features().AttributeLowercaseLocalNamesInRules.contains(attributeName);
    }
    return _ruleSets.Features().AttributeLocalNamesInRules.contains(attributeName);
  }

  bool Resolver::HasViewportDependentMediaQueries() const noexcept
  {
    return _ruleSets.HasViewportDependentMediaQueries();
  }

  Maybe<DynamicMediaQueryEvaluationChanges> Resolver::EvaluateDynamicMediaQueries() noexcept
  {
    return _ruleSets.EvaluateDynamicMediaQueryRules(_mediaQueryEvaluator);
  }

  Maybe<ResolvedStyle> Resolver::StyleForPseudoElement(dom::Element &element,
                                                       const PseudoElementRequest &pseudoElementRequest,
                                                       const ResolutionContext &context) noexcept
  {
    auto state = State(element, context.ParentStyle, context.DocumentElementStyle);

    if (state.ParentStyle())
    {
      state.SetStyle(RenderStyle::CreatePtrWithRegisteredInitialValues(
        Document().StyleScope().GetCustomPropertyRegistry()));
      state.Style()->InheritFrom(*state.ParentStyle());
    }
    else
    {
      state.SetStyle(DefaultStyleForElement(Document(), &element));
      state.SetParentStyle(RenderStyle::ClonePtr(*state.Style()));
    }

    ElementRuleCollector collector(element, _ruleSets, context.SelectorMatchingState);
    collector.SetPseudoElementRequest(pseudoElementRequest);
    collector.SetMedium(_mediaQueryEvaluator);
    collector.MatchUARules();

    if (_matchAuthorAndUserStyles)
    {
      collector.MatchUserRules();
      collector.MatchAuthorRules();
    }

    krys_debug_assert(!collector.MatchedPseudoElements());

    if (collector.GetMatchResult().IsEmpty())
    {
      return {};
    }

    state.Style()->SetPseudoElementIdentifier(
      Maybe<PseudoElementIdentifier> {pseudoElementRequest.Identifier()});

    ApplyMatchedProperties(state, collector.GetMatchResult(), PropertyCascade::NormalProperties());

    Adjuster adjuster(Document(), *state.ParentStyle(), context.ParentBoxStyle, nullptr);
    adjuster.Adjust(*state.Style());

    // FIXME: Adjuster::adjustVisibilityForPseudoElement comes with the visibility adjustment of elements.

    return ResolvedStyle {state.TakeStyle(), nullptr, collector.ReleaseMatchResult()};
  }

  UniquePtr<RenderStyle> Resolver::StyleForPage(int32 pageIndex) noexcept
  {
    auto documentElement = Document().DocumentElement();
    if (!documentElement || !documentElement->GetRenderStyle())
    {
      return RenderStyle::CreatePtr();
    }

    auto state = State(*documentElement, Document().InitialContainingBlockStyle(), nullptr);

    state.SetStyle(RenderStyle::CreatePtr());
    state.Style()->InheritFrom(*state.RootElementStyle());

    PageRuleCollector collector(_ruleSets, documentElement->GetRenderStyle()->GetWritingMode());
    collector.MatchAllPageRules(pageIndex);

    auto &result = collector.GetMatchResult();

    Builder builder(*state.Style(), GetBuilderContext(state), result);
    builder.ApplyAllProperties();

    // Now return the style.
    return state.TakeStyle();
  }

  UniquePtr<RenderStyle> Resolver::DefaultStyleForElement(const dom::Document &document,
                                                          const dom::Element *element) noexcept
  {
    auto style =
      RenderStyle::CreatePtrWithRegisteredInitialValues(document.StyleScope().GetCustomPropertyRegistry());

    FontCascadeDescription fontDescription;
    fontDescription.SetOneFamily(css::WebKitFontFamilyNames::StandardFamily());
    fontDescription.SetKeywordSizeFromIdentifier(css::ValueId::Medium);

    auto size = FontSizeForKeyword(static_cast<uint32>(css::ValueId::Medium), false, document);
    fontDescription.SetSpecifiedSize(size);
    auto computedFontSize =
      ComputedFontSizeFromSpecifiedSize(size, fontDescription.IsAbsoluteSize(),
                                        element && Is<svg::SVGElement>(*element), style.get(), document);
    fontDescription.SetComputedSize(computedFontSize.Size, computedFontSize.UsedZoomFactor);

    // FIXME: The settings' shouldAllowUserInstalledFonts comes with Settings (its default, Yes, is the
    // description's).
    style->SetFontDescription(krys::move(fontDescription));

    return style;
  }

  List<RefPtr<const css::StyleRule>> Resolver::StyleRulesForElement(const dom::Element *element,
                                                                    uint32 rulesToInclude) noexcept
  {
    return PseudoStyleRulesForElement(element, {}, rulesToInclude);
  }

  List<RefPtr<const css::StyleRule>>
    Resolver::PseudoStyleRulesForElement(const dom::Element *element,
                                         const Maybe<PseudoElementRequest> &pseudoElementIdentifier,
                                         uint32 rulesToInclude) noexcept
  {
    if (!element)
    {
      return {};
    }

    ElementRuleCollector collector(*element, _ruleSets, nullptr, css::SelectorChecker::Mode::CollectingRules);
    if (pseudoElementIdentifier)
    {
      collector.SetPseudoElementRequest(*pseudoElementIdentifier);
    }
    collector.SetMedium(_mediaQueryEvaluator);
    collector.SetIncludeEmptyRules(rulesToInclude & EmptyCSSRules);

    if (rulesToInclude & UAAndUserCSSRules)
    {
      // First we match rules from the user agent sheet.
      collector.MatchUARules();

      // Now we check user sheet rules.
      if (_matchAuthorAndUserStyles)
      {
        collector.MatchUserRules();
      }
    }

    if (_matchAuthorAndUserStyles && (rulesToInclude & AuthorCSSRules))
    {
      collector.MatchAuthorRules();
    }

    return collector.MatchedRuleList();
  }

  void Resolver::InvalidateMatchedDeclarationsCache() noexcept
  {
    _matchedDeclarationsCache.Invalidate();
  }

  void Resolver::ClearCachedDeclarationsAffectedByViewportUnits() noexcept
  {
    _matchedDeclarationsCache.ClearEntriesAffectedByViewportUnits();
  }

  void Resolver::ApplyMatchedProperties(State &state, const MatchResult &matchResult,
                                        PropertyCascade::IncludedProperties &&includedProperties) noexcept
  {
    auto &style = *state.Style();
    auto &parentStyle = *state.ParentStyle();
    auto &element = *state.Element();

    auto cacheHash =
      MatchedDeclarationsCache::ComputeHash(matchResult, parentStyle.InheritedCustomProperties());

    auto cacheResult = _matchedDeclarationsCache.Find(cacheHash, matchResult,
                                                      parentStyle.InheritedCustomProperties(), parentStyle);

    auto hasUsableEntry = cacheResult && MatchedDeclarationsCache::IsCacheable(element, style, parentStyle);
    if (hasUsableEntry)
    {
      auto &cacheEntry = cacheResult->Entry;
      bool inheritedEqual = cacheResult->InheritedEqual;
      // We can build up the style by copying non-inherited properties from an earlier style object built
      // using the same exact style declarations. We then only need to apply the inherited properties, if any,
      // as their values can depend on the element context. This is fast and saves memory by reusing the style
      // data structures.
      style.CopyNonInheritedFrom(*cacheEntry.RenderStyle);

      bool hasExplicitlyInherited = cacheEntry.RenderStyle->HasExplicitlyInheritedProperties();
      bool explicitlyInheritedEqual =
        !hasExplicitlyInherited || parentStyle.NonInheritedEqual(*cacheEntry.ParentRenderStyle);

      if (inheritedEqual)
      {
        auto linkStatus = style.GetInsideLink();
        // If the cache item parent style has identical inherited properties to the current parent style then
        // the resulting style will be identical too. We copy the inherited properties over from the cache and
        // are done.
        style.InheritFrom(*cacheEntry.RenderStyle);

        // Link status is treated like an inherited property. We need to explicitly restore it.
        style.SetInsideLink(linkStatus);
      }

      includedProperties = {};

      if (!inheritedEqual)
      {
        includedProperties.Types.Add(PropertyCascade::PropertyType::Inherited);
      }
      if (!explicitlyInheritedEqual)
      {
        includedProperties.Types.Add(PropertyCascade::PropertyType::ExplicitlyInherited);
      }
      if (!matchResult.NonCacheablePropertyIds.empty())
      {
        includedProperties.Types.Add(PropertyCascade::PropertyType::NonCacheable);
      }

      if (includedProperties.IsEmpty())
      {
        return;
      }
    }

    Builder builder(style, GetBuilderContext(state), matchResult, krys::move(includedProperties));

    // Top priority properties may affect resolution of high priority ones.
    builder.ApplyTopPriorityProperties();

    // High priority properties may affect resolution of other properties (they are mostly font related).
    builder.ApplyHighPriorityProperties();

    if (cacheResult && !cacheResult->Entry.IsUsableAfterHighPriorityProperties(style))
    {
      // High-priority properties may affect resolution of other properties. Kick out the existing cache entry
      // and try again.
      _matchedDeclarationsCache.Remove(cacheHash);
      ApplyMatchedProperties(state, matchResult, PropertyCascade::NormalProperties());
      return;
    }

    builder.ApplyNonHighPriorityProperties();
    builder.AdjustAfterApplying();

    SetGlobalStateAfterApplyingProperties(builder.State());

    if (((cacheHash && !cacheResult) || (cacheResult && !cacheResult->InheritedEqual))
        && MatchedDeclarationsCache::IsCacheable(element, style, parentStyle))
    {
      _matchedDeclarationsCache.Add(style, parentStyle, cacheHash, matchResult);
    }
  }

  void Resolver::SetGlobalStateAfterApplyingProperties(const BuilderState &builderState) noexcept
  {
    // FIXME: This stuff should be somewhere else.
    for (auto &contentAttribute : builderState.RegisteredContentAttributes())
    {
      RuleSets().MutableFeatures().RegisterContentAttribute(contentAttribute);
    }
    if (builderState.Style().UsesViewportUnits())
    {
      Document().SetHasStyleWithViewportUnits();
    }
  }

  namespace
  {
    KRYS_NODISCARD css::SelectorList ViewTransitionSelector(css::SelectorPseudoElement element,
                                                            const css::CSSOMStringAtom &name) noexcept
    {
      css::MutableSelectorList selectorList;

      auto rootSelector = CreateUnique<css::MutableSelector>();
      rootSelector->SetMatch(css::SelectorMatch::PseudoClass);
      rootSelector->SetPseudoClass(css::SelectorPseudoClass::Root);
      selectorList.push_back(krys::move(rootSelector));

      auto groupSelector = CreateUnique<css::MutableSelector>();
      groupSelector->SetMatch(css::SelectorMatch::PseudoElement);
      groupSelector->SetPseudoElement(element);

      auto selectorName = css::CSSOMStringAtom::Null();
      switch (element)
      {
        case css::SelectorPseudoElement::ViewTransitionGroup:
        {
          selectorName = css::CSSOMStringAtom(u8"view-transition-group");
          break;
        }
        case css::SelectorPseudoElement::ViewTransitionImagePair:
        {
          selectorName = css::CSSOMStringAtom(u8"view-transition-image-pair");
          break;
        }
        case css::SelectorPseudoElement::ViewTransitionNew:
        {
          selectorName = css::CSSOMStringAtom(u8"view-transition-new");
          break;
        }
        case css::SelectorPseudoElement::ViewTransitionOld:
        {
          selectorName = css::CSSOMStringAtom(u8"view-transition-old");
          break;
        }
        default:
        {
          krys_unreachable();
        }
      }

      groupSelector->SetValue(selectorName);
      groupSelector->SetArgumentList(FixedVector<css::CSSOMStringAtom> {name});

      selectorList.front()->PrependComplexSelectorComponent(css::SelectorRelation::Compounding,
                                                            krys::move(groupSelector));

      return css::SelectorList(krys::move(selectorList));
    }
  }

  RefPtr<css::ViewTransitionRule> Resolver::GetViewTransitionRule() const noexcept
  {
    return _ruleSets.GetViewTransitionRule();
  }

  void Resolver::SetViewTransitionStyles(css::SelectorPseudoElement element, const css::CSSOMStringAtom &name,
                                         Ref<css::MutableStyleProperties> properties) noexcept
  {
    if (!_document)
    {
      return;
    }

    auto styleRule =
      css::StyleRule::Create(krys::move(properties), true, ViewTransitionSelector(element, name));

    auto *viewTransitionsStyle = _ruleSets.DynamicViewTransitionsStyle();
    RuleSetBuilder builder(*viewTransitionsStyle, MediaQueryEvaluator(), this);
    builder.AddStyleRule(*styleRule);
  }
}
