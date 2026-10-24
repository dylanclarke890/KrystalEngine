#include "Krystal.Booey/Style/TreeResolver.hpp"
#include "Krystal.Booey/Animation/DocumentTimeline.hpp"
#include "Krystal.Booey/Animation/KeyframeEffectStack.hpp"
#include "Krystal.Booey/CSS/Rules/PositionTryRule.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Iterators/ComposedTreeIterator.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/HTMLMeterElement.hpp"
#include "Krystal.Booey/HTML/HTMLProgressElement.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/Rendering/PositionedLayoutConstraints.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/Style/PositionTryOrder.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/Adjuster.hpp"
#include "Krystal.Booey/Style/Builder.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/MatchResultCache.hpp"
#include "Krystal.Booey/Style/Relations.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"
#include <ranges>

namespace krys::boo::style
{
  // The maximum depth of the render tree; deeper elements aren't styled.
  // FIXME: `Settings::defaultMaximumRenderTreeDepth`, once there are settings.
  static constexpr uint32 DefaultMaximumRenderTreeDepth = 512;

  TreeResolver::TreeResolver(dom::Document &document, UniquePtr<Update> update) noexcept
      : _document(ShareRef(document)), _update(krys::move(update))
  {
  }

  TreeResolver::~TreeResolver() noexcept
  {
    _document->StyleScope().UpdateAnchorPositioningStateAfterStyleResolution();
  }

  TreeResolver::Scope::Scope(dom::Document &document, style::Update &update) noexcept
      : Resolver(ShareRef(document.StyleScope().GetResolver()))
  {
    document.SetIsResolvingTreeStyle(true);

    // Ensure all shadow tree resolvers exist so their construction doesn't depend on traversal.
    for (auto &shadowRoot : document.InDocumentShadowRoots())
    {
      if (shadowRoot)
      {
        (void)shadowRoot->StyleScope().GetResolver();
      }
    }

    SelectorMatchingState.ContainerQueryEvaluationState.StyleUpdate = ShareCheckedPtr(&update);
  }

  TreeResolver::Scope::Scope(dom::ShadowRoot &shadowRoot, Scope &enclosingScope) noexcept
      : Resolver(ShareRef(shadowRoot.StyleScope().GetResolver())), ShadowRoot(ShareRefPtr(&shadowRoot)),
        EnclosingScope(ShareRefPtr(&enclosingScope))
  {
    SelectorMatchingState.ContainerQueryEvaluationState =
      enclosingScope.SelectorMatchingState.ContainerQueryEvaluationState;
  }

  TreeResolver::Scope::~Scope() noexcept
  {
    if (!ShadowRoot)
    {
      Resolver->Document().SetIsResolvingTreeStyle(false);
    }
  }

  TreeResolver::Parent::Parent(dom::Document &document) noexcept
      : Element(nullptr), Style(*document.InitialContainingBlockStyle())
  {
  }

  TreeResolver::Parent::Parent(dom::Element &element, const RenderStyle &style, EnumSet<Change> changes,
                               TreeResolver::DescendantsToResolve descendantsToResolve,
                               style::IsInDisplayNoneTree isInDisplayNoneTree) noexcept
      : Element(&element), Style(style), Changes(changes), DescendantsToResolve(descendantsToResolve),
        IsInDisplayNoneTree(isInDisplayNoneTree)
  {
  }

  void TreeResolver::PushScope(dom::ShadowRoot &shadowRoot) noexcept
  {
    _scopeStack.push_back(CreateRef<Scope>(shadowRoot, GetScope()));
  }

  void TreeResolver::PushEnclosingScope() noexcept
  {
    krys_debug_assert(GetScope().EnclosingScope);
    _scopeStack.push_back(ShareRef(*GetScope().EnclosingScope));
  }

  void TreeResolver::PopScope() noexcept
  {
    _scopeStack.pop_back();
  }

  // Takes an old style (from previous style resolution) and new style (from current style resolution),
  // determine if the last successful position option should be invalidated. This follows the criterias in the
  // spec: https://drafts.csswg.org/css-anchor-position-1/#last-successful-position-option
  static bool ShouldInvalidateLastSuccessfulPositionOptionIndex(const RenderStyle *oldStyle,
                                                                const RenderStyle *newStyle) noexcept
  {
    if (oldStyle && newStyle)
    {
      if (oldStyle->GetPositionTryFallbacks() != newStyle->GetPositionTryFallbacks())
      {
        return true;
      }

      if (oldStyle->GetPositionTryOrder() != newStyle->GetPositionTryOrder())
      {
        return true;
      }

      // FIXME: add missing invalidation criterias in the spec.
    }

    return false;
  }

  ResolvedStyle TreeResolver::StyleForStyleable(const Styleable &styleable, ResolutionType resolutionType,
                                                const ResolutionContext &resolutionContext,
                                                const RenderStyle *existingStyle) noexcept
  {
    if (resolutionType == ResolutionType::AnimationOnly && styleable.LastStyleChangeEventStyle()
        && !styleable.HasPropertiesOverridenAfterAnimation())
    {
      return {RenderStyle::ClonePtr(*styleable.LastStyleChangeEventStyle())};
    }

    auto &element = styleable.Element;

    if (auto optionStyle = TryChoosePositionOption(styleable, resolutionContext))
    {
      return krys::move(*optionStyle);
    }

    if (resolutionType == ResolutionType::FastPathInherit)
    {
      // If the only reason we are computing the style is that some parent inherited properties changed, we
      // can just copy them.
      auto style = RenderStyle::ClonePtr(*existingStyle);
      style->FastPathInheritFrom(GetParent().Style);
      _document->StyleScope().GetMatchResultCache().UpdateForFastPathInherit(element, GetParent().Style);
      return {krys::move(style)};
    }

    auto unadjustedStyle = [&]
    {
      if (element.HasCustomStyleResolveCallbacks())
      {
        auto *shadowHostStyle =
          GetScope().ShadowRoot ? _update->ElementStyle(*GetScope().ShadowRoot->Host()) : nullptr;
        if (auto customStyle = element.ResolveCustomStyle(resolutionContext, shadowHostStyle))
        {
          return krys::move(*customStyle);
        }
      }

      if (resolutionType == ResolutionType::FullWithMatchResultCache)
      {
        if (auto cachedResult =
              _document->StyleScope().GetMatchResultCache().ResultWithCurrentInlineStyle(element))
        {
          auto result = GetScope().Resolver->UnadjustedStyleForCachedMatchResult(element, resolutionContext,
                                                                                 krys::move(*cachedResult));
          MatchResultCache::Update(*cachedResult, *result.Style);
          return result;
        }
      }
      auto result = GetScope().Resolver->UnadjustedStyleForElement(element, resolutionContext);
      _document->StyleScope().GetMatchResultCache().Set(element, result);
      return result;
    }();

    if (unadjustedStyle.Relations)
    {
      CommitRelations(krys::move(unadjustedStyle.Relations), *_update);
    }

    auto style = krys::move(unadjustedStyle.Style);

    // Fully custom styles in UA shadow trees that don't originate from selector matching don't need
    // adjusting.
    if (unadjustedStyle.MatchResult)
    {
      Adjuster adjuster(*_document, *resolutionContext.ParentStyle, resolutionContext.ParentBoxStyle,
                        &element);
      adjuster.Adjust(*style);
    }

    ResolvedStyle resolvedStyle {
      .Style = krys::move(style), .Relations = {}, .MatchResult = krys::move(unadjustedStyle.MatchResult)};

    // Invalidate the last successful position option here. This is the only place where we have access to the
    // old and new style, and hence we could determine whether the style changes enough to invalidate.
    if (ShouldInvalidateLastSuccessfulPositionOptionIndex(existingStyle, resolvedStyle.Style.get()))
    {
      _document->StyleScope().ForgetLastSuccessfulPositionOptionIndex(styleable);
    }

    GeneratePositionOptionsIfNeeded(resolvedStyle, styleable, resolutionContext);

    return resolvedStyle;
  }

  void TreeResolver::ResetStyleForNonRenderedDescendants(dom::Element &subtreeRoot) noexcept
  {
    auto descendants = dom::ComposedTreeDescendants(subtreeRoot);
    for (auto it = descendants.begin(); it != descendants.end();)
    {
      if (auto *element = DynamicDowncast<dom::Element>(*it))
      {
        if (element->NeedsStyleRecalc())
        {
          element->ResetComputedStyle();
          element->ResetStyleRelations();
          element->SetHasValidStyle();
        }
      }

      if (it->ChildNeedsStyleRecalc())
      {
        it->ClearChildNeedsStyleRecalc();
        it.TraverseNext();
      }
      else
      {
        it.TraverseNextSkippingChildren();
      }
    }

    auto nonRenderedElementsWithPositionOptions = [&]
    {
      List<RefPtr<const dom::Element>> result;
      for (const auto &[styleable, options] : _positionOptions)
      {
        if (styleable.first->IsComposedTreeDescendantOf(subtreeRoot))
        {
          result.push_back(styleable.first);
        }
      }

      return result;
    }();

    std::erase_if(_positionOptions, [&nonRenderedElementsWithPositionOptions](const auto &kv)
                  { return std::ranges::contains(nonRenderedElementsWithPositionOptions, kv.first.first); });

    subtreeRoot.ClearChildNeedsStyleRecalc();
  }

  static bool AffectsRenderedSubtree(dom::Element &element, const RenderStyle &newStyle) noexcept
  {
    if (newStyle.Display() != DisplayType::None)
    {
      return true;
    }
    if (element.RenderOrDisplayContentsStyle())
    {
      return true;
    }
    if (element.RendererIsNeeded(newStyle))
    {
      return true;
    }
    return false;
  }

  auto TreeResolver::ComputeDescendantsToResolve(const ElementUpdate &update,
                                                 const RenderStyle *existingStyle,
                                                 Validity validity) const noexcept -> DescendantsToResolve
  {
    if (GetParent().DescendantsToResolve == DescendantsToResolve::All)
    {
      return DescendantsToResolve::All;
    }
    if (validity >= Validity::SubtreeInvalid)
    {
      return DescendantsToResolve::All;
    }

    if (!update.Changes.IsEmpty() && existingStyle)
    {
      auto customPropertyInStyleContainerQueryChanged = [&]
      {
        auto &namesInQueries = GetScope().Resolver->RuleSets().CustomPropertyNamesInStyleContainerQueries();
        for (auto &name : namesInQueries)
        {
          // Any descendant may depend on this changed custom property via a style query.
          if (!existingStyle->CustomPropertyValueEqual(*update.Style, name))
          {
            return true;
          }
        }
        return false;
      }();
      if (customPropertyInStyleContainerQueryChanged)
      {
        return DescendantsToResolve::All;
      }
    }

    if (update.Changes.ContainsAny({Change::Container, Change::Renderer}))
    {
      return DescendantsToResolve::All;
    }

    if (update.Changes.ContainsAny(InheritedChanges()))
    {
      return DescendantsToResolve::Children;
    }

    if (update.Changes.Contains(Change::NonInherited))
    {
      return DescendantsToResolve::ChildrenWithExplicitInherit;
    }

    krys_debug_assert(update.Changes.IsEmpty());
    return DescendantsToResolve::None;
  }

