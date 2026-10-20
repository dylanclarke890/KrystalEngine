#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/FocusEvent.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include <algorithm>
#include <limits>

namespace krys::boo::html
{
  namespace
  {
    KRYS_NODISCARD bool IsHTMLElement(const dom::Element &element, TagName tagName) noexcept
    {
      return Is<HTMLElement>(element) && element.TagQualifiedName().TagName() == tagName;
    }

    /// @brief Whether @p element is a summary for its parent `details`: its first `summary` child.
    /// @see https://html.spec.whatwg.org/#summary-for-its-parent-details
    KRYS_NODISCARD bool IsSummaryForItsParentDetails(const dom::Element &element) noexcept
    {
      if (!IsHTMLElement(element, TagName::summary))
      {
        return false;
      }
      auto *parent = element.ParentElement();
      if (!parent || !IsHTMLElement(*parent, TagName::details))
      {
        return false;
      }
      for (auto *child = dom::TreeQueries::FirstElementChild(*parent); child;
           child = dom::TreeQueries::NextElementSibling(*child))
      {
        if (IsHTMLElement(*child, TagName::summary))
        {
          return child == &element;
        }
      }
      return false;
    }

    /// @brief Whether @p element is an editing host: editable, and not inside an editable element.
    /// @see https://html.spec.whatwg.org/#editing-host
    KRYS_NODISCARD bool IsEditingHost(const dom::Element &element) noexcept
    {
      if (!Is<HTMLElement>(element) || IsHTMLElement(element, TagName::input)
          || IsHTMLElement(element, TagName::textarea))
      {
        return false;
      }
      if (!css::MatchesReadWritePseudoClass(element))
      {
        return false;
      }
      auto *parent = element.ParentElement();
      return !parent || !Is<HTMLElement>(*parent) || IsHTMLElement(*parent, TagName::input)
             || IsHTMLElement(*parent, TagName::textarea) || !css::MatchesReadWritePseudoClass(*parent);
    }

    /// @brief Whether the user agent determines @p element to be focusable when it has no `tabindex`
    /// (following the elements the specification suggests).
    KRYS_NODISCARD bool IsFocusableByDefault(const dom::Element &element) noexcept
    {
      // Links (the shapes of `area` elements are focusable areas of their images, which need image maps).
      // FIXME: The shapes of `area` elements, once there are image maps.
      if (css::IsLink(element) && !IsHTMLElement(element, TagName::area))
      {
        return true;
      }
      if (Is<HTMLElement>(element))
      {
        switch (element.TagQualifiedName().TagName())
        {
          case TagName::button:
          case TagName::select:
          case TagName::textarea:
          case TagName::dialog:
          // Navigable containers.
          case TagName::iframe:
          case TagName::frame:    return true;
          case TagName::input:
          {
            auto type = text::ToASCIILower(
              dom::DOMStringView(element.GetAttribute(u8"type").value_or(dom::DOMString())));
            return type != u8"hidden";
          }
          default: break;
        }
      }
      // FIXME: Elements with a draggable attribute, once there's drag and drop.
      return IsSummaryForItsParentDetails(element) || IsEditingHost(element);
    }

    /// @brief Whether @p element is being rendered or delegating its rendering to its children
    /// (`display: contents`), and visible.
    /// @see https://html.spec.whatwg.org/#being-rendered
    KRYS_NODISCARD bool IsBeingRendered(const dom::Element &element) noexcept
    {
      auto *style = element.RenderOrDisplayContentsStyle();
      if (!style)
      {
        // Compute the style of a yet unstyled subtree without resolving the document's style.
        style = const_cast<dom::Element &>(element).ResolveComputedStyle(
          dom::Element::ResolveComputedStyleMode::RenderedOnly);
      }
      // FIXME: Relevant canvas fallback content, once there's canvas.
      return style && style->Display() != DisplayType::None && style->GetVisibility() == Visibility::Visible;
    }

