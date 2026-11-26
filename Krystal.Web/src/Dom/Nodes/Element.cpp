#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Core/Geometry/Quad.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"
#include "Krystal.Web/Accessibility/AXObjectCache.hpp"
#include "Krystal.Web/Animation/CSSAnimation.hpp"
#include "Krystal.Web/Animation/KeyframeAnimationOptions.hpp"
#include "Krystal.Web/Animation/KeyframeEffect.hpp"
#include "Krystal.Web/Animation/KeyframeEffectStack.hpp"
#include "Krystal.Web/Animation/WebAnimation.hpp"
#include "Krystal.Web/Css/Geometry/DOMRectList.hpp"
#include "Krystal.Web/Css/ResizeObserver/ResizeObserver.hpp"
#include "Krystal.Web/Css/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Web/Css/TypedOM/ComputedStylePropertyMapReadOnly.hpp"
#include "Krystal.Web/Css/TypedOM/StylePropertyMap.hpp"
#include "Krystal.Web/Dom/Collections/DOMStringMap.hpp"
#include "Krystal.Web/Dom/Collections/DOMTokenList.hpp"
#include "Krystal.Web/Dom/Collections/HTMLCollection.hpp"
#include "Krystal.Web/Dom/Collections/NodeList.hpp"
#include "Krystal.Web/Dom/Events/EventDispatcher.hpp"
#include "Krystal.Web/Dom/Events/EventNames.hpp"
#include "Krystal.Web/Dom/Events/EventTargetAlgorithms.hpp"
#include "Krystal.Web/Dom/Internals/IdTargetObserverRegistry.hpp"
#include "Krystal.Web/Dom/Internals/Iterators/ComposedTreeIterator.hpp"
#include "Krystal.Web/Dom/Internals/NameValidation.hpp"
#include "Krystal.Web/Dom/Internals/SelectorQuery.hpp"
#include "Krystal.Web/Dom/Mixins/ChildNode.hpp"
#include "Krystal.Web/Dom/Mixins/NonDocumentTypeChildNode.hpp"
#include "Krystal.Web/Dom/Mixins/ParentNode.hpp"
#include "Krystal.Web/Dom/Mixins/Slottable.hpp"
#include "Krystal.Web/Dom/Nodes/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Web/Dom/Nodes/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Web/Dom/Nodes/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Web/Dom/Nodes/Attr.hpp"
#include "Krystal.Web/Dom/Nodes/DocumentFragment.hpp"
#include "Krystal.Web/Dom/Nodes/ElementRareData.hpp"
#include "Krystal.Web/Dom/Nodes/GetHTMLOptions.hpp"
#include "Krystal.Web/Dom/Nodes/HTMLDocument.hpp"
#include "Krystal.Web/Dom/Nodes/NamedNodeMap.hpp"
#include "Krystal.Web/Dom/Nodes/PseudoElement.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Dom/Nodes/StyledElement.hpp"
#include "Krystal.Web/Dom/Nodes/Text.hpp"
#include "Krystal.Web/Dom/Nodes/TextDirectionState.hpp"
#include "Krystal.Web/Dom/Traversal/TreeTraversal.hpp"
#include "Krystal.Web/Dom/Trees/MutationAlgorithms.hpp"
#include "Krystal.Web/Dom/Trees/SlotAlgorithms.hpp"
#include "Krystal.Web/Dom/Trees/TreeQueries.hpp"
#include "Krystal.Web/Editing/Editing.hpp"
#include "Krystal.Web/Editing/Markup/Markup.hpp"
#include "Krystal.Web/Editing/Selection/FrameSelection.hpp"
#include "Krystal.Web/Events/Pointer/MouseEvent.hpp"
#include "Krystal.Web/Events/Pointer/PointerEvent.hpp"
#include "Krystal.Web/Events/Pointer/WheelEvent.hpp"
#include "Krystal.Web/Events/UI/KeyboardEvent.hpp"
#include "Krystal.Web/Html/Algorithms/ElementTextDirection.hpp"
#include "Krystal.Web/Html/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Web/Html/Algorithms/HTMLInsertionAlgorithms.hpp"
#include "Krystal.Web/Html/CustomElement/CustomElement.hpp"
#include "Krystal.Web/Html/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Web/Html/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Web/Html/CustomElement/CustomStateSet.hpp"
#include "Krystal.Web/Html/Elements/HTMLBodyElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLDialogElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLHtmlElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLInputElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLOptGroupElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLOptionElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLSelectElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLTemplateElement.hpp"
#include "Krystal.Web/Html/EventLoop/EventLoop.hpp"
#include "Krystal.Web/Html/Internals/PopoverData.hpp"
#include "Krystal.Web/Html/Internals/UserActionElementSet.hpp"
#include "Krystal.Web/Html/Sanitizer/SetHTMLOptions.hpp"
#include "Krystal.Web/Html/Window.hpp"
#include "Krystal.Web/Infra/Namespaces.hpp"
#include "Krystal.Web/Infra/StringAlgorithms.hpp"
#include "Krystal.Web/Input/EventHandler.hpp"
#include "Krystal.Web/Input/PlatformMouseEvent.hpp"
#include "Krystal.Web/Input/PointerCaptureController.hpp"
#include "Krystal.Web/Input/PointerLockController.hpp"
#include "Krystal.Web/IntersectionObserver/IntersectionObserver.hpp"
#include "Krystal.Web/Page/Frame.hpp"
#include "Krystal.Web/Page/FrameView.hpp"
#include "Krystal.Web/Page/Scrolling/ScrollBehavior.hpp"
#include "Krystal.Web/Platform/Scrolling/ScrollableArea.hpp"
#include "Krystal.Web/Platform/Scrolling/ScrollAlignment.hpp"
#include "Krystal.Web/Rendering/FormControls/RenderListBox.hpp"
#include "Krystal.Web/Rendering/FormControls/RenderTextControlSingleLine.hpp"
#include "Krystal.Web/Rendering/Layers/RenderLayer.hpp"
#include "Krystal.Web/Rendering/Layers/RenderLayerScrollableArea.hpp"
#include "Krystal.Web/Rendering/RenderBox.hpp"
#include "Krystal.Web/Rendering/RenderBoxModelObject.hpp"
#include "Krystal.Web/Rendering/RenderElement.hpp"
#include "Krystal.Web/Rendering/RenderTheme.hpp"
#include "Krystal.Web/Rendering/RenderView.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Web/Style/Change.hpp"
#include "Krystal.Web/Style/Invalidation/ChildChangeInvalidation.hpp"
#include "Krystal.Web/Style/Invalidation/Invalidator.hpp"
#include "Krystal.Web/Style/Invalidation/PseudoClassChangeInvalidation.hpp"
#include "Krystal.Web/Style/ResolvedStyle.hpp"
#include "Krystal.Web/Style/Resolver.hpp"
#include "Krystal.Web/Style/Scope/Scope.hpp"
#include "Krystal.Web/Style/TreeResolver.hpp"
#include "Krystal.Web/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Web/Svg/Elements/SVGElement.hpp"
#include <ranges>

namespace krys::web
{
  Element::Element(Document &document, const QualifiedName &name, dom::NodeFlags nodeFlags) noexcept
      : ContainerNode(document, dom::NodeType::ELEMENT_NODE, nodeFlags), _qualifiedName(name)
  {
  }

  Element::~Element() noexcept
  {
    krys_debug_assert(!BeforePseudoElement());
    krys_debug_assert(!AfterPseudoElement());

    DisconnectFromIntersectionObservers();
    DisconnectFromResizeObservers();
  }

  void Element::OnAdopt(Document &oldDocument) noexcept
  {
    ContainerNode::OnAdopt(oldDocument);

    // Intersection observers watching the element move to its new document. (Class names and IDs are folded
    // for quirks mode as they're read, so there's nothing to refresh.)
    if (auto *observerData = IntersectionObserverDataIfExists())
    {
      auto &newDocument = NodeDocument();
      for (const auto &observer : observerData->Observers)
      {
        if (observer->HasObservationTargets())
        {
          oldDocument.RemoveIntersectionObserver(*observer);
          newDocument.AddIntersectionObserver(*observer);
        }
      }
    }
  }

  dom::ElementRareData *Element::GetElementRareData() const noexcept
  {
    return static_cast<dom::ElementRareData *>(RareData());
  }

  dom::ElementRareData &Element::EnsureElementRareData() noexcept
  {
    return static_cast<dom::ElementRareData &>(EnsureNodeRareData());
  }

#pragma region Custom elements

  void Element::SetCustomElementState(dom::CustomElementState state) noexcept
  {
    style::PseudoClassChangeInvalidation styleInvalidation(
      *this, css::SelectorPseudoClass::Defined,
      state == dom::CustomElementState::Custom || state == dom::CustomElementState::Uncustomized);
    _customElementState = state;
    // Whether it's a listed element can depend on it.
    InvalidateLiveCollectionCachesInAncestors();
  }

  html::CustomElement *Element::GetCustomElement() const noexcept
  {
    if (!GetElementRareData() || !IsDefinedCustomElement())
    {
      return nullptr;
    }
    return GetElementRareData()->CustomElement();
  }

  html::CustomElementReactionQueue *Element::ReactionQueue() const noexcept
  {
    if (!GetElementRareData())
    {
      return nullptr;
    }
    return GetElementRareData()->CustomElementReactionQueue();
  }

  void Element::SetIsDefinedCustomElement(html::CustomElementInterface &elementInterface) noexcept
  {
    SetCustomElementState(dom::CustomElementState::Custom);
    auto &data = EnsureElementRareData();
    if (!data.CustomElementReactionQueue())
    {
      data.SetCustomElementReactionQueue(CreateUnique<html::CustomElementReactionQueue>(elementInterface));
    }
  }

  void Element::SetIsFailedCustomElement() noexcept
  {
    krys_debug_assert(IsHTMLUnknownElement());
    SetIsFailedCustomElementWithoutClearingReactionQueue();
    ClearReactionQueueFromFailedCustomElement();
  }

  void Element::SetIsFailedCustomElementWithoutClearingReactionQueue() noexcept
  {
    krys_debug_assert(_customElementState == dom::CustomElementState::Undefined
                      || _customElementState == dom::CustomElementState::Precustomized);
    SetCustomElementState(dom::CustomElementState::Failed);
  }

  void Element::SetIsPrecustomizedCustomElement() noexcept
  {
    krys_debug_assert(_customElementState == dom::CustomElementState::Failed);
    SetCustomElementState(dom::CustomElementState::Precustomized);
  }

  void Element::ClearReactionQueueFromFailedCustomElement() noexcept
  {
    krys_debug_assert(_customElementState == dom::CustomElementState::Failed);
    if (GetElementRareData())
    {
      // Clear the queue instead of deleting it since this function can be called inside
      // CustomElementReactionQueue::InvokeAll during upgrades.
      if (auto *queue = GetElementRareData()->CustomElementReactionQueue())
      {
        queue->Clear();
      }
    }
  }

  void Element::SetIsCustomElementUpgradeCandidate() noexcept
  {
    krys_debug_assert(_customElementState == dom::CustomElementState::Uncustomized);
    SetCustomElementState(dom::CustomElementState::Undefined);
  }

  void Element::EnqueueToUpgrade(html::CustomElementInterface &elementInterface) noexcept
  {
    krys_debug_assert(IsCustomElementUpgradeCandidate());
    auto &data = EnsureElementRareData();
    bool alreadyScheduledToUpgrade = data.CustomElementReactionQueue() != nullptr;
    if (!alreadyScheduledToUpgrade)
    {
      data.SetCustomElementReactionQueue(CreateUnique<html::CustomElementReactionQueue>(elementInterface));
    }
    html::CustomElementReactionQueue::EnqueueElementUpgrade(*this, alreadyScheduledToUpgrade);
  }

  void Element::SetCustomElement(UniquePtr<html::CustomElement> &&customElement) noexcept
  {
    EnsureElementRareData().SetCustomElement(krys::move(customElement));
  }

  void Element::SetCustomElementRegistry(RefPtr<html::CustomElementRegistry> &&registry) noexcept
  {
    _customElementRegistry = krys::move(registry);
  }

  DOMString Element::AssignedAccessKey() const noexcept
  {
    // 1. If the element has no accesskey attribute, then skip to the fallback step below.
    auto value = GetAttribute(u8"accesskey");
    if (!value)
    {
      return {};
    }

    // 2. Otherwise, split the attribute's value on ASCII whitespace, and let keys be the resulting tokens.
    // 3. For each value in keys in turn, in the order the tokens appeared in the attribute's value:
    for (auto &key : infra::StringAlgorithms::SplitOnWhitespace(*value))
    {
      // 3.1. If the value is not a string exactly one code point in length, then skip the remainder of these
      //      steps for this value.
      auto isLeadByte = [](char8_t unit)
      {
        return (unit & 0xC0) != 0x80;
      };
      if (std::ranges::count_if(key, isLeadByte) != 1)
      {
        continue;
      }
      // 3.2. If the value does not correspond to a key on the system's keyboard, then skip the remainder of
      //      these steps for this value.
      // (Only the host knows the keyboard; a key that isn't on it just never matches.)
      // 3.3. If the user agent can find a mix of zero or more modifier keys that, combined with the key that
      //      corresponds to the value given in the attribute, can be used as the access key, then the user
      //      agent may assign that combination of keys as the element's assigned access key and return.
      return key;
    }

    // 4. Fallback: Optionally, the user agent may assign a key combination of its choosing as the element's
    //    assigned access key and then return.
    // 5. If this step is reached, the element has no assigned access key.
    return {};
  }

  bool Element::AccessKeyAction(bool) noexcept
  {
    // NOTE: or its focus delegate, such as a date input's first field.
    if (html::FocusAlgorithms::IsFocusableArea(*this) || html::FocusAlgorithms::GetFocusableArea(*this))
    {
      html::mixins::HTMLOrSVGOrMathMLElement::Focus(*this);
    }
    return false;
  }

  bool Element::ShouldUseInputMethod() noexcept
  {
    return ComputeEditability(UserSelectAllTreatment::NotEditable, ShouldUpdateStyle::Update)
           != Editability::ReadOnly;
  }

  bool Element::HasValidTextDirectionState() const noexcept
  {
    return dom::ElementHasValidTextDirectionState(*this);
  }

  bool Element::HasAutoTextDirectionState() const noexcept
  {
    return dom::ElementHasAutoTextDirectionState(*this);
  }

  void Element::UpdateEffectiveTextDirection() noexcept
  {
    auto textDirectionState = dom::ElementTextDirectionState(*this);
    html::UpdateEffectiveTextDirectionState(*this, textDirectionState);
  }

  void Element::UpdateEffectiveTextDirectionIfNeeded() noexcept
  {
    if (SelfOrPrecedingNodesAffectDirAuto()) KRYS_UNLIKELY
    {
      UpdateEffectiveTextDirection();
      return;
    }

    auto *parent = dom::TreeTraversal::ParentOrShadowHostElement(*this);
    if (!(parent && parent->UsesEffectiveTextDirection()))
    {
      return;
    }

    if (HasAttribute(u8"dir") || dom::TreeTraversal::GetShadowRoot(*this))
    {
      UpdateEffectiveTextDirection();
      return;
    }

    if (auto *input = DynamicDowncast<HTMLInputElement>(*this); input && input->IsTelephoneField())
    {
      UpdateEffectiveTextDirection();
      return;
    }

    SetUsesEffectiveTextDirection(parent->UsesEffectiveTextDirection());
    SetEffectiveTextDirection(parent->EffectiveTextDirection());
  }

  bool Element::HasCustomState(DOMStringAtom state) const noexcept
  {
    if (GetElementRareData())
    {
      auto *customStates = GetElementRareData()->CustomStateSet();
      return customStates && customStates->Has(krys::move(state));
    }
    return false;
  }

  html::CustomStateSet &Element::EnsureCustomStateSet() noexcept
  {
    auto &rareData = EnsureElementRareData();
    if (!rareData.CustomStateSet())
    {
      rareData.SetCustomStateSet(html::CustomStateSet::Create(*this));
    }
    return *rareData.CustomStateSet();
  }

#pragma endregion

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
    if (GetElementRareData())
    {
      if (auto *style = GetElementRareData()->ComputedStyle())
      {
        return style;
      }
    }