  static bool StyleChangeAffectsRelativeUnits(const RenderStyle &style,
                                              const RenderStyle *existingStyle) noexcept
  {
    if (!existingStyle)
    {
      return true;
    }
    return !existingStyle->FontCascadeEqual(style)
           || existingStyle->ComputedLineHeight() != style.ComputedLineHeight();
  }

  auto TreeResolver::ResolveElement(dom::Element &element, const RenderStyle *existingStyle,
                                    ResolutionType resolutionType) noexcept
    -> Pair<ElementUpdate, DescendantsToResolve>
  {
    if (_didSeePendingStylesheet && !element.RenderOrDisplayContentsStyle()
        && !_document->IsIgnoringPendingStylesheets())
    {
      _document->SetHasNodesWithMissingStyle();
      return {};
    }

    if (resolutionType == ResolutionType::RebuildUsingExisting)
    {
      return {ElementUpdate {RenderStyle::ClonePtr(*existingStyle), {Change::Renderer}},
              DescendantsToResolve::RebuildAllUsingExisting};
    }

    auto resolutionContext = MakeResolutionContext();

    Styleable styleable {element, null};
    auto resolvedStyle = StyleForStyleable(styleable, resolutionType, resolutionContext, existingStyle);

    UpdateForPositionVisibility(*resolvedStyle.Style, styleable);

    auto update = CreateAnimatedElementUpdate(krys::move(resolvedStyle), styleable, GetParent().Changes,
                                              resolutionContext, GetParent().IsInDisplayNoneTree);

    if (!AffectsRenderedSubtree(element, *update.Style))
    {
      // FIXME: Clear the last style change event style, with animations.
      if (update.Style->Display() == DisplayType::None && element.HasDisplayNone())
      {
        return {krys::move(update), DescendantsToResolve::None};
      }
      return {};
    }

    auto descendantsToResolve = ComputeDescendantsToResolve(update, existingStyle, element.StyleValidity());
    bool isDocumentElement = &element == _document->DocumentElement().get();
    if (isDocumentElement)
    {
      if (StyleChangeAffectsRelativeUnits(*update.Style, existingStyle))
      {
        // "rem" units are relative to the document element's font size so we need to recompute everything.
        GetScope().Resolver->InvalidateMatchedDeclarationsCache();
        descendantsToResolve = DescendantsToResolve::All;
      }
    }

    // This is needed for resolving color:-webkit-text for subsequent elements.
    // FIXME: We shouldn't mutate document when resolving style.
    if (&element == _document->Body().get())
    {
      _document->SetTextColor(update.Style->VisitedDependentColor(css::PropertyId::Color));
    }

    // FIXME: These elements should not change renderer based on appearance property.
    if (auto *htmlElement = DynamicDowncast<html::HTMLElement>(element);
        htmlElement
        && ((Is<html::HTMLInputElement>(*htmlElement)
             && Downcast<html::HTMLInputElement>(*htmlElement).IsSearchField())
            || Is<html::HTMLMeterElement>(*htmlElement) || Is<html::HTMLProgressElement>(*htmlElement)))
    {
      if (existingStyle && update.Style->UsedAppearance() != existingStyle->UsedAppearance())
      {
        update.Changes.Add(Change::Renderer);
        descendantsToResolve = DescendantsToResolve::All;
      }
    }

    auto resolveAndAddPseudoElementStyle = [&](const PseudoElementIdentifier &pseudoElementIdentifier)
    {
      const RenderStyle *existingPseudoStyle =
        existingStyle ? existingStyle->GetCachedPseudoStyle(pseudoElementIdentifier) : nullptr;
      auto pseudoElementUpdate = ResolvePseudoElement(element, pseudoElementIdentifier, update,
                                                      GetParent().IsInDisplayNoneTree, existingPseudoStyle);

      auto pseudoElementChanges = [&]() -> EnumSet<Change>
      {
        if (pseudoElementUpdate)
        {
          if (pseudoElementUpdate->Changes.IsEmpty())
          {
            return {};
          }
          return {Change::NonInherited};
        }
        if (!existingStyle || !existingStyle->GetCachedPseudoStyle(pseudoElementIdentifier))
        {
          return {};
        }
        // If ::first-letter goes aways rebuild the renderers.
        if (pseudoElementIdentifier.Type == PseudoElementType::FirstLetter)
        {
          return {Change::Renderer};
        }
        return {Change::NonInherited};
      }();
      update.Changes.Add(pseudoElementChanges);
      if (!pseudoElementUpdate)
      {
        return pseudoElementChanges;
      }
      if (pseudoElementUpdate->RecompositeLayer)
      {
        update.RecompositeLayer = true;
      }
      update.Style->AddCachedPseudoStyle(krys::move(pseudoElementUpdate->Style));
      return pseudoElementUpdate->Changes;
    };

    if (!resolveAndAddPseudoElementStyle({PseudoElementType::FirstLine}).IsEmpty())
    {
      descendantsToResolve = DescendantsToResolve::All;
    }
    if (!resolveAndAddPseudoElementStyle({PseudoElementType::FirstLetter}).IsEmpty())
    {
      descendantsToResolve = DescendantsToResolve::All;
    }

    (void)resolveAndAddPseudoElementStyle({PseudoElementType::Marker});
    (void)resolveAndAddPseudoElementStyle({PseudoElementType::Before});
    (void)resolveAndAddPseudoElementStyle({PseudoElementType::After});
    (void)resolveAndAddPseudoElementStyle({PseudoElementType::Backdrop});
    (void)resolveAndAddPseudoElementStyle({PseudoElementType::Checkmark});

    // FIXME: The view transition pseudo-elements of the document element, once documents have active view
    // transitions.

    return {krys::move(update), descendantsToResolve};
  }

  Maybe<ElementUpdate> TreeResolver::ResolvePseudoElement(
    dom::Element &element, const PseudoElementIdentifier &pseudoElementIdentifier,
    const ElementUpdate &elementUpdate, style::IsInDisplayNoneTree isInDisplayNoneTree,
    const RenderStyle *existingStyle) noexcept
  {
    if (elementUpdate.Style->Display() == DisplayType::None)
    {
      return null;
    }

    if (pseudoElementIdentifier.Type == PseudoElementType::Backdrop && !element.IsInTopLayer())
    {
      return null;
    }
    if (pseudoElementIdentifier.Type == PseudoElementType::Marker
        && elementUpdate.Style->Display() != DisplayType::ListItem)
    {
      return null;
    }

    auto userAgentShadowTreeEnclosingResolver = [&]() -> Resolver *
    {
      if (element.IsInUserAgentShadowTree())
      {
        return GetScope().EnclosingScope->Resolver.get();
      }
      return nullptr;
    };

    if (pseudoElementIdentifier.Type == PseudoElementType::FirstLine
        && !GetScope().Resolver->UsesFirstLineRules())
    {
      // For user-agent shadow tree elements, also check the enclosing (document) scope
      // because user-agent pseudo-elements like details::details-content::first-line
      // are defined in the document scope but target elements in the shadow tree
      auto *resolver = userAgentShadowTreeEnclosingResolver();
      if (!resolver || !resolver->UsesFirstLineRules())
      {
        return null;
      }
    }

    if (pseudoElementIdentifier.Type == PseudoElementType::FirstLetter
        && !GetScope().Resolver->UsesFirstLetterRules())
    {
      auto *resolver = userAgentShadowTreeEnclosingResolver();
      if (!resolver || !resolver->UsesFirstLetterRules())
      {
        return null;
      }
    }

    if (!elementUpdate.Style->HasPseudoStyle(pseudoElementIdentifier.Type))
    {
      return ResolveAncestorPseudoElement(element, pseudoElementIdentifier, elementUpdate);
    }

    if ((pseudoElementIdentifier.Type == PseudoElementType::FirstLine
         || pseudoElementIdentifier.Type == PseudoElementType::FirstLetter)
        && !SupportsFirstLineAndLetterPseudoElement(*elementUpdate.Style))
    {
      return null;
    }

    auto resolutionContext = MakeResolutionContextForPseudoElement(elementUpdate, pseudoElementIdentifier);

    bool pseudoSupportsPositionTry = pseudoElementIdentifier.Type == PseudoElementType::Before
                                     || pseudoElementIdentifier.Type == PseudoElementType::After
                                     || pseudoElementIdentifier.Type == PseudoElementType::Backdrop;

    Styleable styleable {element, pseudoElementIdentifier};

    auto resolvedStyle = [&]
    {
      Maybe<ResolvedStyle> resolvedStyle;

      if (pseudoSupportsPositionTry)
      {
        resolvedStyle = TryChoosePositionOption(styleable, resolutionContext);
      }

      if (!resolvedStyle)
      {
        resolvedStyle = GetScope().Resolver->StyleForPseudoElement(
          element, PseudoElementRequest(pseudoElementIdentifier), resolutionContext);

        if (resolvedStyle && pseudoSupportsPositionTry)
        {
          if (ShouldInvalidateLastSuccessfulPositionOptionIndex(existingStyle, resolvedStyle->Style.get()))
          {
            _document->StyleScope().ForgetLastSuccessfulPositionOptionIndex(styleable);
          }

          GeneratePositionOptionsIfNeeded(*resolvedStyle, styleable, resolutionContext);
        }
      }

      return resolvedStyle;
    }();

    if (!resolvedStyle)
    {
      return null;
    }

    auto animatedUpdate = CreateAnimatedElementUpdate(
      krys::move(*resolvedStyle), styleable, elementUpdate.Changes, resolutionContext, isInDisplayNoneTree);

    if (pseudoElementIdentifier.Type == PseudoElementType::Before
        || pseudoElementIdentifier.Type == PseudoElementType::After)
    {
      if (GetScope().Resolver->UsesFirstLineRules())
      {
        // ::first-line can inherit to ::before/::after
        if (auto firstLineContext =
              MakeResolutionContextForInheritedFirstLine(elementUpdate, *elementUpdate.Style))
        {
          auto firstLineStyle = GetScope().Resolver->StyleForPseudoElement(
            element, PseudoElementRequest(pseudoElementIdentifier), *firstLineContext);
          firstLineStyle->Style->SetPseudoElementIdentifier(
            PseudoElementIdentifier {PseudoElementType::FirstLine});
          animatedUpdate.Style->AddCachedPseudoStyle(krys::move(firstLineStyle->Style));
        }
      }
      if (GetScope().Resolver->UsesFirstLetterRules())
      {
        auto beforeAfterContext =
          MakeResolutionContextForPseudoElement(animatedUpdate, {PseudoElementType::FirstLetter});
        if (auto firstLetterStyle =
              ResolveAncestorFirstLetterPseudoElement(element, elementUpdate, beforeAfterContext))
        {
          animatedUpdate.Style->AddCachedPseudoStyle(krys::move(firstLetterStyle->Style));
        }
      }
    }

    return animatedUpdate;
  }