    KRYS_NODISCARD bool IsShadowHostThatDelegatesFocus(const dom::Element &element) noexcept
    {
      auto *shadowRoot = dom::TreeQueries::GetShadowRoot(element);
      return shadowRoot && shadowRoot->DelegatesFocus();
    }

    KRYS_NODISCARD bool IsFocusableAreaNode(const dom::Node &node) noexcept
    {
      if (Is<dom::Document>(node))
      {
        // The document's viewport.
        return true;
      }
      auto *element = DynamicDowncast<dom::Element>(node);
      return element && FocusAlgorithms::IsFocusableArea(*element);
    }

    /// @brief Whether @p area is click focusable: the user agent considers every focusable area click
    /// focusable.
    KRYS_NODISCARD bool IsClickFocusable(const dom::Node &area) noexcept
    {
      return IsFocusableAreaNode(area);
    }
  }

  Maybe<int32> FocusAlgorithms::TabIndexValue(const dom::Element &element) noexcept
  {
    auto attribute = element.GetAttribute(u8"tabindex");
    if (!attribute)
    {
      return null;
    }
    auto result = Numbers::ParseInteger(*attribute);
    if (!result.Success() || result.Value < std::numeric_limits<int32>::min()
        || result.Value > std::numeric_limits<int32>::max())
    {
      return null;
    }
    return static_cast<int32>(result.Value);
  }

  bool FocusAlgorithms::IsInert(const dom::Node &node) noexcept
  {
    auto *element = DynamicDowncast<dom::Element>(node);
    if (!element)
    {
      element = node.ParentElementInComposedTree();
    }
    for (; element; element = element->ParentElementInComposedTree())
    {
      if (Is<HTMLElement>(*element) && element->HasAttribute(u8"inert"))
      {
        return true;
      }
    }
    return false;
  }

  bool FocusAlgorithms::IsFocusableArea(const dom::Element &element) noexcept
  {
    if (!element.IsConnected())
    {
      return false;
    }
    if (!TabIndexValue(element) && !IsFocusableByDefault(element))
    {
      return false;
    }
    if (IsShadowHostThatDelegatesFocus(element))
    {
      return false;
    }
    if (css::MatchesDisabledPseudoClass(element))
    {
      return false;
    }
    if (IsInert(element))
    {
      return false;
    }
    return IsBeingRendered(element);
  }

  dom::Node *FocusAlgorithms::GetFocusableArea(dom::Element &focusTarget,
                                               dom::FocusTrigger focusTrigger) noexcept
  {
    // FIXME: The shapes of `area` elements and the scrollable regions of elements, once there are image maps
    // and scrolling.

    // If focus target is the document element of its Document: return the Document's viewport.
    if (&focusTarget == dom::TreeQueries::DocumentElement(focusTarget.NodeDocument()))
    {
      return &focusTarget.NodeDocument();
    }

    // FIXME: Navigables and navigable containers with a content navigable, once there are navigables.

    if (IsShadowHostThatDelegatesFocus(focusTarget))
    {
      auto *focusedArea = CurrentlyFocusedArea(focusTarget.NodeDocument());
      if (focusedArea && Is<dom::Element>(*focusedArea)
          && dom::TreeQueries::IsShadowIncludingInclusiveAncestor(focusTarget, *focusedArea))
      {
        return focusedArea;
      }
      return FocusDelegate(focusTarget, focusTrigger);
    }

    return nullptr;
  }

