#include "Krystal.Booey/Style/TreeResolver.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Iterators/ComposedTreeIterator.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/HTMLMeterElement.hpp"
#include "Krystal.Booey/HTML/HTMLProgressElement.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/Adjuster.hpp"
#include "Krystal.Booey/Style/MatchResultCache.hpp"
#include "Krystal.Booey/Style/Relations.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
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
    // FIXME: Scope::updateAnchorPositioningStateAfterStyleResolution, with anchor positioning.
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

  ResolvedStyle TreeResolver::StyleForStyleable(const Styleable &styleable, ResolutionType resolutionType,
                                                const ResolutionContext &resolutionContext,
                                                const RenderStyle *existingStyle) noexcept
  {
    // FIXME: The animation-only resolution (the last style change event style), with animations. Nothing
    // makes a style animation-invalid yet.
    krys_debug_assert(resolutionType != ResolutionType::AnimationOnly);

    auto &element = styleable.Element;

    // FIXME: Choosing a position option (tryChoosePositionOption), with anchor positioning.

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
      // FIXME: Elements with custom style resolve callbacks (resolveCustomStyle).

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

    // FIXME: Forgetting the last successful position option and generating the position options, with anchor
    // positioning.

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

    // FIXME: Dropping the position options of the subtree's elements, with anchor positioning.

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

    // FIXME: Position visibility (updateForPositionVisibility), with anchor positioning.

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
        && ((Is<html::HTMLInputElement>(*htmlElement) && Downcast<html::HTMLInputElement>(*htmlElement).IsSearchField())
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
    const RenderStyle *) noexcept
  {
    if (elementUpdate.Style->Display() == DisplayType::None)
    {
      return null;
    }

    // FIXME: ::backdrop applies to elements in the top layer, once there is a top layer.
    if (pseudoElementIdentifier.Type == PseudoElementType::Backdrop)
    {
      return null;
    }
    if (pseudoElementIdentifier.Type == PseudoElementType::Marker
        && elementUpdate.Style->Display() != DisplayType::ListItem)
    {
      return null;
    }

    // FIXME: The enclosing resolver of user agent shadow tree elements, once there are user agent shadow
    // trees.

    if (pseudoElementIdentifier.Type == PseudoElementType::FirstLine
        && !GetScope().Resolver->UsesFirstLineRules())
    {
      return null;
    }

    if (pseudoElementIdentifier.Type == PseudoElementType::FirstLetter
        && !GetScope().Resolver->UsesFirstLetterRules())
    {
      return null;
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

    Styleable styleable {element, pseudoElementIdentifier};

    // FIXME: Position options for ::before, ::after and ::backdrop, with anchor positioning.
    auto resolvedStyle = GetScope().Resolver->StyleForPseudoElement(
      element, PseudoElementRequest(pseudoElementIdentifier), resolutionContext);

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
    return {&GetParent().Style, ParentBoxStyle(), DocumentElementStyle(), &GetScope().SelectorMatchingState};
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
            &GetScope().SelectorMatchingState};
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
                              DocumentElementStyle(), &GetScope().SelectorMatchingState};
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

  ElementUpdate TreeResolver::CreateAnimatedElementUpdate(ResolvedStyle &&resolvedStyle,
                                                          const Styleable &styleable,
                                                          EnumSet<Change> parentChanges,
                                                          const ResolutionContext &,
                                                          style::IsInDisplayNoneTree) noexcept
  {
    auto &element = styleable.Element;
    auto *currentStyle = element.RenderOrDisplayContentsStyle(styleable.PseudoElementIdentifier);

    // FIXME: Updating and applying the element's CSS animations, transitions, and scroll and view timelines
    // (against the before-resolution or starting style), with animations. The resolved style is the new
    // style.
    (void)BeforeResolutionStyle(element, styleable.PseudoElementIdentifier);
    auto newStyle = krys::move(resolvedStyle.Style);

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

    // FIXME: Collecting changed anchor names, with anchor positioning.

    bool shouldRecompositeLayer = element.StyleResolutionShouldRecompositeLayer();

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

    // FIXME: Clear the last style change event style unless the element has keyframe effects, with
    // animations.

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
    // FIXME: Combine with the validity of the ::before and ::after pseudo-element nodes, once there are any.
    auto combinedValidity = element.StyleValidity();

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
    // FIXME: Also the ::before and ::after pseudo-element nodes, once there are any.
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
          // FIXME: Text nodes have no renderers yet, so check for one (`!text->renderer()`) once they do.
          if (containsOnlyASCIIWhitespace && parent.Style.PreserveNewline())
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

        // FIXME: willRecalcStyle and didRecalcStyle for elements with custom style resolve callbacks, and the
        // accessibility updates (AXObjectCache).
        auto [elementUpdate, elementDescendantsToResolve] = ResolveElement(element, style, *resolutionType);

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
      // FIXME: Anchor-positioned elements (updateAnchorPositioningState), with anchor positioning.

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
        if (queryContainerAction == LayoutInterleavingAction::SkipDescendants)
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

    // FIXME: Updating the anchor positioning states after an interleaved layout, with anchor positioning.

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

    // FIXME: Revisiting unresolved anchor-positioned elements, unchosen position options and the
    // anchor-positioned elements of changed anchor names, with anchor positioning.

    if (_update->Roots().empty())
    {
      return {};
    }

    // FIXME: Ensure one interleaved layout when there are anchor-positioned elements, with anchor
    // positioning.

    Adjuster::PropagateToDocumentElementAndInitialContainingBlock(*_update, *_document);

    return krys::move(_update);
  }

  const RenderStyle *TreeResolver::BeforeResolutionStyle(
    const dom::Element &element, const Maybe<PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept
  {
    // FIXME: The styles saved for layout interleaving (saveBeforeResolutionStyleForInterleaving), with anchor
    // positioning.
    auto *style = element.RenderOrDisplayContentsStyle();
    if (!pseudoElementIdentifier)
    {
      return style;
    }
    if (!style)
    {
      return nullptr;
    }
    return style->GetCachedPseudoStyle(*pseudoElementIdentifier);
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