  Maybe<ElementUpdate>
    TreeResolver::ResolveAncestorPseudoElement(dom::Element &element,
                                               const PseudoElementIdentifier &pseudoElementIdentifier,
                                               const ElementUpdate &elementUpdate) noexcept
  {
    krys_debug_assert(!elementUpdate.Style->HasPseudoStyle(pseudoElementIdentifier.Type));

    auto pseudoElementStyle = [&]() -> Maybe<ResolvedStyle>
    {
      // ::first-line and ::first-letter defined on an ancestor element may need to be resolved for the
      // current element.
      if (pseudoElementIdentifier.Type == PseudoElementType::FirstLine)
      {
        return ResolveAncestorFirstLinePseudoElement(element, elementUpdate);
      }
      if (pseudoElementIdentifier.Type == PseudoElementType::FirstLetter)
      {
        auto resolutionContext =
          MakeResolutionContextForPseudoElement(elementUpdate, {PseudoElementType::FirstLetter});
        return ResolveAncestorFirstLetterPseudoElement(element, elementUpdate, resolutionContext);
      }
      return null;
    }();

    if (!pseudoElementStyle)
    {
      return null;
    }

    auto *oldStyle = element.RenderOrDisplayContentsStyle(pseudoElementIdentifier);
    auto changes =
      oldStyle ? DetermineChanges(*oldStyle, *pseudoElementStyle->Style) : EnumSet<Change> {Change::Renderer};
    auto resolutionContext = MakeResolutionContextForPseudoElement(elementUpdate, pseudoElementIdentifier);

    return CreateAnimatedElementUpdate(krys::move(*pseudoElementStyle), {element, pseudoElementIdentifier},
                                       changes, resolutionContext);
  }

  static bool IsChildInBlockFormattingContext(const RenderStyle &style) noexcept
  {
    // FIXME: Incomplete. There should be shared code with layout for this.
    if (style.Display() != DisplayType::Block && style.Display() != DisplayType::ListItem)
    {
      return false;
    }
    if (style.HasOutOfFlowPosition())
    {
      return false;
    }
    if (style.Floating() != Float::None)
    {
      return false;
    }
    if (style.OverflowX() != Overflow::Visible || style.OverflowY() != Overflow::Visible)
    {
      return false;
    }
    return true;
  }

  Maybe<ResolvedStyle>
    TreeResolver::ResolveAncestorFirstLinePseudoElement(dom::Element &element,
                                                        const ElementUpdate &elementUpdate) noexcept
  {
    if (elementUpdate.Style->Display() == DisplayType::Inline)
    {
      auto *parent = BoxGeneratingParent();
      if (!parent)
      {
        return null;
      }

      auto resolutionContext = MakeResolutionContextForInheritedFirstLine(elementUpdate, parent->Style);
      if (!resolutionContext)
      {
        return null;
      }

      auto elementStyle = GetScope().Resolver->StyleForElement(element, *resolutionContext);
      elementStyle.Style->SetPseudoElementIdentifier(PseudoElementIdentifier {PseudoElementType::FirstLine});

      return elementStyle;
    }

    auto findFirstLineElementForBlock = [&]() -> dom::Element *
    {
      if (!IsChildInBlockFormattingContext(*elementUpdate.Style))
      {
        return nullptr;
      }

      // ::first-line is only propagated to the first block.
      if (GetParent().ResolvedFirstLineAndLetterChild)
      {
        return nullptr;
      }

      for (auto &parent : _parentStack | std::views::reverse)
      {
        if (parent.Style.Display() == DisplayType::Contents)
        {
          continue;
        }
        if (!SupportsFirstLineAndLetterPseudoElement(parent.Style))
        {
          return nullptr;
        }
        if (parent.Style.HasPseudoStyle(PseudoElementType::FirstLine))
        {
          return parent.Element;
        }
        if (!IsChildInBlockFormattingContext(parent.Style))
        {
          return nullptr;
        }
      }
      return nullptr;
    };

    auto *firstLineElement = findFirstLineElementForBlock();
    if (!firstLineElement)
    {
      return null;
    }

    auto resolutionContext =
      MakeResolutionContextForPseudoElement(elementUpdate, {PseudoElementType::FirstLine});
    // Can't use the cached state since the element being resolved is not the current one.
    resolutionContext.SelectorMatchingState = nullptr;

    return GetScope().Resolver->StyleForPseudoElement(
      *firstLineElement, PseudoElementRequest(PseudoElementType::FirstLine), resolutionContext);
  }

  Maybe<ResolvedStyle> TreeResolver::ResolveAncestorFirstLetterPseudoElement(
    dom::Element &element, const ElementUpdate &elementUpdate, ResolutionContext &resolutionContext) noexcept
  {
    auto findFirstLetterElement = [&]() -> dom::Element *
    {
      if (elementUpdate.Style->HasPseudoStyle(PseudoElementType::FirstLetter)
          && SupportsFirstLineAndLetterPseudoElement(*elementUpdate.Style))
      {
        return &element;
      }

      // ::first-letter is only propagated to the first box.
      if (GetParent().ResolvedFirstLineAndLetterChild)
      {
        return nullptr;
      }

      bool skipInlines = elementUpdate.Style->Display() == DisplayType::Inline;
      if (!skipInlines && !IsChildInBlockFormattingContext(*elementUpdate.Style))
      {
        return nullptr;
      }

      for (auto &parent : _parentStack | std::views::reverse)
      {
        if (parent.Style.Display() == DisplayType::Contents)
        {
          continue;
        }
        if (skipInlines && parent.Style.Display() == DisplayType::Inline)
        {
          continue;
        }
        skipInlines = false;

        if (!SupportsFirstLineAndLetterPseudoElement(parent.Style))
        {
          return nullptr;
        }
        if (parent.Style.HasPseudoStyle(PseudoElementType::FirstLetter))
        {
          return parent.Element;
        }
        if (!IsChildInBlockFormattingContext(parent.Style))
        {
          return nullptr;
        }
      }
      return nullptr;
    };

    auto *firstLetterElement = findFirstLetterElement();
    if (!firstLetterElement)
    {
      return null;
    }

    // Can't use the cached state since the element being resolved is not the current one.
    resolutionContext.SelectorMatchingState = nullptr;

    return GetScope().Resolver->StyleForPseudoElement(
      *firstLetterElement, PseudoElementRequest(PseudoElementType::FirstLetter), resolutionContext);
  }

  ResolutionContext TreeResolver::MakeResolutionContext() noexcept
  {
    return {&GetParent().Style, ParentBoxStyle(), DocumentElementStyle(), &GetScope().SelectorMatchingState,
            &_treeResolutionState};
  }

  ResolutionContext TreeResolver::MakeResolutionContextForPseudoElement(
    const ElementUpdate &elementUpdate, const PseudoElementIdentifier &pseudoElementIdentifier) noexcept
  {
    auto parentStyle = [&]() -> const RenderStyle *
    {
      if (auto parentPseudoId = ParentPseudoElement(pseudoElementIdentifier.Type))
      {
        auto nameArgument = (*parentPseudoId == PseudoElementType::ViewTransitionGroup
                             || *parentPseudoId == PseudoElementType::ViewTransitionImagePair)
                              ? pseudoElementIdentifier.NameArgument
                              : css::CSSOMStringAtom::Null();
        if (auto *parentPseudoStyle = elementUpdate.Style->GetCachedPseudoStyle(
              PseudoElementIdentifier {*parentPseudoId, nameArgument}))
        {
          return parentPseudoStyle;
        }
      }
      return elementUpdate.Style.get();
    };

    return {parentStyle(), ParentBoxStyleForPseudoElement(elementUpdate), DocumentElementStyle(),
            &GetScope().SelectorMatchingState, &_treeResolutionState};
  }

  Maybe<ResolutionContext>
    TreeResolver::MakeResolutionContextForInheritedFirstLine(const ElementUpdate &elementUpdate,
                                                             const RenderStyle &inheritStyle) noexcept
  {
    auto *parentFirstLineStyle = inheritStyle.GetCachedPseudoStyle({PseudoElementType::FirstLine});
    if (!parentFirstLineStyle)
    {
      return null;
    }

    // First line style for inlines is made by inheriting from parent first line style.
    return ResolutionContext {parentFirstLineStyle, ParentBoxStyleForPseudoElement(elementUpdate),
                              DocumentElementStyle(), &GetScope().SelectorMatchingState,
                              &_treeResolutionState};
  }

  const RenderStyle *TreeResolver::DocumentElementStyle() const noexcept
  {
    if (_computedDocumentElementStyle)
    {
      return _computedDocumentElementStyle.get();
    }

    return _document->DocumentElement()->GetRenderStyle();
  }

  auto TreeResolver::BoxGeneratingParent() const noexcept -> const Parent *
  {
    // 'display: contents' doesn't generate boxes.
    for (auto &parent : _parentStack | std::views::reverse)
    {
      if (parent.Style.Display() == DisplayType::None)
      {
        return nullptr;
      }
      if (parent.Style.Display() != DisplayType::Contents)
      {
        return &parent;
      }
    }
    krys_unreachable();
  }

  const RenderStyle *TreeResolver::ParentBoxStyle() const noexcept
  {
    auto *parent = BoxGeneratingParent();
    return parent ? &parent->Style : nullptr;
  }

  const RenderStyle *
    TreeResolver::ParentBoxStyleForPseudoElement(const ElementUpdate &elementUpdate) const noexcept
  {
    switch (elementUpdate.Style->Display())
    {
      case DisplayType::None:     return nullptr;
      case DisplayType::Contents: return ParentBoxStyle();
      default:                    return elementUpdate.Style.get();
    }
  }