  dom::Node *FocusAlgorithms::FocusDelegate(dom::Element &focusTarget,
                                            dom::FocusTrigger focusTrigger) noexcept
  {
    auto *shadowRoot = dom::TreeQueries::GetShadowRoot(focusTarget);
    if (shadowRoot && !shadowRoot->DelegatesFocus())
    {
      return nullptr;
    }

    dom::ContainerNode *whereToLook = &focusTarget;
    if (shadowRoot)
    {
      whereToLook = shadowRoot;
    }

    if (auto *autofocusDelegate = AutofocusDelegate(*whereToLook, focusTrigger))
    {
      return autofocusDelegate;
    }

    for (auto *node = dom::TreeQueries::Next(*whereToLook, whereToLook); node;
         node = dom::TreeQueries::Next(*node, whereToLook))
    {
      auto *descendant = DynamicDowncast<dom::Element>(*node);
      if (!descendant)
      {
        continue;
      }
      // A dialog takes its first sequentially focusable descendant, anything else its first focusable area.
      dom::Node *focusableArea = nullptr;
      if (IsHTMLElement(focusTarget, TagName::dialog) ? IsSequentiallyFocusable(*descendant)
                                                      : IsFocusableArea(*descendant))
      {
        focusableArea = descendant;
      }
      else
      {
        focusableArea = GetFocusableArea(*descendant, focusTrigger);
      }
      if (focusableArea)
      {
        return focusableArea;
      }
    }

    return nullptr;
  }

  dom::Node *FocusAlgorithms::AutofocusDelegate(dom::ContainerNode &focusTarget,
                                                dom::FocusTrigger focusTrigger) noexcept
  {
    for (auto *node = dom::TreeQueries::Next(focusTarget, &focusTarget); node;
         node = dom::TreeQueries::Next(*node, &focusTarget))
    {
      auto *descendant = DynamicDowncast<dom::Element>(*node);
      if (!descendant || !descendant->HasAttribute(u8"autofocus"))
      {
        continue;
      }
      dom::Node *focusableArea =
        IsFocusableArea(*descendant) ? descendant : GetFocusableArea(*descendant, focusTrigger);
      if (!focusableArea)
      {
        continue;
      }
      if (!IsClickFocusable(*focusableArea) && focusTrigger == dom::FocusTrigger::Click)
      {
        continue;
      }
      return focusableArea;
    }
    return nullptr;
  }

  dom::Node *FocusAlgorithms::CurrentlyFocusedArea(dom::Document &document) noexcept
  {
    // FIXME: Nested navigables, once there are any; a document is its own top-level traversable's active
    // document.
    if (!document.IsFocusedAndActive())
    {
      return nullptr;
    }
    if (auto *focusedElement = document.FocusedElement())
    {
      return focusedElement;
    }
    return &document;
  }

  FocusAlgorithms::FocusChain FocusAlgorithms::GetFocusChain(dom::Node &subject) noexcept
  {
    // FIXME: Parent navigables, once there are any.
    FocusChain output;
    output.push_back(ShareRef(subject));
    if (!Is<dom::Document>(subject))
    {
      output.push_back(ShareRef<dom::Node>(subject.NodeDocument()));
    }
    return output;
  }

  void FocusAlgorithms::FocusingSteps(dom::Node &newFocusTarget, dom::Node *fallbackTarget,
                                      const dom::FocusOptions &options) noexcept
  {
    dom::Node *target = &newFocusTarget;
    if (!IsFocusableAreaNode(*target))
    {
      auto *element = DynamicDowncast<dom::Element>(*target);
      target = element ? GetFocusableArea(*element, options.Trigger) : nullptr;
    }

    if (!target)
    {
      if (!fallbackTarget)
      {
        return;
      }
      target = fallbackTarget;
    }

    // FIXME: Navigable containers with a content navigable, once there are navigables.

    if (Is<dom::Element>(*target) && IsInert(*target))
    {
      return;
    }

    auto &document = target->NodeDocument();
    if (target == CurrentlyFocusedArea(document))
    {
      return;
    }

    auto *currentlyFocusedArea = CurrentlyFocusedArea(document);
    auto oldChain = currentlyFocusedArea ? GetFocusChain(*currentlyFocusedArea) : FocusChain {};
    auto newChain = GetFocusChain(*target);
    FocusUpdateSteps(krys::move(oldChain), krys::move(newChain), target, options);
  }

