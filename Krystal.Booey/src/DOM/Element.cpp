#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Animation/CSSAnimation.hpp"
#include "Krystal.Booey/Animation/KeyframeEffectStack.hpp"
#include "Krystal.Booey/Constants/Attributes.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/OrderedSet.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/HTMLCollection.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Internals/RareData/ElementRareData.hpp"
#include "Krystal.Booey/DOM/Iterators/ComposedTreeIterator.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/Mixins/ChildNode.hpp"
#include "Krystal.Booey/DOM/Mixins/NonDocumentTypeChildNode.hpp"
#include "Krystal.Booey/DOM/Mixins/ParentNode.hpp"
#include "Krystal.Booey/DOM/Mixins/Slottable.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/DOM/NamedNodeMap.hpp"
#include "Krystal.Booey/DOM/NodeList.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/DOM/SelectorQuery.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/StyledElement.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/DOM/UserActionElementSet.hpp"
#include "Krystal.Booey/DOM/WheelEvent.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Platform/PlatformMouseEvent.hpp"
#include "Krystal.Booey/Platform/ScrollAlignment.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderBoxModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/Change.hpp"
#include "Krystal.Booey/Style/ChildChangeInvalidation.hpp"
#include "Krystal.Booey/Style/Invalidator.hpp"
#include "Krystal.Booey/Style/PseudoClassChangeInvalidation.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/TreeResolver.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <ranges>

namespace krys::boo::dom
{
  Element::Element(Document &document, const QualifiedName &name, NodeFlags nodeFlags) noexcept
      : ContainerNode(document, NodeType::ELEMENT_NODE, nodeFlags), _qualifiedName(name)
  {
  }

  Element::~Element() noexcept
  {
    krys_debug_assert(!BeforePseudoElement());
    krys_debug_assert(!AfterPseudoElement());
  }

  ElementRareData &Element::EnsureElementRareData() noexcept
  {
    if (!_elementRareData)
    {
      _elementRareData = CreateUnique<ElementRareData>();
    }
    return *_elementRareData;
  }

  double
    Element::LookupCSSRandomBaseValue(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier,
                                      const css::calc::RandomCachingKey &key) const noexcept
  {
    return const_cast<Element *>(this)
      ->EnsureElementRareData()
      .EnsureRandomCachingKeyMap(pseudoElementIdentifier)
      ->LookupCSSRandomBaseValue(key);
  }

#pragma region Style

  const RenderStyle *Element::ExistingComputedStyle() const noexcept
  {
    if (_elementRareData)
    {
      if (auto *style = _elementRareData->ComputedStyle())
      {
        return style;
      }
    }

    if (HasDisplayNone())
    {
      return _elementRareData->DisplayContentsOrNoneStyle();
    }

    return RenderOrDisplayContentsStyle();
  }

  const RenderStyle *Element::RenderOrDisplayContentsStyle() const noexcept
  {
    return RenderOrDisplayContentsStyle(null);
  }