  ElementUpdate TreeResolver::CreateAnimatedElementUpdate(
    ResolvedStyle &&resolvedStyle, const Styleable &styleable, EnumSet<Change> parentChanges,
    const ResolutionContext &resolutionContext, style::IsInDisplayNoneTree isInDisplayNoneTree) noexcept
  {
    auto &element = styleable.Element;
    auto *currentStyle = element.RenderOrDisplayContentsStyle(styleable.PseudoElementIdentifier);

    UniquePtr<RenderStyle> startingStyle;

    auto *oldStyle = [&]() -> const RenderStyle *
    {
      if (auto *styleBefore = BeforeResolutionStyle(element, styleable.PseudoElementIdentifier))
      {
        return styleBefore;
      }

      if (resolvedStyle.Style->HasTransitions())
      {
        // https://drafts.csswg.org/css-transitions-2/#at-ruledef-starting-style
        // "If an element does not have a before-change style for a given style change event, the starting
        // style is used instead."
        startingStyle = ResolveStartingStyle(resolvedStyle, styleable, resolutionContext);
        return startingStyle.get();
      }

      return nullptr;
    }();

    auto unanimatedDisplay = resolvedStyle.Style->Display();
    auto hasUnresolvedAnchorPosition = HasUnresolvedAnchorPosition(styleable);

    WeakStyleOriginatedAnimations newStyleOriginatedAnimations;

    auto updateAnimations = [&]
    {
      // FIXME: Not updating animations in the back/forward cache or while printing, once there are those.

      if (hasUnresolvedAnchorPosition)
      {
        return;
      }

      if (oldStyle && (oldStyle->HasTransitions() || resolvedStyle.Style->HasTransitions()))
      {
        styleable.UpdateCSSTransitions(*oldStyle, *resolvedStyle.Style, newStyleOriginatedAnimations);
      }

      // FIXME: Scroll and view timelines and timeline scopes, once there are those.

      // The order in which CSS Transitions and CSS Animations are updated matters since CSS Transitions
      // define the after-change style to use CSS Animations as defined in the previous style change event. As
      // such, we update CSS Animations after CSS Transitions such that when CSS Transitions are updated the
      // CSS Animations data is the same as during the previous style change event.
      if ((oldStyle && oldStyle->HasAnimations()) || resolvedStyle.Style->HasAnimations())
      {
        styleable.UpdateCSSAnimations(oldStyle, *resolvedStyle.Style, resolutionContext,
                                      newStyleOriginatedAnimations, isInDisplayNoneTree);
      }
    };

    auto applyAnimations = [&]() -> Pair<UniquePtr<RenderStyle>, EnumSet<AnimationImpact>>
    {
      if (hasUnresolvedAnchorPosition)
      {
        auto newStyle = krys::move(resolvedStyle.Style);
        krys_debug_assert(newStyle);

        // When display transitions to none, it keeps its old value throughout the transition, and only
        // switches to none after the transition ends. CSS transition machinery normally handles this, except
        // we skip applying transitions when anchor position hasn't been resolved. This results in display:
        // none until its anchor position is resolved, when it switches back to the old value. To remedy this,
        // we manually patch display to be the old value if:
        // 1. the old style's display is not none, and
        // 2. the new style has display: none and specifies a transition on display.
        if (oldStyle && oldStyle->HasTransitions() && oldStyle->Display() != DisplayType::None
            && StyleHasDisplayTransition(*newStyle) && newStyle->Display() == DisplayType::None)
        {
          newStyle->SetDisplay(oldStyle->Display());
        }
        return {krys::move(newStyle), EnumSet<AnimationImpact> {}};
      }

      if (!styleable.HasKeyframeEffects())
      {
        // FIXME: Push after-change style into parent stack instead.
        styleable.SetLastStyleChangeEventStyle(
          ResolveAfterChangeStyleForNonAnimated(resolvedStyle, styleable, resolutionContext));
        styleable.SetHasPropertiesOverridenAfterAnimation(false);
        return {krys::move(resolvedStyle.Style), EnumSet<AnimationImpact> {}};
      }

      auto previousLastStyleChangeEventStyle =
        styleable.LastStyleChangeEventStyle() ? RenderStyle::ClonePtr(*styleable.LastStyleChangeEventStyle())
                                              : nullptr;

      // Record the style prior to applying animations for this style change event.
      styleable.SetLastStyleChangeEventStyle(RenderStyle::ClonePtr(*resolvedStyle.Style));

      // Apply all keyframe effects to the new style.
      AnimatableCSSPropertySet animatedProperties;
      auto animatedStyle = RenderStyle::ClonePtr(*resolvedStyle.Style);

      auto animationImpact = styleable.ApplyKeyframeEffects(
        *animatedStyle, animatedProperties, previousLastStyleChangeEventStyle.get(), resolutionContext);

      if (*resolvedStyle.Style == *animatedStyle && animationImpact.IsEmpty()
          && previousLastStyleChangeEventStyle)
      {
        return {krys::move(resolvedStyle.Style), animationImpact};
      }

      if (resolvedStyle.MatchResult)
      {
        // The cascade may override animated properties and have dependencies to them.
        // FIXME: This is wrong if there are both transitions and animations running on the same element.
        auto overriddenAnimatedProperties =
          ApplyCascadeAfterAnimation(*animatedStyle, animatedProperties, styleable.HasRunningTransitions(),
                                     *resolvedStyle.MatchResult, element, resolutionContext);
        krys_debug_assert(styleable.GetKeyframeEffectStack());
        styleable.GetKeyframeEffectStack()->CascadeDidOverrideProperties(
          AnimatableCSSPropertySet(overriddenAnimatedProperties.begin(), overriddenAnimatedProperties.end()),
          *_document);
        styleable.SetHasPropertiesOverridenAfterAnimation(!overriddenAnimatedProperties.empty());
      }

      Adjuster adjuster(*_document, *resolutionContext.ParentStyle, resolutionContext.ParentBoxStyle,
                        !styleable.PseudoElementIdentifier ? &styleable.Element : nullptr);
      adjuster.AdjustAnimatedStyle(*animatedStyle, animationImpact);

      return {krys::move(animatedStyle), animationImpact};
    };

    // FIXME: Something like this is also needed for viewport units.
    if (currentStyle && GetParent().NeedsUpdateQueryContainerDependentStyle)
    {
      styleable.QueryContainerDidChange();
    }

    // First, we need to make sure that any new CSS animation occuring on this element has a matching
    // WebAnimation on the document timeline.
    updateAnimations();

    // Now we can update all Web animations, which will include CSS Animations as well as animations created
    // via the JS API.
    auto [newStyle, animationImpact] = applyAnimations();

    // Deduplication speeds up equality comparisons as the properties inherit to descendants.
    // FIXME: There should be a more general mechanism for this.
    if (currentStyle)
    {
      newStyle->DeduplicateCustomProperties(*currentStyle);
    }

    auto changes =
      currentStyle ? DetermineChanges(*currentStyle, *newStyle) : EnumSet<Change> {Change::Renderer};

    if (element.HasInvalidRenderer() || parentChanges.Contains(Change::Renderer))
    {
      changes.Add(Change::Renderer);
    }

    CollectChangedAnchorNames(*newStyle, currentStyle);

    auto animationsAffectedDisplay = [&, animatedDisplay = newStyle->Display()]()
    {
      auto *keyframeEffectStack = styleable.GetKeyframeEffectStack();
      if (!keyframeEffectStack)
      {
        return false;
      }
      if (unanimatedDisplay != animatedDisplay)
      {
        return true;
      }
      return keyframeEffectStack->ContainsProperty(css::PropertyId::Display);
    }();

    if (!AffectsRenderedSubtree(styleable.Element, *newStyle) && !animationsAffectedDisplay)
    {
      // If after updating animations we end up not rendering this element or its subtree and the update did
      // not change the "display" value then we should cancel all style-originated animations while ensuring
      // that the new ones are canceled silently, as if they hadn't been created.
      styleable.CancelStyleOriginatedAnimations(newStyleOriginatedAnimations);
    }
    else if (!newStyleOriginatedAnimations.empty())
    {
      // If style-originated animations were not canceled, then we should make sure that the creation of new
      // style-originated animations during this update is known to the document's timeline as animation
      // scheduling was paused for any animation created during this update.
      if (auto *timeline = _document->ExistingTimeline())
      {
        timeline->StyleOriginatedAnimationsWereCreated();
      }
    }

    if (animationsAffectedDisplay)
    {
      newStyle->SetHasDisplayAffectedByAnimations();
    }

    bool shouldRecompositeLayer = animationImpact.Contains(AnimationImpact::RequiresRecomposite)
                                  || element.StyleResolutionShouldRecompositeLayer();

    auto mayNeedRebuildRoot = [&, newStyle = newStyle.get()]
    {
      if (changes.Contains(Change::Renderer))
      {
        return true;
      }
      // We may need to rebuild the tree starting from further up if there is a position property change to
      // clean up continuations.
      if (currentStyle && currentStyle->GetPosition() != newStyle->GetPosition())
      {
        return true;
      }
      return false;
    }();

    return {krys::move(newStyle), changes, shouldRecompositeLayer, mayNeedRebuildRoot};
  }

  UniquePtr<RenderStyle>
    TreeResolver::ResolveStartingStyle(const ResolvedStyle &resolvedStyle, const Styleable &styleable,
                                       const ResolutionContext &resolutionContext) noexcept
  {
    if (!resolvedStyle.MatchResult
        || !resolvedStyle.MatchResult->UsedRuleTypes.Contains(UsedRuleType::StartingStyle))
    {
      return nullptr;
    }

    // "Starting style inherits from the parent’s after-change style just like after-change style does."
    const auto &parentStyle = ParentAfterChangeStyle(styleable, resolutionContext);

    // We now resolve the starting style by applying all rules (including @starting-style ones) again. We
    // could compute it along with the primary style and include it in MatchedPropertiesCache but it is not
    // clear this would be benefitial as it is typically only used once.
    return ResolveAgainInDifferentContext(resolvedStyle, styleable, parentStyle,
                                          PropertyCascade::StartingStylePropertyTypes(), null,
                                          resolutionContext);
  }

  UniquePtr<RenderStyle>
    TreeResolver::ResolveAfterChangeStyleForNonAnimated(const ResolvedStyle &resolvedStyle,
                                                        const Styleable &styleable,
                                                        const ResolutionContext &resolutionContext) noexcept
  {
    // Element may have after-change style differing from the current style in case they are inheriting from a
    // transitioning element. We need after-change style for non-animating elements only in case there
    // @starting-style rules in the subtree.
    if (!GetScope().Resolver->UsesStartingStyleRules())
    {
      return nullptr;
    }

    if (!resolvedStyle.MatchResult)
    {
      return nullptr;
    }

    if (styleable.PseudoElementIdentifier)
    {
      return nullptr;
    }

    if (!GetParent().Element || !GetParent().Element->LastStyleChangeEventStyle(null))
    {
      return nullptr;
    }

    // "Likewise, define the after-change style as.. and inheriting from the after-change style of the
    // parent."
    const auto &parentStyle = ParentAfterChangeStyle(styleable, resolutionContext);

    return ResolveAgainInDifferentContext(resolvedStyle, styleable, parentStyle,
                                          PropertyCascade::NormalPropertyTypes(), null, resolutionContext);
  }