  void FocusAlgorithms::UnfocusingSteps(dom::Element &oldFocusTarget) noexcept
  {
    auto &document = oldFocusTarget.NodeDocument();
    dom::Node *target = &oldFocusTarget;

    auto *currentlyFocusedArea = CurrentlyFocusedArea(document);
    if (IsShadowHostThatDelegatesFocus(oldFocusTarget) && currentlyFocusedArea
        && dom::TreeQueries::IsShadowIncludingInclusiveAncestor(
          *dom::TreeQueries::GetShadowRoot(oldFocusTarget), *currentlyFocusedArea))
    {
      target = currentlyFocusedArea;
    }

    if (IsInert(*target))
    {
      return;
    }

    // FIXME: The shapes of `area` elements and scrollable regions, once there are image maps and scrolling.

    auto oldChain = currentlyFocusedArea ? GetFocusChain(*currentlyFocusedArea) : FocusChain {};
    if (std::ranges::none_of(oldChain, [&](const Ref<dom::Node> &entry) { return entry.get() == target; }))
    {
      return;
    }

    if (!IsFocusableAreaNode(*target))
    {
      return;
    }

    auto &topDocument = Downcast<dom::Document>(*oldChain.back());
    if (topDocument.IsFocusedAndActive())
    {
      FocusingSteps(topDocument);
    }
    else
    {
      FocusUpdateSteps(krys::move(oldChain), {}, nullptr);
    }
  }

  void FocusAlgorithms::FocusUpdateSteps(FocusChain oldChain, FocusChain newChain, dom::Node *newFocusTarget,
                                         const dom::FocusOptions &options) noexcept
  {
    // If the last entry in old chain and the last entry in new chain are the same, pop the last entry from
    // old chain and the last entry from new chain and redo this step.
    while (!oldChain.empty() && !newChain.empty() && oldChain.back().get() == newChain.back().get())
    {
      oldChain.pop_back();
      newChain.pop_back();
    }

    // The old focused element loses its focus before the blur events (as other engines do), so they see the
    // viewport focused.
    for (auto &entry : oldChain)
    {
      if (auto *element = DynamicDowncast<dom::Element>(*entry);
          element && element->NodeDocument().FocusedElement() == element)
      {
        DesignateFocusedArea(element->NodeDocument(), nullptr, dom::FocusVisibility::Invisible);
      }
    }

    for (size_t i = 0; i < oldChain.size(); ++i)
    {
      auto &entry = oldChain[i];
      // FIXME: Fire change at text controls the user changed without committing, once there's editing.

      // FIXME: Fire blur at a document's Window, once there's a Window.
      auto *blurEventTarget = DynamicDowncast<dom::Element>(*entry);

      dom::EventTarget *relatedBlurTarget = nullptr;
      if (i == oldChain.size() - 1 && blurEventTarget && !newChain.empty()
          && Is<dom::Element>(*newChain.back()))
      {
        relatedBlurTarget = newChain.back().get();
      }

      if (blurEventTarget)
      {
        FireFocusEvent(u8"blur", *blurEventTarget, relatedBlurTarget);
      }
    }

    // FIXME: Apply platform-specific conventions for focusing the new focus target (selecting a text
    // control's contents), once there's editing.
    (void)newFocusTarget;

    for (size_t i = newChain.size(); i-- > 0;)
    {
      auto &entry = newChain[i];
      if (IsFocusableAreaNode(*entry))
      {
        auto &document = entry->NodeDocument();
        auto *element = DynamicDowncast<dom::Element>(*entry);
        auto *focusedArea = document.FocusedElement() ? static_cast<dom::Node *>(document.FocusedElement())
                                                      : static_cast<dom::Node *>(&document);
        if (focusedArea != entry.get())
        {
          // FIXME: Set the navigation API's focus changed during ongoing navigation, once there's a Window.
          DesignateFocusedArea(document, element, options.Visibility);
        }
      }

      // FIXME: Fire focus at a document's Window, once there's a Window.
      auto *focusEventTarget = DynamicDowncast<dom::Element>(*entry);

      dom::EventTarget *relatedFocusTarget = nullptr;
      if (i == newChain.size() - 1 && focusEventTarget && !oldChain.empty()
          && Is<dom::Element>(*oldChain.back()))
      {
        relatedFocusTarget = oldChain.back().get();
      }

      if (focusEventTarget)
      {
        FireFocusEvent(u8"focus", *focusEventTarget, relatedFocusTarget);
      }
    }

    if (options.Trigger != dom::FocusTrigger::Bindings && !newChain.empty())
    {
      newChain.back()->NodeDocument()._latestFocusTrigger = options.Trigger;
    }
  }