  const RenderStyle *Element::RenderOrDisplayContentsStyle(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept
  {
    if (pseudoElementIdentifier)
    {
      if (auto *style = RenderOrDisplayContentsStyle())
      {
        if (auto *cachedPseudoStyle = style->GetCachedPseudoStyle(*pseudoElementIdentifier))
        {
          return cachedPseudoStyle;
        }
      }

      return nullptr;
    }

    if (HasDisplayContents())
    {
      return _elementRareData->DisplayContentsOrNoneStyle();
    }

    return GetRenderStyle();
  }

  const RenderStyle *Element::ResolveComputedStyle(ResolveComputedStyleMode mode) noexcept
  {
    krys_debug_assert(IsConnected());

    auto document = ShareRef(NodeDocument());
    document->StyleScope().FlushPendingUpdate();

    bool isInDisplayNoneTree = false;

    // Traverse the ancestor chain to find the rootmost element that has invalid computed style.
    auto rootmostInvalidElement = [&]() -> RefPtr<const Element>
    {
      // In ResolveComputedStyleMode::RenderedOnly case we check for display:none ancestors.
      if (mode != ResolveComputedStyleMode::RenderedOnly && !document->HasPendingStyleRecalc()
          && ExistingComputedStyle())
      {
        return nullptr;
      }

      if (document->HasPendingFullStyleRebuild())
      {
        return document->DocumentElement();
      }

      auto documentElement = document->DocumentElement();
      if (!documentElement || documentElement->HasNodeFlag(NodeFlags::IsComputedStyleInvalid))
      {
        return documentElement;
      }

      RefPtr<const Element> rootmost;

      for (auto element = ShareRefPtr<const Element>(this); element;
           element = ShareRefPtr<const Element>(element->ParentElementInComposedTree()))
      {
        if (element->HasNodeFlag(NodeFlags::IsComputedStyleInvalid))
        {
          rootmost = element;
          continue;
        }
        auto *existing = element->ExistingComputedStyle();
        if (!existing)
        {
          rootmost = element;
          continue;
        }
        if (mode == ResolveComputedStyleMode::RenderedOnly && existing->Display() == DisplayType::None)
        {
          isInDisplayNoneTree = true;
          // Invalid ancestor style may still affect this display:none style.
          rootmost = nullptr;
        }
      }
      return rootmost;
    }();

    if (!rootmostInvalidElement)
    {
      if (isInDisplayNoneTree)
      {
        return nullptr;
      }
      return ExistingComputedStyle();
    }

    auto ancestorWithValidStyle = ShareRefPtr(rootmostInvalidElement->ParentElementInComposedTree());

    List<RefPtr<Element>> elementsRequiringComputedStyle;
    for (auto toResolve = ShareRefPtr(this); toResolve != ancestorWithValidStyle;
         toResolve = ShareRefPtr(toResolve->ParentElementInComposedTree()))
    {
      elementsRequiringComputedStyle.push_back(toResolve);
    }

    auto *computedStyle = ancestorWithValidStyle ? ancestorWithValidStyle->ExistingComputedStyle() : nullptr;

    style::PostResolutionCallbackDisabler disabler(*document,
                                                   style::PostResolutionCallbackDisabler::DrainCallbacks::No);

    // Resolve and cache styles starting from the most distant ancestor.
    // FIXME: This is not as efficient as it could be. For example if an ancestor has a non-inherited style
    // change but the styles are otherwise clean we would not need to re-resolve descendants.
    for (auto &element : elementsRequiringComputedStyle | std::views::reverse)
    {
      if (computedStyle && computedStyle->GetContainerType() != ContainerType::Normal
          && mode != ResolveComputedStyleMode::Editability)
      {
        // If we find a query container we need to bail out and do full style update to resolve it.
        if (document->UpdateStyleIfNeeded())
        {
          return ComputedStyle();
        }
      }
      auto style = document->StyleForElementIgnoringPendingStylesheets(*element, computedStyle);
      computedStyle = style.get();
      auto &rareData = element->EnsureElementRareData();
      if (auto *existing = rareData.ComputedStyle())
      {
        auto changes = style::DetermineChanges(*existing, *style);
        changes.Remove(style::Change::NonInherited);
        if (!changes.IsEmpty())
        {
          for (auto &child : ComposedTreeChildren(*element))
          {
            if (auto *childElement = DynamicDowncast<Element>(child))
            {
              childElement->SetNodeFlag(NodeFlags::IsComputedStyleInvalid);
            }
          }
        }
      }
      rareData.SetComputedStyle(krys::move(style));
      element->ClearNodeFlag(NodeFlags::IsComputedStyleInvalid);

      if (mode == ResolveComputedStyleMode::RenderedOnly && computedStyle->Display() == DisplayType::None)
      {
        return nullptr;
      }
    }

    return computedStyle;
  }

  const RenderStyle &
    Element::ResolvePseudoElementStyle(const style::PseudoElementIdentifier &pseudoElementIdentifier) noexcept
  {
    auto *parentStyle = ExistingComputedStyle();
    krys_debug_assert(parentStyle);
    krys_debug_assert(!parentStyle->GetCachedPseudoStyle(pseudoElementIdentifier));

    auto document = ShareRef(NodeDocument());
    style::PostResolutionCallbackDisabler disabler(*document,
                                                   style::PostResolutionCallbackDisabler::DrainCallbacks::No);

    auto style =
      document->StyleForElementIgnoringPendingStylesheets(*this, parentStyle, pseudoElementIdentifier);
    if (!style)
    {
      style = RenderStyle::CreatePtr();
      style->InheritFrom(*parentStyle);
      style->SetPseudoElementIdentifier(pseudoElementIdentifier);
    }

    auto *computedStyle = style.get();
    const_cast<RenderStyle *>(parentStyle)->AddCachedPseudoStyle(krys::move(style));
    krys_debug_assert(parentStyle->GetCachedPseudoStyle(pseudoElementIdentifier));
    return *computedStyle;
  }

  const RenderStyle *
    Element::ComputedStyle(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
  {
    if (!IsConnected())
    {
      return nullptr;
    }

    if (pseudoElementIdentifier)
    {
      if (auto pseudoElement = PseudoElementIfExists(*pseudoElementIdentifier))
      {
        return pseudoElement->ComputedStyle();
      }
    }

    // FIXME: This should call resolveComputedStyle() unconditionally so we check if the style is valid.
    auto *style = ExistingComputedStyle();
    if (!style)
    {
      style = ResolveComputedStyle();
    }

    if (pseudoElementIdentifier)
    {
      if (auto *cachedPseudoStyle = style->GetCachedPseudoStyle(*pseudoElementIdentifier))
      {
        return cachedPseudoStyle;
      }
      return &ResolvePseudoElementStyle(*pseudoElementIdentifier);
    }

    return style;
  }

  void Element::ResetComputedStyle() noexcept
  {
    if (!_elementRareData || !_elementRareData->ComputedStyle())
    {
      return;
    }

    // FIXME: willResetComputedStyle for elements with custom style resolve callbacks.
    _elementRareData->SetComputedStyle(nullptr);
    for (auto *descendant = TreeQueries::Next(*this, this); descendant;
         descendant = TreeQueries::Next(*descendant, this))
    {
      auto *child = DynamicDowncast<Element>(*descendant);
      if (!child || !child->_elementRareData || !child->_elementRareData->ComputedStyle()
          || child->HasDisplayContents() || child->HasDisplayNone())
      {
        continue;
      }
      child->_elementRareData->SetComputedStyle(nullptr);
    }
  }

  void Element::ResetStyleRelations() noexcept
  {
    ClearStyleFlags(NodeStyleFlags::StyleAffectedByEmpty);
    ClearStyleFlags(NodeStyleFlags::AffectedByHasWithPositionalPseudoClass);
    if (!_elementRareData)
    {
      return;
    }
    _elementRareData->SetChildIndex(0);
  }

  void Element::ResetChildStyleRelations() noexcept
  {
    ClearStyleFlags(NodeStyleFlags::ChildrenAffectedByFirstChildRules
                    | NodeStyleFlags::ChildrenAffectedByLastChildRules
                    | NodeStyleFlags::ChildrenAffectedByForwardPositionalRules
                    | NodeStyleFlags::ChildrenAffectedByBackwardPositionalRules);
  }

  void Element::ResetAllDescendantStyleRelations() noexcept
  {
    ResetChildStyleRelations();

    ClearStyleFlags(NodeStyleFlags::DescendantsAffectedByForwardPositionalRules
                    | NodeStyleFlags::DescendantsAffectedByBackwardPositionalRules);
  }

  bool Element::HasDisplayContents() const noexcept
  {
    if (!_elementRareData)
    {
      return false;
    }
    auto *style = _elementRareData->DisplayContentsOrNoneStyle();
    return style && style->Display() == DisplayType::Contents;
  }

  bool Element::HasDisplayNone() const noexcept
  {
    if (!_elementRareData)
    {
      return false;
    }
    auto *style = _elementRareData->DisplayContentsOrNoneStyle();
    return style && style->Display() == DisplayType::None;
  }

  void Element::StoreDisplayContentsOrNoneStyle(UniquePtr<RenderStyle> style) noexcept
  {
    // This is used by RenderTreeBuilder to store the style for Elements with display:{contents|none}.
    // Normally style is held in renderers but display:contents doesn't generate one.
    // This is kept distinct from ElementRareData::computedStyle() which can update outside style resolution.
    // This way renderOrDisplayContentsStyle() always returns consistent styles matching the rendering state.
    krys_debug_assert(
      style && (style->Display() == DisplayType::Contents || style->Display() == DisplayType::None));
    krys_debug_assert(!Renderer() || IsPseudoElement());
    EnsureElementRareData().SetDisplayContentsOrNoneStyle(krys::move(style));
  }

  void Element::ClearDisplayContentsOrNoneStyle() noexcept
  {
    if (!_elementRareData)
    {
      return;
    }
    _elementRareData->SetDisplayContentsOrNoneStyle(nullptr);
  }

  RenderPtr<RenderElement> Element::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return RenderElement::CreateFor(*this, krys::move(style));
  }

  bool Element::RendererIsNeeded(const RenderStyle &style) noexcept
  {
    return style.Display() != DisplayType::None && style.Display() != DisplayType::Contents;
  }

  Maybe<DOMString> Element::ImageSourceURL() const noexcept
  {
    return GetAttribute(u8"src");
  }

  RenderElement *Element::Renderer() const noexcept
  {
    return static_cast<RenderElement *>(Node::Renderer());
  }

  PseudoElement &Element::EnsurePseudoElement(PseudoElementType type) noexcept
  {
    if (type == PseudoElementType::Before)
    {
      if (!BeforePseudoElement())
      {
        EnsureElementRareData().SetBeforePseudoElement(PseudoElement::Create(*this, type));
      }
      return *BeforePseudoElement();
    }

    krys_debug_assert(type == PseudoElementType::After);
    if (!AfterPseudoElement())
    {
      EnsureElementRareData().SetAfterPseudoElement(PseudoElement::Create(*this, type));
    }
    return *AfterPseudoElement();
  }

  PseudoElement *Element::BeforePseudoElement() const noexcept
  {
    return _elementRareData ? _elementRareData->BeforePseudoElement() : nullptr;
  }

  PseudoElement *Element::AfterPseudoElement() const noexcept
  {
    return _elementRareData ? _elementRareData->AfterPseudoElement() : nullptr;
  }

  IntPoint Element::SavedLayerScrollPosition() const noexcept
  {
    return _elementRareData ? _elementRareData->SavedLayerScrollPosition() : IntPoint();
  }

  void Element::SetSavedLayerScrollPosition(const IntPoint &position) noexcept
  {
    if (position.IsZero() && !_elementRareData)
    {
      return;
    }
    EnsureElementRareData().SetSavedLayerScrollPosition(position);
  }

  RefPtr<PseudoElement>
    Element::PseudoElementIfExists(const style::PseudoElementIdentifier &pseudoElementIdentifier) noexcept
  {
    if (pseudoElementIdentifier.Type == PseudoElementType::Before)
    {
      return ShareRefPtr(BeforePseudoElement());
    }
    if (pseudoElementIdentifier.Type == PseudoElementType::After)
    {
      return ShareRefPtr(AfterPseudoElement());
    }
    return nullptr;
  }

  RefPtr<const PseudoElement> Element::PseudoElementIfExists(
    const style::PseudoElementIdentifier &pseudoElementIdentifier) const noexcept
  {
    return const_cast<Element &>(*this).PseudoElementIfExists(pseudoElementIdentifier);
  }

  static void DisconnectPseudoElement(PseudoElement *pseudoElement) noexcept
  {
    if (!pseudoElement)
    {
      return;
    }
    krys_debug_assert(!pseudoElement->Renderer());
    krys_debug_assert(pseudoElement->HostElement());
    pseudoElement->ClearHostElement();
  }

  void Element::ClearBeforePseudoElement() noexcept
  {
    if (!_elementRareData || !_elementRareData->BeforePseudoElement())
    {
      return;
    }
    DisconnectPseudoElement(_elementRareData->BeforePseudoElement());
    _elementRareData->SetBeforePseudoElement(nullptr);
  }

  void Element::ClearAfterPseudoElement() noexcept
  {
    if (!_elementRareData || !_elementRareData->AfterPseudoElement())
    {
      return;
    }
    DisconnectPseudoElement(_elementRareData->AfterPseudoElement());
    _elementRareData->SetAfterPseudoElement(nullptr);
  }

  namespace
  {
    void InvalidateSiblingsIfNeeded(Element &element) noexcept
    {
      if (!element.AffectsNextSiblingElementStyle())
      {
        return;
      }
      auto *parent = element.ParentElement();
      if (parent && parent->StyleValidity() >= style::Validity::SubtreeInvalid)
      {
        return;
      }

      InvalidateForSiblingCombinators(TreeQueries::NextElementSibling(element));
    }
  }

  void Element::InvalidateStyle() noexcept
  {
    Node::InvalidateStyle(style::Validity::ElementInvalid);
    InvalidateSiblingsIfNeeded(*this);
  }

  void Element::InvalidateStyleAndLayerComposition() noexcept
  {
    Node::InvalidateStyle(style::Validity::ElementInvalid, style::InvalidationMode::RecompositeLayer);
    InvalidateSiblingsIfNeeded(*this);
  }

  void Element::InvalidateStyleForSubtree() noexcept
  {
    Node::InvalidateStyle(style::Validity::SubtreeInvalid);
    InvalidateSiblingsIfNeeded(*this);
  }

  void Element::InvalidateStyleAndRenderersForSubtree() noexcept
  {
    Node::InvalidateStyle(style::Validity::SubtreeInvalid, style::InvalidationMode::RebuildRenderer);
  }

  void Element::InvalidateRenderer() noexcept
  {
    Node::InvalidateStyle(style::Validity::Valid, style::InvalidationMode::RebuildRenderer);
  }

  void Element::InvalidateStyleInternal() noexcept
  {
    Node::InvalidateStyle(style::Validity::ElementInvalid);
  }

  void Element::InvalidateStyleForSubtreeInternal() noexcept
  {
    Node::InvalidateStyle(style::Validity::SubtreeInvalid);
  }

  void Element::InvalidateForQueryContainerSizeChange() noexcept
  {
    // FIXME: Ideally we would just recompute things that are actually affected by containers queries within
    // the subtree.
    Node::InvalidateStyle(style::Validity::SubtreeInvalid);
    SetNodeFlag(NodeFlags::NeedsUpdateQueryContainerDependentStyle);
  }

  void Element::InvalidateForResumingQueryContainerResolution() noexcept
  {
    SetChildNeedsStyleRecalc();
    MarkAncestorsForInvalidatedStyle();
  }

  void Element::FinishParsingChildren() noexcept
  {
    ClearNodeFlag(NodeFlags::IsParsingChildren);
    style::ChildChangeInvalidation::InvalidateAfterFinishedParsingChildren(*this);
  }

#pragma region User Action State

  bool Element::IsUserActionElementInActiveChain() const noexcept
  {
    krys_debug_assert(IsUserActionElement());
    return NodeDocument().UserActionElements().IsInActiveChain(*this);
  }

  bool Element::IsUserActionElementActive() const noexcept
  {
    krys_debug_assert(IsUserActionElement());
    return NodeDocument().UserActionElements().IsActive(*this);
  }

  bool Element::IsUserActionElementFocused() const noexcept
  {
    krys_debug_assert(IsUserActionElement());
    return NodeDocument().UserActionElements().IsFocused(*this);
  }

  Element::DispatchMouseEventResult Element::DispatchMouseEvent(const PlatformMouseEvent &platformEvent,
                                                                DOMStringAtom eventType, int32 detail,
                                                                Element *relatedTarget) noexcept
  {
    auto eventIsDefaultPrevented = EventIsDefaultPrevented::No;

    Ref<MouseEvent> mouseEvent =
      MouseEvent::Create(eventType, NodeDocument().View(), platformEvent, detail, relatedTarget);

    if (mouseEvent->Type().View().empty())
    {
      return {EventIsDispatched::Yes, eventIsDefaultPrevented}; // Shouldn't happen.
    }

    Ref protectedThis = ShareRef(*this);
    bool didNotSwallowEvent = true;

    // FIXME: Pointer events (dispatchPointerEventIfNeeded), which dispatch clicks as pointer events; until
    // then clicks are mouse events.
    krys_debug_assert(!mouseEvent->Target() || mouseEvent->Target() != relatedTarget);
    (void)EventDispatcher::DispatchToTarget(*mouseEvent, this);
    if (mouseEvent->DefaultPrevented())
    {
      eventIsDefaultPrevented = EventIsDefaultPrevented::Yes;
      didNotSwallowEvent = false;
    }

    // The document should not receive dblclick for non-primary buttons.
    if (mouseEvent->Type() == EventNames::Click && mouseEvent->Detail() == 2)
    {
      // Special case: If it's a double click event, we also send the dblclick event. This is not part
      // of the DOM specs, but is used for compatibility with the ondblclick="" attribute. This is treated
      // as a separate event in other DOM-compliant browsers like Firefox, and so we do the same.
      Ref<MouseEvent> doubleClickEvent = MouseEvent::Create(
        DOMStringAtom(EventNames::DblClick), mouseEvent->Bubbles(), mouseEvent->Cancelable(), true,
        mouseEvent->TimeStamp(), mouseEvent->View(), mouseEvent->Detail(), mouseEvent->ScreenX(),
        mouseEvent->ScreenY(), mouseEvent->ClientX(), mouseEvent->ClientY(), mouseEvent->ModifierKeys(),
        mouseEvent->ButtonType(), mouseEvent->Buttons(), relatedTarget);

      (void)EventDispatcher::DispatchToTarget(*doubleClickEvent, this);
      if (doubleClickEvent->DefaultPrevented())
      {
        return {EventIsDispatched::No, eventIsDefaultPrevented};
      }
    }
    return {didNotSwallowEvent ? EventIsDispatched::Yes : EventIsDispatched::No, eventIsDefaultPrevented};
  }

  bool Element::DispatchWheelEvent(const PlatformWheelEvent &platformEvent,
                                   EnumSet<EventHandling> &processing, bool isCancelable) noexcept
  {
    Ref<WheelEvent> event =
      WheelEvent::Create(platformEvent, NodeDocument().View(),
                         isCancelable ? WheelEvent::IsCancelable::Yes : WheelEvent::IsCancelable::No);

    // Events with no deltas are important because they convey platform information about scroll gestures
    // and momentum beginning or ending. However, those events should not be sent to the DOM since some
    // websites will break. They need to be dispatched because dispatching them will call into the default
    // event handler, and our platform code will correctly handle the phase changes. Calling stopPropagation()
    // will prevent the event from being sent to the DOM, but will still call the default event handler.
    if (platformEvent.Delta().IsZero())
    {
      event->StopPropagation();
    }
    else
    {
      processing.Add(EventHandling::DispatchedToDOM);
    }

    Ref protectedThis = ShareRef(*this);
    (void)EventDispatcher::DispatchToTarget(*event, this);

    if (event->DefaultPrevented())
    {
      processing.Add(EventHandling::DefaultPrevented);
      return false;
    }

    // The default action: if we don't have a renderer, send the wheel event to the first node we find with a
    // renderer. This is needed for <option> and <optgroup> elements so that <select>s get a wheel scroll.
    Node *startNode = this;
    while (startNode && !startNode->Renderer())
    {
      startNode = TreeQueries::ParentOrShadowHostElement(*startNode);
    }
    if (startNode)
    {
      if (auto *frame = NodeDocument().Frame();
          frame && frame->GetEventHandler().DefaultWheelEventHandler(startNode, *event))
      {
        processing.Add(EventHandling::DefaultHandled);
        return false;
      }
    }
    return true;
  }

  bool Element::DispatchKeyEvent(const PlatformKeyboardEvent &platformEvent) noexcept
  {
    Ref<KeyboardEvent> event = KeyboardEvent::Create(platformEvent);
    // FIXME: Accessibility stopping the propagation (accessibilityPreventsEventPropagation), once there's
    // accessibility.
    (void)EventDispatcher::DispatchToTarget(*event, this);
    if (event->DefaultPrevented())
    {
      return false;
    }
    auto *frame = NodeDocument().Frame();
    return !frame || !frame->GetEventHandler().DefaultKeyboardEventHandler(*event);
  }

  bool Element::IsUserActionElementHovered() const noexcept
  {
    krys_debug_assert(IsUserActionElement());
    return NodeDocument().UserActionElements().IsHovered(*this);
  }

  bool Element::IsUserActionElementHasFocusVisible() const noexcept
  {
    krys_debug_assert(IsUserActionElement());
    return NodeDocument().UserActionElements().HasFocusVisible(*this);
  }

  bool Element::IsUserActionElementHasFocusWithin() const noexcept
  {
    krys_debug_assert(IsUserActionElement());
    return NodeDocument().UserActionElements().HasFocusWithin(*this);
  }

  void Element::SetActive(bool value, style::InvalidationScope invalidationScope) noexcept
  {
    if (value == Active())
    {
      return;
    }
    {
      style::PseudoClassChangeInvalidation styleInvalidation(*this, css::SelectorPseudoClass::Active, value,
                                                             invalidationScope);
      NodeDocument().UserActionElements().SetActive(*this, value);
    }

    // FIXME: Repaint elements with a used appearance, once there are renderers.
  }

  bool Element::CanContainRangeEndPoint() const noexcept
  {
    auto role = GetAttribute(u8"role");
    return !role || !infra::StringAlgorithms::ASCIICaseInsensitiveMatch(DOMStringView(*role), u8"img");
  }

  namespace
  {
    /// @brief Whether @p element is a text field: a text area, or an input of a text-like type.
    KRYS_NODISCARD bool IsTextField(const Element &element) noexcept
    {
      if (!Is<html::HTMLElement>(element))
      {
        return false;
      }
      auto tagName = element.TagQualifiedName().TagName();
      if (tagName == html::TagName::textarea)
      {
        return true;
      }
      if (tagName != html::TagName::input)
      {
        return false;
      }
      // Missing and unknown types are text.
      auto type = text::ToASCIILower(DOMStringView(element.GetAttribute(u8"type").value_or(DOMString())));
      static constexpr DOMStringView nonTextFieldTypes[] = {
        u8"hidden",   u8"date",  u8"month", u8"week",   u8"time",  u8"datetime-local", u8"range", u8"color",
        u8"checkbox", u8"radio", u8"file",  u8"submit", u8"image", u8"reset",          u8"button"};
      return std::ranges::find(nonTextFieldTypes, DOMStringView(type)) == std::end(nonTextFieldTypes);
    }

    KRYS_NODISCARD bool ShouldAlwaysHaveFocusVisibleWhenFocused(const Element &element) noexcept
    {
      if (IsTextField(element))
      {
        return true;
      }
      if (!Is<html::HTMLElement>(element))
      {
        return false;
      }
      switch (element.TagQualifiedName().TagName())
      {
        case html::TagName::select:   return true;
        case html::TagName::input:
        case html::TagName::textarea: return false;
        // Whether the element is editable (`contenteditable`).
        default:                      return css::MatchesReadWritePseudoClass(element);
      }
    }
  }

  void Element::SetFocus(bool value, FocusVisibility visibility) noexcept
  {
    if (value == Focused())
    {
      return;
    }

    style::PseudoClassChangeInvalidation focusStyleInvalidation(
      *this, {{css::SelectorPseudoClass::Focus, value}, {css::SelectorPseudoClass::FocusVisible, value}});
    NodeDocument().UserActionElements().SetFocused(*this, value);

    // Shadow host with a slot that contain focused element is not considered focused.
    for (auto *root = TreeQueries::ContainingShadowRoot(*this); root;
         root = TreeQueries::ContainingShadowRoot(*root->Host()))
    {
      root->SetContainsFocusedElement(value);
      root->Host()->InvalidateStyle();
    }

    for (auto *element = this; element; element = element->ParentElementInComposedTree())
    {
      element->SetHasFocusWithin(value);
    }

    SetHasFocusVisible(
      value
      && (visibility == FocusVisibility::Visible
          || (visibility == FocusVisibility::Invisible && ShouldAlwaysHaveFocusVisibleWhenFocused(*this))));
  }

  void Element::SetHasFocusVisible(bool value) noexcept
  {
    krys_debug_assert(!value || Focused());

    if (HasFocusVisible() == value)
    {
      return;
    }

    NodeDocument().UserActionElements().SetHasFocusVisible(*this, value);
  }

  void Element::SetHasFocusWithin(bool value) noexcept
  {
    if (HasFocusWithin() == value)
    {
      return;
    }
    {
      style::PseudoClassChangeInvalidation styleInvalidation(*this, css::SelectorPseudoClass::FocusWithin,
                                                             value);
      NodeDocument().UserActionElements().SetHasFocusWithin(*this, value);
    }
  }

  void Element::SetHasTentativeFocus(bool value) noexcept
  {
    // Tentative focus is used when trying to set the focus on a new element.
    for (auto *ancestor = ParentElementInComposedTree(); ancestor;
         ancestor = ancestor->ParentElementInComposedTree())
    {
      krys_debug_assert(ancestor->HasFocusWithin() != value);
      NodeDocument().UserActionElements().SetHasFocusWithin(*ancestor, value);
    }
  }

  void Element::SetHovered(bool value, style::InvalidationScope invalidationScope, HitTestRequest) noexcept
  {
    if (value == Hovered())
    {
      return;
    }
    {
      style::PseudoClassChangeInvalidation styleInvalidation(*this, css::SelectorPseudoClass::Hover, value,
                                                             invalidationScope);
      NodeDocument().UserActionElements().SetHovered(*this, value);
    }

    // FIXME: Repaint elements with a used appearance whose theme supports hover, once there are renderers.
  }

  void Element::ClearHoverAndActiveStatusBeforeDetachingRenderer() noexcept
  {
    if (!IsUserActionElement())
    {
      return;
    }
    auto &document = NodeDocument();
    if (Hovered())
    {
      document.HoveredElementDidDetach(*this);
    }
    if (IsInActiveChain())
    {
      document.ElementInActiveChainDidDetach(*this);
    }
    document.UserActionElements().ClearActiveAndHovered(*this);
  }

#pragma endregion

  bool Element::NeedsStyleInvalidation() const noexcept
  {
    if (!IsConnected())
    {
      return false;
    }
    // If :has() is present a change in an element may affect elements outside its subtree.
    if (StyleValidity() >= style::Validity::SubtreeInvalid
        && !style::Scope::ForNode(*this).UsesHasPseudoClass())
    {
      return false;
    }
    auto *documentElement = TreeQueries::DocumentElement(NodeDocument());
    if (documentElement && documentElement->StyleValidity() >= style::Validity::SubtreeInvalid)
    {
      return false;
    }
    if (NodeDocument().HasPendingFullStyleRebuild())
    {
      return false;
    }

    return true;
  }

  style::Resolver &Element::StyleResolver() noexcept
  {
    if (auto *shadowRoot = TreeQueries::ContainingShadowRoot(*this))
    {
      return shadowRoot->StyleScope().GetResolver();
    }

    return NodeDocument().StyleScope().GetResolver();
  }

  void InvalidateForSiblingCombinators(Element *sibling) noexcept
  {
    for (auto *element = sibling; element; element = TreeQueries::NextElementSibling(*element))
    {
      if (element->StyleIsAffectedByPreviousSibling())
      {
        element->InvalidateStyleInternal();
      }
      if (element->DescendantsAffectedByPreviousSibling())
      {
        for (auto *siblingChild = TreeQueries::FirstElementChild(*element); siblingChild;
             siblingChild = TreeQueries::NextElementSibling(*siblingChild))
        {
          siblingChild->InvalidateStyleForSubtreeInternal();
        }
      }
      if (!element->AffectsNextSiblingElementStyle())
      {
        return;
      }
    }
  }

  bool Element::HasFlagsSetDuringStylingOfChildren() const noexcept
  {
    return ChildrenAffectedByFirstChildRules() || ChildrenAffectedByLastChildRules()
           || ChildrenAffectedByForwardPositionalRules() || DescendantsAffectedByForwardPositionalRules()
           || ChildrenAffectedByBackwardPositionalRules() || DescendantsAffectedByBackwardPositionalRules();
  }

  uint32 Element::ChildIndex() const noexcept
  {
    return _elementRareData ? _elementRareData->ChildIndex() : 0;
  }

  void Element::SetChildIndex(uint32 index) noexcept
  {
    EnsureElementRareData().SetChildIndex(index);
  }

#pragma endregion

#pragma region Animations

  bool Element::MayHaveKeyframeEffects() const noexcept
  {
    return _elementRareData && _elementRareData->HasAnimationRareData();
  }

  ElementAnimationRareData *Element::AnimationRareData(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept
  {
    return _elementRareData ? _elementRareData->AnimationRareData(pseudoElementIdentifier) : nullptr;
  }

  ElementAnimationRareData &Element::EnsureAnimationRareData(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
  {
    return EnsureElementRareData().EnsureAnimationRareData(pseudoElementIdentifier);
  }

  KeyframeEffectStack *Element::GetKeyframeEffectStack(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept
  {
    if (auto *animationData = AnimationRareData(pseudoElementIdentifier))
    {
      return animationData->KeyframeEffectStack();
    }
    return nullptr;
  }

  KeyframeEffectStack &Element::EnsureKeyframeEffectStack(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
  {
    return EnsureAnimationRareData(pseudoElementIdentifier).EnsureKeyframeEffectStack();
  }

  bool Element::HasKeyframeEffects(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept
  {
    if (auto *animationData = AnimationRareData(pseudoElementIdentifier))
    {
      if (auto *keyframeEffectStack = animationData->KeyframeEffectStack())
      {
        return keyframeEffectStack->HasEffects();
      }
    }
    return false;
  }

  const AnimationCollection *
    Element::Animations(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept
  {
    if (auto *animationData = AnimationRareData(pseudoElementIdentifier))
    {
      return &animationData->Animations();
    }
    return nullptr;
  }

  AnimationCollection &
    Element::EnsureAnimations(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
  {
    return EnsureAnimationRareData(pseudoElementIdentifier).Animations();
  }

  bool Element::HasCompletedTransitionForProperty(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier,
    const AnimatableCSSProperty &property) const noexcept
  {
    if (auto *animationData = AnimationRareData(pseudoElementIdentifier))
    {
      return animationData->CompletedTransitionsByProperty().contains(property);
    }
    return false;
  }

  bool Element::HasRunningTransitionForProperty(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier,
    const AnimatableCSSProperty &property) const noexcept
  {
    if (auto *animationData = AnimationRareData(pseudoElementIdentifier))
    {
      return animationData->RunningTransitionsByProperty().contains(property);
    }
    return false;
  }

  bool Element::HasRunningTransitions(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept
  {
    if (auto *animationData = AnimationRareData(pseudoElementIdentifier))
    {
      return !animationData->RunningTransitionsByProperty().empty();
    }
    return false;
  }

  const AnimatableCSSPropertyToTransitionMap *Element::CompletedTransitionsByProperty(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept
  {
    if (auto *animationData = AnimationRareData(pseudoElementIdentifier))
    {
      return &animationData->CompletedTransitionsByProperty();
    }
    return nullptr;
  }

  const AnimatableCSSPropertyToTransitionMap *Element::RunningTransitionsByProperty(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept
  {
    if (auto *animationData = AnimationRareData(pseudoElementIdentifier))
    {
      return &animationData->RunningTransitionsByProperty();
    }
    return nullptr;
  }

  AnimatableCSSPropertyToTransitionMap &Element::EnsureCompletedTransitionsByProperty(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
  {
    return EnsureAnimationRareData(pseudoElementIdentifier).CompletedTransitionsByProperty();
  }

  AnimatableCSSPropertyToTransitionMap &Element::EnsureRunningTransitionsByProperty(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
  {
    return EnsureAnimationRareData(pseudoElementIdentifier).RunningTransitionsByProperty();
  }

  const RenderStyle *Element::LastStyleChangeEventStyle(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept
  {
    if (auto *animationData = AnimationRareData(pseudoElementIdentifier))
    {
      return animationData->LastStyleChangeEventStyle();
    }
    return nullptr;
  }

  void Element::SetLastStyleChangeEventStyle(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier,
    UniquePtr<const RenderStyle> &&style) noexcept
  {
    if (auto *animationData = AnimationRareData(pseudoElementIdentifier))
    {
      animationData->SetLastStyleChangeEventStyle(krys::move(style));
    }
    else if (style)
    {
      EnsureAnimationRareData(pseudoElementIdentifier).SetLastStyleChangeEventStyle(krys::move(style));
    }
  }

  bool Element::HasPropertiesOverridenAfterAnimation(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept
  {
    if (auto *animationData = AnimationRareData(pseudoElementIdentifier))
    {
      return animationData->HasPropertiesOverridenAfterAnimation();
    }
    return false;
  }

  void Element::SetHasPropertiesOverridenAfterAnimation(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier, bool value) noexcept
  {
    if (auto *animationData = AnimationRareData(pseudoElementIdentifier))
    {
      animationData->SetHasPropertiesOverridenAfterAnimation(value);
      return;
    }
    if (value)
    {
      EnsureAnimationRareData(pseudoElementIdentifier).SetHasPropertiesOverridenAfterAnimation(true);
    }
  }

  CSSAnimationCollection &Element::AnimationsCreatedByMarkup(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
  {
    return EnsureAnimationRareData(pseudoElementIdentifier).AnimationsCreatedByMarkup();
  }

  void Element::SetAnimationsCreatedByMarkup(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier,
    CSSAnimationCollection &&animations) noexcept
  {
    if (animations.isEmpty() && !AnimationRareData(pseudoElementIdentifier))
    {
      return;
    }
    EnsureAnimationRareData(pseudoElementIdentifier).SetAnimationsCreatedByMarkup(krys::move(animations));
  }

  void Element::CSSAnimationsDidUpdate(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
  {
    EnsureAnimationRareData(pseudoElementIdentifier).CSSAnimationsDidUpdate();
  }

  void Element::KeyframesRuleDidChange(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
  {
    EnsureAnimationRareData(pseudoElementIdentifier).KeyframesRuleDidChange();
  }

  bool Element::HasPendingKeyframesUpdate(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept
  {
    auto *data = AnimationRareData(pseudoElementIdentifier);
    return data && data->HasPendingKeyframesUpdate();
  }

  void Element::InvalidateStyleForAnimation() noexcept
  {
    Node::InvalidateStyle(style::Validity::AnimationInvalid);
  }

#pragma endregion

  bool Element::HasRandomCachingKeyMap() const noexcept
  {
    if (!_elementRareData)
    {
      return false;
    }
    return _elementRareData->HasRandomCachingKeyMap();
  }

#pragma region Element

  DOMString Element::TagName() const noexcept
  {
    auto qualifiedName = _qualifiedName.Name();

    if (NamespaceURI() == infra::Namespaces::HTML && Is<HTMLDocument>(NodeDocument()))
    {
      qualifiedName = krys::text::ToASCIIUpper(qualifiedName);
    }

    return qualifiedName;
  }

  void Element::Id(DOMString &&id) noexcept
  {
    ElementAlgorithms::SetAttributeValue(*this, u8"id", krys::move(id));
  }

  DOMString Element::Id() const noexcept
  {
    return ElementAlgorithms::GetAttributeValue(*this, u8"id");
  }

  bool Element::HasClassName(DOMStringView className) const noexcept
  {
    auto classes = GetAttribute(u8"class");
    if (!classes || className.empty())
    {
      return false;
    }
    bool foldCase = NodeDocument().InQuirksMode();
    DOMStringView remaining = *classes;
    while (!remaining.empty())
    {
      auto start = std::ranges::find_if_not(remaining, [](char8 c) { return text::IsASCIIWhitespace(c); });
      auto end =
        std::ranges::find_if(start, remaining.end(), [](char8 c) { return text::IsASCIIWhitespace(c); });
      DOMStringView token(start, end);
      if (token.size() == className.size()
          && (foldCase
                ? std::ranges::equal(token, className, {}, [](char8 c) { return text::ToASCIILower(c); })
                : token == className))
      {
        return true;
      }
      remaining = DOMStringView(end, remaining.end());
    }
    return false;
  }

  List<DOMStringAtom> Element::ClassNames() const noexcept
  {
    auto classes = GetAttribute(u8"class");
    return ClassNamesForValue(classes ? Maybe<DOMStringView>(*classes) : null);
  }

  List<DOMStringAtom> Element::ClassNamesForValue(Maybe<DOMStringView> classes) const noexcept
  {
    List<DOMStringAtom> classNames;
    if (!classes)
    {
      return classNames;
    }
    bool foldCase = NodeDocument().InQuirksMode();
    DOMStringView remaining = *classes;
    while (!remaining.empty())
    {
      auto start = std::ranges::find_if_not(remaining, [](char8 c) { return text::IsASCIIWhitespace(c); });
      auto end =
        std::ranges::find_if(start, remaining.end(), [](char8 c) { return text::IsASCIIWhitespace(c); });
      if (start != end)
      {
        DOMStringAtom className = foldCase ? DOMStringAtom(text::ToASCIILower(DOMStringView(start, end)))
                                           : DOMStringAtom(DOMStringView(start, end));
        if (std::ranges::find(classNames, className) == classNames.end())
        {
          classNames.push_back(className);
        }
      }
      remaining = DOMStringView(end, remaining.end());
    }
    return classNames;
  }

  DOMString Element::IdForStyleResolution() const noexcept
  {
    auto id = GetAttribute(u8"id").value_or(DOMString());
    if (NodeDocument().InQuirksMode())
    {
      return text::ToASCIILower(DOMStringView(id));
    }
    return id;
  }

  void Element::ClassName(DOMString &&className) noexcept
  {
    ElementAlgorithms::SetAttributeValue(*this, u8"class", krys::move(className));
  }

  DOMString Element::ClassName() const noexcept
  {
    return ElementAlgorithms::GetAttributeValue(*this, u8"class");
  }

  DOMTokenList &Element::ClassList() noexcept
  {
    if (_domTokenList == nullptr)
    {
      _domTokenList = ElementAlgorithms::CreateDOMTokenList(*this, u8"class");
    }

    return *_domTokenList;
  }

  void Element::Slot(DOMString &&slot) noexcept
  {
    ElementAlgorithms::SetAttributeValue(*this, u8"slot", krys::move(slot));
  }

  DOMString Element::Slot() const noexcept
  {
    return ElementAlgorithms::GetAttributeValue(*this, u8"slot");
  }

  bool Element::HasAttributes() const noexcept
  {
    SynchronizeAllAttributes();
    return !_attributes.empty();
  }

  NamedNodeMap &Element::Attributes() noexcept
  {
    if (_namedNodeMap == nullptr)
    {
      _namedNodeMap = UniquePtr<NamedNodeMap>(new NamedNodeMap(*this));
    }

    return *_namedNodeMap;
  }

  List<DOMString> Element::GetAttributeNames() const noexcept
  {
    SynchronizeAllAttributes();
    return std::ranges::views::transform(_attributes, [](const Ref<Attr> &attr) { return attr->Name(); })
           | std::ranges::to<List<DOMString>>();
  }

  Maybe<DOMString> Element::GetAttribute(DOMStringAtom qualifiedName) const noexcept
  {
    Attr *attr = ElementAlgorithms::GetAttributeByName(qualifiedName, *this);
    if (attr == nullptr)
    {
      return null;
    }

    return attr->Value();
  }

  Maybe<DOMString> Element::GetAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) const noexcept
  {
    Attr *attr = ElementAlgorithms::GetAttributeByNamespace(namespaceURI, localName, *this);
    if (attr == nullptr)
    {
      return std::nullopt;
    }

    return attr->Value();
  }

  ExceptionOr<void> Element::SetAttribute(DOMStringAtom qualifiedName, DOMString &&value) noexcept
  {
    if (!NameValidation::IsValidAttributeLocalName(qualifiedName.View()))
    {
      return Exception {ExceptionCode::InvalidCharacterError};
    }

    if (NamespaceURI() == infra::Namespaces::HTML && Is<HTMLDocument>(NodeDocument()))
    {
      qualifiedName = krys::text::ToASCIILower(qualifiedName.View());
    }

    // SPEC-VIOLATION(TRUSTED-TYPES): Let verifiedValue be the result of calling get trusted type compliant
    // attribute value with qualifiedName, null, this, and value.

    SynchronizeAllAttributes();
    auto it = std::find_if(_attributes.begin(), _attributes.end(),
                           [&](const Ref<Attr> &attr) { return attr->Name() == qualifiedName; });
    if (it != _attributes.end())
    {
      ElementAlgorithms::ChangeAttribute(**it, krys::move(value));
    }
    else
    {
      auto attr = AdoptRef<Attr>(
        *new Attr(NodeDocument(), QualifiedName {DOMStringAtom::Null(), DOMStringAtom::Null(), qualifiedName},
                  krys::move(value)));
      ElementAlgorithms::AppendAttribute(*attr, *this);
    }

    return {};
  }

  ExceptionOr<void> Element::SetAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom qualifiedName,
                                            DOMString &&value) noexcept
  {
    auto validateAndExtractResult =
      NameValidation::ValidateAndExtract(namespaceURI, qualifiedName, ValidateAndExtractContext::Attribute);

    if (validateAndExtractResult.HasException())
    {
      return validateAndExtractResult.ReleaseException();
    }

    // SPEC-VIOLATION(TRUSTED-TYPES): Let verifiedValue be the result of calling get trusted type compliant
    // attribute value with localName, namespace, this, and value.

    const auto &qName = validateAndExtractResult.Value();
    ElementAlgorithms::SetAttributeValue(*this, qName.LocalName(), std::move(value), qName.NamespacePrefix(),
                                         qName.NamespaceURI());

    return {};
  }

  void Element::RemoveAttribute(DOMStringAtom qualifiedName) noexcept
  {
    ElementAlgorithms::RemoveAttributeByName(qualifiedName, *this);
  }

  void Element::RemoveAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) noexcept
  {
    ElementAlgorithms::RemoveAttributeByNamespace(namespaceURI, localName, *this);
  }

  ExceptionOr<bool> Element::ToggleAttribute(DOMStringAtom qualifiedName, const Maybe<bool> &force) noexcept
  {
    if (!NameValidation::IsValidAttributeLocalName(qualifiedName.View()))
    {
      return Exception {ExceptionCode::InvalidCharacterError};
    }

    if (NamespaceURI() == infra::Namespaces::HTML && Is<HTMLDocument>(NodeDocument()))
    {
      qualifiedName = krys::text::ToASCIILower(qualifiedName.View());
    }

    SynchronizeAllAttributes();
    auto it = std::find_if(_attributes.begin(), _attributes.end(),
                           [&](const Ref<Attr> &attr) { return attr->Name() == qualifiedName; });

    if (it == _attributes.end())
    {
      if (!force.has_value() || force.value())
      {
        auto attr = AdoptRef<Attr>(*new Attr(
          NodeDocument(), QualifiedName {DOMStringAtom::Null(), DOMStringAtom::Null(), qualifiedName}, u8""));
        ElementAlgorithms::AppendAttribute(*attr, *this);
        return true;
      }

      return false;
    }

    if (!force.has_value() || !force.value())
    {
      ElementAlgorithms::RemoveAttribute(**it);
      return false;
    }

    return true;
  }

  bool Element::HasAttribute(DOMStringAtom qualifiedName) const noexcept
  {
    if (NamespaceURI() == infra::Namespaces::HTML && Is<HTMLDocument>(NodeDocument()))
    {
      qualifiedName = krys::text::ToASCIILower(qualifiedName.View());
    }

    SynchronizeAllAttributes();
    return std::ranges::any_of(_attributes, [&](const Ref<Attr> &a) { return a->Name() == qualifiedName; });
  }

  bool Element::HasAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) const noexcept
  {
    if (namespaceURI == DOMStringAtom::Empty())
    {
      namespaceURI = DOMStringAtom::Null();
    }

    SynchronizeAllAttributes();
    return std::ranges::any_of(_attributes, [&](const Ref<Attr> &a)
                               { return a->NamespaceURI() == namespaceURI && a->LocalName() == localName; });
  }

  Attr *Element::GetAttributeNode(DOMStringAtom qualifiedName) const noexcept
  {
    return ElementAlgorithms::GetAttributeByName(qualifiedName, *this);
  }

  Attr *Element::GetAttributeNodeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) const noexcept
  {
    return ElementAlgorithms::GetAttributeByNamespace(namespaceURI, localName, *this);
  }

  ExceptionOr<RefPtr<Attr>> Element::SetAttributeNode(Attr &attr) noexcept
  {
    return ElementAlgorithms::SetAttribute(attr, *this);
  }

  ExceptionOr<RefPtr<Attr>> Element::SetAttributeNodeNS(Attr &attr) noexcept
  {
    return ElementAlgorithms::SetAttribute(attr, *this);
  }

  ExceptionOr<Ref<Attr>> Element::RemoveAttributeNode(Attr &attr) noexcept
  {
    if (!std::ranges::any_of(_attributes, [&](const Ref<Attr> &a) { return a.get() == &attr; }))
    {
      return Exception {ExceptionCode::NotFoundError};
    }

    ElementAlgorithms::RemoveAttribute(attr);

    return ShareRef(attr);
  }

  ExceptionOr<Ref<ShadowRoot>> Element::AttachShadow(const ShadowRootInit &init) noexcept
  {
    auto registry =
      init.CustomElementRegistry ? init.CustomElementRegistry : NodeDocument().CustomElementRegistry().get();

    if (registry != nullptr && !registry->IsScoped() && registry != NodeDocument().CustomElementRegistry())
    {
      return Exception {ExceptionCode::NotSupportedError};
    }

    if (auto attachResult =
          ElementAlgorithms::AttachShadowRoot(*this, init.Mode, init.Clonable, init.Serialisable,
                                              init.DelegatesFocus, init.SlotAssignment, registry);
        attachResult.HasException())
    {
      return attachResult.ReleaseException();
    }

    return ShareRef(*_shadowRoot);
  }

  RefPtr<ShadowRoot> Element::ShadowRoot() const noexcept
  {
    if (!_shadowRoot || _shadowRoot->Mode() == ShadowRootMode::Closed)
    {
      return nullptr;
    }

    return _shadowRoot;
  }

  ExceptionOr<RefPtr<Element>> Element::Closest(DOMStringView selectors) noexcept
  {
    auto query = NodeDocument().SelectorQueryForString(selectors);
    if (query.HasException())
    {
      return query.ReleaseException();
    }
    return ShareRefPtr(query.Value().Closest(*this));
  }

  ExceptionOr<bool> Element::Matches(DOMStringView selectors) const noexcept
  {
    auto query = NodeDocument().SelectorQueryForString(selectors);
    if (query.HasException())
    {
      return query.ReleaseException();
    }
    return query.Value().Matches(*this);
  }

  Ref<HTMLCollection> Element::GetElementsByTagName(DOMStringAtom qualifiedName) noexcept
  {
    return NodeAlgorithms::GetElementsByTagName(*this, qualifiedName);
  }

  Ref<HTMLCollection> Element::GetElementsByTagNameNS(DOMStringAtom namespaceURI,
                                                      DOMStringAtom localName) noexcept
  {
    return NodeAlgorithms::GetElementsByTagNameNS(*this, namespaceURI, localName);
  }

  Ref<HTMLCollection> Element::GetElementsByClassName(DOMStringAtom classNames) noexcept
  {
    return NodeAlgorithms::GetElementsByClassName(*this, classNames);
  }

  ExceptionOr<Element *> Element::InsertAdjacentElement(InsertAdjacentWhere where, Element &element) noexcept
  {
    if (auto result = ElementAlgorithms::InsertAdjacent(*this, where, element); result.HasException())
    {
      return result.ReleaseException();
    }

    return &element;
  }

  ExceptionOr<void> Element::InsertAdjacentText(InsertAdjacentWhere where, DOMString &&data) noexcept
  {
    auto textNode = CreateRef<Text>(NodeDocument(), krys::move(data));
    if (auto result = ElementAlgorithms::InsertAdjacent(*this, where, *textNode); result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

#pragma endregion

#pragma region Node

  DOMString Element::NodeName() const noexcept
  {
    return TagName();
  }

  Maybe<DOMString> Element::TextContent() const noexcept
  {
    return TextAlgorithms::DescendantTextContent(*this);
  }

  ExceptionOr<void> Element::TextContent(DOMString &&value) noexcept
  {
    return NodeAlgorithms::StringReplaceAll(krys::move(value), *this);
  }

#pragma endregion

#pragma region ChildNode

  ExceptionOr<void> Element::Before(const List<NodeOrString> &nodes) noexcept
  {
    return mixins::ChildNode::Before(*this, nodes);
  }

  ExceptionOr<void> Element::After(const List<NodeOrString> &nodes) noexcept
  {
    return mixins::ChildNode::After(*this, nodes);
  }

  ExceptionOr<void> Element::ReplaceWith(const List<NodeOrString> &nodes) noexcept
  {
    return mixins::ChildNode::ReplaceWith(*this, nodes);
  }

  ExceptionOr<void> Element::Remove() noexcept
  {
    return mixins::ChildNode::Remove(*this);
  }

#pragma endregion

#pragma region NonDocumentTypeChildNode

  RefPtr<const Element> Element::PreviousElementSibling() const noexcept
  {
    return mixins::NonDocumentTypeChildNode::PreviousElementSibling(*this);
  }

  RefPtr<Element> Element::PreviousElementSibling() noexcept
  {
    return mixins::NonDocumentTypeChildNode::PreviousElementSibling(*this);
  }

  RefPtr<const Element> Element::NextElementSibling() const noexcept
  {
    return mixins::NonDocumentTypeChildNode::NextElementSibling(*this);
  }

  RefPtr<Element> Element::NextElementSibling() noexcept
  {
    return mixins::NonDocumentTypeChildNode::NextElementSibling(*this);
  }

#pragma endregion

#pragma region ParentNode

  Ref<HTMLCollection> Element::Children() noexcept
  {
    return EnsureElementRareData().Children(*this);
  }

  RefPtr<const Element> Element::FirstElementChild() const noexcept
  {
    return mixins::ParentNode::FirstElementChild(*this);
  }

  RefPtr<Element> Element::FirstElementChild() noexcept
  {
    return mixins::ParentNode::FirstElementChild(*this);
  }

  RefPtr<const Element> Element::LastElementChild() const noexcept
  {
    return mixins::ParentNode::LastElementChild(*this);
  }

  RefPtr<Element> Element::LastElementChild() noexcept
  {
    return mixins::ParentNode::LastElementChild(*this);
  }

  size_t Element::ChildElementCount() const noexcept
  {
    return mixins::ParentNode::ChildElementCount(*this);
  }

  ExceptionOr<void> Element::Prepend(const List<NodeOrString> &nodes) noexcept
  {
    return mixins::ParentNode::Prepend(*this, nodes);
  }

  ExceptionOr<void> Element::Append(const List<NodeOrString> &nodes) noexcept
  {
    return mixins::ParentNode::Append(*this, nodes);
  }

  ExceptionOr<void> Element::ReplaceChildren(const List<NodeOrString> &nodes) noexcept
  {
    return mixins::ParentNode::ReplaceChildren(*this, nodes);
  }

  ExceptionOr<void> Element::MoveBefore(Node &node, Node *refChild) noexcept
  {
    return mixins::ParentNode::MoveBefore(*this, node, refChild);
  }

  ExceptionOr<RefPtr<Element>> Element::QuerySelector(DOMStringView selectors) noexcept
  {
    return mixins::ParentNode::QuerySelector(*this, selectors);
  }

  ExceptionOr<Ref<NodeList>> Element::QuerySelectorAll(DOMStringView selectors) noexcept
  {
    return mixins::ParentNode::QuerySelectorAll(*this, selectors);
  }

#pragma endregion

#pragma region Slottable

  RefPtr<html::HTMLSlotElement> Element::AssignedSlot() noexcept
  {
    return mixins::Slottable::AssignedSlot(*this);
  }

#pragma endregion

#pragma region Lazy Attributes

  void Element::SetSynchronizedLazyAttribute(DOMStringAtom localName, DOMString &&value) noexcept
  {
    auto it = std::ranges::find_if(
      _attributes, [&](const Ref<Attr> &attr)
      { return attr->LocalName() == localName && attr->NamespaceURI() == DOMStringAtom::Null(); });
    if (it != _attributes.end())
    {
      (*it)->_value = krys::move(value);
      return;
    }

    auto attr = AdoptRef<Attr>(
      *new Attr(NodeDocument(), QualifiedName {DOMStringAtom::Null(), DOMStringAtom::Null(), localName},
                krys::move(value)));
    attr->_ownerElement = CreateWeakPtr(this);
    _attributes.push_back(krys::move(attr));
  }

  void Element::SynchronizeStyleAttribute() const noexcept
  {
    const_cast<StyledElement &>(Downcast<StyledElement>(*this)).SynchronizeStyleAttributeInternal();
  }

#pragma endregion

#pragma region Extension Hooks

  void Element::OnAttributeChanged(DOMStringAtom localName, Maybe<DOMStringView> oldValue,
                                   Maybe<DOMStringView> value, DOMStringAtom namespaceURI) noexcept
  {
    if (namespaceURI != DOMStringAtom::Null())
    {
      return;
    }

    // NOTE: we currently don't need to handle `id` as we just use the value stored in the attribute. Later,
    // when we optimise we can use this hook to update a member variable to skip needing to look up the
    // attribute. This will be particularly important for a faster implementation of GetElementById.

    if (localName == Attributes::Class)
    {
      auto &domTokenList = ClassList();
      domTokenList._tokens.clear();
      if (value.has_value())
      {
        auto tokens = OrderedSet::Parser(*value);
        domTokenList._tokens.append(tokens.begin(), tokens.end());
      }
    }
    else if (localName == Attributes::Slot)
    {
      if (value == oldValue)
      {
        return;
      }

      if (!value.has_value() && oldValue.has_value() && oldValue->empty())
      {
        return;
      }

      if (value.has_value() && value->empty() && !oldValue.has_value())
      {
        return;
      }

      if (!value.has_value() || value->empty())
      {
        _slottableName = u8"";
      }
      else
      {
        _slottableName = DOMString(*value);
      }

      if (auto *assignedSlot = SlotAlgorithms::GetAssignedSlot(*this))
      {
        SlotAlgorithms::AssignSlottables(*assignedSlot);
      }

      SlotAlgorithms::AssignSlot(*this);
    }
    else if (localName.View() == u8"part")
    {
      if (NeedsStyleInvalidation() && IsInShadowTree())
      {
        InvalidateStyleInternal();
      }
    }
    else if (localName.View() == u8"exportparts")
    {
      if (auto *shadowRoot = TreeQueries::GetShadowRoot(*this))
      {
        style::Invalidator::InvalidateShadowParts(*shadowRoot);
      }
    }
  }

#pragma endregion

  bool IsInTopLayerOrBackdrop(const RenderStyle &style, const Element *) noexcept
  {
    // FIXME: Whether the element is in the top layer (modal dialogs, popovers, fullscreen), once there's a
    // top layer.
    return style.GetPseudoElementType() == PseudoElementType::Backdrop;
  }

  namespace
  {
    /// @brief The frame showing @p document, if it has a view.
    KRYS_NODISCARD LocalFrame *DocumentFrameWithNonNullView(Document &document) noexcept
    {
      auto *frame = document.Frame();
      return frame && frame->View() ? frame : nullptr;
    }

    KRYS_NODISCARD int32 AdjustContentsScrollPositionOrSizeForZoom(int32 value,
                                                                   const LocalFrame &frame) noexcept
    {
      // FIXME: The frame scale factor (frameScaleFactor), once frames can be scaled.
      double zoomFactor = frame.PageZoomFactor();
      if (zoomFactor == 1)
      {
        return value;
      }
      // FIXME (webkit.org/b/189397): Why can't we just ceil/floor?
      // Needed because of truncation (rather than rounding) when scaling up.
      if (zoomFactor > 1)
      {
        value++;
      }
      return static_cast<int32>(value / zoomFactor);
    }

    /// @brief Scrolls @p document's view as the window's `scrollTo()` does.
    // FIXME: Through the Window (LocalDOMWindow::scrollTo), once there's a Window.
    void ScrollWindowTo(Document &document, const ScrollToOptions &options) noexcept
    {
      auto *frame = DocumentFrameWithNonNullView(document);
      if (!frame)
      {
        return;
      }
      auto &view = *frame->View();

      ScrollToOptions scrollToOptions = NormalizeNonFiniteCoordinatesOrFallBackTo(
        options, view.ContentsScrollPosition().X(), view.ContentsScrollPosition().Y());

      // This is an optimization for the common case of scrolling to (0, 0) when the scroller is already at
      // the origin. If an animated scroll is in progress, this optimization is skipped to ensure that the
      // animated scroll is really stopped.
      if (view.GetScrollAnimationStatus() == ScrollAnimationStatus::NotAnimating && !*scrollToOptions.Left
          && !*scrollToOptions.Top && view.ContentsScrollPosition().IsZero())
      {
        return;
      }

      // FIXME: Cancelling the scheduled scrolls (cancelScheduledScrolls), once they're scheduled.
      document.UpdateLayoutIgnorePendingStylesheets();

      // The frame scale factor is 1 (FIXME: once frames can be scaled).
      IntPoint layoutPos(static_cast<int32>(*scrollToOptions.Left * frame->PageZoomFactor()),
                         static_cast<int32>(*scrollToOptions.Top * frame->PageZoomFactor()));

      // FIXME: Smooth scrolling (useSmoothScrolling), once there are scroll animations.
      view.SetContentsScrollPosition(layoutPos, ScrollPositionChangeOptions::CreateProgrammatic());
    }

    KRYS_NODISCARD ScrollAlignment ToScrollAlignmentForInlineDirection(Maybe<ScrollLogicalPosition> position,
                                                                       WritingMode writingMode) noexcept
    {
      switch (position.value_or(ScrollLogicalPosition::Nearest))
      {
        case ScrollLogicalPosition::Start:
        {
          switch (writingMode.BlockDirection())
          {
            case FlowDirection::TopToBottom:
            case FlowDirection::BottomToTop:
              return writingMode.IsInlineLeftToRight() ? ScrollAlignment::AlignLeftAlways
                                                       : ScrollAlignment::AlignRightAlways;
            case FlowDirection::LeftToRight:
            case FlowDirection::RightToLeft:
              return writingMode.IsInlineTopToBottom() ? ScrollAlignment::AlignTopAlways
                                                       : ScrollAlignment::AlignBottomAlways;
          }
          return ScrollAlignment::AlignLeftAlways;
        }
        case ScrollLogicalPosition::Center: return ScrollAlignment::AlignCenterAlways;
        case ScrollLogicalPosition::End:
        {
          switch (writingMode.BlockDirection())
          {
            case FlowDirection::TopToBottom:
            case FlowDirection::BottomToTop:
              return writingMode.IsInlineLeftToRight() ? ScrollAlignment::AlignRightAlways
                                                       : ScrollAlignment::AlignLeftAlways;
            case FlowDirection::LeftToRight:
            case FlowDirection::RightToLeft:
              return writingMode.IsInlineTopToBottom() ? ScrollAlignment::AlignBottomAlways
                                                       : ScrollAlignment::AlignTopAlways;
          }
          return ScrollAlignment::AlignRightAlways;
        }
        case ScrollLogicalPosition::Nearest: return ScrollAlignment::AlignToEdgeIfNeeded;
      }
      return ScrollAlignment::AlignToEdgeIfNeeded;
    }

    KRYS_NODISCARD ScrollAlignment ToScrollAlignmentForBlockDirection(Maybe<ScrollLogicalPosition> position,
                                                                      WritingMode writingMode) noexcept
    {
      switch (position.value_or(ScrollLogicalPosition::Start))
      {
        case ScrollLogicalPosition::Start:
        {
          switch (writingMode.BlockDirection())
          {
            case FlowDirection::TopToBottom: return ScrollAlignment::AlignTopAlways;
            case FlowDirection::BottomToTop: return ScrollAlignment::AlignBottomAlways;
            case FlowDirection::LeftToRight: return ScrollAlignment::AlignLeftAlways;
            case FlowDirection::RightToLeft: return ScrollAlignment::AlignRightAlways;
          }
          return ScrollAlignment::AlignTopAlways;
        }
        case ScrollLogicalPosition::Center: return ScrollAlignment::AlignCenterAlways;
        case ScrollLogicalPosition::End:
        {
          switch (writingMode.BlockDirection())
          {
            case FlowDirection::TopToBottom: return ScrollAlignment::AlignBottomAlways;
            case FlowDirection::BottomToTop: return ScrollAlignment::AlignTopAlways;
            case FlowDirection::LeftToRight: return ScrollAlignment::AlignRightAlways;
            case FlowDirection::RightToLeft: return ScrollAlignment::AlignLeftAlways;
          }
          return ScrollAlignment::AlignBottomAlways;
        }
        case ScrollLogicalPosition::Nearest: return ScrollAlignment::AlignToEdgeIfNeeded;
      }
      return ScrollAlignment::AlignToEdgeIfNeeded;
    }

    KRYS_NODISCARD double LocalZoomForRenderer(const RenderElement &renderer) noexcept
    {
      // FIXME: This does the wrong thing if two opposing zooms are in effect and canceled each
      // other out, but the alternative is that we'd have to crawl up the whole render tree every
      // time (or store an additional bit in the RenderStyle to indicate that a zoom was specified).
      double zoomFactor = 1;
      if (renderer.Style().UsedZoom() != 1)
      {
        // Need to find the nearest enclosing RenderElement that set up
        // a differing zoom, and then we divide our result by it to eliminate the zoom.
        const RenderElement *prev = &renderer;
        for (const RenderElement *curr = prev->Parent(); curr; curr = curr->Parent())
        {
          if (curr->Style().UsedZoom() != prev->Style().UsedZoom())
          {
            zoomFactor = style::Evaluate<double>(prev->Style().GetZoom());
            break;
          }
          prev = curr;
        }
        if (prev->IsRenderView())
        {
          zoomFactor = style::Evaluate<double>(prev->Style().GetZoom());
        }
      }
      return zoomFactor;
    }

    enum class LegacyCSSOMElementMetricsRoundingStrategy : bool
    {
      Round,
      Floor
    };

    KRYS_NODISCARD int32
      ConvertToNonSubpixelValue(double value, LegacyCSSOMElementMetricsRoundingStrategy roundStrategy =
                                                LegacyCSSOMElementMetricsRoundingStrategy::Round) noexcept
    {
      return static_cast<int32>(roundStrategy == LegacyCSSOMElementMetricsRoundingStrategy::Round
                                  ? std::round(value)
                                  : std::floor(value));
    }

    KRYS_NODISCARD int32 AdjustOffsetForZoomAndSubpixelLayout(const RenderBoxModelObject &renderer,
                                                              const LayoutUnit &offset) noexcept
    {
      auto offsetLeft = LayoutUnit(RoundToInt(offset));
      double zoomFactor = LocalZoomForRenderer(renderer);
      if (zoomFactor == 1)
      {
        return ConvertToNonSubpixelValue(offsetLeft.ToDouble(),
                                         LegacyCSSOMElementMetricsRoundingStrategy::Floor);
      }
      return ConvertToNonSubpixelValue(offsetLeft.ToDouble() / zoomFactor);
    }

    KRYS_NODISCARD int32 ClampToInteger(double value) noexcept
    {
      if (std::isnan(value))
      {
        return 0;
      }
      return static_cast<int32>(std::clamp(value, static_cast<double>(std::numeric_limits<int32>::min()),
                                           static_cast<double>(std::numeric_limits<int32>::max())));
    }

    /// @brief Whether @p ancestorRoot, the root of a tree, is the root of the tree of @p node or of one of
    /// the trees its shadow-including ancestors are in.
    KRYS_NODISCARD bool IsAncestorTreeScope(const Node &ancestorRoot, const Node &node) noexcept
    {
      return TreeQueries::IsShadowIncludingInclusiveAncestor(ancestorRoot, node);
    }

    KRYS_NODISCARD bool ShouldObtainBoundsFromBoxModel(const Element &element) noexcept
    {
      // FIXME: SVG renderers (RenderSVGModelObject), once there's SVG.
      return element.Renderer() && element.Renderer()->IsRenderBoxModelObject();
    }

    KRYS_NODISCARD RenderBox *RenderBoxOf(const Element &element) noexcept
    {
      return DynamicDowncast<RenderBox>(element.Renderer());
    }
  }

  Ref<DOMRectList> Element::GetClientRects() noexcept
  {
    NodeDocument().UpdateLayoutIgnorePendingStylesheets();

    auto *renderer = Renderer();

    List<FloatQuad> quads;

    // FIXME: SVG elements' bounding boxes, and the option and optgroup elements of list boxes, once there's
    // SVG and form controls.
    if (ShouldObtainBoundsFromBoxModel(*this))
    {
      renderer->AbsoluteQuads(quads);
    }

    // FIXME: Handle table/inline-table with a caption.

    if (quads.empty())
    {
      return DOMRectList::Create();
    }

    NodeDocument().ConvertAbsoluteToClientQuads(quads, renderer->Style());
    return DOMRectList::Create(quads);
  }

  Maybe<Pair<RenderElement *, FloatRect>> Element::BoundingAbsoluteRectWithoutLayout() const noexcept
  {
    auto *renderer = Renderer();
    List<FloatQuad> quads;
    // FIXME: SVG elements' bounding boxes, and the option and optgroup elements of list boxes, once there's
    // SVG and form controls.
    if (ShouldObtainBoundsFromBoxModel(*this))
    {
      renderer->AbsoluteQuads(quads);
    }

    if (quads.empty())
    {
      return null;
    }

    return Pair<RenderElement *, FloatRect> {renderer, UnitedBoundingBoxes(quads)};
  }

  FloatRect Element::BoundingClientRect() noexcept
  {
    auto &document = NodeDocument();
    document.UpdateLayoutIgnorePendingStylesheets();
    auto pair = BoundingAbsoluteRectWithoutLayout();
    if (!pair)
    {
      return {};
    }
    auto *renderer = pair->first;
    auto result = pair->second;
    document.ConvertAbsoluteToClientRect(result, renderer->Style());
    return result;
  }

  Ref<DOMRect> Element::GetBoundingClientRect() noexcept
  {
    return DOMRect::Create(BoundingClientRect());
  }

  void Element::ScrollIntoView(const ScrollIntoViewOptions &options) noexcept
  {
    auto &document = NodeDocument();
    // FIXME: Make content-visibility: auto content relevant (updateContentRelevancyForScrollIfNeeded), once
    // content relevancy is computed.
    document.UpdateLayoutIgnorePendingStylesheets();

    // FIXME: Scroll a list box to the option or optgroup, once there are form controls.
    auto *renderer = Renderer();
    if (!renderer)
    {
      return;
    }

    bool insideFixed = false;
    auto absoluteBounds = renderer->AbsoluteAnchorRectWithScrollMargin(&insideFixed).first;

    auto writingMode = renderer->GetWritingMode();
    auto alignX = ToScrollAlignmentForInlineDirection(options.InlinePosition, writingMode);
    auto alignY = ToScrollAlignmentForBlockDirection(options.BlockPosition, writingMode);
    alignX.DisableLegacyHorizontalVisibilityThreshold();

    bool isHorizontal = writingMode.IsHorizontal();
    ScrollRectToVisibleOptions visibleOptions;
    visibleOptions.RevealMode = SelectionRevealMode::Reveal;
    visibleOptions.AlignX = isHorizontal ? alignX : alignY;
    visibleOptions.AlignY = isHorizontal ? alignY : alignX;
    visibleOptions.Behavior = options.Behavior.value_or(ScrollBehavior::Auto);
    LocalFrameView::ScrollRectToVisible(absoluteBounds, *renderer, insideFixed, visibleOptions);
  }

  void Element::ScrollIntoView(bool alignToTop) noexcept
  {
    ScrollIntoViewOptions options;
    if (!alignToTop)
    {
      options.BlockPosition = ScrollLogicalPosition::End;
    }
    ScrollIntoView(options);
  }

  void Element::Scroll(const ScrollToOptions &options) noexcept
  {
    ScrollTo(options);
  }

  void Element::Scroll(double x, double y) noexcept
  {
    ScrollTo(x, y);
  }

  void Element::ScrollBy(const ScrollToOptions &options) noexcept
  {
    auto scrollToOptions = NormalizeNonFiniteCoordinatesOrFallBackTo(options, 0, 0);
    *scrollToOptions.Left += ScrollLeft();
    *scrollToOptions.Top += ScrollTop();
    // FIXME: Directional scroll snapping with the original delta, once there's scroll snapping.
    ScrollTo(scrollToOptions);
  }

  void Element::ScrollBy(double x, double y) noexcept
  {
    ScrollBy(ScrollToOptions(x, y));
  }

  void Element::ScrollTo(const ScrollToOptions &options) noexcept
  {
    auto &document = NodeDocument();

    // We can avoid triggering layout if this is a scrollTo(0,0) on a element that cannot be the
    // scrollingElement, and was last scrolled to 0,0.
    auto canShortCircuitScroll = [&]()
    {
      if (!options.Left || !options.Top)
      {
        return false;
      }

      if (*options.Left || *options.Top)
      {
        return false;
      }

      // document().scrollingElement() requires up-to-date style, so bail if this element could be the
      // scrollingElement().
      if (document.DocumentElement() == this || document.Body().get() == this)
      {
        return false;
      }

      // FIXME: Elements that have ever been smooth-scrolled (hasEverHadSmoothScroll), once there's smooth
      // scrolling.
      return SavedLayerScrollPosition().IsZero();
    };

    if (canShortCircuitScroll())
    {
      return;
    }

    document.UpdateLayoutIgnorePendingStylesheets();

    if (document.ScrollingElement() == this)
    {
      // If the element is the scrolling element and is not potentially scrollable,
      // invoke scroll() on window with options as the only argument, and terminate these steps.
      ScrollWindowTo(document, options);
      return;
    }

    // If the element does not have any associated CSS layout box, the element has no associated scrolling
    // box, or the element has no overflow, terminate these steps.
    auto *renderer = RenderBoxOf(*this);
    // FIXME: Single-line text controls scroll their inner text element, once there are form controls.
    if (!renderer || !renderer->HasNonVisibleOverflow())
    {
      return;
    }

    auto scrollToOptions = NormalizeNonFiniteCoordinatesOrFallBackTo(
      options, AdjustForAbsoluteZoom(renderer->ScrollLeft(), renderer->Style()),
      AdjustForAbsoluteZoom(renderer->ScrollTop(), renderer->Style()));
    IntPoint scrollPosition(ClampToInteger(*scrollToOptions.Left * renderer->Style().UsedZoom()),
                            ClampToInteger(*scrollToOptions.Top * renderer->Style().UsedZoom()));

    // FIXME: Smooth scrolling (useSmoothScrolling), once there are scroll animations.
    renderer->SetScrollPosition(scrollPosition, ScrollPositionChangeOptions::CreateProgrammatic());
  }

  void Element::ScrollTo(double x, double y) noexcept
  {
    ScrollTo(ScrollToOptions(x, y));
  }

  int32 Element::OffsetLeft() noexcept
  {
    auto offset = 0;
    NodeDocument().UpdateLayoutIgnorePendingStylesheets();
    if (auto *renderer = DynamicDowncast<RenderBoxModelObject>(Renderer()))
    {
      offset = AdjustOffsetForZoomAndSubpixelLayout(*renderer, renderer->OffsetLeft());
    }

    // Add the offsets of the offset parents in shadow trees the element isn't in.
    auto *parent = OffsetParentIgnoringShadowTrees();
    if (!parent || !parent->IsInShadowTree() || TreeQueries::SameRoot(*parent, *this))
    {
      return offset;
    }

    while (parent && !IsAncestorTreeScope(TreeQueries::Root(*parent), *this))
    {
      if (auto *parentRenderer = DynamicDowncast<RenderBoxModelObject>(parent->Renderer()))
      {
        offset += AdjustOffsetForZoomAndSubpixelLayout(*parentRenderer, parentRenderer->OffsetLeft());
      }
      parent = parent->OffsetParentIgnoringShadowTrees();
    }

    return offset;
  }

  int32 Element::OffsetTop() noexcept
  {
    auto offset = 0;
    NodeDocument().UpdateLayoutIgnorePendingStylesheets();
    if (auto *renderer = DynamicDowncast<RenderBoxModelObject>(Renderer()))
    {
      offset = AdjustOffsetForZoomAndSubpixelLayout(*renderer, renderer->OffsetTop());
    }

    // Add the offsets of the offset parents in shadow trees the element isn't in.
    auto *parent = OffsetParentIgnoringShadowTrees();
    if (!parent || !parent->IsInShadowTree() || TreeQueries::SameRoot(*parent, *this))
    {
      return offset;
    }

    while (parent && !IsAncestorTreeScope(TreeQueries::Root(*parent), *this))
    {
      if (auto *parentRenderer = DynamicDowncast<RenderBoxModelObject>(parent->Renderer()))
      {
        offset += AdjustOffsetForZoomAndSubpixelLayout(*parentRenderer, parentRenderer->OffsetTop());
      }
      parent = parent->OffsetParentIgnoringShadowTrees();
    }

    return offset;
  }

  int32 Element::OffsetWidth() noexcept
  {
    NodeDocument().UpdateLayoutIgnorePendingStylesheets();
    if (auto *renderer = DynamicDowncast<RenderBoxModelObject>(Renderer()))
    {
      auto offsetWidth = LayoutUnit(RoundToInt(renderer->OffsetWidth()));
      return ConvertToNonSubpixelValue(
        AdjustLayoutUnitForAbsoluteZoom(offsetWidth, renderer->Style()).ToDouble());
    }
    return 0;
  }

  int32 Element::OffsetHeight() noexcept
  {
    NodeDocument().UpdateLayoutIgnorePendingStylesheets();
    if (auto *renderer = DynamicDowncast<RenderBoxModelObject>(Renderer()))
    {
      auto offsetHeight = LayoutUnit(RoundToInt(renderer->OffsetHeight()));
      return ConvertToNonSubpixelValue(
        AdjustLayoutUnitForAbsoluteZoom(offsetHeight, renderer->Style()).ToDouble());
    }
    return 0;
  }

  Element *Element::OffsetParent() noexcept
  {
    auto *element = OffsetParentIgnoringShadowTrees();
    if (!element || !element->IsInShadowTree())
    {
      return element;
    }
    while (element && !TreeQueries::IsShadowIncludingDescendant(*this, TreeQueries::Root(*element)))
    {
      element = element->OffsetParentIgnoringShadowTrees();
    }
    return element;
  }

  Element *Element::OffsetParentIgnoringShadowTrees() noexcept
  {
    NodeDocument().UpdateLayoutIgnorePendingStylesheets();
    auto *renderer = Renderer();
    if (!renderer)
    {
      return nullptr;
    }
    auto *offsetParent = renderer->OffsetParent();
    return offsetParent ? offsetParent->GetElement() : nullptr;
  }

  int32 Element::ClientLeft() noexcept
  {
    NodeDocument().UpdateLayoutIgnorePendingStylesheets();

    if (auto *renderer = RenderBoxOf(*this))
    {
      auto clientLeft = LayoutUnit(RoundToInt(renderer->ClientLeft()));
      return ConvertToNonSubpixelValue(
        AdjustLayoutUnitForAbsoluteZoom(clientLeft, renderer->Style()).ToDouble());
    }
    return 0;
  }

  int32 Element::ClientTop() noexcept
  {
    NodeDocument().UpdateLayoutIgnorePendingStylesheets();

    if (auto *renderer = RenderBoxOf(*this))
    {
      auto clientTop = LayoutUnit(RoundToInt(renderer->ClientTop()));
      return ConvertToNonSubpixelValue(
        AdjustLayoutUnitForAbsoluteZoom(clientTop, renderer->Style()).ToDouble());
    }
    return 0;
  }

  int32 Element::ClientWidth() noexcept
  {
    auto &document = NodeDocument();
    document.UpdateLayoutIgnorePendingStylesheets();

    if (!document.HasLivingRenderTree())
    {
      return 0;
    }

    auto &renderView = *document.GetRenderView();

    // When in strict mode, clientWidth for the document element should return the width of the containing
    // frame. When in quirks mode, clientWidth for the body element should return the width of the containing
    // frame.
    bool inQuirksMode = document.InQuirksMode();
    if ((!inQuirksMode && document.DocumentElement() == this)
        || (inQuirksMode && IsHTMLElement() && document.BodyOrFrameset() == this))
    {
      return AdjustForAbsoluteZoom(renderView.FrameView().LayoutWidth(), renderView);
    }

    if (auto *renderer = RenderBoxOf(*this))
    {
      auto clientWidth = LayoutUnit(RoundToInt(renderer->ClientWidth()));
      // clientWidth/Height is the visual portion of the box content, not including
      // borders or scroll bars, but includes padding. And per
      // https://www.w3.org/TR/CSS2/tables.html#model,
      // table wrapper box is a principal block box that contains the table box
      // itself and any caption boxes, and table grid box is a block-level box that
      // contains the table's internal table boxes. When table's border is specified
      // in CSS, the border is added to table grid box, not table wrapper box.
      // There's no table wrapper box, and we are supposed to retrieve clientWidth/Height
      // from table wrapper box, not table grid box. So when we retrieve
      // clientWidth/Height, it includes table's border size.
      if (renderer->IsRenderTable())
      {
        if (renderer->Style().GetBoxSizing() == BoxSizing::ContentBox)
        {
          clientWidth += renderer->PaddingLeft() + renderer->PaddingRight();
        }
        clientWidth += renderer->BorderLeft() + renderer->BorderRight();
      }
      return ConvertToNonSubpixelValue(
        AdjustLayoutUnitForAbsoluteZoom(clientWidth, renderer->Style()).ToDouble());
    }
    return 0;
  }

  int32 Element::ClientHeight() noexcept
  {
    auto &document = NodeDocument();
    document.UpdateLayoutIgnorePendingStylesheets();

    if (!document.HasLivingRenderTree())
    {
      return 0;
    }

    auto &renderView = *document.GetRenderView();

    // When in strict mode, clientHeight for the document element should return the height of the containing
    // frame. When in quirks mode, clientHeight for the body element should return the height of the
    // containing frame.
    bool inQuirksMode = document.InQuirksMode();
    if ((!inQuirksMode && document.DocumentElement() == this)
        || (inQuirksMode && IsHTMLElement() && document.BodyOrFrameset() == this))
    {
      return AdjustForAbsoluteZoom(renderView.FrameView().LayoutHeight(), renderView);
    }

    if (auto *renderer = RenderBoxOf(*this))
    {
      auto clientHeight = LayoutUnit(RoundToInt(renderer->ClientHeight()));
      // See ClientWidth() for why the table's padding and borders are added.
      if (renderer->IsRenderTable())
      {
        if (renderer->Style().GetBoxSizing() == BoxSizing::ContentBox)
        {
          clientHeight += renderer->PaddingTop() + renderer->PaddingBottom();
        }
        clientHeight += renderer->BorderTop() + renderer->BorderBottom();
      }
      return ConvertToNonSubpixelValue(
        AdjustLayoutUnitForAbsoluteZoom(clientHeight, renderer->Style()).ToDouble());
    }
    return 0;
  }

  int32 Element::ScrollLeft() noexcept
  {
    auto &document = NodeDocument();
    document.UpdateLayoutIgnorePendingStylesheets();

    if (document.ScrollingElement() == this)
    {
      if (auto *frame = DocumentFrameWithNonNullView(document))
      {
        return AdjustContentsScrollPositionOrSizeForZoom(frame->View()->ContentsScrollPosition().X(), *frame);
      }
      return 0;
    }

    if (auto *renderer = RenderBoxOf(*this))
    {
      return AdjustForAbsoluteZoom(renderer->ScrollLeft(), renderer->Style());
    }
    return 0;
  }

  int32 Element::ScrollTop() noexcept
  {
    auto &document = NodeDocument();
    document.UpdateLayoutIgnorePendingStylesheets();

    if (document.ScrollingElement() == this)
    {
      if (auto *frame = DocumentFrameWithNonNullView(document))
      {
        return AdjustContentsScrollPositionOrSizeForZoom(frame->View()->ContentsScrollPosition().Y(), *frame);
      }
      return 0;
    }

    if (auto *renderer = RenderBoxOf(*this))
    {
      return AdjustForAbsoluteZoom(renderer->ScrollTop(), renderer->Style());
    }
    return 0;
  }

  void Element::SetScrollLeft(int32 newLeft) noexcept
  {
    auto &document = NodeDocument();
    document.UpdateLayoutIgnorePendingStylesheets();

    // FIXME: Smooth scrolling (useSmoothScrolling), once there are scroll animations.
    auto options = ScrollPositionChangeOptions::CreateProgrammatic();

    if (document.ScrollingElement() == this)
    {
      if (auto *frame = DocumentFrameWithNonNullView(document))
      {
        IntPoint position(static_cast<int32>(newLeft * frame->PageZoomFactor()), frame->View()->ScrollY());
        frame->View()->SetScrollPosition(position, options);
      }
      return;
    }

    if (auto *renderer = RenderBoxOf(*this))
    {
      int32 clampedLeft = ClampToInteger(newLeft * renderer->Style().UsedZoom());
      renderer->SetScrollLeft(clampedLeft, options);
      // FIXME: Clear the latched wheel event state (setScrollShouldClearLatchedState), once there are wheel
      // events.
    }
  }

  void Element::SetScrollTop(int32 newTop) noexcept
  {
    auto &document = NodeDocument();
    document.UpdateLayoutIgnorePendingStylesheets();

    // FIXME: Smooth scrolling (useSmoothScrolling), once there are scroll animations.
    auto options = ScrollPositionChangeOptions::CreateProgrammatic();

    if (document.ScrollingElement() == this)
    {
      if (auto *frame = DocumentFrameWithNonNullView(document))
      {
        IntPoint position(frame->View()->ScrollX(), static_cast<int32>(newTop * frame->PageZoomFactor()));
        frame->View()->SetScrollPosition(position, options);
      }
      return;
    }

    if (auto *renderer = RenderBoxOf(*this))
    {
      int32 clampedTop = ClampToInteger(newTop * renderer->Style().UsedZoom());
      renderer->SetScrollTop(clampedTop, options);
      // FIXME: Clear the latched wheel event state (setScrollShouldClearLatchedState), once there are wheel
      // events.
    }
  }

  int32 Element::ScrollWidth() noexcept
  {
    auto &document = NodeDocument();
    document.UpdateLayoutIgnorePendingStylesheets();

    if (document.ScrollingElement() == this)
    {
      if (auto *frame = DocumentFrameWithNonNullView(document))
      {
        return AdjustContentsScrollPositionOrSizeForZoom(frame->View()->ContentsSize().Width(), *frame);
      }
      return 0;
    }

    if (auto *renderer = RenderBoxOf(*this))
    {
      return AdjustForAbsoluteZoom(renderer->ScrollWidth(), renderer->Style());
    }
    return 0;
  }

  int32 Element::ScrollHeight() noexcept
  {
    auto &document = NodeDocument();
    document.UpdateLayoutIgnorePendingStylesheets();

    if (document.ScrollingElement() == this)
    {
      if (auto *frame = DocumentFrameWithNonNullView(document))
      {
        return AdjustContentsScrollPositionOrSizeForZoom(frame->View()->ContentsSize().Height(), *frame);
      }
      return 0;
    }

    if (auto *renderer = RenderBoxOf(*this))
    {
      return AdjustForAbsoluteZoom(renderer->ScrollHeight(), renderer->Style());
    }
    return 0;
  }
}