  UniquePtr<RenderStyle>
    TreeResolver::ResolveAgainInDifferentContext(const ResolvedStyle &resolvedStyle,
                                                 const Styleable &styleable, const RenderStyle &parentStyle,
                                                 EnumSet<PropertyCascade::PropertyType> properties,
                                                 Maybe<BuilderPositionTryFallback> &&positionTryFallback,
                                                 const ResolutionContext &resolutionContext) noexcept
  {
    krys_debug_assert(resolvedStyle.MatchResult);

    auto newStyle = RenderStyle::CreatePtr();
    newStyle->InheritFrom(parentStyle);
    newStyle->SetPseudoElementIdentifier(resolvedStyle.Style->GetPseudoElementIdentifier());
    newStyle->CopyPseudoElementBitsFrom(*resolvedStyle.Style);

    auto builderContext = BuilderContext {
      .Document = ShareRefPtr<const dom::Document>(_document.get()),
      .ParentStyle = &parentStyle,
      .RootElementStyle = resolutionContext.DocumentElementStyle,
      .Element = ShareRefPtr<const dom::Element>(&styleable.Element),
      .TreeResolutionState = &_treeResolutionState,
      .PositionTryFallback = krys::move(positionTryFallback),
    };

    auto styleBuilder =
      Builder {*newStyle, krys::move(builderContext), *resolvedStyle.MatchResult, {properties}};

    styleBuilder.ApplyAllProperties();

    if (newStyle->Display() == DisplayType::None)
    {
      return nullptr;
    }

    Adjuster adjuster(*_document, parentStyle, resolutionContext.ParentBoxStyle,
                      !styleable.PseudoElementIdentifier ? &styleable.Element : nullptr);
    adjuster.Adjust(*newStyle);

    return newStyle;
  }

  const RenderStyle &
    TreeResolver::ParentAfterChangeStyle(const Styleable &styleable,
                                         const ResolutionContext &resolutionContext) const noexcept
  {
    if (auto *parentElement = !styleable.PseudoElementIdentifier ? GetParent().Element : &styleable.Element)
    {
      if (auto *afterChangeStyle = parentElement->LastStyleChangeEventStyle(null))
      {
        return *afterChangeStyle;
      }
    }
    return *resolutionContext.ParentStyle;
  }

  HashSet<AnimatableCSSProperty> TreeResolver::ApplyCascadeAfterAnimation(
    RenderStyle &animatedStyle, const AnimatableCSSPropertySet &animatedProperties, bool isTransition,
    const MatchResult &matchResult, const dom::Element &element,
    const ResolutionContext &resolutionContext) noexcept
  {
    auto builderContext = BuilderContext {
      .Document = ShareRefPtr<const dom::Document>(_document.get()),
      .ParentStyle = resolutionContext.ParentStyle,
      .RootElementStyle = resolutionContext.DocumentElementStyle,
      .Element = ShareRefPtr(&element),
      .TreeResolutionState = &_treeResolutionState,
    };

    HashSet<AnimatableCSSProperty> properties(animatedProperties.begin(), animatedProperties.end());

    auto styleBuilder = Builder {animatedStyle,
                                 krys::move(builderContext),
                                 matchResult,
                                 {isTransition ? PropertyCascade::PropertyType::AfterTransition
                                               : PropertyCascade::PropertyType::AfterAnimation},
                                 &properties};

    styleBuilder.ApplyAllProperties();

    return styleBuilder.OverriddenAnimatedProperties();
  }

  void TreeResolver::PushParent(dom::Element &element, const RenderStyle &style, EnumSet<Change> changes,
                                DescendantsToResolve descendantsToResolve,
                                style::IsInDisplayNoneTree isInDisplayNoneTree) noexcept
  {
    GetScope().SelectorMatchingState.SelectorFilter.PushParent(&element);
    if (style.GetContainerType() != ContainerType::Normal)
    {
      GetScope().SelectorMatchingState.ContainerQueryEvaluationState.SizeQueryContainers.push_back(
        ShareRef<const dom::Element>(element));
    }

    Parent parent(element, style, changes, descendantsToResolve, isInDisplayNoneTree);

    if (auto *shadowRoot = dom::TreeQueries::GetShadowRoot(element))
    {
      PushScope(*shadowRoot);
      parent.DidPushScope = true;
    }
    else if (auto *slot = DynamicDowncast<html::HTMLSlotElement>(element); slot && slot->AssignedNodes())
    {
      PushEnclosingScope();
      parent.DidPushScope = true;
    }

    parent.NeedsUpdateQueryContainerDependentStyle =
      _parentStack.back().NeedsUpdateQueryContainerDependentStyle
      || element.NeedsUpdateQueryContainerDependentStyle();
    element.ClearNeedsUpdateQueryContainerDependentStyle();

    _parentStack.push_back(krys::move(parent));
  }

  void TreeResolver::PopParent() noexcept
  {
    auto &parentElement = *GetParent().Element;

    parentElement.SetHasValidStyle();
    parentElement.ClearChildNeedsStyleRecalc();

    // FIXME: Push after-change style into parent stack instead.
    if (!parentElement.HasKeyframeEffects(null))
    {
      parentElement.SetLastStyleChangeEventStyle(null, nullptr);
    }

    if (GetParent().DidPushScope)
    {
      PopScope();
    }

    GetScope().SelectorMatchingState.SelectorFilter.PopParent();

    auto &queryContainers =
      GetScope().SelectorMatchingState.ContainerQueryEvaluationState.SizeQueryContainers;
    if (!queryContainers.empty() && queryContainers.back().get() == &parentElement)
    {
      queryContainers.pop_back();
    }

    _parentStack.pop_back();
  }

  void TreeResolver::PopParentsToDepth(uint32 depth) noexcept
  {
    krys_debug_assert(depth);
    krys_debug_assert(_parentStack.size() >= depth);

    while (_parentStack.size() > depth)
    {
      PopParent();
    }
  }

  auto TreeResolver::DetermineResolutionType(const dom::Element &element, const RenderStyle *existingStyle,
                                             DescendantsToResolve parentDescendantsToResolve,
                                             EnumSet<Change> parentChanges) noexcept -> Maybe<ResolutionType>
  {
    auto combinedValidity = [&]
    {
      auto validity = element.StyleValidity();
      if (auto *pseudoElement = element.BeforePseudoElement())
      {
        validity = std::max(validity, pseudoElement->StyleValidity());
      }
      if (auto *pseudoElement = element.AfterPseudoElement())
      {
        validity = std::max(validity, pseudoElement->StyleValidity());
      }
      return validity;
    }();

    if (parentDescendantsToResolve == DescendantsToResolve::None)
    {
      if (combinedValidity == Validity::AnimationInvalid)
      {
        return ResolutionType::AnimationOnly;
      }
      if (combinedValidity == Validity::Valid && element.HasInvalidRenderer())
      {
        return existingStyle ? ResolutionType::RebuildUsingExisting : ResolutionType::Full;
      }
      if (combinedValidity == Validity::InlineStyleInvalid && existingStyle)
      {
        return ResolutionType::FullWithMatchResultCache;
      }
    }

    if (combinedValidity > Validity::Valid)
    {
      return ResolutionType::Full;
    }

    switch (parentDescendantsToResolve)
    {
      case DescendantsToResolve::None: return null;
      case DescendantsToResolve::RebuildAllUsingExisting:
        return existingStyle ? ResolutionType::RebuildUsingExisting : ResolutionType::Full;
      case DescendantsToResolve::Children:
      {
        auto canFastPathInherit = [&]
        {
          if (parentChanges.Contains(Change::Inherited))
          {
            return false;
          }
          if (!parentChanges.Contains(Change::FastPathInherited))
          {
            return false;
          }
          if (!existingStyle || existingStyle->DisallowsFastPathInheritance())
          {
            return false;
          }
          // Some non-inherited property changed along with a fast-path property and we may need to inherit it
          // too.
          if (parentChanges.Contains(Change::NonInherited)
              && existingStyle->HasExplicitlyInheritedProperties())
          {
            return false;
          }
          return true;
        };
        return canFastPathInherit() ? ResolutionType::FastPathInherit : ResolutionType::Full;
      }
      case DescendantsToResolve::All: return ResolutionType::Full;
      case DescendantsToResolve::ChildrenWithExplicitInherit:
        if (!existingStyle || existingStyle->HasExplicitlyInheritedProperties())
        {
          return ResolutionType::Full;
        }
        return null;
    }
    krys_unreachable();
  }

  static void ClearNeedsStyleResolution(dom::Element &element) noexcept
  {
    element.SetHasValidStyle();
    if (auto *before = element.BeforePseudoElement())
    {
      before->SetHasValidStyle();
    }
    if (auto *after = element.AfterPseudoElement())
    {
      after->SetHasValidStyle();
    }
  }

  static bool HasLoadingStylesheet(const Scope &styleScope, const dom::Element &element,
                                   bool checkDescendants) noexcept
  {
    if (!styleScope.HasPendingSheetsInBody())
    {
      return false;
    }
    if (styleScope.HasPendingSheetInBody(element))
    {
      return true;
    }
    if (!checkDescendants)
    {
      return false;
    }
    for (auto *descendant = dom::TreeQueries::Next(element, &element); descendant;
         descendant = dom::TreeQueries::Next(*descendant, &element))
    {
      if (auto *descendantElement = DynamicDowncast<dom::Element>(*descendant);
          descendantElement && styleScope.HasPendingSheetInBody(*descendantElement))
      {
        return true;
      }
    }
    return false;
  }

  static UniquePtr<RenderStyle>
    CreateInheritedDisplayContentsStyleIfNeeded(const RenderStyle &parentElementStyle,
                                                const RenderStyle *parentBoxStyle) noexcept
  {
    if (parentElementStyle.Display() != DisplayType::Contents)
    {
      return nullptr;
    }
    if (parentBoxStyle && parentBoxStyle->InheritedEqual(parentElementStyle))
    {
      return nullptr;
    }
    // Compute style for imaginary unstyled <span> around the text node.
    auto style = RenderStyle::CreatePtr();
    style->InheritFrom(parentElementStyle);
    return style;
  }

  void TreeResolver::ResetDescendantStyleRelations(dom::Element &element,
                                                   DescendantsToResolve descendantsToResolve) noexcept
  {
    switch (descendantsToResolve)
    {
      case DescendantsToResolve::None:
      case DescendantsToResolve::RebuildAllUsingExisting:
      case DescendantsToResolve::ChildrenWithExplicitInherit: break;
      case DescendantsToResolve::Children:                    element.ResetChildStyleRelations(); break;
      case DescendantsToResolve::All:                         element.ResetAllDescendantStyleRelations(); break;
    }
  }