  void FocusAlgorithms::FireFocusEvent(dom::DOMStringAtom e, dom::Element &target,
                                       dom::EventTarget *relatedTarget) noexcept
  {
    auto initialize = [&](dom::FocusEvent &event)
    {
      event._relatedTarget = ShareRefPtr(relatedTarget);
      event._composed = true;
      // FIXME: The view, once there's a Window.
    };
    (void)dom::EventDispatcher::FireEvent<dom::FocusEvent>(e, target, initialize);

    // The bubbling `focusin` and `focusout` events follow `focus` and `blur`.
    // https://w3c.github.io/uievents/#events-focusevent-event-order
    auto bubblingType = e.View() == u8"focus" ? u8"focusin" : e.View() == u8"blur" ? u8"focusout" : nullptr;
    if (bubblingType)
    {
      (void)dom::EventDispatcher::FireEvent<dom::FocusEvent>(dom::DOMStringAtom(bubblingType), target,
                                                             [&](dom::FocusEvent &event)
                                                             {
                                                               initialize(event);
                                                               event._bubbles = true;
                                                             });
    }
  }

  bool FocusAlgorithms::HasFocusSteps(const dom::Document &target) noexcept
  {
    // FIXME: Nested navigables, once there are any; a document is its top-level traversable's active
    // document.
    return target.IsFocusedAndActive();
  }

  void FocusAlgorithms::FocusForClick(dom::Element &element) noexcept
  {
    element.NodeDocument().UpdateStyleIfNeeded();
    dom::FocusOptions options {.Trigger = dom::FocusTrigger::Click};
    for (auto *candidate = &element; candidate; candidate = candidate->ParentElementInComposedTree())
    {
      if (IsFocusableArea(*candidate) && IsClickFocusable(*candidate))
      {
        FocusingSteps(*candidate, nullptr, options);
        return;
      }
    }
    FocusingSteps(element.NodeDocument(), nullptr, options);
  }

  bool FocusAlgorithms::IsFocusNavigationScopeOwner(const dom::Node &node) noexcept
  {
    if (Is<dom::Document>(node))
    {
      return true;
    }
    auto *element = DynamicDowncast<dom::Element>(node);
    return element && (dom::TreeQueries::IsShadowHost(*element) || Is<HTMLSlotElement>(*element));
  }

  dom::Node *FocusAlgorithms::AssociatedFocusNavigationOwner(const dom::Element &element) noexcept
  {
    auto *parent = element.ParentNode();
    if (!parent)
    {
      return nullptr;
    }
    if (dom::TreeQueries::IsShadowHost(*parent))
    {
      return dom::SlotAlgorithms::GetAssignedSlot(element);
    }
    if (auto *shadowRoot = DynamicDowncast<dom::ShadowRoot>(*parent))
    {
      return shadowRoot->Host();
    }
    if (parent == dom::TreeQueries::DocumentElement(element.NodeDocument()))
    {
      return &element.NodeDocument();
    }
    // The document element itself (the specification doesn't say), or an element outside a document.
    if (Is<dom::Document>(*parent))
    {
      return parent;
    }
    auto *parentElement = DynamicDowncast<dom::Element>(*parent);
    return parentElement ? AssociatedFocusNavigationOwner(*parentElement) : nullptr;
  }