    if (HasDisplayNone())
    {
      return GetElementRareData()->DisplayContentsOrNoneStyle();
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
      return GetElementRareData()->DisplayContentsOrNoneStyle();
    }

    return GetRenderStyle();
  }

  const RenderStyle *Element::ResolveComputedStyle(ResolveComputedStyleMode mode) noexcept
  {
    krys_debug_assert(IsConnected());

    auto document = ShareRef(NodeDocument());
    document->StyleScope().FlushPendingUpdate();

    bool isInDisplayNoneTree = false;

    // The furthest ancestor whose computed style is out of date, which resolution starts from.
    auto rootmostInvalidElement = [&]() -> RefPtr<const Element>
    {
      // With current style there's nothing to resolve, unless the caller wants only rendered elements and
      // so needs to know about display: none ancestors.
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
      if (!documentElement || documentElement->_nodeFlags.contains(dom::NodeFlag::IsComputedStyleInvalid))
      {
        return documentElement;
      }

      RefPtr<const Element> rootmost;

      for (auto element = ShareRefPtr<const Element>(this); element;
           element = ShareRefPtr<const Element>(element->ParentElementInComposedTree()))
      {
        if (element->_nodeFlags.contains(dom::NodeFlag::IsComputedStyleInvalid))
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
          // Nothing below a display: none element is rendered, so only out-of-date ancestors above it count.
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

    // From the furthest ancestor down, each style resolved and cached for the next to inherit from.
    for (auto &element : elementsRequiringComputedStyle | std::views::reverse)
    {
      if (computedStyle && computedStyle->ContainerType() != ContainerType::Normal
          && mode != ResolveComputedStyleMode::Editability)
      {
        // Container queries are only resolved by a full style update, so one is done instead.
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
        if (!changes.empty())
        {
          for (auto &child : dom::ComposedTreeChildren(*element))
          {
            if (auto *childElement = DynamicDowncast<Element>(child))
            {
              childElement->_nodeFlags.Add(dom::NodeFlag::IsComputedStyleInvalid);
            }
          }
        }
      }
      rareData.SetComputedStyle(krys::move(style));
      element->_nodeFlags.Remove(dom::NodeFlag::IsComputedStyleInvalid);

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

    // An existing style is taken as current: callers bring style up to date first.
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
    if (!GetElementRareData() || !GetElementRareData()->ComputedStyle())
    {
      return;
    }

    WillResetComputedStyle();
    GetElementRareData()->SetComputedStyle(nullptr);
    for (auto *descendant = dom::TreeQueries::Next(*this, this); descendant;
         descendant = dom::TreeQueries::Next(*descendant, this))
    {
      auto *child = DynamicDowncast<Element>(*descendant);
      if (!child || !child->GetElementRareData() || !child->GetElementRareData()->ComputedStyle()
          || child->HasDisplayContents() || child->HasDisplayNone())
      {
        continue;
      }
      child->WillResetComputedStyle();
      child->GetElementRareData()->SetComputedStyle(nullptr);
    }
  }

  void Element::ResetStyleRelations() noexcept
  {
    _nodeStyleFlags.Remove(dom::NodeStyleFlag::StyleAffectedByEmpty);
    _nodeStyleFlags.Remove(dom::NodeStyleFlag::AffectedByHasWithPositionalPseudoClass);
    if (!GetElementRareData())
    {
      return;
    }
    GetElementRareData()->SetChildIndex(0);
  }

  void Element::ResetChildStyleRelations() noexcept
  {
    _nodeStyleFlags.Remove(
      dom::NodeStyleFlags {dom::NodeStyleFlag::ChildrenAffectedByFirstChildRules,
                           dom::NodeStyleFlag::ChildrenAffectedByLastChildRules,
                           dom::NodeStyleFlag::ChildrenAffectedByForwardPositionalRules,
                           dom::NodeStyleFlag::ChildrenAffectedByBackwardPositionalRules});
  }

  void Element::ResetAllDescendantStyleRelations() noexcept
  {
    ResetChildStyleRelations();

    _nodeStyleFlags.Remove(
      dom::NodeStyleFlags {dom::NodeStyleFlag::DescendantsAffectedByForwardPositionalRules,
                           dom::NodeStyleFlag::DescendantsAffectedByBackwardPositionalRules});
  }

  bool Element::HasDisplayContents() const noexcept
  {
    if (!GetElementRareData())
    {
      return false;
    }
    auto *style = GetElementRareData()->DisplayContentsOrNoneStyle();
    return style && style->Display() == DisplayType::Contents;
  }

  bool Element::HasDisplayNone() const noexcept
  {
    if (!GetElementRareData())
    {
      return false;
    }
    auto *style = GetElementRareData()->DisplayContentsOrNoneStyle();
    return style && style->Display() == DisplayType::None;
  }

  void Element::StoreDisplayContentsOrNoneStyle(UniquePtr<RenderStyle> style) noexcept
  {
    // The render tree builder keeps the style of a display: contents or display: none element here, as it has
    // no renderer to hold it. It's apart from the computed style, which can change outside style resolution,
    // so it always matches what's rendered.
    krys_debug_assert(
      style && (style->Display() == DisplayType::Contents || style->Display() == DisplayType::None));
    krys_debug_assert(!Renderer() || IsPseudoElement());
    EnsureElementRareData().SetDisplayContentsOrNoneStyle(krys::move(style));
  }

  void Element::ClearDisplayContentsOrNoneStyle() noexcept
  {
    if (!GetElementRareData())
    {
      return;
    }
    GetElementRareData()->SetDisplayContentsOrNoneStyle(nullptr);
  }

  RenderPtr<RenderElement> Element::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return RenderElement::CreateFor(*this, krys::move(style));
  }

  bool Element::RendererIsNeeded(const RenderStyle &style) noexcept
  {
    return style.Display() != DisplayType::None && style.Display() != DisplayType::Contents;
  }

  void Element::DispatchFocusEvent(EventTarget *relatedTarget, const dom::FocusOptions &) noexcept
  {
    // While the document's window doesn't have system focus, the focus moves without events, as in Chrome:
    // the element gets them when the window gets system focus (FocusAlgorithms::SystemFocusChanged).
    if (NodeDocument().IsFocusedAndActive())
    {
      html::FocusAlgorithms::FireFocusEvent(u8"focus", *this, relatedTarget);
    }
  }

  void Element::DispatchBlurEvent(EventTarget *relatedTarget) noexcept
  {
    // (See DispatchFocusEvent: the element had its blur when the window lost system focus.)
    if (NodeDocument().IsFocusedAndActive())
    {
      html::FocusAlgorithms::FireFocusEvent(u8"blur", *this, relatedTarget);
    }
  }

  void Element::FindTargetAndUpdateFocusAppearance(dom::SelectionRestorationMode restorationMode,
                                                   SelectionRevealMode revealMode) noexcept
  {
    auto target = FocusAppearanceUpdateTarget();
    if (!target)
    {
      return;
    }

    target->UpdateFocusAppearance(restorationMode, revealMode);
  }

  RefPtr<Element> Element::FocusAppearanceUpdateTarget() noexcept
  {
    return ShareRefPtr(this);
  }

  void Element::UpdateFocusAppearance(dom::SelectionRestorationMode, SelectionRevealMode revealMode) noexcept
  {
    if (IsRootEditableElement())
    {
      // Keep the document alive in this method, since SetSelection() may run script.
      Ref<Document> document = ShareRef(NodeDocument());
      if (!document->Frame())
      {
        return;
      }
      auto &selection = document->Selection();

      // When focusing an editable element in an iframe, don't reset the selection if it already contains a
      // selection.
      if (this == selection.Selection().RootEditableElement())
      {
        selection.RevealSelection();
        return;
      }

      // The caret goes to the start, as in Chrome, which doesn't restore an earlier selection either.
      VisibleSelection newSelection = VisibleSelection(FirstPositionInOrBeforeNode(this));

      if (selection.ShouldChangeSelection(newSelection))
      {
        selection.SetSelection(newSelection, FrameSelection::DefaultSetSelectionOptions());
        selection.RevealSelection({revealMode});
        return;
      }
    }

    if (auto *view = NodeDocument().View())
    {
      view->ScheduleScrollToFocusedElement(revealMode);
    }
  }

  Maybe<DOMString> Element::ImageSourceUrl() const noexcept
  {
    return GetAttribute(u8"src");
  }

  net::Url Element::AbsoluteLinkUrl() const noexcept
  {
    if (!css::IsLink(*this))
    {
      return {};
    }

    Maybe<DOMString> linkAttribute;
    if (NamespaceURI().View() == infra::namespaces::SVG && LocalName().View() == u8"a")
    {
      linkAttribute = GetAttribute(u8"href");
      if (!linkAttribute)
      {
        linkAttribute = GetAttributeNS(infra::namespaces::XLink, u8"href");
      }
    }
    else
    {
      linkAttribute = GetAttribute(u8"href");
    }

    if (!linkAttribute || linkAttribute->empty())
    {
      return {};
    }

    return NodeDocument().CompleteUrl(*linkAttribute);
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
    return GetElementRareData() ? GetElementRareData()->BeforePseudoElement() : nullptr;
  }

  PseudoElement *Element::AfterPseudoElement() const noexcept
  {
    return GetElementRareData() ? GetElementRareData()->AfterPseudoElement() : nullptr;
  }

  PointI Element::SavedLayerScrollPosition() const noexcept
  {
    return GetElementRareData() ? GetElementRareData()->SavedLayerScrollPosition() : PointI();
  }

  void Element::SetSavedLayerScrollPosition(const PointI &position) noexcept
  {
    if (position.IsZero() && !GetElementRareData())
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
    if (!GetElementRareData() || !GetElementRareData()->BeforePseudoElement())
    {
      return;
    }
    DisconnectPseudoElement(GetElementRareData()->BeforePseudoElement());
    GetElementRareData()->SetBeforePseudoElement(nullptr);
  }

  void Element::ClearAfterPseudoElement() noexcept
  {
    if (!GetElementRareData() || !GetElementRareData()->AfterPseudoElement())
    {
      return;
    }
    DisconnectPseudoElement(GetElementRareData()->AfterPseudoElement());
    GetElementRareData()->SetAfterPseudoElement(nullptr);
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

      dom::InvalidateForSiblingCombinators(dom::TreeTraversal::NextElementSibling(element));
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
    // The whole subtree, as any of it may query the container.
    Node::InvalidateStyle(style::Validity::SubtreeInvalid);
    _nodeFlags.Add(dom::NodeFlag::NeedsUpdateQueryContainerDependentStyle);
  }

  void Element::InvalidateForResumingQueryContainerResolution() noexcept
  {
    SetChildNeedsStyleRecalc();
    MarkAncestorsForInvalidatedStyle();
  }

  void Element::InvalidateForAnchorRectChange() noexcept
  {
    Node::InvalidateStyle(style::Validity::ElementInvalid);
  }

  void Element::InvalidateForResumingAnchorPositionedElementResolution() noexcept
  {
    InvalidateStyleInternal();
    MarkAncestorsForInvalidatedStyle();
  }

  void Element::FinishParsingChildren() noexcept
  {
    _nodeFlags.Remove(dom::NodeFlag::IsParsingChildren);
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

  namespace
  {
    KRYS_NODISCARD bool IsCompatibilityMouseEvent(const MouseEvent &mouseEvent) noexcept
    {
      // https://www.w3.org/TR/pointerevents/#compatibility-mapping-with-mouse-events
      auto type = mouseEvent.Type();
      return !pointer_events::IsAnyClick(mouseEvent) && type != event_names::MouseOver
             && type != event_names::MouseOut && type != event_names::MouseEnter
             && type != event_names::MouseLeave;
    }

    enum class ShouldIgnoreMouseEvent : bool
    {
      No,
      Yes,
    };

    KRYS_NODISCARD ShouldIgnoreMouseEvent
      DispatchPointerEventIfNeeded(Element &element, const MouseEvent &mouseEvent,
                                   const PlatformMouseEvent &platformEvent, bool &didNotSwallowEvent) noexcept
    {
      if (auto *frame = element.NodeDocument().Frame())
      {
        auto &pointerCaptureController = frame->GetPointerCaptureController();
        if (platformEvent.PointerId() != MousePointerId && !pointer_events::IsAnyClick(mouseEvent)
            && pointerCaptureController.PreventsCompatibilityMouseEventsForIdentifier(
              platformEvent.PointerId()))
        {
          return ShouldIgnoreMouseEvent::Yes;
        }

        if (platformEvent.GetSyntheticClickType() != SyntheticClickType::NoTap
            && !pointer_events::IsAnyClick(mouseEvent) && mouseEvent.Type() != event_names::ContextMenu)
        {
          return ShouldIgnoreMouseEvent::No;
        }

        // https://w3c.github.io/pointerevents/#the-pointerrawupdate-event
        // The user agent MUST dispatch the pointerrawupdate event before the corresponding pointermove.
        bool isMove = mouseEvent.Type() == event_names::MouseMove;
        if (isMove)
        {
          pointerCaptureController.DispatchPointerRawUpdateForMouseEvent(
            mouseEvent, platformEvent.PointerId(), platformEvent.PointerType(), element);
        }

        if (pointerCaptureController.SkipPointerMoveForMouseEvent(mouseEvent, platformEvent.PointerId(),
                                                                  platformEvent.PointerType(), element))
        {
          return pointerCaptureController.PreventsCompatibilityMouseEventsForIdentifier(
                   platformEvent.PointerId())
                   ? ShouldIgnoreMouseEvent::Yes
                   : ShouldIgnoreMouseEvent::No;
        }

        if (auto pointerEvent = pointerCaptureController.PointerEventForMouseEvent(
              mouseEvent, platformEvent.PointerId(), platformEvent.PointerType()))
        {
          // A chorded button's press or release is a pointermove.
          if (!isMove && pointerEvent->Type() == event_names::PointerMove)
          {
            pointerCaptureController.DispatchPointerRawUpdateForMouseEvent(
              mouseEvent, platformEvent.PointerId(), platformEvent.PointerType(), element);
          }

          pointerCaptureController.DispatchEvent(*pointerEvent, &element);
          if (IsCompatibilityMouseEvent(mouseEvent)
              && pointerCaptureController.PreventsCompatibilityMouseEventsForIdentifier(
                pointerEvent->PointerId()))
          {
            return ShouldIgnoreMouseEvent::Yes;
          }

          if (pointerEvent->DefaultPrevented() || pointerEvent->DefaultHandled())
          {
            didNotSwallowEvent = false;
            if (pointerEvent->Type() == event_names::PointerDown)
            {
              return ShouldIgnoreMouseEvent::Yes;
            }
          }
        }
      }

      return ShouldIgnoreMouseEvent::No;
    }
  }

  Element::DispatchMouseEventResult Element::DispatchMouseEvent(const PlatformMouseEvent &platformEvent,
                                                                const DOMStringAtom &eventType, int32 detail,
                                                                Element *relatedTarget) noexcept
  {
    auto eventIsDefaultPrevented = EventIsDefaultPrevented::No;
    // FIXME: Force events (MouseForceChanged, MouseForceDown and MouseForceUp), once the host reports force
    // clicks.

    List<Ref<MouseEvent>> childMouseEvents;
    for (const auto &childPlatformEvent : platformEvent.CoalescedEvents())
    {
      childMouseEvents.push_back(MouseEvent::Create(eventType, NodeDocument().View(), childPlatformEvent, {},
                                                    {}, detail, relatedTarget));
    }

    List<Ref<MouseEvent>> predictedEvents;
    for (const auto &childPlatformEvent : platformEvent.PredictedEvents())
    {
      predictedEvents.push_back(MouseEvent::Create(eventType, NodeDocument().View(), childPlatformEvent, {},
                                                   {}, detail, relatedTarget));
    }

    Ref<MouseEvent> mouseEvent = MouseEvent::Create(eventType, NodeDocument().View(), platformEvent,
                                                    childMouseEvents, predictedEvents, detail, relatedTarget);

    if (mouseEvent->Type().View().empty())
    {
      return {EventIsDispatched::Yes, eventIsDefaultPrevented}; // Shouldn't happen.
    }

    Ref protectedThis = ShareRef(*this);
    bool didNotSwallowEvent = true;

    if (DispatchPointerEventIfNeeded(*this, *mouseEvent, platformEvent, didNotSwallowEvent)
        == ShouldIgnoreMouseEvent::Yes)
    {
      return {EventIsDispatched::No, eventIsDefaultPrevented};
    }

    bool shouldNotDispatchMouseEvent =
      pointer_events::IsAnyClick(*mouseEvent) || mouseEvent->Type() == event_names::ContextMenu;
    if (!shouldNotDispatchMouseEvent)
    {
      krys_debug_assert(!mouseEvent->Target() || mouseEvent->Target() != relatedTarget);
      (void)dom::EventDispatcher::DispatchToTarget(*mouseEvent, this);
      if (mouseEvent->DefaultPrevented())
      {
        eventIsDefaultPrevented = EventIsDefaultPrevented::Yes;
      }
      if (mouseEvent->DefaultPrevented() || mouseEvent->DefaultHandled())
      {
        didNotSwallowEvent = false;
      }
    }

    // The second click of a double click is followed by dblclick. Only the primary button fires click, so
    // only it double clicks.
    if (mouseEvent->Type() == event_names::Click && mouseEvent->Detail() == 2)
    {
      Ref<MouseEvent> doubleClickEvent =
        MouseEvent::Create(event_names::DblClick, mouseEvent->Bubbles(), mouseEvent->Cancelable(), true,
                           html::EventLoop::Current().Now(), mouseEvent->FrameView(), mouseEvent->Detail(),
                           mouseEvent->ScreenX(), mouseEvent->ScreenY(), mouseEvent->ClientX(),
                           mouseEvent->ClientY(), mouseEvent->ModifierKeys(), mouseEvent->ButtonType(),
                           mouseEvent->Buttons(), mouseEvent->GetSyntheticClickType(), relatedTarget);

      if (mouseEvent->DefaultHandled())
      {
        doubleClickEvent->SetDefaultHandled();
      }

      (void)dom::EventDispatcher::DispatchToTarget(*doubleClickEvent, this);
      if (doubleClickEvent->DefaultHandled() || doubleClickEvent->DefaultPrevented())
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

    // A wheel event without deltas only marks a gesture or its momentum starting or ending. The page doesn't
    // hear it (it's stopped before any listener), but the default event handler still sees the phase change.
    if (platformEvent.Delta().IsZero())
    {
      event->StopPropagation();
    }
    else
    {
      processing.Add(EventHandling::DispatchedToDOM);
    }

    Ref protectedThis = ShareRef(*this);
    (void)dom::EventDispatcher::DispatchToTarget(*event, this);

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
      startNode = dom::TreeTraversal::ParentOrShadowHostElement(*startNode);
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

  bool Element::DispatchSimulatedClick(Event *underlyingEvent,
                                       html::SimulatedClickMouseEventOptions eventOptions,
                                       html::SimulatedClickVisualOptions visualOptions) noexcept
  {
    return html::SimulateClick(*this, underlyingEvent, eventOptions, visualOptions,
                               html::SimulatedClickSource::UserAgent);
  }

  bool Element::DispatchKeyEvent(const PlatformKeyboardEvent &platformEvent) noexcept
  {
    Ref<KeyboardEvent> event = KeyboardEvent::Create(platformEvent, NodeDocument().Window());
    (void)dom::EventDispatcher::DispatchToTarget(*event, this);
    if (event->DefaultPrevented() || event->DefaultHandled())
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

    auto *renderer = Renderer();
    if (!renderer)
    {
      return;
    }

    if (!IsDisabledFormControl() && renderer->Style().HasUsedAppearance())
    {
      renderer->Repaint();
    }
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
      if (!Is<HTMLElement>(element))
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
      constexpr static DOMStringView nonTextFieldTypes[] = {
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
      if (!Is<HTMLElement>(element))
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

  void Element::SetFocus(bool value, dom::FocusVisibility visibility) noexcept
  {
    if (value == Focused())
    {
      return;
    }

    style::PseudoClassChangeInvalidation focusStyleInvalidation(
      *this, {{css::SelectorPseudoClass::Focus, value}, {css::SelectorPseudoClass::FocusVisible, value}});
    NodeDocument().UserActionElements().SetFocused(*this, value);

    // Each shadow root above the element records that it holds the focused element, and its host restyles, as
    // the host's :focus depends on it.
    for (auto *root = dom::TreeTraversal::ContainingShadowRoot(*this); root;
         root = dom::TreeTraversal::ContainingShadowRoot(*root->Host()))
    {
      root->SetContainsFocusedElement(value);
      root->Host()->InvalidateStyle();
    }

    for (auto *element = this; element; element = element->ParentElementInComposedTree())
    {
      element->SetHasFocusWithin(value);
    }

    SetHasFocusVisible(value
                       && (visibility == dom::FocusVisibility::Visible
                           || (visibility == dom::FocusVisibility::Invisible
                               && ShouldAlwaysHaveFocusVisibleWhenFocused(*this))));
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

    if (const auto *style = GetRenderStyle(); style && style->HasUsedAppearance())
    {
      if (auto *renderer = Renderer(); renderer && RenderTheme::Singleton().SupportsHover())
      {
        renderer->Repaint();
      }
    }
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
    // With :has() in use, a change can restyle elements outside the subtree, so an invalid subtree isn't
    // enough to know nothing else needs resolving.
    if (StyleValidity() >= style::Validity::SubtreeInvalid
        && !style::Scope::ForNode(*this).UsesHasPseudoClass())
    {
      return false;
    }
    auto *documentElement = dom::TreeQueries::DocumentElement(NodeDocument());
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
    if (auto *shadowRoot = dom::TreeTraversal::ContainingShadowRoot(*this))
    {
      return shadowRoot->StyleScope().GetResolver();
    }

    return NodeDocument().StyleScope().GetResolver();
  }

  style::UnadjustedStyle Element::ResolveStyle(const style::ResolutionContext &resolutionContext) noexcept
  {
    return StyleResolver().UnadjustedStyleForElement(*this, resolutionContext);
  }

  void Element::WillRecalcStyle(EnumSet<style::Change>) noexcept
  {
    krys_debug_assert(HasCustomStyleResolveCallbacks());
  }

  void Element::DidRecalcStyle(EnumSet<style::Change>) noexcept
  {
    krys_debug_assert(HasCustomStyleResolveCallbacks());
  }

  Maybe<style::UnadjustedStyle> Element::ResolveCustomStyle(const style::ResolutionContext &,
                                                            const RenderStyle *) noexcept
  {
    krys_debug_assert(HasCustomStyleResolveCallbacks());
    return null;
  }

  void dom::InvalidateForSiblingCombinators(Element *sibling) noexcept
  {
    for (auto *element = sibling; element; element = dom::TreeTraversal::NextElementSibling(*element))
    {
      if (element->StyleIsAffectedByPreviousSibling())
      {
        element->InvalidateStyleInternal();
      }
      if (element->DescendantsAffectedByPreviousSibling())
      {
        for (auto *siblingChild = dom::TreeTraversal::FirstElementChild(*element); siblingChild;
             siblingChild = dom::TreeTraversal::NextElementSibling(*siblingChild))
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

  uint32 Element::ChildIndex() const noexcept
  {
    return GetElementRareData() ? GetElementRareData()->ChildIndex() : 0;
  }

  void Element::SetChildIndex(uint32 index) noexcept
  {
    EnsureElementRareData().SetChildIndex(index);
  }

#pragma endregion

#pragma region Animations

  ExceptionOr<Ref<WebAnimation>> Element::Animate(KeyframesInput &&keyframes) noexcept
  {
    return Animate(krys::move(keyframes), null);
  }

  ExceptionOr<Ref<WebAnimation>>
    Element::Animate(KeyframesInput &&keyframes,
                     Maybe<Variant<double, KeyframeAnimationOptions>> options) noexcept
  {
    DOMString id;
    Maybe<RefPtr<AnimationTimeline>> timeline;
    Maybe<Variant<double, KeyframeEffectOptions>> keyframeEffectOptions;
    TimelineRangeValue animationRangeStart;
    TimelineRangeValue animationRangeEnd;
    if (options)
    {
      auto &optionsValue = *options;
      if (const auto *duration = std::get_if<double>(&optionsValue))
      {
        keyframeEffectOptions = Variant<double, KeyframeEffectOptions> {*duration};
      }
      else
      {
        auto &keyframeAnimationOptions = std::get<KeyframeAnimationOptions>(optionsValue);
        id = keyframeAnimationOptions.Id;
        timeline = keyframeAnimationOptions.Timeline;
        animationRangeStart = keyframeAnimationOptions.RangeStart;
        animationRangeEnd = keyframeAnimationOptions.RangeEnd;
        keyframeEffectOptions = Variant<double, KeyframeEffectOptions> {
          static_cast<KeyframeEffectOptions &&>(krys::move(keyframeAnimationOptions))};
      }
    }

    Ref document = ShareRef(NodeDocument());
    auto keyframeEffectResult =
      KeyframeEffect::Create(*document, this, krys::move(keyframes), krys::move(keyframeEffectOptions));
    if (keyframeEffectResult.HasException())
    {
      return keyframeEffectResult.ReleaseException();
    }

    Ref animation = WebAnimation::Create(*document, keyframeEffectResult.Value().get());
    animation->SetId(krys::move(id));
    if (timeline)
    {
      animation->SetTimeline(RefPtr<AnimationTimeline> {*timeline});
    }
    animation->SetBindingsRangeStart(krys::move(animationRangeStart));
    animation->SetBindingsRangeEnd(krys::move(animationRangeEnd));

    auto animationPlayResult = animation->Play();
    if (animationPlayResult.HasException())
    {
      return animationPlayResult.ReleaseException();
    }

    return animation;
  }

  List<RefPtr<WebAnimation>> Element::GetAnimations(const Maybe<GetAnimationsOptions> &options) noexcept
  {
    // For the subtree, the document's animations whose targets (or their pseudo-elements' elements) are in
    // it.
    Ref document = ShareRef(NodeDocument());
    if (options && options->Subtree)
    {
      return document->MatchingAnimations([&](Element &target) -> bool { return Contains(&target); });
    }

    // Style is brought up to date first, so CSS animations and transitions that style changes start or end
    // are listed.
    (void)document->UpdateStyleIfNeeded();

    List<RefPtr<WebAnimation>> animations;
    if (auto *effectStack = GetKeyframeEffectStack({}))
    {
      for (const auto &effect : effectStack->SortedEffects())
      {
        if (effect && effect->Animation()->IsRelevant())
        {
          animations.push_back(ShareRefPtr(effect->Animation()));
        }
      }
    }
    return animations;
  }

  bool Element::MayHaveKeyframeEffects() const noexcept
  {
    return GetElementRareData() && GetElementRareData()->HasAnimationRareData();
  }

  ElementAnimationRareData *Element::AnimationRareData(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept
  {
    return GetElementRareData() ? GetElementRareData()->AnimationRareData(pseudoElementIdentifier) : nullptr;
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
    if (animations.empty() && !AnimationRareData(pseudoElementIdentifier))
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
    if (!GetElementRareData())
    {
      return false;
    }
    return GetElementRareData()->HasRandomCachingKeyMap();
  }

#pragma region Element

  DOMString Element::TagName() const noexcept
  {
    // 1. Let qualifiedName be this's qualified name.
    auto qualifiedName = _qualifiedName.Name();

    // 2. If this is in the HTML namespace and its node document is an HTML document, then return
    //    qualifiedName in ASCII uppercase.
    if (NamespaceURI() == infra::namespaces::HTML && Is<HTMLDocument>(NodeDocument()))
    {
      qualifiedName = krys::text::ToASCIIUpper(qualifiedName);
    }

    // 3. Return qualifiedName.
    return qualifiedName;
  }

  void Element::Id(DOMString id) noexcept
  {
    html::CEReactionsScope ceReactions;
    dom::ElementAlgorithms::SetAttributeValue(*this, u8"id", krys::move(id));
  }

  DOMString Element::Id() const noexcept
  {
    return dom::ElementAlgorithms::GetAttributeValue(*this, u8"id");
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

  List<DOMStringAtom> Element::ClassNamesForValue(Maybe<DOMStringView> value) const noexcept
  {
    List<DOMStringAtom> classNames;
    if (!value)
    {
      return classNames;
    }
    bool foldCase = NodeDocument().InQuirksMode();
    DOMStringView remaining = *value;
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

  void Element::ClassName(DOMString className) noexcept
  {
    html::CEReactionsScope ceReactions;
    dom::ElementAlgorithms::SetAttributeValue(*this, u8"class", krys::move(className));
  }

  DOMString Element::ClassName() const noexcept
  {
    return dom::ElementAlgorithms::GetAttributeValue(*this, u8"class");
  }

  DOMTokenList &Element::ClassList() noexcept
  {
    if (_domTokenList == nullptr)
    {
      _domTokenList = DOMTokenList::Create(*this, u8"class");
    }

    return *_domTokenList;
  }

  DOMTokenList &Element::Part() noexcept
  {
    auto &rareData = EnsureElementRareData();
    if (!rareData.PartList())
    {
      rareData.SetPartList(DOMTokenList::Create(*this, u8"part"));
    }
    return *rareData.PartList();
  }

  DOMStringMap &Element::Dataset() noexcept
  {
    auto &rareData = EnsureElementRareData();
    if (!rareData.Dataset())
    {
      rareData.SetDataset(CreateUnique<DOMStringMap>(*this));
    }
    return *rareData.Dataset();
  }

  StylePropertyMapReadOnly &Element::ComputedStyleMap() noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-element-computedstylemap
    auto &rareData = EnsureElementRareData();
    if (!rareData.ComputedStyleMap())
    {
      rareData.SetComputedStyleMap(ComputedStylePropertyMapReadOnly::Create(*this));
    }
    return *rareData.ComputedStyleMap();
  }

  StylePropertyMap *Element::ExistingAttributeStyleMap() const noexcept
  {
    auto *rareData = GetElementRareData();
    return rareData ? rareData->AttributeStyleMap() : nullptr;
  }

  void Element::SetAttributeStyleMap(Ref<StylePropertyMap> map) noexcept
  {
    EnsureElementRareData().SetAttributeStyleMap(krys::move(map));
  }

  void Element::Slot(DOMString slot) noexcept
  {
    html::CEReactionsScope ceReactions;
    dom::ElementAlgorithms::SetAttributeValue(*this, u8"slot", krys::move(slot));
  }

  DOMString Element::Slot() const noexcept
  {
    return dom::ElementAlgorithms::GetAttributeValue(*this, u8"slot");
  }

  bool Element::HasAttributes() const noexcept
  {
    SynchronizeAllAttributes();
    return !_attributes.empty();
  }

  dom::NamedNodeMap &Element::Attributes() noexcept
  {
    if (_namedNodeMap == nullptr)
    {
      _namedNodeMap = UniquePtr<dom::NamedNodeMap>(new dom::NamedNodeMap(*this));
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
    // 1. Let attr be the result of getting an attribute given qualifiedName and this.
    Attr *attr = dom::ElementAlgorithms::GetAttributeByName(krys::move(qualifiedName), *this);
    // 2. If attr is null, return null.
    if (attr == nullptr)
    {
      return null;
    }

    // 3. Return attr's value.
    return attr->Value();
  }

  Maybe<DOMString> Element::GetAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) const noexcept
  {
    // 1. Let attr be the result of getting an attribute given namespace, localName, and this.
    Attr *attr =
      dom::ElementAlgorithms::GetAttributeByNamespace(krys::move(namespaceURI), krys::move(localName), *this);
    // 2. If attr is null, return null.
    if (attr == nullptr)
    {
      return null;
    }

    // 3. Return attr's value.
    return attr->Value();
  }

  ExceptionOr<void> Element::SetAttribute(DOMStringAtom qualifiedName, DOMString value) noexcept
  {
    html::CEReactionsScope ceReactions;
    // 1. If qualifiedName is not a valid attribute local name, then throw an "InvalidCharacterError"
    //    DOMException.
    if (!dom::NameValidation::IsValidAttributeLocalName(qualifiedName.View()))
    {
      return Exception {ExceptionCode::InvalidCharacterError};
    }

    // 2. If this is in the HTML namespace and its node document is an HTML document, then set qualifiedName
    //    to qualifiedName in ASCII lowercase.
    if (NamespaceURI() == infra::namespaces::HTML && Is<HTMLDocument>(NodeDocument()))
    {
      qualifiedName = qualifiedName.ToASCIILower();
    }

    // 3. (Skipped: Trusted Types aren't supported, so value is used as it is.)

    // 4. Let attribute be the first attribute in this's attribute list whose qualified name is qualifiedName,
    //    and null otherwise.
    SynchronizeAllAttributes();
    auto it = std::find_if(_attributes.begin(), _attributes.end(),
                           [&](const Ref<Attr> &attr) { return attr->Name() == qualifiedName; });
    // 5. If attribute is non-null, then change attribute to verifiedValue and return.
    if (it != _attributes.end())
    {
      dom::ElementAlgorithms::ChangeAttribute(**it, krys::move(value));
    }
    else
    {
      // 6. Set attribute to the result of creating an attribute given this's node document, qualifiedName,
      //    null, null, and verifiedValue.
      // 7. Append attribute to this.
      auto attr = AdoptRef<Attr>(
        *new Attr(NodeDocument(), QualifiedName {null, null, qualifiedName}, krys::move(value)));
      dom::ElementAlgorithms::AppendAttribute(*attr, *this);
    }

    return {};
  }

  ExceptionOr<void> Element::SetAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom qualifiedName,
                                            DOMString value) noexcept
  {
    html::CEReactionsScope ceReactions;
    // 1. Let (namespace, prefix, localName) be the result of validating and extracting namespace and
    //    qualifiedName given "attribute".
    auto validateAndExtractResult = dom::NameValidation::ValidateAndExtract(
      krys::move(namespaceURI), krys::move(qualifiedName), dom::ValidateAndExtractContext::Attribute);

    if (validateAndExtractResult.HasException())
    {
      return validateAndExtractResult.ReleaseException();
    }

    // 2. (Skipped: Trusted Types aren't supported, so value is used as it is.)

    // 3. Set an attribute value for this using localName, verifiedValue, prefix, and namespace.
    const auto &qName = validateAndExtractResult.Value();
    dom::ElementAlgorithms::SetAttributeValue(*this, qName.LocalName(), krys::move(value),
                                              qName.NamespacePrefix(), qName.NamespaceURI());

    return {};
  }

  void Element::RemoveAttribute(DOMStringAtom qualifiedName) noexcept
  {
    html::CEReactionsScope ceReactions;
    dom::ElementAlgorithms::RemoveAttributeByName(krys::move(qualifiedName), *this);
  }

  void Element::RemoveAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) noexcept
  {
    html::CEReactionsScope ceReactions;
    dom::ElementAlgorithms::RemoveAttributeByNamespace(krys::move(namespaceURI), krys::move(localName),
                                                       *this);
  }

  ExceptionOr<bool> Element::ToggleAttribute(DOMStringAtom qualifiedName, const Maybe<bool> &force) noexcept
  {
    html::CEReactionsScope ceReactions;
    // 1. If qualifiedName is not a valid attribute local name, then throw an "InvalidCharacterError"
    //    DOMException.
    if (!dom::NameValidation::IsValidAttributeLocalName(qualifiedName.View()))
    {
      return Exception {ExceptionCode::InvalidCharacterError};
    }

    // 2. If this is in the HTML namespace and its node document is an HTML document, then set qualifiedName
    //    to qualifiedName in ASCII lowercase.
    if (NamespaceURI() == infra::namespaces::HTML && Is<HTMLDocument>(NodeDocument()))
    {
      qualifiedName = qualifiedName.ToASCIILower();
    }

    // 3. Let attribute be the first attribute in this's attribute list whose qualified name is qualifiedName,
    //    and null otherwise.
    SynchronizeAllAttributes();
    auto it = std::find_if(_attributes.begin(), _attributes.end(),
                           [&](const Ref<Attr> &attr) { return attr->Name() == qualifiedName; });

    // 4. If attribute is null:
    if (it == _attributes.end())
    {
      // 4a. If force is not given or is true, then append the result of creating an attribute given this's
      //     node document and qualifiedName to this, and then return true.
      if (!force.has_value() || force.value())
      {
        auto attr =
          AdoptRef<Attr>(*new Attr(NodeDocument(), QualifiedName {null, null, qualifiedName}, u8""));
        dom::ElementAlgorithms::AppendAttribute(*attr, *this);
        return true;
      }

      // 4b. Return false.
      return false;
    }

    // 5. If force is not given or is false, remove an attribute given qualifiedName and this, and then return
    //    false.
    if (!force.has_value() || !force.value())
    {
      dom::ElementAlgorithms::RemoveAttribute(**it);
      return false;
    }

    // 6. Return true.
    return true;
  }

  bool Element::HasAttribute(DOMStringAtom qualifiedName) const noexcept
  {
    // 1. If this is in the HTML namespace and its node document is an HTML document, then set qualifiedName
    //    to qualifiedName in ASCII lowercase.
    if (NamespaceURI() == infra::namespaces::HTML && Is<HTMLDocument>(NodeDocument()))
    {
      qualifiedName = qualifiedName.ToASCIILower();
    }

    // 2. Return true if this has an attribute whose qualified name is qualifiedName; otherwise false.
    SynchronizeAllAttributes();
    return std::ranges::any_of(_attributes, [&](const Ref<Attr> &a) { return a->Name() == qualifiedName; });
  }

  bool Element::HasAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) const noexcept
  {
    // 1. If namespace is the empty string, then set it to null.
    if (namespaceURI == DOMStringAtom::Empty())
    {
      namespaceURI = null;
    }

    // 2. Return true if this has an attribute whose namespace is namespace and local name is localName;
    //    otherwise false.
    SynchronizeAllAttributes();
    return std::ranges::any_of(_attributes, [&](const Ref<Attr> &a)
                               { return a->NamespaceURI() == namespaceURI && a->LocalName() == localName; });
  }

  Attr *Element::GetAttributeNode(DOMStringAtom qualifiedName) const noexcept
  {
    return dom::ElementAlgorithms::GetAttributeByName(krys::move(qualifiedName), *this);
  }

  Attr *Element::GetAttributeNodeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) const noexcept
  {
    return dom::ElementAlgorithms::GetAttributeByNamespace(krys::move(namespaceURI), krys::move(localName),
                                                           *this);
  }

  ExceptionOr<RefPtr<Attr>> Element::SetAttributeNode(Attr &attr) noexcept
  {
    html::CEReactionsScope ceReactions;
    return dom::ElementAlgorithms::SetAttribute(attr, *this);
  }

  ExceptionOr<RefPtr<Attr>> Element::SetAttributeNodeNS(Attr &attr) noexcept
  {
    html::CEReactionsScope ceReactions;
    return dom::ElementAlgorithms::SetAttribute(attr, *this);
  }

  ExceptionOr<Ref<Attr>> Element::RemoveAttributeNode(Attr &attr) noexcept
  {
    html::CEReactionsScope ceReactions;
    // 1. If this's attribute list does not contain attr, then throw a "NotFoundError" DOMException.
    if (!std::ranges::any_of(_attributes, [&](const Ref<Attr> &a) { return a.get() == &attr; }))
    {
      return Exception {ExceptionCode::NotFoundError};
    }

    // 2. Remove attr.
    dom::ElementAlgorithms::RemoveAttribute(attr);

    // 3. Return attr.
    return ShareRef(attr);
  }

  i18n::Locale &Element::GetLocale() const noexcept
  {
    return NodeDocument().GetCachedLocale(css::EffectiveLang(*this));
  }

  ExceptionOr<Ref<ShadowRoot>> Element::AttachShadow(const dom::ShadowRootInit &init) noexcept
  {
    // 1. Let registry be this's node document's custom element registry.
    // 2. If init["customElementRegistry"] exists, then set registry to it.
    auto registry =
      init.CustomElementRegistry ? init.CustomElementRegistry : NodeDocument().CustomElementRegistry();

    // 3. If registry is non-null, registry's is scoped is false, and registry is not this's node document's
    //    custom element registry, then throw a "NotSupportedError" DOMException.
    if (registry != nullptr && !registry->IsScoped() && registry != NodeDocument().CustomElementRegistry())
    {
      return Exception {ExceptionCode::NotSupportedError};
    }

    // 4. Run attach a shadow root with this, init["mode"], init["delegatesFocus"], init["serializable"],
    //    init["slotAssignment"], init["clonable"], init["referenceTarget"], and registry.
    if (auto attachResult = dom::ElementAlgorithms::AttachShadowRoot(
          *this, init.Mode, init.Clonable, init.Serialisable, init.DelegatesFocus, init.SlotAssignment,
          init.ReferenceTarget, registry);
        attachResult.HasException())
    {
      return attachResult.ReleaseException();
    }

    // 5. Return this's shadow root.
    return ShareRef(*_shadowRoot);
  }

  RefPtr<ShadowRoot> Element::ShadowRoot() const noexcept
  {
    // 1. Let shadow be this's shadow root.
    // 2. If shadow is null or its mode is "closed", then return null. (A user agent shadow root counts as
    //    closed.)
    if (!_shadowRoot || _shadowRoot->Mode() != dom::ShadowRootMode::Open)
    {
      return nullptr;
    }

    // 3. Return shadow.
    return _shadowRoot;
  }

  ShadowRoot *Element::UserAgentShadowRoot() const noexcept
  {
    krys_debug_assert(!_shadowRoot || _shadowRoot->Mode() == dom::ShadowRootMode::UserAgent);
    return _shadowRoot.get();
  }

  ShadowRoot &Element::EnsureUserAgentShadowRoot() noexcept
  {
    if (auto *shadow = UserAgentShadowRoot())
    {
      return *shadow;
    }
    return CreateUserAgentShadowRoot();
  }

  ShadowRoot &Element::CreateUserAgentShadowRoot() noexcept
  {
    krys_debug_assert(!UserAgentShadowRoot());
    return dom::ElementAlgorithms::AttachUserAgentShadowRoot(*this);
  }

  DOMStringAtom Element::UserAgentPart() const noexcept
  {
    krys_debug_assert(IsInUserAgentShadowTree());
    auto part = GetAttribute(u8"useragentpart");
    return part ? DOMStringAtom(*part) : null;
  }

  void Element::SetUserAgentPart(const DOMStringAtom &part) noexcept
  {
    // It may be set before the element goes into its user agent shadow tree.
    krys_debug_assert(!IsConnected() || IsInUserAgentShadowTree());
    (void)SetAttribute(u8"useragentpart", part.ToString());
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
    return dom::NodeAlgorithms::GetElementsByTagName(*this, krys::move(qualifiedName));
  }

  Ref<HTMLCollection> Element::GetElementsByTagNameNS(DOMStringAtom namespaceURI,
                                                      DOMStringAtom localName) noexcept
  {
    return dom::NodeAlgorithms::GetElementsByTagNameNS(*this, krys::move(namespaceURI),
                                                       krys::move(localName));
  }

  Ref<HTMLCollection> Element::GetElementsByClassName(DOMStringAtom classNames) noexcept
  {
    return dom::NodeAlgorithms::GetElementsByClassName(*this, krys::move(classNames));
  }

  ExceptionOr<Element *> Element::InsertAdjacentElement(dom::InsertAdjacentWhere where,
                                                        Element &element) noexcept
  {
    html::CEReactionsScope ceReactions;
    auto result = dom::ElementAlgorithms::InsertAdjacent(*this, where, element);
    if (result.HasException())
    {
      return result.ReleaseException();
    }

    // Null if the element has no parent to insert it before or after.
    return result.Value() ? &element : nullptr;
  }

  ExceptionOr<void> Element::InsertAdjacentText(dom::InsertAdjacentWhere where, DOMString data) noexcept
  {
    html::CEReactionsScope ceReactions;
    // 1. Let text be the result of creating a text node given this's node document and data.
    auto textNode = CreateRef<Text>(NodeDocument(), krys::move(data));
    // 2. Run insert adjacent, given this, where, and text.
    if (auto result = dom::ElementAlgorithms::InsertAdjacent(*this, where, *textNode); result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

#pragma endregion

#pragma region Element (DOM Parsing)

  namespace
  {
    // The element whose children insertAdjacentHTML() parses the markup as: the element, or its parent for
    // beforebegin and afterend, with a body element in place of a context that isn't an element or is an
    // html element in an HTML document.
    KRYS_NODISCARD ExceptionOr<Ref<Element>>
      ContextElementForInsertAdjacentHTML(Element &element, dom::InsertAdjacentWhere where) noexcept
    {
      Ref<ContainerNode> contextNode = ShareRef<ContainerNode>(element);
      if (where == dom::InsertAdjacentWhere::BeforeBegin || where == dom::InsertAdjacentWhere::AfterEnd)
      {
        auto *parent = element.ParentNode();
        if (!parent || Is<Document>(*parent))
        {
          return Exception {ExceptionCode::NoModificationAllowedError};
        }
        contextNode = ShareRef(*parent);
      }

      auto *contextElement = DynamicDowncast<Element>(contextNode.get());
      if (!contextElement
          || (Is<HTMLDocument>(contextNode->NodeDocument()) && Is<HTMLHtmlElement>(*contextElement)))
      {
        return CreateHTMLElement(contextNode->NodeDocument(), u8"body");
      }
      return ShareRef(*contextElement);
    }

    // Appends the data of the text node after node, if there is one, to node's, and removes it.
    KRYS_NODISCARD ExceptionOr<void> MergeWithNextTextNode(Text &node) noexcept
    {
      auto *next = DynamicDowncast<Text>(node.NextSibling());
      if (!next)
      {
        return {};
      }
      Ref<Text> protectedNext = ShareRef(*next);
      if (auto result = node.AppendData(protectedNext->Data()); result.HasException())
      {
        return result.ReleaseException();
      }
      return protectedNext->Remove();
    }
  }

  ExceptionOr<void> Element::ReplaceChildrenWithMarkup(DOMStringView markup) noexcept
  {
    Ref<ContainerNode> container = ShareRef<ContainerNode>(*this);
    if (auto *templateElement = DynamicDowncast<HTMLTemplateElement>(*this))
    {
      container = ShareRef<ContainerNode>(*templateElement->Content());
    }

    // An empty string only parses to new elements in an <html> element (which gets a head and a body); in
    // anything else it just removes the children.
    // https://html.spec.whatwg.org/multipage/parsing.html#parsing-main-inhtml
    if (markup.empty() && !Is<HTMLHtmlElement>(*container))
    {
      return dom::MutationAlgorithms::ReplaceAll(nullptr, *container);
    }

    auto fragment = CreateFragmentForInnerOuterHTML(*this, markup);
    if (fragment.HasException())
    {
      return fragment.ReleaseException();
    }
    return ReplaceChildrenWithFragment(*container, fragment.ReleaseValue());
  }

  ExceptionOr<void> Element::SetHTML(DOMStringView html) noexcept
  {
    return SetHTML(html, {});
  }

  ExceptionOr<void> Element::SetHTML(DOMStringView html, const html::SetHTMLOptions &options) noexcept
  {
    html::CEReactionsScope ceReactions;
    // 1. Let target be the target for HTML insertion given this.
    // 2. Filter and set HTML given target, html, options, and Safe.
    return html::HTMLInsertionAlgorithms::FilterAndSetHTML(
      html::HTMLInsertionAlgorithms::TargetForHTMLInsertion(*this), html, &options.Sanitizer,
      html::FragmentParserMode::Safe);
  }

  ExceptionOr<void> Element::AppendHTML(DOMStringView html) noexcept
  {
    return AppendHTML(html, {});
  }

  ExceptionOr<void> Element::AppendHTML(DOMStringView html, const html::SetHTMLOptions &options) noexcept
  {
    html::CEReactionsScope ceReactions;
    // 1. Let target be the target for HTML insertion given this.
    // 2. Filter and pre-insert HTML given target, null, html, options, and Safe.
    return html::HTMLInsertionAlgorithms::FilterAndPreInsertHTML(
      html::HTMLInsertionAlgorithms::TargetForHTMLInsertion(*this), nullptr, html, &options.Sanitizer,
      html::FragmentParserMode::Safe);
  }

  ExceptionOr<void> Element::PrependHTML(DOMStringView html) noexcept
  {
    return PrependHTML(html, {});
  }

  ExceptionOr<void> Element::PrependHTML(DOMStringView html, const html::SetHTMLOptions &options) noexcept
  {
    html::CEReactionsScope ceReactions;
    // 1. Let target be the target for HTML insertion given this.
    // 2. Filter and pre-insert HTML given target, target's first child, html, options, and Safe.
    auto &target = html::HTMLInsertionAlgorithms::TargetForHTMLInsertion(*this);
    return html::HTMLInsertionAlgorithms::FilterAndPreInsertHTML(
      target, target.FirstChild(), html, &options.Sanitizer, html::FragmentParserMode::Safe);
  }

  ExceptionOr<void> Element::SetHTMLUnsafe(DOMStringView html) noexcept
  {
    return SetHTMLUnsafe(html, {});
  }

  ExceptionOr<void> Element::SetHTMLUnsafe(DOMStringView html,
                                           const html::SetHTMLUnsafeOptions &options) noexcept
  {
    html::CEReactionsScope ceReactions;
    // 1. Let (compliantHTML, compliantOptions) be the result of invoking the get trusted type compliant input
    //    algorithm with this's relevant global object, html, options, and "Element setHTMLUnsafe".
    // (There are no Trusted Types.)
    // 2. Let target be the target for HTML insertion given this.
    // 3. Filter and set HTML given target, html, options, and Unsafe.
    return html::HTMLInsertionAlgorithms::FilterAndSetHTML(
      html::HTMLInsertionAlgorithms::TargetForHTMLInsertion(*this), html,
      options.Sanitizer ? &*options.Sanitizer : nullptr, html::FragmentParserMode::Unsafe);
  }

  ExceptionOr<void> Element::AppendHTMLUnsafe(DOMStringView html) noexcept
  {
    return AppendHTMLUnsafe(html, {});
  }

  ExceptionOr<void> Element::AppendHTMLUnsafe(DOMStringView html,
                                              const html::SetHTMLUnsafeOptions &options) noexcept
  {
    html::CEReactionsScope ceReactions;
    // 1. Let (compliantHTML, compliantOptions) be the result of invoking the get trusted type compliant input
    //    algorithm with this's relevant global object, html, options, and "Element appendHTMLUnsafe".
    // (There are no Trusted Types.)
    // 2. Let target be the target for HTML insertion given this.
    // 3. Filter and pre-insert HTML given target, null, html, options, and Unsafe.
    return html::HTMLInsertionAlgorithms::FilterAndPreInsertHTML(
      html::HTMLInsertionAlgorithms::TargetForHTMLInsertion(*this), nullptr, html,
      options.Sanitizer ? &*options.Sanitizer : nullptr, html::FragmentParserMode::Unsafe);
  }

  ExceptionOr<void> Element::PrependHTMLUnsafe(DOMStringView html) noexcept
  {
    return PrependHTMLUnsafe(html, {});
  }

  ExceptionOr<void> Element::PrependHTMLUnsafe(DOMStringView html,
                                               const html::SetHTMLUnsafeOptions &options) noexcept
  {
    html::CEReactionsScope ceReactions;
    // 1. Let (compliantHTML, compliantOptions) be the result of invoking the get trusted type compliant input
    //    algorithm with this's relevant global object, html, options, and "Element prependHTMLUnsafe".
    // (There are no Trusted Types.)
    // 2. Let target be the target for HTML insertion given this.
    // 3. Filter and pre-insert HTML given target, target's first child, html, options, and Unsafe.
    auto &target = html::HTMLInsertionAlgorithms::TargetForHTMLInsertion(*this);
    return html::HTMLInsertionAlgorithms::FilterAndPreInsertHTML(
      target, target.FirstChild(), html, options.Sanitizer ? &*options.Sanitizer : nullptr,
      html::FragmentParserMode::Unsafe);
  }

  ExceptionOr<void> Element::BeforeHTML(DOMStringView html) noexcept
  {
    return BeforeHTML(html, {});
  }

  ExceptionOr<void> Element::BeforeHTML(DOMStringView html, const html::SetHTMLOptions &options) noexcept
  {
    html::CEReactionsScope ceReactions;
    return html::HTMLInsertionAlgorithms::BeforeHTML(*this, html, &options.Sanitizer,
                                                     html::FragmentParserMode::Safe);
  }

  ExceptionOr<void> Element::AfterHTML(DOMStringView html) noexcept
  {
    return AfterHTML(html, {});
  }

  ExceptionOr<void> Element::AfterHTML(DOMStringView html, const html::SetHTMLOptions &options) noexcept
  {
    html::CEReactionsScope ceReactions;
    return html::HTMLInsertionAlgorithms::AfterHTML(*this, html, &options.Sanitizer,
                                                    html::FragmentParserMode::Safe);
  }

  ExceptionOr<void> Element::ReplaceWithHTML(DOMStringView html) noexcept
  {
    return ReplaceWithHTML(html, {});
  }

  ExceptionOr<void> Element::ReplaceWithHTML(DOMStringView html, const html::SetHTMLOptions &options) noexcept
  {
    html::CEReactionsScope ceReactions;
    return html::HTMLInsertionAlgorithms::ReplaceWithHTML(*this, html, &options.Sanitizer,
                                                          html::FragmentParserMode::Safe);
  }

  ExceptionOr<void> Element::BeforeHTMLUnsafe(DOMStringView html) noexcept
  {
    return BeforeHTMLUnsafe(html, {});
  }

  ExceptionOr<void> Element::BeforeHTMLUnsafe(DOMStringView html,
                                              const html::SetHTMLUnsafeOptions &options) noexcept
  {
    html::CEReactionsScope ceReactions;
    return html::HTMLInsertionAlgorithms::BeforeHTML(
      *this, html, options.Sanitizer ? &*options.Sanitizer : nullptr, html::FragmentParserMode::Unsafe);
  }

  ExceptionOr<void> Element::AfterHTMLUnsafe(DOMStringView html) noexcept
  {
    return AfterHTMLUnsafe(html, {});
  }

  ExceptionOr<void> Element::AfterHTMLUnsafe(DOMStringView html,
                                             const html::SetHTMLUnsafeOptions &options) noexcept
  {
    html::CEReactionsScope ceReactions;
    return html::HTMLInsertionAlgorithms::AfterHTML(
      *this, html, options.Sanitizer ? &*options.Sanitizer : nullptr, html::FragmentParserMode::Unsafe);
  }

  ExceptionOr<void> Element::ReplaceWithHTMLUnsafe(DOMStringView html) noexcept
  {
    return ReplaceWithHTMLUnsafe(html, {});
  }

  ExceptionOr<void> Element::ReplaceWithHTMLUnsafe(DOMStringView html,
                                                   const html::SetHTMLUnsafeOptions &options) noexcept
  {
    html::CEReactionsScope ceReactions;
    return html::HTMLInsertionAlgorithms::ReplaceWithHTML(
      *this, html, options.Sanitizer ? &*options.Sanitizer : nullptr, html::FragmentParserMode::Unsafe);
  }

  DOMString Element::GetHTML() const noexcept
  {
    return GetHTML(dom::GetHTMLOptions {});
  }

  DOMString Element::GetHTML(dom::GetHTMLOptions options) const noexcept
  {
    return SerializeFragment(*this, SerializedNodes::SubtreesOfChildren, nullptr,
                             ResolveUrls::NoExcludingUrlsForPrivacy, SerializationSyntax::HTML,
                             options.SerializableShadowRoots ? SerializeShadowRoots::Serializable
                                                             : SerializeShadowRoots::Explicit,
                             krys::move(options.ShadowRoots));
  }

  ExceptionOr<DOMString> Element::InnerHTML() const noexcept
  {
    // The innerHTML getter steps are to return the result of running fragment serializing algorithm steps
    // with this and true.
    return SerializeFragmentRequiringWellFormed(*this, SerializedNodes::SubtreesOfChildren);
  }

  ExceptionOr<void> Element::InnerHTML(DOMStringView markup) noexcept
  {
    html::CEReactionsScope ceReactions;
    return ReplaceChildrenWithMarkup(markup);
  }

  ExceptionOr<DOMString> Element::OuterHTML() const noexcept
  {
    // 1. Let element be a fictional node whose only child is this.
    // 2. Return the result of running fragment serializing algorithm steps with element and true.
    return SerializeFragmentRequiringWellFormed(*this, SerializedNodes::SubtreeIncludingNode);
  }

  ExceptionOr<void> Element::OuterHTML(DOMStringView markup) noexcept
  {
    html::CEReactionsScope ceReactions;

    // An element without a parent element: nothing happens without a parent, a Document parent throws, and
    // so does a DocumentFragment one, as in Chrome (see DIFFERENCES.md).
    // https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#dom-element-outerhtml
    RefPtr<Element> parent = ShareRefPtr(ParentElement());
    if (!parent) KRYS_UNLIKELY
    {
      if (!ParentNode())
      {
        return {};
      }
      return Exception {ExceptionCode::NoModificationAllowedError};
    }

    RefPtr<Node> previous = ShareRefPtr(PreviousSibling());
    RefPtr<Node> next = ShareRefPtr(NextSibling());

    auto fragment = CreateFragmentForInnerOuterHTML(*parent, markup);
    if (fragment.HasException())
    {
      return fragment.ReleaseException();
    }

    Ref<DocumentFragment> newChildren = fragment.ReleaseValue();
    if (auto result = parent->ReplaceChild(*newChildren, *this); result.HasException())
    {
      return result.ReleaseException();
    }

    // Not in the specification: the text nodes that end up next to each other are merged, as in Chrome.
    if (auto *nodeText = next ? DynamicDowncast<Text>(next->PreviousSibling()) : nullptr)
    {
      if (auto result = MergeWithNextTextNode(*nodeText); result.HasException())
      {
        return result.ReleaseException();
      }
    }
    if (auto *previousText = DynamicDowncast<Text>(previous.get()))
    {
      if (auto result = MergeWithNextTextNode(*previousText); result.HasException())
      {
        return result.ReleaseException();
      }
    }
    return {};
  }

  ExceptionOr<void> Element::InsertAdjacentHTML(dom::InsertAdjacentWhere where, DOMStringView markup) noexcept
  {
    html::CEReactionsScope ceReactions;

    // Steps 1 and 2.
    auto contextElement = ContextElementForInsertAdjacentHTML(*this, where);
    if (contextElement.HasException())
    {
      return contextElement.ReleaseException();
    }

    // Step 3.
    auto fragment = CreateFragmentForInnerOuterHTML(*contextElement.Value(), markup);
    if (fragment.HasException())
    {
      return fragment.ReleaseException();
    }

    // Step 4.
    Ref<DocumentFragment> newChildren = fragment.ReleaseValue();
    if (auto result = dom::ElementAlgorithms::InsertAdjacent(*this, where, *newChildren);
        result.HasException())
    {
      return result.ReleaseException();
    }
    return {};
  }

#pragma endregion

#pragma region Element (Pointer Events)

  ExceptionOr<void> Element::SetPointerCapture(int32 pointerId) noexcept
  {
    if (auto *frame = NodeDocument().Frame())
    {
      return frame->GetPointerCaptureController().SetPointerCapture(this, static_cast<PointerId>(pointerId));
    }
    return {};
  }

  ExceptionOr<void> Element::ReleasePointerCapture(int32 pointerId) noexcept
  {
    if (auto *frame = NodeDocument().Frame())
    {
      return frame->GetPointerCaptureController().ReleasePointerCapture(this,
                                                                        static_cast<PointerId>(pointerId));
    }
    return {};
  }

  bool Element::HasPointerCapture(int32 pointerId) noexcept
  {
    if (auto *frame = NodeDocument().Frame())
    {
      return frame->GetPointerCaptureController().HasPointerCapture(this, static_cast<PointerId>(pointerId));
    }
    return false;
  }

#pragma endregion

#pragma region Element (Pointer Lock)

  void Element::RequestPointerLock(dom::PointerLockOptions options,
                                   Func<void(ExceptionOr<void> &&)> &&promise) noexcept
  {
    if (auto *frame = NodeDocument().Frame())
    {
      frame->GetPointerLockController().RequestPointerLock(this, Maybe<dom::PointerLockOptions>(options),
                                                           krys::move(promise));
    }
  }

#pragma endregion

#pragma region Node

  DOMString Element::NodeName() const noexcept
  {
    return TagName();
  }

  Maybe<DOMString> Element::TextContent() const noexcept
  {
    return dom::TextAlgorithms::DescendantTextContent(*this);
  }

  ExceptionOr<void> Element::TextContent(DOMString value) noexcept
  {
    html::CEReactionsScope ceReactions;
    return dom::NodeAlgorithms::StringReplaceAll(krys::move(value), *this);
  }

#pragma endregion

#pragma region ChildNode

  ExceptionOr<void> Element::Before(const List<dom::NodeOrString> &nodes) noexcept
  {
    html::CEReactionsScope ceReactions;
    return dom::mixins::ChildNode::Before(*this, nodes);
  }

  ExceptionOr<void> Element::After(const List<dom::NodeOrString> &nodes) noexcept
  {
    html::CEReactionsScope ceReactions;
    return dom::mixins::ChildNode::After(*this, nodes);
  }

  ExceptionOr<void> Element::ReplaceWith(const List<dom::NodeOrString> &nodes) noexcept
  {
    html::CEReactionsScope ceReactions;
    return dom::mixins::ChildNode::ReplaceWith(*this, nodes);
  }

  ExceptionOr<void> Element::Remove() noexcept
  {
    html::CEReactionsScope ceReactions;
    return dom::mixins::ChildNode::Remove(*this);
  }

#pragma endregion

#pragma region NonDocumentTypeChildNode

  RefPtr<const Element> Element::PreviousElementSibling() const noexcept
  {
    return dom::mixins::NonDocumentTypeChildNode::PreviousElementSibling(*this);
  }

  RefPtr<Element> Element::PreviousElementSibling() noexcept
  {
    return dom::mixins::NonDocumentTypeChildNode::PreviousElementSibling(*this);
  }

  RefPtr<const Element> Element::NextElementSibling() const noexcept
  {
    return dom::mixins::NonDocumentTypeChildNode::NextElementSibling(*this);
  }

  RefPtr<Element> Element::NextElementSibling() noexcept
  {
    return dom::mixins::NonDocumentTypeChildNode::NextElementSibling(*this);
  }

#pragma endregion

#pragma region ParentNode

  Ref<HTMLCollection> Element::Children() noexcept
  {
    return EnsureElementRareData().Children(*this);
  }

  RefPtr<const Element> Element::FirstElementChild() const noexcept
  {
    return dom::mixins::ParentNode::FirstElementChild(*this);
  }

  RefPtr<Element> Element::FirstElementChild() noexcept
  {
    return dom::mixins::ParentNode::FirstElementChild(*this);
  }

  RefPtr<const Element> Element::LastElementChild() const noexcept
  {
    return dom::mixins::ParentNode::LastElementChild(*this);
  }

  RefPtr<Element> Element::LastElementChild() noexcept
  {
    return dom::mixins::ParentNode::LastElementChild(*this);
  }

  size_t Element::ChildElementCount() const noexcept
  {
    return dom::mixins::ParentNode::ChildElementCount(*this);
  }

  ExceptionOr<void> Element::Prepend(const List<dom::NodeOrString> &nodes) noexcept
  {
    html::CEReactionsScope ceReactions;
    return dom::mixins::ParentNode::Prepend(*this, nodes);
  }

  ExceptionOr<void> Element::Append(const List<dom::NodeOrString> &nodes) noexcept
  {
    html::CEReactionsScope ceReactions;
    return dom::mixins::ParentNode::Append(*this, nodes);
  }

  ExceptionOr<void> Element::ReplaceChildren(const List<dom::NodeOrString> &nodes) noexcept
  {
    html::CEReactionsScope ceReactions;
    return dom::mixins::ParentNode::ReplaceChildren(*this, nodes);
  }

  ExceptionOr<void> Element::MoveBefore(Node &node, Node *refChild) noexcept
  {
    html::CEReactionsScope ceReactions;
    return dom::mixins::ParentNode::MoveBefore(*this, node, refChild);
  }

  ExceptionOr<RefPtr<Element>> Element::QuerySelector(DOMStringView selectors) noexcept
  {
    return dom::mixins::ParentNode::QuerySelector(*this, selectors);
  }

  ExceptionOr<Ref<NodeList>> Element::QuerySelectorAll(DOMStringView selectors) noexcept
  {
    return dom::mixins::ParentNode::QuerySelectorAll(*this, selectors);
  }

#pragma endregion

#pragma region Slottable

  RefPtr<HTMLSlotElement> Element::AssignedSlot() noexcept
  {
    return dom::mixins::Slottable::AssignedSlot(*this);
  }

#pragma endregion

#pragma region Lazy Attributes

  void Element::SetSynchronizedLazyAttribute(DOMStringAtom localName, DOMString value) noexcept
  {
    auto it =
      std::ranges::find_if(_attributes, [&](const Ref<Attr> &attr)
                           { return attr->LocalName() == localName && attr->NamespaceURI() == null; });
    if (it != _attributes.end())
    {
      (*it)->_value = krys::move(value);
      return;
    }

    auto attr =
      AdoptRef<Attr>(*new Attr(NodeDocument(), QualifiedName {null, null, localName}, krys::move(value)));
    attr->_ownerElement = CreateWeakPtr(this);
    _attributes.push_back(krys::move(attr));
  }

  void Element::SetSynchronizedLazyAttribute(const QualifiedName &name, DOMString value) noexcept
  {
    auto it = std::ranges::find_if(
      _attributes, [&](const Ref<Attr> &attr)
      { return attr->LocalName() == name.LocalName() && attr->NamespaceURI() == name.NamespaceURI(); });
    if (it != _attributes.end())
    {
      (*it)->_value = krys::move(value);
      return;
    }

    auto attr = AdoptRef<Attr>(*new Attr(NodeDocument(), name, krys::move(value)));
    attr->_ownerElement = CreateWeakPtr(this);
    _attributes.push_back(krys::move(attr));
  }

  void Element::SynchronizeStyleAttribute() const noexcept
  {
    const_cast<StyledElement &>(Downcast<StyledElement>(*this)).SynchronizeStyleAttributeInternal();
  }

  void Element::SynchronizeAnimatedSVGAttributes() const noexcept
  {
    auto &svgElement = const_cast<SVGElement &>(Downcast<SVGElement>(*this));
    svgElement._nodeFlags.Remove(dom::NodeFlag::AnimatedSVGAttributesAreDirty);
    svgElement.SynchronizeAllSVGAttributes();
  }

#pragma endregion

#pragma region Extension Hooks

  void Element::OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                                   Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept
  {
    if (namespaceURI != null)
    {
      return;
    }

    if (GetElementRareData() && IsElementReflectionAttribute(localName))
    {
      GetElementRareData()->GetExplicitlySetAttrElementsMap().erase(localName);
    }
    if (GetElementRareData() && IsElementsArrayReflectionAttribute(localName))
    {
      GetElementRareData()->GetExplicitlySetAttrElementListsMap().erase(localName);
    }

    // https://dom.spec.whatwg.org/#concept-id
    // 1. If localName is id, namespace is null, and value is null or the empty string, then unset element's
    //    ID.
    // 2. Otherwise, if localName is id, namespace is null, then set element's ID to value.
    // The ID is read from the attribute (an empty one is no ID), so only the tree scope and the ID's
    // observers need telling.
    if (localName == u8"id")
    {
      if (oldValue != value && (IsConnected() || IsInShadowTree()))
      {
        auto &scope = TreeScope();
        if (oldValue && !oldValue->empty())
        {
          scope.RemoveElementById(DOMStringAtom(*oldValue), *this);
        }
        if (value && !value->empty())
        {
          scope.AddElementById(DOMStringAtom(*value), *this);
        }

        if (oldValue && !oldValue->empty())
        {
          scope.NotifyIdTargetObservers(*this, DOMStringAtom(*oldValue));
        }
        if (value && !value->empty())
        {
          scope.NotifyIdTargetObservers(*this, DOMStringAtom(*value));
        }
      }
    }
    else if (localName == u8"class")
    {
      if (_domTokenList)
      {
        _domTokenList->AssociatedAttributeValueChanged();
      }
    }
    // https://dom.spec.whatwg.org/#slotable-name
    // 1. If localName is slot and namespace is null:
    else if (localName == u8"slot")
    {
      // 1a. If value is oldValue, then return.
      if (value == oldValue)
      {
        return;
      }

      // 1b. If value is null and oldValue is the empty string, then return.
      if (!value.has_value() && oldValue.has_value() && oldValue->empty())
      {
        return;
      }

      // 1c. If value is the empty string and oldValue is null, then return.
      if (value.has_value() && value->empty() && !oldValue.has_value())
      {
        return;
      }

      // 1d. If value is null or the empty string, then set element's name to the empty string.
      if (!value.has_value() || value->empty())
      {
        _slottableName = u8"";
      }
      // 1e. Otherwise, set element's name to value.
      else
      {
        _slottableName = DOMString(*value);
      }

      // 1f. If element is assigned, then run assign slottables for element's assigned slot.
      if (auto *assignedSlot = dom::SlotAlgorithms::GetAssignedSlot(*this))
      {
        dom::SlotAlgorithms::AssignSlottables(*assignedSlot);
      }

      // 1g. Run assign a slot for element.
      dom::SlotAlgorithms::AssignSlot(*this);
    }
    else if (localName == u8"dir")
    {
      html::TextDirectionStateChanged(*this, dom::ParseTextDirectionState(value));
    }
    else if (localName.View() == u8"part")
    {
      if (auto *partList = GetElementRareData() ? GetElementRareData()->PartList() : nullptr)
      {
        partList->AssociatedAttributeValueChanged();
      }

      if (NeedsStyleInvalidation() && IsInShadowTree())
      {
        InvalidateStyleInternal();
      }
    }
    else if (localName.View() == u8"exportparts")
    {
      if (auto *shadowRoot = dom::TreeTraversal::GetShadowRoot(*this))
      {
        shadowRoot->InvalidatePartMappings();
        style::Invalidator::InvalidateShadowParts(*shadowRoot);
      }
    }
  }

#pragma endregion

#pragma region Top Layer

  namespace
  {
    template <typename TFunction>
    void ForEachRenderLayer(Element &element, const TFunction &function) noexcept
    {
      auto *layerModelObject = DynamicDowncast<RenderLayerModelObject>(element.Renderer());
      if (!layerModelObject)
      {
        return;
      }

      if (layerModelObject->HasLayer())
      {
        function(*layerModelObject->Layer());
      }
    }
  }

  void Element::AddToTopLayer() noexcept
  {
    if (IsInTopLayer())
    {
      krys_assert(NodeDocument().IsPendingTopLayerRemoval(*this));
      RemoveFromTopLayer();
    }
    krys_assert(IsConnected());

    ForEachRenderLayer(*this, [](RenderLayer &layer) { layer.EstablishesTopLayerWillChange(); });

    Ref<Document> document = ShareRef(NodeDocument());
    document->AddTopLayerElement(*this);
    _eventTargetFlags.Add(dom::EventTargetFlag::IsInTopLayer);

    document->ScheduleContentRelevancyUpdate(css::ContentRelevancy::IsInTopLayer);

    // Being in the top layer changes what's inert, so the element and the root restyle.
    InvalidateStyleInternal();
    if (RefPtr<Element> documentElement = document->DocumentElement())
    {
      documentElement->InvalidateStyleInternal();
    }

    ForEachRenderLayer(*this, [](RenderLayer &layer) { layer.EstablishesTopLayerDidChange(); });
  }

  void Element::RequestRemoveFromTopLayer() noexcept
  {
    auto &document = NodeDocument();
    if (!IsInTopLayer() || document.IsPendingTopLayerRemoval(*this))
    {
      return;
    }

    document.AddPendingTopLayerRemoval(*this);
    // The overlay: auto rule no longer applies.
    InvalidateStyleInternal();
  }

  void Element::RemoveFromTopLayer() noexcept
  {
    krys_assert(IsInTopLayer());

    ForEachRenderLayer(*this, [](RenderLayer &layer) { layer.EstablishesTopLayerWillChange(); });

    // A ::backdrop's element is found through the top layer, so this runs before the element leaves it.
    if (auto *renderer = Renderer())
    {
      if (auto *backdrop = renderer->BackdropRenderer())
      {
        if (auto styleable = style::Styleable::FromRenderer(*backdrop))
        {
          styleable->CancelStyleOriginatedAnimations();
        }
      }
    }

    // The document isn't protected: it may already be being destroyed.
    NodeDocument().RemoveTopLayerElement(*this);
    _eventTargetFlags.Remove(dom::EventTargetFlag::IsInTopLayer);

    NodeDocument().ScheduleContentRelevancyUpdate(css::ContentRelevancy::IsInTopLayer);

    // Being in the top layer changes what's inert, so the element and the root restyle.
    InvalidateStyleInternal();
    if (RefPtr<Element> documentElement = NodeDocument().DocumentElement())
    {
      documentElement->InvalidateStyleInternal();
    }
    if (RefPtr<Element> modalElement = ShareRefPtr(NodeDocument().ActiveModalDialog()))
    {
      modalElement->InvalidateStyleInternal();
    }

    ForEachRenderLayer(*this, [](RenderLayer &layer) { layer.EstablishesTopLayerDidChange(); });
  }

#pragma endregion

#pragma region Popovers

  html::PopoverData *Element::GetPopoverData() const noexcept
  {
    return GetElementRareData() ? GetElementRareData()->GetPopoverData() : nullptr;
  }

  html::PopoverData &Element::EnsurePopoverData() noexcept
  {
    auto &rareData = EnsureElementRareData();
    if (!rareData.GetPopoverData())
    {
      rareData.SetPopoverData(CreateUnique<html::PopoverData>());
    }
    return *rareData.GetPopoverData();
  }

  void Element::ClearPopoverData() noexcept
  {
    if (GetElementRareData())
    {
      GetElementRareData()->SetPopoverData(nullptr);
    }
  }

  bool Element::IsPopoverShowing() const noexcept
  {
    auto *popoverData = GetPopoverData();
    return popoverData && popoverData->VisibilityState() == html::PopoverVisibilityState::Showing;
  }

  html::PopoverState Element::GetPopoverState() const noexcept
  {
    auto *popoverData = GetPopoverData();
    return popoverData ? popoverData->GetPopoverState() : html::PopoverState::None;
  }

  Element *Element::InvokedPopover() const noexcept
  {
    return GetElementRareData() ? GetElementRareData()->InvokedPopover() : nullptr;
  }

  void Element::SetInvokedPopover(Element *element) noexcept
  {
    EnsureElementRareData().SetInvokedPopover(element);

    // Invalidate so the popover invoker style bit gets updated.
    InvalidateStyleInternal();
  }

  // https://html.spec.whatwg.org/multipage/popover.html#topmost-popover-ancestor
  RefPtr<HTMLElement> Element::TopmostPopoverAncestor(HTMLElement *source, bool isPopover) noexcept
  {
    // 1. If isPopover is true: assert: newPopoverOrTopLayerElement is an HTML element; its popover attribute
    //    is not in the No Popover State or the Manual state; its popover visibility state is not in the
    //    popover showing state.
    // 2. Otherwise: Assert: source is null.
    (void)isPopover;
    krys_debug_assert(isPopover || !source);

    // 3. Let document be newPopoverOrTopLayerElement's node document.
    // 4. Let combinedPopovers be document's showing auto popover list extended with document's showing hint
    //    popover list.
    auto &document = NodeDocument();
    auto combinedPopovers = document.ShowingAutoPopoverList();
    auto hintPopovers = document.ShowingHintPopoverList();
    combinedPopovers.insert(combinedPopovers.end(), hintPopovers.begin(), hintPopovers.end());

    // The index of the last of them that's a flat tree ancestor of the walk's start, or -1.
    auto ancestorIndex = [&](Element *start) -> int64
    {
      int64 index = -1;
      for (auto *element = start; element; element = element->ParentElementInComposedTree())
      {
        auto it =
          std::ranges::find_if(combinedPopovers, [&](auto &popover) { return popover.get() == element; });
        if (it != combinedPopovers.end())
        {
          index = std::max(index, static_cast<int64>(it - combinedPopovers.begin()));
        }
      }
      return index;
    };

    // 5. Let popoverAncestorIndex be the index of the last item in combinedPopovers of which
    //    newPopoverOrTopLayerElement is a flat tree descendant, otherwise -1.
    auto index = ancestorIndex(ParentElementInComposedTree());
    // 6. Let sourceAncestorIndex be -1.
    // 7. If source is not null, then set sourceAncestorIndex to the index of the last item in
    //    combinedPopovers of which source is a flat tree descendant, otherwise -1.
    // (Or source itself, as other engines have it.)
    // 8. Let ancestorIndex be the maximum of popoverAncestorIndex and sourceAncestorIndex.
    if (source)
    {
      index = std::max(index, ancestorIndex(source));
    }
    // 9. If ancestorIndex is -1, then return null.
    if (index == -1)
    {
      return nullptr;
    }
    // 10. Return combinedPopovers[ancestorIndex].
    return combinedPopovers[static_cast<size_t>(index)];
  }

#pragma endregion

#pragma region Resize Observers

  void Element::DisconnectFromIntersectionObservers() noexcept
  {
    auto *observerData = IntersectionObserverDataIfExists();
    if (!observerData)
    {
      return;
    }

    for (const auto &registration : observerData->Registrations)
    {
      if (registration.Observer)
      {
        registration.Observer->TargetDestroyed(*this);
      }
    }
    observerData->Registrations.clear();

    for (const auto &observer : observerData->Observers)
    {
      if (observer)
      {
        observer->RootDestroyed();
      }
    }
    observerData->Observers.clear();
  }

  IntersectionObserverData &Element::EnsureIntersectionObserverData() noexcept
  {
    auto &rareData = EnsureElementRareData();
    if (!rareData.GetIntersectionObserverData())
    {
      rareData.SetIntersectionObserverData(CreateUnique<IntersectionObserverData>());
    }
    return *rareData.GetIntersectionObserverData();
  }

  IntersectionObserverData *Element::IntersectionObserverDataIfExists() const noexcept
  {
    return GetElementRareData() ? GetElementRareData()->GetIntersectionObserverData() : nullptr;
  }

  void Element::DisconnectFromResizeObservers() noexcept
  {
    auto *observerData = ResizeObserverDataIfExists();
    if (!observerData)
    {
      return;
    }

    for (const auto &observer : observerData->Observers)
    {
      if (observer)
      {
        observer->TargetDestroyed(*this);
      }
    }
    observerData->Observers.clear();
  }

  ResizeObserverData &Element::EnsureResizeObserverData() noexcept
  {
    auto &rareData = EnsureElementRareData();
    if (!rareData.GetResizeObserverData())
    {
      rareData.SetResizeObserverData(CreateUnique<ResizeObserverData>());
    }
    return *rareData.GetResizeObserverData();
  }

  ResizeObserverData *Element::ResizeObserverDataIfExists() const noexcept
  {
    return GetElementRareData() ? GetElementRareData()->GetResizeObserverData() : nullptr;
  }

  // https://drafts.csswg.org/css-contain/#relevant-to-the-user
  bool Element::IsRelevantToUser() const noexcept
  {
    if (auto relevancy = GetContentRelevancy())
    {
      return !relevancy->empty();
    }
    return false;
  }

  Maybe<EnumSet<css::ContentRelevancy>> Element::GetContentRelevancy() const noexcept
  {
    if (!GetElementRareData())
    {
      return null;
    }
    return GetElementRareData()->GetContentRelevancy();
  }

  void Element::SetContentRelevancy(EnumSet<css::ContentRelevancy> contentRelevancy) noexcept
  {
    EnsureElementRareData().SetContentRelevancy(contentRelevancy);
  }

  Maybe<LayoutUnit> Element::LastRememberedLogicalWidth() const noexcept
  {
    return GetElementRareData() ? GetElementRareData()->LastRememberedLogicalWidth() : null;
  }

  Maybe<LayoutUnit> Element::LastRememberedLogicalHeight() const noexcept
  {
    return GetElementRareData() ? GetElementRareData()->LastRememberedLogicalHeight() : null;
  }

  void Element::SetLastRememberedLogicalWidth(LayoutUnit width) noexcept
  {
    krys_debug_assert(width >= 0);
    EnsureElementRareData().SetLastRememberedLogicalWidth(width);
  }

  void Element::ClearLastRememberedLogicalWidth() noexcept
  {
    if (!GetElementRareData())
    {
      return;
    }
    GetElementRareData()->ClearLastRememberedLogicalWidth();
  }

  void Element::SetLastRememberedLogicalHeight(LayoutUnit height) noexcept
  {
    krys_debug_assert(height >= 0);
    EnsureElementRareData().SetLastRememberedLogicalHeight(height);
  }

  void Element::ClearLastRememberedLogicalHeight() noexcept
  {
    if (!GetElementRareData())
    {
      return;
    }
    GetElementRareData()->ClearLastRememberedLogicalHeight();
  }

#pragma endregion

#pragma region Element Reflection

  RefPtr<Element> Element::ResolvedElementForAttribute(DOMStringAtom attributeName) const noexcept
  {
    // 1. Let unresolvedTarget be the result of running this's get the unresolved attr target element.
    auto unresolvedTarget = ElementForAttributeInternal(krys::move(attributeName));
    // 2. If unresolvedTarget is null, then return null.
    if (!unresolvedTarget)
    {
      return nullptr;
    }
    // 3. Return the result of resolving the reference target on unresolvedTarget.
    return unresolvedTarget->ResolveReferenceTarget();
  }

  List<Ref<Element>> Element::ResolvedElementsForAttribute(DOMStringAtom attributeName) const noexcept
  {
    // 1. Let unresolvedTargets be the result of running this's get the unresolved attr target elements.
    auto unresolvedTargets = ElementsForAttributeInternal(krys::move(attributeName));
    // 2. If unresolvedTargets is null, then return « ».
    // 3. Let resolvedTargets be « ».
    List<Ref<Element>> resolvedTargets;
    if (!unresolvedTargets)
    {
      return resolvedTargets;
    }
    // 4. For each unresolvedTarget of unresolvedTargets:
    for (auto &unresolvedTarget : *unresolvedTargets)
    {
      // 4a. Let resolvedTarget be the result of resolving the reference target on unresolvedTarget.
      // 4b. If resolvedTarget is not null, then append resolvedTarget to resolvedTargets.
      if (auto resolvedTarget = unresolvedTarget->ResolveReferenceTarget())
      {
        resolvedTargets.push_back(ShareRef(*resolvedTarget));
      }
    }
    // 5. Return resolvedTargets.
    return resolvedTargets;
  }

  RefPtr<Element> Element::RetargetForBindings(Element *element) const noexcept
  {
    if (!element)
    {
      return nullptr;
    }
    // Retargeting an element gives it or a shadow host, an element either way.
    auto *retargeted = dom::EventTargetAlgorithms::Retarget(element, const_cast<Element &>(*this));
    return ShareRefPtr(&Downcast<Element>(Downcast<Node>(*retargeted)));
  }

  RefPtr<Element> Element::ResolveReferenceTarget() noexcept
  {
    // 1. If element is not a shadow host, or element's shadow root's reference target is null, then return
    //    element.
    auto *shadowRoot = dom::TreeTraversal::GetShadowRoot(*this);
    if (!shadowRoot || !shadowRoot->ReferenceTarget())
    {
      return ShareRefPtr(this);
    }

    // 2. Let candidate be the first element, in tree order, whose root is element's shadow root and whose ID
    //    is element's shadow root's reference target, if there is one; otherwise null.
    auto candidate = shadowRoot->GetElementById(*shadowRoot->ReferenceTarget());

    // 3. If candidate is null, then return null.
    if (!candidate)
    {
      return nullptr;
    }

    // 4. Return the result of resolving the reference target on candidate.
    return candidate->ResolveReferenceTarget();
  }

  RefPtr<Element> Element::ElementForAttributeInternal(const DOMStringAtom &attributeName) const noexcept
  {
    if (GetElementRareData())
    {
      auto &map = GetElementRareData()->GetExplicitlySetAttrElementsMap();
      if (auto it = map.find(attributeName); it != map.end())
      {
        auto *explicitlySetElement = it->second.get();
        if (explicitlySetElement
            && dom::TreeQueries::IsShadowIncludingDescendant(*this,
                                                             dom::TreeQueries::Root(*explicitlySetElement)))
        {
          return ShareRefPtr(explicitlySetElement);
        }
        return nullptr;
      }
    }

    // The element with the attribute's value as its ID in the element's tree, connected or not.
    auto id = GetAttribute(attributeName);
    if (!id || id->empty())
    {
      return nullptr;
    }
    return ElementInTreeWithId(*id);
  }

  RefPtr<Element> Element::ElementInTreeWithId(DOMStringView id) const noexcept
  {
    if (id.empty())
    {
      return nullptr;
    }
    auto &root = dom::TreeQueries::Root(const_cast<Element &>(*this));
    if (auto *document = DynamicDowncast<Document>(root))
    {
      return document->GetElementById(id);
    }
    if (auto *fragment = DynamicDowncast<DocumentFragment>(root))
    {
      return fragment->GetElementById(id);
    }
    for (auto *node = &root; node; node = dom::TreeQueries::Next(*node, &root))
    {
      if (auto *element = DynamicDowncast<Element>(*node);
          element && element->GetAttribute(u8"id") == DOMString(id))
      {
        return ShareRefPtr(element);
      }
    }
    return nullptr;
  }

  void Element::SetElementAttribute(const DOMStringAtom &attributeName, Element *element) noexcept
  {
    krys_debug_assert(IsElementReflectionAttribute(attributeName));

    if (!element)
    {
      if (GetElementRareData())
      {
        GetElementRareData()->GetExplicitlySetAttrElementsMap().erase(attributeName);
      }
      RemoveAttribute(attributeName);
      return;
    }

    (void)SetAttribute(attributeName, DOMString());
    EnsureElementRareData().GetExplicitlySetAttrElementsMap()[attributeName] = CreateWeakPtr(element);
    if (auto *cache = NodeDocument().ExistingAXObjectCache())
    {
      cache->UpdateRelations(*this, attributeName.View());
    }
  }

  bool Element::IsElementReflectionAttribute(const DOMStringAtom &attributeName) noexcept
  {
    return attributeName == u8"popovertarget" || attributeName == u8"commandfor"
           || attributeName == u8"aria-activedescendant";
  }

  Maybe<List<Ref<Element>>>
    Element::ElementsForAttributeInternal(const DOMStringAtom &attributeName) const noexcept
  {
    // 1. Let elements be an empty list.
    List<Ref<Element>> elements;

    // 2. Let element be the result of running reflectedTarget's get the element.
    // 3. If reflectedTarget's explicitly set attr-elements is not null:
    if (GetElementRareData())
    {
      auto &map = GetElementRareData()->GetExplicitlySetAttrElementListsMap();
      if (auto it = map.find(attributeName); it != map.end())
      {
        // 3a. For each attrElement in reflectedTarget's explicitly set attr-elements:
        for (auto &weakElement : it->second)
        {
          // 3a.i. If attrElement is not a descendant of any of element's shadow-including ancestors, then
          //       continue.
          auto *attrElement = weakElement.get();
          if (!attrElement
              || !dom::TreeQueries::IsShadowIncludingDescendant(*this, dom::TreeQueries::Root(*attrElement)))
          {
            continue;
          }

          // 3a.ii. Append attrElement to elements.
          elements.push_back(ShareRef(*attrElement));
        }
        // 5. Return elements.
        return elements;
      }
    }

    // 4. Otherwise:
    // 4a. Let contentAttributeValue be the result of running reflectedTarget's get the content attribute.
    auto contentAttributeValue = GetAttribute(attributeName);

    // 4b. If contentAttributeValue is null, then return null.
    if (!contentAttributeValue)
    {
      return null;
    }

    // 4c. Let tokens be contentAttributeValue, split on ASCII whitespace.
    // 4d. For each id of tokens:
    for (const auto &id : infra::StringAlgorithms::SplitOnWhitespace(*contentAttributeValue))
    {
      // 4d.i. Let candidate be the first element, in tree order, that meets the following criteria:
      //       candidate's root is the same as element's root; candidate's ID is id; and candidate implements
      //       T. If no such element exists, then continue.
      // 4d.ii. Append candidate to elements.
      if (auto candidate = ElementInTreeWithId(id))
      {
        elements.push_back(ShareRef(*candidate));
      }
    }

    // 5. Return elements.
    return elements;
  }

  void Element::SetElementsArrayAttribute(const DOMStringAtom &attributeName,
                                          const Maybe<List<Ref<Element>>> &elements) noexcept
  {
    krys_debug_assert(IsElementsArrayReflectionAttribute(attributeName));

    // 1. If the given value is null:
    if (!elements)
    {
      // 1a. Set this's explicitly set attr-elements to null.
      if (GetElementRareData())
      {
        GetElementRareData()->GetExplicitlySetAttrElementListsMap().erase(attributeName);
      }
      // 1b. Run this's delete the content attribute.
      RemoveAttribute(attributeName);
      // 1c. Return.
      return;
    }

    // 2. Run this's set the content attribute with the empty string.
    (void)SetAttribute(attributeName, DOMString());

    // 3. Let elements be an empty list.
    // 4. For each element in the given value:
    // 4a. Append a weak reference to element to elements.
    List<WeakPtr<Element>> weakElements;
    weakElements.reserve(elements->size());
    for (auto &element : *elements)
    {
      weakElements.push_back(CreateWeakPtr(element.get()));
    }

    // 5. Set this's explicitly set attr-elements to elements.
    EnsureElementRareData().GetExplicitlySetAttrElementListsMap()[attributeName] = krys::move(weakElements);
    if (auto *cache = NodeDocument().ExistingAXObjectCache())
    {
      cache->UpdateRelations(*this, attributeName.View());
    }
  }

  bool Element::IsElementsArrayReflectionAttribute(const DOMStringAtom &attributeName) noexcept
  {
    return attributeName == u8"aria-controls" || attributeName == u8"aria-describedby"
           || attributeName == u8"aria-details" || attributeName == u8"aria-errormessage"
           || attributeName == u8"aria-flowto" || attributeName == u8"aria-labelledby"
           || attributeName == u8"aria-owns";
  }

#pragma endregion

  bool dom::IsInTopLayerOrBackdrop(const RenderStyle &style, const Element *element) noexcept
  {
    // Elements in the top layer are rendered there while their overlay is auto.
    // https://drafts.csswg.org/css-position-4/#rendered-in-the-top-layer
    return (element && element->IsInTopLayer() && style.Overlay() == Overlay::Auto)
           || style.GetPseudoElementType() == PseudoElementType::Backdrop;
  }

  namespace
  {
    /// @brief The frame showing @p document, if it has a view.
    KRYS_NODISCARD Frame *DocumentFrameWithNonNullView(Document &document) noexcept
    {
      auto *frame = document.Frame();
      return frame && frame->View() ? frame : nullptr;
    }

    KRYS_NODISCARD int32 AdjustContentsScrollPositionOrSizeForZoom(int32 value, const Frame &frame) noexcept
    {
      double zoomFactor = frame.PageZoomFactor() * frame.FrameScaleFactor();
      if (zoomFactor == 1)
      {
        return value;
      }
      // Scaling up truncated the value, so one is added back before scaling it down.
      if (zoomFactor > 1)
      {
        value++;
      }
      return static_cast<int32>(value / zoomFactor);
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

    enum class LegacyCSSOMElementMetricsRoundingStrategy : bool
    {
      Round,
      Floor,
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
      // The offsets are in the element's own CSS pixels: its used zoom, page zoom included, is divided out.
      auto offsetLeft = LayoutUnit(RoundToInt(offset));
      double zoomFactor = renderer.Style().UsedZoom();
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
      return dom::TreeQueries::IsShadowIncludingInclusiveAncestor(ancestorRoot, node);
    }

    KRYS_NODISCARD const SVGElement *ElementWithSVGLayoutBox(const Element &element) noexcept
    {
      auto *svg = DynamicDowncast<SVGElement>(element);
      return svg && svg->HasAssociatedSVGLayoutBox() ? svg : nullptr;
    }

    KRYS_NODISCARD bool ShouldObtainBoundsFromBoxModel(const Element &element) noexcept
    {
      return element.Renderer() && element.Renderer()->IsRenderBoxModelObject();
    }

    /// @brief What a legend adds to a fieldset's border. It's left out of the client box, which script sees
    /// as inside the border that's specified, as in other browsers.
    KRYS_NODISCARD LayoutUnit IntrinsicBorderForFieldsetOf(const RenderBox &renderer) noexcept
    {
      auto *block = DynamicDowncast<RenderBlock>(renderer);
      return block ? block->IntrinsicBorderForFieldset() : LayoutUnit {};
    }

    KRYS_NODISCARD RenderBox *RenderBoxOf(const Element &element) noexcept
    {
      return DynamicDowncast<RenderBox>(element.Renderer());
    }
  }

  namespace
  {
    /// @brief The select element whose list box draws @p element, an option or optgroup.
    KRYS_NODISCARD HTMLSelectElement *OwningSelectElementOfListBoxItem(const Element &element) noexcept
    {
      if (auto *optionElement = DynamicDowncast<HTMLOptionElement>(element))
      {
        return optionElement->OwnerSelectElement();
      }
      if (auto *optGroupElement = DynamicDowncast<HTMLOptGroupElement>(element))
      {
        return optGroupElement->OwnerSelectElement();
      }
      return nullptr;
    }

    /// @brief The list box that draws @p element, an option or optgroup, and its bounds in the list box.
    KRYS_NODISCARD Maybe<Pair<RenderListBox *, LayoutRect>>
      ListBoxElementBoundingBox(const Element &element) noexcept
    {
      auto *selectElement = OwningSelectElementOfListBoxItem(element);
      if (!selectElement)
      {
        return null;
      }

      auto *listBoxRenderer = DynamicDowncast<RenderListBox>(selectElement->Renderer());
      if (!listBoxRenderer)
      {
        return null;
      }

      Maybe<LayoutRect> boundingBox;
      if (auto *optionElement = DynamicDowncast<HTMLOptionElement>(element))
      {
        boundingBox = listBoxRenderer->LocalBoundsOfOption(*optionElement);
      }
      else if (auto *optGroupElement = DynamicDowncast<HTMLOptGroupElement>(element))
      {
        boundingBox = listBoxRenderer->LocalBoundsOfOptGroup(*optGroupElement);
      }

      if (!boundingBox)
      {
        return null;
      }
      return Pair<RenderListBox *, LayoutRect> {listBoxRenderer, *boundingBox};
    }

    /// @brief Scrolls the list box that draws @p element, an option or optgroup, to it.
    /// @return The list box and the item's bounds in it.
    KRYS_NODISCARD Maybe<Pair<RenderListBox *, LayoutRect>>
      ListBoxElementScrollIntoView(const Element &element) noexcept
    {
      auto *selectElement = OwningSelectElementOfListBoxItem(element);
      if (!selectElement)
      {
        return null;
      }

      auto *listBoxRenderer = DynamicDowncast<RenderListBox>(selectElement->Renderer());
      if (!listBoxRenderer)
      {
        return null;
      }

      int32 itemIndex = 0;
      auto &listItems = selectElement->ListItems();
      for (size_t i = 0; i < listItems.size(); ++i)
      {
        if (listItems[i].get() == &element)
        {
          itemIndex = static_cast<int32>(i);
          (void)listBoxRenderer->ScrollToRevealElementAtListIndex(itemIndex);
          break;
        }
      }

      return Pair<RenderListBox *, LayoutRect> {listBoxRenderer,
                                                listBoxRenderer->ItemBoundingBoxRect({}, itemIndex)};
    }
  }

  Ref<DOMRectList> Element::GetClientRects() noexcept
  {
    NodeDocument().UpdateLayoutIgnorePendingStylesheets(
      {layout::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
       layout::LayoutOptions::TreatContentVisibilityAutoAsVisible},
      this);

    auto *renderer = Renderer();

    List<QuadF> quads;

    if (auto *svgElement = ElementWithSVGLayoutBox(*this))
    {
      if (auto localRect = svgElement->GetBoundingBox())
      {
        quads.push_back(renderer->LocalToAbsoluteQuad(*localRect));
      }
    }
    else if (auto listBoxItem = ListBoxElementBoundingBox(*this))
    {
      renderer = listBoxItem->first;
      quads.push_back(renderer->LocalToAbsoluteQuad(QuadF(RectF(listBoxItem->second))));
    }
    else if (ShouldObtainBoundsFromBoxModel(*this))
    {
      renderer->AbsoluteQuads(quads);
    }

    // A table's box covers its caption, so it has one rect, as in Chrome (see DIFFERENCES.md).

    if (quads.empty())
    {
      return DOMRectList::Create();
    }

    NodeDocument().ConvertAbsoluteToClientQuads(quads, renderer->Style());
    return DOMRectList::Create(quads);
  }

  Maybe<Pair<RenderElement *, RectF>> Element::BoundingAbsoluteRectWithoutLayout() const noexcept
  {
    auto *renderer = Renderer();
    List<QuadF> quads;
    if (auto *svgElement = ElementWithSVGLayoutBox(*this))
    {
      if (auto localRect = svgElement->GetBoundingBox())
      {
        quads.push_back(renderer->LocalToAbsoluteQuad(*localRect));
      }
    }
    else if (auto listBoxItem = ListBoxElementBoundingBox(*this))
    {
      renderer = listBoxItem->first;
      quads.push_back(renderer->LocalToAbsoluteQuad(QuadF(RectF(listBoxItem->second))));
    }
    else if (ShouldObtainBoundsFromBoxModel(*this))
    {
      renderer->AbsoluteQuads(quads);
    }

    if (quads.empty())
    {
      return null;
    }

    // The union of the rects that have an area, or else the first.
    // https://drafts.csswg.org/cssom-view/#dom-element-getboundingclientrect
    RectF bounds;
    bool hasArea = false;
    for (const auto &quad : quads)
    {
      auto box = quad.BoundingBox();
      if (box.IsEmpty())
      {
        continue;
      }
      if (hasArea)
      {
        bounds.Unite(box);
      }
      else
      {
        bounds = box;
        hasArea = true;
      }
    }
    return Pair<RenderElement *, RectF> {renderer, hasArea ? bounds : quads.front().BoundingBox()};
  }

  RectF Element::BoundingClientRect() noexcept
  {
    auto &document = NodeDocument();
    document.UpdateLayoutIgnorePendingStylesheets(
      {layout::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
       layout::LayoutOptions::TreatContentVisibilityAutoAsVisible},
      this);
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

  RectI Element::ScreenRect() const noexcept
  {
    auto *renderer = Renderer();
    auto *view = NodeDocument().View();
    if (!renderer || !view)
    {
      return {};
    }
    return view->ContentsToScreen(renderer->AbsoluteBoundingBoxRect());
  }

  Ref<DOMRect> Element::GetBoundingClientRect() noexcept
  {
    return DOMRect::Create(BoundingClientRect());
  }

  void Element::ScrollIntoView(const ScrollIntoViewOptions &options) noexcept
  {
    auto &document = NodeDocument();
    document.UpdateContentRelevancyForScrollIfNeeded(*this);

    document.UpdateLayoutIgnorePendingStylesheets();

    RenderElement *renderer = nullptr;
    bool insideFixed = false;
    LayoutRect absoluteBounds;
    if (auto listBoxScrollResult = ListBoxElementScrollIntoView(*this))
    {
      renderer = listBoxScrollResult->first;
      absoluteBounds = listBoxScrollResult->second;

      auto listBoxAbsoluteBounds = renderer->AbsoluteAnchorRectWithScrollMargin(&insideFixed).first;
      absoluteBounds.MoveBy(listBoxAbsoluteBounds.Location());
    }
    else
    {
      renderer = Renderer();
      if (!renderer)
      {
        return;
      }

      absoluteBounds = renderer->AbsoluteAnchorRectWithScrollMargin(&insideFixed).first;
    }

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
    FrameView::ScrollRectToVisible(absoluteBounds, *renderer, insideFixed, visibleOptions);
  }

  void Element::ScrollIntoViewIfNotVisible(bool centerIfNotVisible) noexcept
  {
    NodeDocument().UpdateLayoutIgnorePendingStylesheets();

    auto *renderer = Renderer();
    if (!renderer)
    {
      return;
    }

    bool insideFixed = false;
    auto absoluteBounds = renderer->AbsoluteAnchorRectWithScrollMargin(&insideFixed).first;
    auto align = centerIfNotVisible ? ScrollAlignment::AlignCenterIfNotVisible
                                    : ScrollAlignment::AlignToEdgeIfNotVisible;
    ScrollRectToVisibleOptions options;
    options.RevealMode = SelectionRevealMode::Reveal;
    options.AlignX = align;
    options.AlignY = align;
    FrameView::ScrollRectToVisible(absoluteBounds, *renderer, insideFixed, options);
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
    ScrollTo(scrollToOptions, ScrollSnapPointSelectionMethod::Directional);
  }

  void Element::ScrollBy(double x, double y) noexcept
  {
    ScrollBy(ScrollToOptions(x, y));
  }

  void Element::ScrollTo(const ScrollToOptions &options) noexcept
  {
    ScrollTo(options, ScrollSnapPointSelectionMethod::Closest);
  }

  void Element::ScrollTo(const ScrollToOptions &options,
                         ScrollSnapPointSelectionMethod snapPointSelectionMethod) noexcept
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

      // A smooth scroll has to be stopped.
      auto *box = RenderBoxOf(*this);
      auto *scrollableArea = box && box->Layer() ? box->Layer()->GetScrollableArea() : nullptr;
      if (scrollableArea && scrollableArea->GetScrollAnimationStatus() == ScrollAnimationStatus::Animating)
      {
        return false;
      }

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
      if (auto *window = document.Window())
      {
        window->ScrollTo(options, snapPointSelectionMethod);
      }
      return;
    }

    // If the element does not have any associated CSS layout box, the element has no associated scrolling
    // box, or the element has no overflow, terminate these steps.
    auto *renderer = RenderBoxOf(*this);
    if (!renderer)
    {
      return;
    }

    auto rendererCanScroll = [&]
    {
      if (auto *renderTextControlSingleLine = DynamicDowncast<RenderTextControlSingleLine>(*renderer))
      {
        return renderTextControlSingleLine->InnerTextElementHasNonVisibleOverflow();
      }
      // A list box scrolls its items itself, as Chrome's does, where WebKit's doesn't scroll for scrollTo().
      if (Is<RenderListBox>(*renderer))
      {
        return true;
      }
      return renderer->HasNonVisibleOverflow();
    };
    if (!rendererCanScroll())
    {
      return;
    }

    auto scrollToOptions = NormalizeNonFiniteCoordinatesOrFallBackTo(
      options, AdjustForAbsoluteZoom(renderer->ScrollLeft(), renderer->Style()),
      AdjustForAbsoluteZoom(renderer->ScrollTop(), renderer->Style()));
    PointI scrollPosition(ClampToInteger(*scrollToOptions.Left * renderer->Style().UsedZoom()),
                          ClampToInteger(*scrollToOptions.Top * renderer->Style().UsedZoom()));

    auto animated = UseSmoothScrolling(scrollToOptions.Behavior.value_or(ScrollBehavior::Auto), this)
                      ? ScrollIsAnimated::Yes
                      : ScrollIsAnimated::No;
    renderer->SetScrollPosition(scrollPosition, ScrollPositionChangeOptions::CreateProgrammaticWithOptions(
                                                  animated, snapPointSelectionMethod));
  }

  void Element::ScrollTo(double x, double y) noexcept
  {
    ScrollTo(ScrollToOptions(x, y));
  }

  int32 Element::OffsetLeft() noexcept
  {
    auto offset = 0;
    NodeDocument().UpdateLayoutIgnorePendingStylesheets(
      {layout::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
       layout::LayoutOptions::TreatContentVisibilityAutoAsVisible},
      this);
    if (auto *renderer = DynamicDowncast<RenderBoxModelObject>(Renderer()))
    {
      offset = AdjustOffsetForZoomAndSubpixelLayout(*renderer, renderer->OffsetLeft());
    }

    // Add the offsets of the offset parents in shadow trees the element isn't in.
    auto *parent = OffsetParentIgnoringShadowTrees();
    if (!parent || !parent->IsInShadowTree() || dom::TreeTraversal::SameRoot(*parent, *this))
    {
      return offset;
    }

    while (parent && !IsAncestorTreeScope(dom::TreeQueries::Root(*parent), *this))
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
    NodeDocument().UpdateLayoutIgnorePendingStylesheets(
      {layout::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
       layout::LayoutOptions::TreatContentVisibilityAutoAsVisible},
      this);
    if (auto *renderer = DynamicDowncast<RenderBoxModelObject>(Renderer()))
    {
      offset = AdjustOffsetForZoomAndSubpixelLayout(*renderer, renderer->OffsetTop());
    }

    // Add the offsets of the offset parents in shadow trees the element isn't in.
    auto *parent = OffsetParentIgnoringShadowTrees();
    if (!parent || !parent->IsInShadowTree() || dom::TreeTraversal::SameRoot(*parent, *this))
    {
      return offset;
    }

    while (parent && !IsAncestorTreeScope(dom::TreeQueries::Root(*parent), *this))
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
    NodeDocument().UpdateLayoutIgnorePendingStylesheets(
      {layout::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
       layout::LayoutOptions::TreatContentVisibilityAutoAsVisible},
      this);
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
    NodeDocument().UpdateLayoutIgnorePendingStylesheets(
      {layout::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
       layout::LayoutOptions::TreatContentVisibilityAutoAsVisible},
      this);
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
    while (element && !dom::TreeQueries::IsShadowIncludingDescendant(*this, dom::TreeQueries::Root(*element)))
    {
      element = element->OffsetParentIgnoringShadowTrees();
    }
    return element;
  }

  Element *Element::OffsetParentIgnoringShadowTrees() noexcept
  {
    NodeDocument().UpdateLayoutIgnorePendingStylesheets(
      {layout::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
       layout::LayoutOptions::TreatContentVisibilityAutoAsVisible},
      this);
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
    NodeDocument().UpdateLayoutIgnorePendingStylesheets(
      {layout::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
       layout::LayoutOptions::TreatContentVisibilityAutoAsVisible},
      this);

    if (auto *renderer = RenderBoxOf(*this))
    {
      // The left border, and a vertical scrollbar placed between it and the padding.
      auto clientLeft = LayoutUnit(RoundToInt(renderer->ClientLeft()));
      if (renderer->GetWritingMode().IsBlockLeftToRight())
      {
        clientLeft -= IntrinsicBorderForFieldsetOf(*renderer);
      }
      if (renderer->Style().ShouldPlaceVerticalScrollbarOnLeft())
      {
        clientLeft += renderer->VerticalScrollbarWidth();
      }
      return ConvertToNonSubpixelValue(
        AdjustLayoutUnitForAbsoluteZoom(clientLeft, renderer->Style()).ToDouble());
    }
    return 0;
  }

  int32 Element::ClientTop() noexcept
  {
    NodeDocument().UpdateLayoutIgnorePendingStylesheets(
      {layout::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
       layout::LayoutOptions::TreatContentVisibilityAutoAsVisible},
      this);

    if (auto *renderer = RenderBoxOf(*this))
    {
      auto clientTop = LayoutUnit(RoundToInt(renderer->ClientTop()));
      if (renderer->GetWritingMode().IsBlockTopToBottom())
      {
        clientTop -= IntrinsicBorderForFieldsetOf(*renderer);
      }
      return ConvertToNonSubpixelValue(
        AdjustLayoutUnitForAbsoluteZoom(clientTop, renderer->Style()).ToDouble());
    }
    return 0;
  }

  int32 Element::ClientWidth() noexcept
  {
    auto &document = NodeDocument();
    document.UpdateLayoutIgnorePendingStylesheets(
      {layout::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
       layout::LayoutOptions::TreatContentVisibilityAutoAsVisible},
      this);

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
      return AdjustForAbsoluteZoom(renderView.FrameView().LayoutWidth(), renderView.Style());
    }

    if (auto *renderer = RenderBoxOf(*this))
    {
      auto clientWidth = LayoutUnit(RoundToInt(renderer->ClientWidth()));
      if (!renderer->GetWritingMode().IsHorizontal())
      {
        clientWidth += IntrinsicBorderForFieldsetOf(*renderer);
      }
      // A table's client size is its wrapper box's, which holds the table's border and padding inside it:
      // the table renderer stands for both boxes, so they're added back.
      // https://www.w3.org/TR/CSS2/tables.html#model
      if (renderer->IsRenderTable())
      {
        if (renderer->Style().BoxSizing() == BoxSizing::ContentBox)
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
    document.UpdateLayoutIgnorePendingStylesheets(
      {layout::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
       layout::LayoutOptions::TreatContentVisibilityAutoAsVisible},
      this);

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
      return AdjustForAbsoluteZoom(renderView.FrameView().LayoutHeight(), renderView.Style());
    }

    if (auto *renderer = RenderBoxOf(*this))
    {
      auto clientHeight = LayoutUnit(RoundToInt(renderer->ClientHeight()));
      if (renderer->GetWritingMode().IsHorizontal())
      {
        clientHeight += IntrinsicBorderForFieldsetOf(*renderer);
      }
      // See ClientWidth() for why the table's padding and borders are added.
      if (renderer->IsRenderTable())
      {
        if (renderer->Style().BoxSizing() == BoxSizing::ContentBox)
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
    document.UpdateLayoutIgnorePendingStylesheets(
      {layout::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
       layout::LayoutOptions::TreatContentVisibilityAutoAsVisible},
      this);

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
    document.UpdateLayoutIgnorePendingStylesheets(
      {layout::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
       layout::LayoutOptions::TreatContentVisibilityAutoAsVisible},
      this);

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
    document.UpdateLayoutIgnorePendingStylesheets(
      {layout::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
       layout::LayoutOptions::TreatContentVisibilityAutoAsVisible},
      this);

    auto options = ScrollPositionChangeOptions::CreateProgrammaticWithOptions(
      UseSmoothScrolling(ScrollBehavior::Auto, this) ? ScrollIsAnimated::Yes : ScrollIsAnimated::No);

    if (document.ScrollingElement() == this)
    {
      if (auto *frame = DocumentFrameWithNonNullView(document))
      {
        PointI position(static_cast<int32>(newLeft * frame->PageZoomFactor() * frame->FrameScaleFactor()),
                        frame->View()->ScrollY());
        frame->View()->SetScrollPosition(position, options);
      }
      return;
    }

    if (auto *renderer = RenderBoxOf(*this))
    {
      int32 clampedLeft = ClampToInteger(newLeft * renderer->Style().UsedZoom());
      renderer->SetScrollLeft(clampedLeft, options);
      if (auto *scrollableArea = renderer->HasLayer() ? renderer->Layer()->GetScrollableArea() : nullptr)
      {
        scrollableArea->SetScrollShouldClearLatchedState(true);
      }
    }
  }

  void Element::SetScrollTop(int32 newTop) noexcept
  {
    auto &document = NodeDocument();
    document.UpdateLayoutIgnorePendingStylesheets(
      {layout::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
       layout::LayoutOptions::TreatContentVisibilityAutoAsVisible},
      this);

    auto options = ScrollPositionChangeOptions::CreateProgrammaticWithOptions(
      UseSmoothScrolling(ScrollBehavior::Auto, this) ? ScrollIsAnimated::Yes : ScrollIsAnimated::No);

    if (document.ScrollingElement() == this)
    {
      if (auto *frame = DocumentFrameWithNonNullView(document))
      {
        PointI position(frame->View()->ScrollX(),
                        static_cast<int32>(newTop * frame->PageZoomFactor() * frame->FrameScaleFactor()));
        frame->View()->SetScrollPosition(position, options);
      }
      return;
    }

    if (auto *renderer = RenderBoxOf(*this))
    {
      int32 clampedTop = ClampToInteger(newTop * renderer->Style().UsedZoom());
      renderer->SetScrollTop(clampedTop, options);
      if (auto *scrollableArea = renderer->HasLayer() ? renderer->Layer()->GetScrollableArea() : nullptr)
      {
        scrollableArea->SetScrollShouldClearLatchedState(true);
      }
    }
  }

  int32 Element::ScrollWidth() noexcept
  {
    auto &document = NodeDocument();
    document.UpdateLayoutIgnorePendingStylesheets(
      {layout::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
       layout::LayoutOptions::TreatContentVisibilityAutoAsVisible},
      this);

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
    document.UpdateLayoutIgnorePendingStylesheets(
      {layout::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
       layout::LayoutOptions::TreatContentVisibilityAutoAsVisible},
      this);

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