  void TreeResolver::ResolveComposedTree() noexcept
  {
    krys_debug_assert(_parentStack.size() == 1);
    krys_debug_assert(_scopeStack.size() == 1);

    auto descendants = dom::ComposedTreeDescendants(*_document);
    auto it = descendants.begin();
    auto end = descendants.end();

    while (it != end)
    {
      PopParentsToDepth(it.Depth());

      auto &node = *it;
      auto &parent = GetParent();

      krys_debug_assert(node.IsConnected());
      krys_debug_assert(dom::TreeQueries::ContainingShadowRoot(node) == GetScope().ShadowRoot.get());

      if (auto text = ShareRefPtr(DynamicDowncast<dom::Text>(node)))
      {
        auto containsOnlyASCIIWhitespace = text->ContainsOnlyASCIIWhitespace();
        auto needsTextUpdate = [&]
        {
          if ((text->HasInvalidRenderer() && parent.Changes != EnumSet<Change> {Change::Renderer})
              || parent.Style.Display() == DisplayType::Contents)
          {
            return true;
          }
          if (!text->Renderer() && containsOnlyASCIIWhitespace && parent.Style.PreserveNewline())
          {
            // FIXME: This really needs to be done only when parent.style.preserveNewline() changes value.
            return true;
          }
          return false;
        };
        if (needsTextUpdate())
        {
          TextUpdate textUpdate;
          textUpdate.InheritedDisplayContentsStyle =
            CreateInheritedDisplayContentsStyleIfNeeded(parent.Style, ParentBoxStyle());

          _update->AddText(*text, parent.Element, krys::move(textUpdate));
        }

        if (!containsOnlyASCIIWhitespace)
        {
          parent.ResolvedFirstLineAndLetterChild = true;
        }

        text->SetHasValidStyle();
        it.TraverseNextSkippingChildren();
        continue;
      }

      auto &element = Downcast<dom::Element>(node);

      if (it.Depth() > DefaultMaximumRenderTreeDepth)
      {
        ResetStyleForNonRenderedDescendants(element);
        it.TraverseNextSkippingChildren();
        continue;
      }

      auto *style = ExistingStyle(element);

      auto changes = EnumSet<Change> {};
      auto descendantsToResolve = DescendantsToResolve::None;

      auto resolutionType =
        DetermineResolutionType(element, style, parent.DescendantsToResolve, parent.Changes);
      if (resolutionType)
      {
        element.ResetComputedStyle();

        if (*resolutionType == ResolutionType::Full)
        {
          element.ResetStyleRelations();
        }

        if (element.HasCustomStyleResolveCallbacks())
        {
          element.WillRecalcStyle(parent.Changes);
        }

        auto [elementUpdate, elementDescendantsToResolve] = ResolveElement(element, style, *resolutionType);

        if (element.HasCustomStyleResolveCallbacks())
        {
          element.DidRecalcStyle(elementUpdate.Changes);
        }
        // FIXME: The accessibility updates (AXObjectCache).

        style = elementUpdate.Style.get();
        changes = elementUpdate.Changes;
        descendantsToResolve = elementDescendantsToResolve;

        if (style || element.HasDisplayNone())
        {
          _update->AddElement(element, parent.Element, krys::move(elementUpdate));
        }
        if (style && &element == _document->DocumentElement().get())
        {
          _computedDocumentElementStyle = RenderStyle::ClonePtr(*style);
        }
        ClearNeedsStyleResolution(element);
      }

      if (!style)
      {
        ResetStyleForNonRenderedDescendants(element);
      }

      auto queryContainerAction = UpdateStateForQueryContainer(element, style, descendantsToResolve);
      auto anchorPositionedElementAction = UpdateAnchorPositioningState(element, style);

      ResumeDescendantResolutionIfNeeded(element, changes, descendantsToResolve);

      bool shouldIterateChildren = [&]
      {
        // display::none, no need to resolve descendants.
        if (!style)
        {
          return false;
        }

        // Style resolution will be resumed after the container or anchor-positioned element has been
        // resolved.
        if (queryContainerAction == LayoutInterleavingAction::SkipDescendants
            || anchorPositionedElementAction == LayoutInterleavingAction::SkipDescendants)
        {
          DeferDescendantResolution(element, changes, descendantsToResolve);
          return false;
        }

        return element.ChildNeedsStyleRecalc() || descendantsToResolve != DescendantsToResolve::None;
      }();

      if (!_didSeePendingStylesheet)
      {
        _didSeePendingStylesheet =
          HasLoadingStylesheet(_document->StyleScope(), element, !shouldIterateChildren);
      }

      if (!parent.ResolvedFirstLineAndLetterChild && style && GeneratesBox(*style)
          && SupportsFirstLineAndLetterPseudoElement(*style))
      {
        parent.ResolvedFirstLineAndLetterChild = true;
      }

      if (!shouldIterateChildren)
      {
        it.TraverseNextSkippingChildren();
        continue;
      }

      ResetDescendantStyleRelations(element, descendantsToResolve);

      auto isInDisplayNoneTree = parent.IsInDisplayNoneTree == style::IsInDisplayNoneTree::Yes || !style
                                 || style->Display() == DisplayType::None;
      PushParent(element, *style, changes, descendantsToResolve,
                 isInDisplayNoneTree ? style::IsInDisplayNoneTree::Yes : style::IsInDisplayNoneTree::No);
      it.TraverseNext();
    }

    PopParentsToDepth(1);
  }

  const RenderStyle *TreeResolver::ExistingStyle(const dom::Element &element) noexcept
  {
    auto *style = element.RenderOrDisplayContentsStyle();

    if (style && &element == _document->DocumentElement().get())
    {
      // Document element style may have got adjusted based on body style but we don't want to inherit those
      // adjustments.
      _computedDocumentElementStyle = Adjuster::RestoreUsedDocumentElementStyleToComputed(*style);
      if (_computedDocumentElementStyle)
      {
        style = _computedDocumentElementStyle.get();
      }
    }

    return style;
  }

  void TreeResolver::DeferDescendantResolution(dom::Element &element, EnumSet<Change> changes,
                                               DescendantsToResolve descendantsToResolve) noexcept
  {
    _deferredDescendantResolutionStates.emplace(
      ShareRef(element),
      DeferredDescendantResolutionState {.Changes = changes, .DescendantsToResolve = descendantsToResolve});
  }

  void TreeResolver::ResumeDescendantResolutionIfNeeded(dom::Element &element, EnumSet<Change> &changes,
                                                        DescendantsToResolve &descendantsToResolve) noexcept
  {
    auto it = _deferredDescendantResolutionStates.find(ShareRef(element));
    if (it == _deferredDescendantResolutionStates.end())
    {
      return;
    }

    const auto &state = it->second;

    changes.Add(state.Changes);
    descendantsToResolve = std::max(descendantsToResolve, state.DescendantsToResolve);

    _deferredDescendantResolutionStates.erase(it);
  }

  auto TreeResolver::UpdateStateForQueryContainer(dom::Element &element, const RenderStyle *style,
                                                  DescendantsToResolve &descendantsToResolve) noexcept
    -> LayoutInterleavingAction
  {
    if (!style)
    {
      return LayoutInterleavingAction::None;
    }

    if (auto it = _queryContainerStates.find(ShareRef(element));
        it != _queryContainerStates.end() && it->second.Invalidated)
    {
      return LayoutInterleavingAction::None;
    }

    auto *existingStyle = element.RenderOrDisplayContentsStyle();
    if (style->GetContainerType() != ContainerType::Normal
        || (existingStyle && existingStyle->GetContainerType() != ContainerType::Normal))
    {
      // If any of the queries use font-size relative units then a font size change
      // may affect their evaluation, so force re-evaluating all descendants.
      if (StyleChangeAffectsRelativeUnits(*style, existingStyle))
      {
        descendantsToResolve = DescendantsToResolve::All;
      }

      _queryContainerStates.try_emplace(ShareRef(element), QueryContainerState {});

      return LayoutInterleavingAction::SkipDescendants;
    }

    return LayoutInterleavingAction::None;
  }

  UniquePtr<Update> TreeResolver::Resolve() noexcept
  {
    auto didInterleavedLayout = std::exchange(_needsInterleavedLayout, false);
    if (didInterleavedLayout)
    {
      _didFirstInterleavedLayout = true;
    }

    auto documentElement = _document->DocumentElement();
    if (!documentElement)
    {
      (void)_document->StyleScope().GetResolver();
      return nullptr;
    }

    if (didInterleavedLayout)
    {
      AnchorPositionEvaluator::UpdateAnchorPositioningStatesAfterInterleavedLayout(
        *_document, _treeResolutionState.AnchorPositionedStates);
    }

    if (!documentElement->ChildNeedsStyleRecalc() && !documentElement->NeedsStyleRecalc())
    {
      return krys::move(_update);
    }

    _didSeePendingStylesheet = _document->StyleScope().HasPendingSheetsBeforeBody();

    if (!_update)
    {
      _update = CreateUnique<Update>(*_document);
    }
    _scopeStack.push_back(CreateRef<Scope>(*_document, *_update));
    _parentStack.push_back(Parent(*_document));

    ResolveComposedTree();

    krys_debug_assert(_scopeStack.size() == 1);
    krys_debug_assert(_parentStack.size() == 1);
    _parentStack.clear();
    PopScope();

    for (auto &[element, state] : _queryContainerStates)
    {
      // Ensure that resumed resolution reaches the container.
      if (!state.Invalidated)
      {
        element->InvalidateForResumingQueryContainerResolution();
        state.Invalidated = true;

        _needsInterleavedLayout = true;
      }
    }

    for (auto &[key, state] : _treeResolutionState.AnchorPositionedStates)
    {
      // Ensure that style resolution visits any unresolved anchor-positioned elements.
      if (state->Stage < AnchorPositionResolutionStage::Resolved)
      {
        const_cast<dom::Element &>(*key.first).InvalidateForResumingAnchorPositionedElementResolution();
        _needsInterleavedLayout = true;
        SaveBeforeResolutionStyleForInterleaving(*key.first);
      }
    }

    for (auto &[styleable, options] : _positionOptions)
    {
      if (!options.Chosen)
      {
        krys_debug_assert(styleable.first);
        const_cast<dom::Element &>(*styleable.first).InvalidateForResumingAnchorPositionedElementResolution();
        _needsInterleavedLayout = true;
        SaveBeforeResolutionStyleForInterleaving(*styleable.first);
      }
    }

    if (!_changedAnchorNames.empty() || _allAnchorNamesInvalid)
    {
      // If there are changes to the anchor names then loop through the existing anchors and see if any of
      // them references those names.
      for (auto &[address, anchors] : _document->StyleScope().AnchorPositionedToAnchorMap())
      {
        RefPtr<dom::Element> anchorPositionedElement = ShareRefPtr(anchors.Element.get());
        if (!anchorPositionedElement)
        {
          continue;
        }

        bool anchorPositionedReferencesChangedAnchorNames = [&]
        {
          if (_allAnchorNamesInvalid)
          {
            return true;
          }

          for (const auto &anchor : anchors.Anchors)
          {
            if (_changedAnchorNames.contains(anchor.Name.Name()))
            {
              return true;
            }
          }

          return false;
        }();

        if (anchorPositionedReferencesChangedAnchorNames)
        {
          // Invalidate the anchor-positioned element, so subsequent style resolution rounds would visit it.
          anchorPositionedElement->InvalidateForResumingAnchorPositionedElementResolution();

          // Mark that additional style resolution round is needed.
          _needsInterleavedLayout = true;

          // If the anchor-positioned element is currently being tracked for resolution, reset the resolution
          // stage to FindAnchor. This re-runs anchor resolution to pick up new anchor name changes.
          AnchorPositionedKey anchorPositionedKey {anchorPositionedElement, anchors.PseudoElementIdentifier};
          auto stateIt = _treeResolutionState.AnchorPositionedStates.find(anchorPositionedKey);
          if (stateIt != _treeResolutionState.AnchorPositionedStates.end())
          {
            krys_debug_assert(stateIt->second);
            stateIt->second->Stage = AnchorPositionResolutionStage::FindAnchors;
          }
        }
      }

      _changedAnchorNames.clear();
      _allAnchorNamesInvalid = false;
    }

    if (_update->Roots().empty())
    {
      return {};
    }

    // Ensure we do at least one interleaved layout as any style change may affect existing anchor positions.
    if (!_didFirstInterleavedLayout
        && std::ranges::any_of(_document->StyleScope().AnchorPositionedToAnchorMap(),
                               [](const auto &entry) { return entry.second.Element.get(); }))
    {
      _needsInterleavedLayout = true;
    }

    Adjuster::PropagateToDocumentElementAndInitialContainingBlock(*_update, *_document);

    return krys::move(_update);
  }