  bool FocusAlgorithms::IsSequentiallyFocusable(const dom::Element &element) noexcept
  {
    return IsFocusableArea(element) && TabIndexValue(element).value_or(0) >= 0;
  }

  List<dom::Element *> FocusAlgorithms::SequentialFocusNavigationOrder(dom::Document &document) noexcept
  {
    // Each focus navigation scope's elements, in shadow-including tree order.
    HashMap<const dom::Node *, List<dom::Element *>> scopes;
    for (auto *node = dom::TreeQueries::NextShadowIncluding(document); node;
         node = dom::TreeQueries::NextShadowIncluding(*node))
    {
      if (auto *element = DynamicDowncast<dom::Element>(*node))
      {
        if (auto *owner = AssociatedFocusNavigationOwner(*element))
        {
          scopes[owner].push_back(element);
        }
      }
    }

    List<dom::Element *> result;
    auto appendFlattenedScope = [&](this auto &self, const dom::Node &owner) -> void
    {
      auto scope = scopes.find(&owner);
      if (scope == scopes.end())
      {
        return;
      }

      // The tabindex-ordered focus navigation scope: the scope owners and focusable areas in the scope,
      // without those with negative tabindex values; positive values first, ascending, then zero and omitted
      // values (owners without one count as zero), each in shadow-including tree order.
      auto orderingKey = [](const dom::Element &element)
      {
        auto value = TabIndexValue(element).value_or(0);
        return value > 0 ? value : std::numeric_limits<int32>::max();
      };
      List<dom::Element *> ordered;
      for (auto *element : scope->second)
      {
        if ((IsFocusNavigationScopeOwner(*element) || IsFocusableArea(*element))
            && TabIndexValue(*element).value_or(0) >= 0)
        {
          ordered.push_back(element);
        }
      }
      std::ranges::stable_sort(ordered, [&](const dom::Element *a, const dom::Element *b)
                               { return orderingKey(*a) < orderingKey(*b); });

      // Flattened: a scope owner is followed by its own flattened scope, or replaced by it if it isn't a
      // focusable area.
      for (auto *item : ordered)
      {
        if (!IsFocusNavigationScopeOwner(*item) || IsFocusableArea(*item))
        {
          result.push_back(item);
        }
        if (IsFocusNavigationScopeOwner(*item))
        {
          self(*item);
        }
      }
    };
    appendFlattenedScope(document);
    return result;
  }