  const RenderStyle *TreeResolver::BeforeResolutionStyle(
    const dom::Element &element, const Maybe<PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept
  {
    auto resolvePseudoStyle = [&](const RenderStyle *style) -> const RenderStyle *
    {
      if (!pseudoElementIdentifier)
      {
        return style;
      }
      if (!style)
      {
        return nullptr;
      }
      return style->GetCachedPseudoStyle(*pseudoElementIdentifier);
    };

    auto it = _savedBeforeResolutionStylesForInterleaving.find(ShareRef(element));
    if (it != _savedBeforeResolutionStylesForInterleaving.end())
    {
      return resolvePseudoStyle(it->second.get());
    }

    return resolvePseudoStyle(element.RenderOrDisplayContentsStyle());
  }

  void TreeResolver::SaveBeforeResolutionStyleForInterleaving(const dom::Element &element) noexcept
  {
    auto [it, inserted] = _savedBeforeResolutionStylesForInterleaving.try_emplace(ShareRef(element));
    if (!inserted)
    {
      return;
    }
    if (auto *style = element.RenderOrDisplayContentsStyle())
    {
      it->second = CreateUnique<RenderStyle>(RenderStyle::CloneIncludingPseudoElements(*style));
    }
  }

  auto TreeResolver::UpdateAnchorPositioningState(dom::Element &element, const RenderStyle *style) noexcept
    -> LayoutInterleavingAction
  {
    if (!style)
    {
      return LayoutInterleavingAction::None;
    }

    auto update = [&](const RenderStyle *style)
    {
      if (!style)
      {
        return;
      }
      AnchorPositionEvaluator::UpdateAnchorPositionedStateForDefaultAnchorAndPositionVisibility(
        element, *style, _treeResolutionState.AnchorPositionedStates);
    };

    update(style);
    update(style->GetCachedPseudoStyle({PseudoElementType::Before}));
    update(style->GetCachedPseudoStyle({PseudoElementType::After}));

    auto needsInterleavedLayout = HasUnresolvedAnchorPosition({element, null});
    if (needsInterleavedLayout)
    {
      return LayoutInterleavingAction::SkipDescendants;
    }

    return LayoutInterleavingAction::None;
  }

  static Maybe<LayoutSize> ScrollContainerSizeForPositionOptions(const Styleable &anchored) noexcept
  {
    auto *anchoredRenderer = anchored.Renderer();
    if (!anchoredRenderer)
    {
      return null;
    }
    // Overlay scrollbars can't affect anchor() function resolution so we don't need to save the size.
    const auto &containingBlock = *anchoredRenderer->ContainingBlock();
    if (containingBlock.CanUseOverlayScrollbars())
    {
      return null;
    }
    // FIXME: The second test is likely meant to be IsScrollContainerX.
    bool isOverflowScroller = containingBlock.IsScrollContainerY() || containingBlock.IsScrollContainerY();
    if (!containingBlock.IsRenderView() && !isOverflowScroller)
    {
      return null;
    }
    return containingBlock.ContentBoxSize();
  }

  void TreeResolver::GeneratePositionOptionsIfNeeded(const ResolvedStyle &resolvedStyle,
                                                     const Styleable &styleable,
                                                     const ResolutionContext &resolutionContext) noexcept
  {
    // https://drafts.csswg.org/css-anchor-position-1/#fallback-apply

    if (!resolvedStyle.Style || resolvedStyle.Style->GetPositionTryFallbacks().IsNone())
    {
      return;
    }

    if (!resolvedStyle.Style->HasOutOfFlowPosition())
    {
      return;
    }

    AnchorPositionedKey positionOptionsKey {ShareRefPtr<const dom::Element>(&styleable.Element),
                                            styleable.PseudoElementIdentifier};

    if (_positionOptions.contains(positionOptionsKey))
    {
      return;
    }

    auto generatePositionOptions = [&]
    {
      ResolvedStyle clonedResolvedStyle {.Style = RenderStyle::ClonePtr(*resolvedStyle.Style),
                                         .Relations = {},
                                         .MatchResult = resolvedStyle.MatchResult};
      PositionOptions options {.OriginalResolvedStyle = krys::move(clonedResolvedStyle)};

      auto scrollContainerSizeOnGeneration = ScrollContainerSizeForPositionOptions(styleable);
      options.OptionStyles.push_back(
        {RenderStyle::ClonePtr(*resolvedStyle.Style), null, scrollContainerSizeOnGeneration});

      size_t i = 0;
      for (const auto &fallback : resolvedStyle.Style->GetPositionTryFallbacks())
      {
        auto index = i++;
        auto optionStyle =
          GeneratePositionOption(fallback, options.OriginalResolvedStyle, styleable, resolutionContext);
        if (!optionStyle)
        {
          continue;
        }
        optionStyle->SetUsedPositionOptionIndex(index);

        options.OptionStyles.push_back({krys::move(optionStyle), fallback, scrollContainerSizeOnGeneration});
      }

      return options;
    };

    auto options = generatePositionOptions();

    // If the fallbacks contain anchor references we need to resolve the anchors first and regenerate the
    // options.
    if (HasUnresolvedAnchorPosition(styleable))
    {
      return;
    }

    _positionOptions.emplace(krys::move(positionOptionsKey), krys::move(options));
  }

  UniquePtr<RenderStyle>
    TreeResolver::GeneratePositionOption(const PositionTryFallback &fallback,
                                         const ResolvedStyle &resolvedStyle, const Styleable &styleable,
                                         const ResolutionContext &resolutionContext) noexcept
  {
    // https://drafts.csswg.org/css-anchor-position-1/#fallback-apply

    if (!resolvedStyle.MatchResult)
    {
      return nullptr;
    }

    auto resolveFallbackProperties = [&] -> RefPtr<const css::StyleProperties>
    {
      if (fallback.PositionAreaFallback.Properties)
      {
        krys_debug_assert(!fallback.RuleAndTacticsFallback.Rule);
        krys_debug_assert(!fallback.RuleAndTacticsFallback.Tactics);
        return fallback.PositionAreaFallback.Properties;
      }
      if (!fallback.RuleAndTacticsFallback.Rule)
      {
        return nullptr;
      }

      // "If an at-rule or property defines a name that other CSS constructs can refer to it by, ... it must
      // be defined as a tree-scoped name." https://drafts.csswg.org/css-scoping-1/#shadow-names
      return style::Scope::ResolveTreeScopedReference(
        styleable.Element, *fallback.RuleAndTacticsFallback.Rule,
        [](const style::Scope &scope, const css::CSSOMStringAtom &name) -> RefPtr<const css::StyleProperties>
        {
          const auto &ruleSet = scope.ResolverIfExists()->RuleSets().AuthorStyle();
          auto rule = ruleSet.PositionTryRuleForName(name);
          if (!rule)
          {
            return nullptr;
          }
          return rule->ProtectedProperties();
        });
    };

    const auto &tactics = fallback.RuleAndTacticsFallback.Tactics->Value;
    auto builderFallback = BuilderPositionTryFallback {
      .Properties = resolveFallbackProperties(),
      .Tactics = List<PositionTryFallbackTactic>(tactics.begin(), tactics.end()),
    };

    return ResolveAgainInDifferentContext(resolvedStyle, styleable, *resolutionContext.ParentStyle,
                                          PropertyCascade::NormalPropertyTypes(), krys::move(builderFallback),
                                          resolutionContext);
  }

  const RenderStyle &TreeResolver::PositionOptions::OriginalStyle() const noexcept
  {
    krys_debug_assert(!OptionStyles.empty());
    krys_debug_assert(OptionStyles[0].Style);
    return *OptionStyles[0].Style;
  }

  UniquePtr<RenderStyle> TreeResolver::PositionOptions::CurrentOption() const noexcept
  {
    krys_debug_assert(Index < OptionStyles.size());
    krys_debug_assert(OptionStyles[Index].Style);
    return RenderStyle::ClonePtr(*OptionStyles[Index].Style);
  }

  void TreeResolver::SortPositionOptionsIfNeeded(PositionOptions &options,
                                                 const Styleable &styleable) noexcept
  {
    if (options.Sorted)
    {
      return;
    }
    options.Sorted = true;

    auto *box = DynamicDowncast<RenderBox>(styleable.Renderer());
    if (!box || !box->IsOutOfFlowPositioned())
    {
      return;
    }

    auto order = options.OriginalStyle().GetPositionTryOrder();
    if (order != PositionTryOrder::Normal && options.OptionStyles.size() > 2)
    {
      // "For each entry in the position options list, apply that position option to the box, and find the
      // specified inset-modified containing block size that results from those styles."
      // https://drafts.csswg.org/css-anchor-position-1/#position-try-order-property
      auto boxAxis = BoxAxisForPositionTryOrder(order, options.OriginalStyle().GetWritingMode());

      struct SortingOption
      {
        PositionOption Option;
        LayoutUnit ContainingBlockSize;
      };
      List<SortingOption> optionsForSorting;
      optionsForSorting.reserve(options.OptionStyles.size());

      for (size_t i = 1; i < options.OptionStyles.size(); ++i)
      {
        auto constraints = PositionedLayoutConstraints {*box, *options.OptionStyles[i].Style, boxAxis};
        constraints.ComputeInsets();
        optionsForSorting.push_back(
          {krys::move(options.OptionStyles[i]), constraints.InsetModifiedContainingSize()});
      }

      // "Stably sort the position options list according to this size, with the largest coming first."
      std::ranges::stable_sort(optionsForSorting, std::ranges::greater {},
                               &SortingOption::ContainingBlockSize);

      for (size_t i = 0; i < optionsForSorting.size(); ++i)
      {
        options.OptionStyles[i + 1] = krys::move(optionsForSorting[i].Option);
      }
    }

    // If the styleable has a last successful position option...
    if (auto lastSuccessfulIndex = _document->StyleScope().LastSuccessfulPositionOptionIndexFor(styleable))
    {
      // ... find which style in options.OptionStyles has that index
      auto lastSuccessfulOption =
        std::ranges::find_if(options.OptionStyles,
                             [&](const auto &option)
                             {
                               krys_debug_assert(option.Style);
                               return option.Style->UsedPositionOptionIndex() == *lastSuccessfulIndex;
                             });

      // If there's one, move it to the beginning.
      // (if it's at index zero, do nothing since it's already at the beginning)
      if (lastSuccessfulOption != options.OptionStyles.end()
          && lastSuccessfulOption != options.OptionStyles.begin())
      {
        std::rotate(options.OptionStyles.begin(), lastSuccessfulOption, lastSuccessfulOption + 1);
      }
    }
  }

  Maybe<ResolvedStyle>
    TreeResolver::TryChoosePositionOption(const Styleable &styleable,
                                          const ResolutionContext &resolutionContext) noexcept
  {
    // https://drafts.csswg.org/css-anchor-position-1/#fallback-apply

    AnchorPositionedKey anchorPositionedKey {ShareRefPtr<const dom::Element>(&styleable.Element),
                                             styleable.PseudoElementIdentifier};

    auto optionIt = _positionOptions.find(anchorPositionedKey);
    if (optionIt == _positionOptions.end())
    {
      return null;
    }

    auto &options = optionIt->second;

    SortPositionOptionsIfNeeded(options, styleable);

    auto invalidateQueryContainer = [&]
    {
      // Pseudo-elements can't be query containers, so skip invalidating for pseudo-elements.
      if (styleable.PseudoElementIdentifier)
      {
        return;
      }

      auto iter = _queryContainerStates.find(ShareRef(styleable.Element));
      if (iter == _queryContainerStates.end())
      {
        return;
      }

      iter->second.Invalidated = false;
    };

    auto *renderer = DynamicDowncast<RenderBox>(styleable.Renderer());
    if (!renderer)
    {
      invalidateQueryContainer();

      options.Chosen = true;
      options.Index = 0;
      return ResolvedStyle {RenderStyle::ClonePtr(options.OriginalStyle())};
    }

    // On the first try, we force apply the original style (which _could_ be different from the existing
    // style, since the original style might have a fallback applied to it)
    if (options.IsFirstTry)
    {
      invalidateQueryContainer();

      options.IsFirstTry = false;
      options.Index = 0;
      return ResolvedStyle {options.CurrentOption()};
    }

    if (!options.Chosen)
    {
      krys_debug_assert(options.Index < options.OptionStyles.size());

      auto &option = options.OptionStyles[options.Index];
      auto newScrollContainerSize = ScrollContainerSizeForPositionOptions(styleable);

      // Re-generate the options if a scrollbar change changes the view size. It may affect anchor() function
      // resolution.
      if (option.ScrollContainerSizeOnGeneration != newScrollContainerSize)
      {
        option.ScrollContainerSizeOnGeneration = newScrollContainerSize;
        if (option.Option)
        {
          option.Style = GeneratePositionOption(*option.Option, options.OriginalResolvedStyle, styleable,
                                                resolutionContext);
        }

        return ResolvedStyle {options.CurrentOption()};
      }
    }

    // We can't test for overflow before the box has been positioned.
    auto stateIt = _treeResolutionState.AnchorPositionedStates.find(anchorPositionedKey);
    if (stateIt != _treeResolutionState.AnchorPositionedStates.end()
        && stateIt->second->Stage < AnchorPositionResolutionStage::Positioned)
    {
      return ResolvedStyle {options.CurrentOption()};
    }

    if (!AnchorPositionEvaluator::OverflowsInsetModifiedContainingBlock(*renderer))
    {
      // We don't overflow anymore so this is a good style.
      options.Chosen = true;
      return ResolvedStyle {options.CurrentOption()};
    }

    if (options.Chosen)
    {
      // We have already chosen.
      return ResolvedStyle {options.CurrentOption()};
    }

    // Next option to try if this doesn't work.
    ++options.Index;

    invalidateQueryContainer();

    if (options.Index >= options.OptionStyles.size())
    {
      // None of the options worked, return back to the original.
      options.Index = 0;
      options.Chosen = true;
      return ResolvedStyle {options.CurrentOption()};
    }

    return ResolvedStyle {options.CurrentOption()};
  }

  void TreeResolver::UpdateForPositionVisibility(RenderStyle &style, const Styleable &styleable) noexcept
  {
    if (!HasResolvedAnchorPosition(styleable))
    {
      return;
    }

    auto shouldHideAnchorPositioned = [&]
    {
      auto *anchored = DynamicDowncast<RenderBox>(styleable.Renderer());
      if (!anchored)
      {
        return false;
      }

      if (style.GetPositionVisibility().Contains(PositionVisibilityValue::AnchorsVisible))
      {
        // "If the box has a default anchor box but that anchor box is invisible or clipped by intervening
        // boxes, the box’s visibility property computes to force-hidden."
        if (AnchorPositionEvaluator::IsDefaultAnchorInvisibleOrClippedByInterveningBoxes(*anchored))
        {
          return true;
        }
      }
      if (style.GetPositionVisibility().Contains(PositionVisibilityValue::NoOverflow))
      {
        if (AnchorPositionEvaluator::OverflowsInsetModifiedContainingBlock(*anchored))
        {
          return true;
        }
      }
      if (style.GetPositionVisibility().Contains(PositionVisibilityValue::AnchorsValid))
      {
        auto stateIt = _treeResolutionState.AnchorPositionedStates.find(
          {ShareRefPtr<const dom::Element>(&styleable.Element), styleable.PseudoElementIdentifier});
        if (stateIt != _treeResolutionState.AnchorPositionedStates.end())
        {
          for (const auto &[name, anchorElement] : stateIt->second->AnchorElements)
          {
            if (!anchorElement)
            {
              return true;
            }
          }
        }
      }
      return false;
    };

    // FIXME: Implement via "visibility: force-hidden".
    if (shouldHideAnchorPositioned())
    {
      style.SetIsForceHidden();
    }
  }

  bool TreeResolver::HasUnresolvedAnchorPosition(const Styleable &styleable) const noexcept
  {
    auto it = _treeResolutionState.AnchorPositionedStates.find(
      {ShareRefPtr<const dom::Element>(&styleable.Element), styleable.PseudoElementIdentifier});
    return it != _treeResolutionState.AnchorPositionedStates.end()
           && it->second->Stage < AnchorPositionResolutionStage::Resolved;
  }

  bool TreeResolver::HasResolvedAnchorPosition(const Styleable &styleable) const noexcept
  {
    auto it = _treeResolutionState.AnchorPositionedStates.find(
      {ShareRefPtr<const dom::Element>(&styleable.Element), styleable.PseudoElementIdentifier});
    return it != _treeResolutionState.AnchorPositionedStates.end()
           && it->second->Stage >= AnchorPositionResolutionStage::Resolved;
  }

  void TreeResolver::CollectChangedAnchorNames(const RenderStyle &newStyle,
                                               const RenderStyle *currentStyle) noexcept
  {
    // A changed anchor name is either a name being added, a name being removed, or a name whose
    // interpretation changes. This may change which elements get anchored to it.

    if (!currentStyle || currentStyle->GetAnchorNames() != newStyle.GetAnchorNames())
    {
      // This could check which individual names differ but usually there is just one.
      auto addChanged = [&](const RenderStyle &style)
      {
        for (const auto &name : style.GetAnchorNames())
        {
          _changedAnchorNames.insert(name.Name);
        }
      };
      if (currentStyle)
      {
        addChanged(*currentStyle);
      }
      addChanged(newStyle);
    }

    // Only anchor-names in self and descendants are affected by anchor-scope so we need to check only if
    // there is an existing style.
    if (currentStyle && currentStyle->AnchorScope() != newStyle.AnchorScope())
    {
      auto addChanged = [&](const RenderStyle &style)
      {
        switch (style.AnchorScope().Type)
        {
          case NameScope::Type::None: break;
          case NameScope::Type::All:
            // This affects desdendants too so lets just say all names are invalid.
            _allAnchorNamesInvalid = true;
            break;
          case NameScope::Type::Ident:
            // A scope change changes interpretation of these names.
            for (const auto &name : style.AnchorScope().Names)
            {
              _changedAnchorNames.insert(name.Value);
            }
            break;
        }
      };
      addChanged(*currentStyle);
      addChanged(newStyle);
    }
  }

  static List<Func<void()>> &PostResolutionCallbackQueue() noexcept
  {
    static List<Func<void()>> vector;
    return vector;
  }

  void DeprecatedQueuePostResolutionCallback(Func<void()> &&callback) noexcept
  {
    PostResolutionCallbackQueue().push_back(krys::move(callback));
  }

  static uint32 ResolutionNestingDepth = 0;

  PostResolutionCallbackDisabler::PostResolutionCallbackDisabler(dom::Document &,
                                                                 DrainCallbacks drainCallbacks) noexcept
      : _drainCallbacks(drainCallbacks)
  {
    ++ResolutionNestingDepth;

    // FIXME: Suspend the loader's pending requests and the page's memory cache client calls, once there's a
    // loader and a page.
  }

  PostResolutionCallbackDisabler::~PostResolutionCallbackDisabler() noexcept
  {
    if (ResolutionNestingDepth == 1)
    {
      if (_drainCallbacks == DrainCallbacks::Yes)
      {
        // Get size each time through the loop because a callback can add more callbacks to the end of the
        // queue.
        auto &queue = PostResolutionCallbackQueue();
        for (size_t i = 0; i < queue.size(); ++i)
        {
          auto callback = krys::move(queue[i]);
          callback();
        }
        queue.clear();
      }

      // FIXME: Resume the page's memory cache client calls and the loader's pending requests.
    }

    --ResolutionNestingDepth;
  }

  bool PostResolutionCallbacksAreSuspended() noexcept
  {
    return ResolutionNestingDepth;
  }
}