  bool FocusAlgorithms::SequentialNavigation(dom::Document &document, dom::FocusDirection direction) noexcept
  {
    krys_debug_assert(direction == dom::FocusDirection::Forward
                      || direction == dom::FocusDirection::Backward);
    bool forward = direction == dom::FocusDirection::Forward;

    // Focusability depends on style.
    document.UpdateStyleIfNeeded();
    auto order = SequentialFocusNavigationOrder(document);

    // The starting point is the focused area (the document standing for its viewport), or the sequential
    // focus navigation starting point if it's inside it.
    using Position = dom::SequentialFocusNavigationStartingPointPosition;
    dom::Node *startingPoint = document.FocusedElement();
    if (!startingPoint)
    {
      startingPoint = &document;
    }
    auto position = Position::At;
    if (auto *point = document._sequentialFocusNavigationStartingPoint.get();
        point && point->IsConnected()
        && dom::TreeQueries::IsShadowIncludingInclusiveAncestor(*startingPoint, *point))
    {
      startingPoint = point;
      position = document._sequentialFocusNavigationStartingPointPosition;
    }

    // The sequential navigation search algorithm.
    dom::Element *candidate = nullptr;
    auto inOrder = std::ranges::find(order, startingPoint);
    if (startingPoint == &document && position == Position::At)
    {
      // The navigable: its first or last area.
      if (!order.empty())
      {
        candidate = forward ? order.front() : order.back();
      }
    }
    else if (position == Position::At && inOrder != order.end())
    {
      // Sequential: the area after or before the starting point in the order.
      if (forward && std::next(inOrder) != order.end())
      {
        candidate = *std::next(inOrder);
      }
      else if (!forward && inOrder != order.begin())
      {
        candidate = *std::prev(inOrder);
      }
    }
    else
    {
      // DOM: the nearest suitable area after or before the starting point in shadow-including tree order.
      List<dom::Node *> nodes;
      for (dom::Node *node = &document; node; node = dom::TreeQueries::NextShadowIncluding(*node))
      {
        nodes.push_back(node);
      }
      auto start = std::ranges::find(nodes, startingPoint);
      if (start != nodes.end())
      {
        auto index = static_cast<size_t>(start - nodes.begin());
        // The first node after and the last node before the starting point.
        size_t after = index + 1;
        size_t before = index;
        if (position == Position::After)
        {
          while (after < nodes.size()
                 && dom::TreeQueries::IsShadowIncludingDescendant(*nodes[after], *startingPoint))
          {
            ++after;
          }
          before = after;
        }
        else if (position == Position::Inside)
        {
          before = index + 1;
        }
        auto isCandidate = [&](dom::Node *node)
        {
          return std::ranges::find(order, node) != order.end();
        };
        if (forward)
        {
          for (size_t i = after; i < nodes.size() && !candidate; ++i)
          {
            candidate = isCandidate(nodes[i]) ? static_cast<dom::Element *>(nodes[i]) : nullptr;
          }
        }
        else
        {
          for (size_t i = before; i-- > 0 && !candidate;)
          {
            candidate = isCandidate(nodes[i]) ? static_cast<dom::Element *>(nodes[i]) : nullptr;
          }
        }
      }
    }

    if (!candidate)
    {
      document._sequentialFocusNavigationStartingPoint = nullptr;
      // There are no user agent controls to move the focus to, so it wraps around to the first or last area,
      // as from the viewport.
      if (order.empty())
      {
        return false;
      }
      candidate = forward ? order.front() : order.back();
    }

    FocusingSteps(*candidate, nullptr, {.Direction = direction, .Visibility = dom::FocusVisibility::Visible});
    // What navigation focuses is scrolled into view, as `focus()` scrolls it.
    if (auto *view = document.View())
    {
      view->ScheduleScrollToFocusedElement(SelectionRevealMode::Reveal);
    }
    return true;
  }

  void FocusAlgorithms::SystemFocusChanged(dom::Document &document, bool focused) noexcept
  {
    if (document.IsFocusedAndActive() == focused)
    {
      return;
    }
    document.SetFocusedAndActive(focused);

    // The focused element gets `blur` before the window, and `focus` after it.
    auto fire = [](dom::DOMStringAtom type, dom::Element &target)
    {
      (void)dom::EventDispatcher::FireEvent<dom::FocusEvent>(type, target, [](dom::FocusEvent &event)
                                                             { event._composed = true; });
    };
    RefPtr<dom::Element> focusedElement = ShareRefPtr(document.FocusedElement());
    if (!focused && focusedElement)
    {
      fire(dom::DOMStringAtom(u8"blur"), *focusedElement);
    }
    // FIXME: Fire `focus` or `blur` at the document's Window, once there's a Window.
    if (focused && focusedElement)
    {
      fire(dom::DOMStringAtom(u8"focus"), *focusedElement);
    }
  }

  void FocusAlgorithms::DesignateFocusedArea(dom::Document &document, dom::Element *element,
                                             dom::FocusVisibility visibility) noexcept
  {
    // The sequential focus navigation starting point follows the focused element, so navigation continues
    // from it (as other engines do; the specification only uses the starting point inside the focused area).
    RefPtr<dom::Element> oldFocusedElement = std::exchange(document._focusedElement, ShareRefPtr(element));
    if (oldFocusedElement)
    {
      oldFocusedElement->SetFocus(false);
      document.SetSequentialFocusNavigationStartingPoint(nullptr);
    }
    if (element)
    {
      document.SetSequentialFocusNavigationStartingPoint(element);
      element->SetFocus(true, visibility);
    }
  }
}
