#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/FocusEvent.hpp"
#include "Krystal.Booey/DOM/PopoverData.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/HTML/HTMLAreaElement.hpp"
#include "Krystal.Booey/HTML/HTMLDialogElement.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLFormControlElement.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/HTML/HTMLSummaryElement.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Page/EditorClient.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Navigation.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/SpatialNavigation.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
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
      // Links, including the shapes of `area` elements.
      if (css::IsLink(element))
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
      // FIXME: Elements with a draggable attribute, once a drag can start without a pointing device.
      if (auto *summary = DynamicDowncast<HTMLSummaryElement>(element); summary && summary->IsActiveSummary())
      {
        return true;
      }
      return IsEditingHost(element);
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

    /// @brief Whether @p node is in a user agent shadow tree whose root delegates focus: a part of a control
    /// that's focused instead of the control.
    KRYS_NODISCARD bool IsInUserAgentShadowTreeThatDelegatesFocus(const dom::Node &node) noexcept
    {
      if (!node.IsInUserAgentShadowTree())
      {
        return false;
      }
      auto *shadowRoot = dom::TreeQueries::ContainingShadowRoot(node);
      return shadowRoot && shadowRoot->DelegatesFocus();
    }

    /// @brief Whether @p area is click focusable: the user agent considers every focusable area click
    /// focusable.
    KRYS_NODISCARD bool IsClickFocusable(const dom::Node &area) noexcept
    {
      // The insides of a control (a text field's inner text, which is an editing host) aren't focused by a
      // click; the control is, unless the control delegates its focus to them.
      if (area.IsInUserAgentShadowTree() && !IsInUserAgentShadowTreeThatDelegatesFocus(area))
      {
        return false;
      }
      return IsFocusableAreaNode(area);
    }

    /// @brief Whether @p element is a part of a control in its user agent shadow tree, and the control (or a
    /// control it's part of) is disabled.
    KRYS_NODISCARD bool IsPartOfDisabledControl(const dom::Element &element) noexcept
    {
      for (auto *host = dom::TreeQueries::ShadowHost(element); host;
           host = host->IsInUserAgentShadowTree() ? dom::TreeQueries::ShadowHost(*host) : nullptr)
      {
        if (css::MatchesDisabledPseudoClass(*host))
        {
          return true;
        }
      }
      return false;
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
    // While the document is blocked by a modal dialog, every connected node but the dialog and its flat tree
    // descendants is inert.
    auto *modalDialog = node.IsConnected() ? node.NodeDocument().ActiveModalDialog() : nullptr;
    bool isInModalDialog = false;
    for (; element; element = element->ParentElementInComposedTree())
    {
      if (Is<HTMLElement>(*element) && element->HasAttribute(u8"inert"))
      {
        return true;
      }
      if (element == modalDialog)
      {
        isInModalDialog = true;
      }
    }
    return modalDialog && !isInModalDialog;
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
    if (element.IsInUserAgentShadowTree() && IsPartOfDisabledControl(element))
    {
      return false;
    }
    if (IsInert(element))
    {
      return false;
    }
    // The shapes of an `area` element are focusable areas while the image that uses its image map is being
    // rendered (the area itself never is).
    if (auto *area = DynamicDowncast<HTMLAreaElement>(element))
    {
      auto image = area->ImageElement();
      return image && IsBeingRendered(*image);
    }
    return IsBeingRendered(element);
  }

  dom::Node *FocusAlgorithms::GetFocusableArea(dom::Element &focusTarget,
                                               dom::FocusTrigger focusTrigger) noexcept
  {
    // An `area` element stands for its shapes, as the focusable areas of its image.
    // FIXME: The scrollable regions of elements, once there's scrolling.

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
      // Dispatch a change event for form control elements that have been edited.
      if (auto *formControlElement = DynamicDowncast<HTMLFormControlElement>(*entry);
          formControlElement && formControlElement->WasChangedSinceLastFormControlChangeEvent())
      {
        formControlElement->DispatchFormControlChangeEvent();
      }

      // FIXME: Fire blur at a document's Window, once there's a Window.
      RefPtr<dom::Element> blurEventTarget = ShareRefPtr(DynamicDowncast<dom::Element>(*entry));

      dom::EventTarget *relatedBlurTarget = nullptr;
      if (i == oldChain.size() - 1 && blurEventTarget && !newChain.empty()
          && Is<dom::Element>(*newChain.back()))
      {
        relatedBlurTarget = newChain.back().get();
      }

      if (blurEventTarget)
      {
        blurEventTarget->DispatchBlurEvent(relatedBlurTarget);
        blurEventTarget->DidBlur();
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
        focusEventTarget->DispatchFocusEvent(relatedFocusTarget, options);
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

  void FocusAlgorithms::AutofocusInsertionSteps(dom::Element &element) noexcept
  {
    if (!element.HasAttribute(u8"autofocus"))
    {
      return;
    }

    // NOTE: "inserted into a document" means into a document tree.
    if (!element.IsConnected() || element.IsInShadowTree())
    {
      return;
    }

    // 1. If the user has indicated (for example, by starting to type in a form control) that they do not wish
    // focus to be changed, then optionally return.

    // 2. Let target be the element's node document.
    auto &target = element.NodeDocument();

    // 3. If target is not fully active, then return.
    // NOTE: a document is fully active while it's shown in a frame.
    if (!target.Frame())
    {
      return;
    }

    // FIXME: 4. The sandboxed automatic features browsing context flag, and 5. the ancestor navigables'
    // origins, once there are sandboxes and frames.

    // 6. Let topDocument be target's node navigable's top-level traversable's active document.
    // FIXME: The main frame's document, once there are frames.
    auto &topDocument = target;

    // 7. If topDocument's autofocus processed flag is false, then remove the element from topDocument's
    // autofocus candidates, and append the element to topDocument's autofocus candidates.
    if (topDocument.AutofocusProcessed())
    {
      return;
    }
    auto &candidates = topDocument.AutofocusCandidates();
    std::erase_if(candidates,
                  [&](const WeakPtr<dom::Element> &candidate) { return candidate.get() == &element; });
    candidates.push_back(WeakPtr<dom::Element>::WithRef(element));
    topDocument.ScheduleRenderingUpdate({RenderingUpdateStep::FlushAutofocusCandidates});
  }

  void FocusAlgorithms::FlushAutofocusCandidates(dom::Document &topDocument) noexcept
  {
    // 1. If topDocument's autofocus processed flag is true, then return.
    if (topDocument.AutofocusProcessed())
    {
      return;
    }

    // 2. Let candidates be topDocument's autofocus candidates.
    auto &candidates = topDocument.AutofocusCandidates();

    // 3. If candidates is empty, then return.
    if (candidates.empty())
    {
      return;
    }

    // 4. If topDocument's focused area is not topDocument itself, or topDocument has non-null target element,
    // then: empty candidates, set topDocument's autofocus processed flag to true, and return.
    // FIXME: The target element, once there are fragments.
    if (topDocument.FocusedElement())
    {
      candidates.clear();
      topDocument.SetAutofocusProcessed();
      return;
    }

    // 5. While candidates is not empty:
    while (!candidates.empty())
    {
      // 1. Let element be candidates[0].
      RefPtr<dom::Element> element = ShareRefPtr(candidates.front().get());

      // 2. Let doc be element's node document.
      // 3. If doc is not fully active, then remove element from candidates, and continue.
      if (!element || !element->NodeDocument().Frame())
      {
        candidates.erase(candidates.begin());
        continue;
      }

      // 4. If doc's node navigable's top-level traversable is not the same as topDocument's node navigable,
      // then remove element from candidates, and continue.
      if (&element->NodeDocument() != &topDocument)
      {
        candidates.erase(candidates.begin());
        continue;
      }

      // 5. If doc's script-blocking style sheet set is not empty, then return.
      // FIXME: Once style sheets load.

      // 6. Remove element from candidates.
      candidates.erase(candidates.begin());

      // FIXME: 7. The inclusive ancestor documents' target elements, once there are fragments.

      // 8. Let target be element.
      // 9. If target is not a focusable area, then set target to the result of getting the focusable area for
      // target.
      // FIXME: Getting the focusable area of an element that isn't one, as upstream doesn't yet.
      // 10. If target is not null, then: empty candidates, set topDocument's autofocus processed flag to
      // true, and run the focusing steps for target.
      element->NodeDocument().UpdateStyleIfNeeded();
      if (IsFocusableArea(*element))
      {
        candidates.clear();
        topDocument.SetAutofocusProcessed();
        FocusingSteps(*element);
        return;
      }
    }
  }

  void FocusAlgorithms::FocusForClick(dom::Element &element) noexcept
  {
    dom::FocusOptions options {.Trigger = dom::FocusTrigger::Click};
    if (auto *candidate = ClickFocusableInclusiveAncestor(element))
    {
      FocusingSteps(*candidate, nullptr, options);
      return;
    }
    FocusingSteps(element.NodeDocument(), nullptr, options);
  }

  dom::Element *FocusAlgorithms::ClickFocusableInclusiveAncestor(dom::Element &element) noexcept
  {
    element.NodeDocument().UpdateStyleIfNeeded();
    for (auto *candidate = &element; candidate; candidate = candidate->ParentElementInComposedTree())
    {
      if (IsFocusableArea(*candidate) && IsClickFocusable(*candidate))
      {
        return candidate;
      }
      // A shadow host that delegates focus is focused through its focus delegate, by the focusing steps.
      if (IsShadowHostThatDelegatesFocus(*candidate) && !candidate->IsInUserAgentShadowTree()
          && GetFocusableArea(*candidate, dom::FocusTrigger::Click))
      {
        return candidate;
      }
    }
    return nullptr;
  }

  bool FocusAlgorithms::IsFocusNavigationScopeOwner(const dom::Node &node) noexcept
  {
    if (Is<dom::Document>(node))
    {
      return true;
    }
    auto *element = DynamicDowncast<dom::Element>(node);
    if (!element)
    {
      return false;
    }
    // Sequential navigation doesn't enter a control's user agent shadow tree, unless the control delegates
    // its focus to its parts.
    if (auto *shadowRoot = dom::TreeQueries::GetShadowRoot(*element);
        shadowRoot && (shadowRoot->Mode() != dom::ShadowRootMode::UserAgent || shadowRoot->DelegatesFocus()))
    {
      return true;
    }
    // The trigger of a showing popover, whose scope the popover is in.
    if (auto *popover = DynamicDowncast<HTMLElement>(element->InvokedPopover());
        popover && popover->IsPopoverShowing() && popover->GetPopoverData()->Invoker() == element)
    {
      return true;
    }
    return Is<HTMLSlotElement>(*element);
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
    // A showing popover is in its trigger's scope.
    if (auto *popover = DynamicDowncast<HTMLElement>(element); popover && popover->IsPopoverShowing())
    {
      if (auto *trigger = popover->GetPopoverData()->Invoker())
      {
        return trigger;
      }
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
    // What navigation focuses is shown focused (scrolled into view, or with a caret in it), as `focus()`
    // shows it.
    if (RefPtr<dom::Element> focused = ShareRefPtr(document.FocusedElement()))
    {
      focused->FindTargetAndUpdateFocusAppearance(dom::SelectionRestorationMode::SelectAll,
                                                  SelectionRevealMode::Reveal);
    }
    return true;
  }

  namespace
  {
    void UpdateFocusCandidateIfNeeded(dom::FocusDirection direction, const FocusCandidate &current,
                                      FocusCandidate &candidate, FocusCandidate &closest) noexcept
    {
      krys_debug_assert(candidate.VisibleNode->Renderer());

      // Ignore off screen child nodes of containers that do not scroll (overflow:hidden)
      if (candidate.IsOffscreen && !CanBeScrolledIntoView(direction, candidate))
      {
        return;
      }

      DistanceDataForNode(direction, current, candidate);
      if (candidate.Distance == MaxDistance())
      {
        return;
      }

      if (candidate.IsOffscreenAfterScrolling && candidate.Alignment < RectsAlignment::Full)
      {
        return;
      }

      if (closest.IsNull())
      {
        closest = candidate;
        return;
      }

      LayoutRect intersectionRect = Intersection(candidate.Rect, closest.Rect);
      if (!intersectionRect.IsEmpty() && !AreElementsOnSameLine(closest, candidate))
      {
        // If 2 nodes are intersecting, do hit test to find which node in on top.
        auto center = FlooredIntPoint(intersectionRect.Center()); // FIXME: Would roundedIntPoint be better?
        constexpr EnumSet<HitTestRequest::Type> hitType {
          HitTestRequest::Type::ReadOnly, HitTestRequest::Type::Active, HitTestRequest::Type::IgnoreClipping,
          HitTestRequest::Type::DisallowUserAgentShadowContent, HitTestRequest::Type::AllowChildFrameContent};
        // The main frame: there's one frame.
        auto *frame = candidate.VisibleNode->NodeDocument().Frame();
        if (!frame)
        {
          return;
        }
        HitTestResult result = frame->GetEventHandler().HitTestResultAtPoint(LayoutPoint(center), hitType);
        if (candidate.VisibleNode->Contains(result.InnerNode()))
        {
          closest = candidate;
          return;
        }
        if (closest.VisibleNode->Contains(result.InnerNode()))
        {
          return;
        }
      }

      if (candidate.Alignment == closest.Alignment)
      {
        if (candidate.Distance < closest.Distance)
        {
          closest = candidate;
        }
        return;
      }

      if (candidate.Alignment > closest.Alignment)
      {
        closest = candidate;
      }
    }

    void FindFocusCandidateInContainer(const dom::ContainerNode &container, const LayoutRect &startingRect,
                                       dom::FocusDirection direction, FocusCandidate &closest) noexcept
    {
      auto *focusedNode = container.NodeDocument().FocusedElement();

      FocusCandidate current;
      current.Rect = startingRect;
      current.FocusableNode = CreateWeakPtr(focusedNode);
      current.VisibleNode = CreateWeakPtr(focusedNode);

      auto next = [&](const dom::Element &element) -> dom::Element *
      {
        dom::Node *node = CanScrollInDirection(element, direction)
                            ? dom::TreeQueries::NextSkippingChildren(element, &container)
                            : dom::TreeQueries::Next(element, &container);
        while (node && !Is<dom::Element>(*node))
        {
          node = dom::TreeQueries::Next(*node, &container);
        }
        return static_cast<dom::Element *>(node);
      };
      dom::Node *first = dom::TreeQueries::Next(container, &container);
      while (first && !Is<dom::Element>(*first))
      {
        first = dom::TreeQueries::Next(*first, &container);
      }
      for (auto *element = static_cast<dom::Element *>(first); element; element = next(*element))
      {
        if (element == focusedNode)
        {
          continue;
        }

        if (!FocusAlgorithms::IsSequentiallyFocusable(*element) && !CanScrollInDirection(*element, direction))
        {
          continue;
        }

        FocusCandidate candidate = FocusCandidate(element, direction);
        if (candidate.IsNull())
        {
          continue;
        }

        if (!IsValidCandidate(direction, current, candidate))
        {
          continue;
        }

        candidate.EnclosingScrollableBox = CreateWeakPtr(const_cast<dom::ContainerNode *>(&container));
        UpdateFocusCandidateIfNeeded(direction, current, candidate, closest);
      }
    }

    bool AdvanceFocusDirectionallyInContainer(const dom::ContainerNode &container,
                                              const LayoutRect &startingRect,
                                              dom::FocusDirection direction) noexcept
    {
      LayoutRect newStartingRect = startingRect;

      if (startingRect.IsEmpty())
      {
        newStartingRect = VirtualRectForDirection(direction, NodeRectInAbsoluteCoordinates(container));
      }

      // Find the closest node within current container in the direction of the navigation.
      FocusCandidate focusCandidate;
      FindFocusCandidateInContainer(container, newStartingRect, direction, focusCandidate);

      if (focusCandidate.IsNull())
      {
        // Nothing to focus, scroll if possible.
        // NOTE: If no scrolling is performed (i.e. scrollInDirection returns false), the
        // spatial navigation algorithm will skip this container.
        return ScrollInDirection(container, direction);
      }

      if (RefPtr<dom::Element> visibleNode = ShareRefPtr(focusCandidate.VisibleNode.get());
          visibleNode && CanScrollInDirection(*visibleNode, direction))
      {
        if (focusCandidate.IsOffscreenAfterScrolling)
        {
          (void)ScrollInDirection(*visibleNode, direction);
          return true;
        }
        // Navigate into a new scrollable container.
        LayoutRect newRect;
        auto *focusedElement = container.NodeDocument().FocusedElement();
        if (focusedElement && !HasOffscreenRect(*focusedElement))
        {
          newRect = NodeRectInAbsoluteCoordinates(*focusedElement, true);
        }
        return AdvanceFocusDirectionallyInContainer(*visibleNode, newRect, direction);
      }
      if (focusCandidate.IsOffscreenAfterScrolling)
      {
        RefPtr<dom::ContainerNode> scrollableBox = ShareRefPtr(focusCandidate.EnclosingScrollableBox.get());
        (void)ScrollInDirection(*scrollableBox, direction);
        return true;
      }

      // We found a new focus node, navigate to it.
      RefPtr<dom::Element> element = ShareRefPtr(focusCandidate.FocusableNode.get());
      FocusAlgorithms::FocusingSteps(*element, nullptr,
                                     {.SelectionRestoration = dom::SelectionRestorationMode::SelectAll,
                                      .Direction = direction,
                                      .Visibility = dom::FocusVisibility::Visible});
      if (element->NodeDocument().FocusedElement() == element.get())
      {
        element->FindTargetAndUpdateFocusAppearance(dom::SelectionRestorationMode::SelectAll,
                                                    SelectionRevealMode::Reveal);
      }
      return true;
    }
  }

  bool FocusAlgorithms::DirectionalNavigation(dom::Document &document, dom::FocusDirection direction) noexcept
  {
    krys_debug_assert(direction == dom::FocusDirection::Up || direction == dom::FocusDirection::Down
                      || direction == dom::FocusDirection::Left || direction == dom::FocusDirection::Right);
    if (!document.View())
    {
      return false;
    }

    (void)document.UpdateLayoutIgnorePendingStylesheets();

    // Figure out the starting rect.
    RefPtr<dom::ContainerNode> container = ShareRefPtr(&document);
    LayoutRect startingRect;
    if (auto *focusedElement = document.FocusedElement())
    {
      if (!HasOffscreenRect(*focusedElement))
      {
        container =
          ShareRefPtr(ScrollableEnclosingBoxOrParentFrameForNodeInDirection(direction, *focusedElement));
        startingRect = NodeRectInAbsoluteCoordinates(*focusedElement, true /* ignore border */);
      }
      else if (auto *area = DynamicDowncast<HTMLAreaElement>(*focusedElement); area && area->ImageElement())
      {
        container = ShareRefPtr(
          ScrollableEnclosingBoxOrParentFrameForNodeInDirection(direction, *area->ImageElement()));
        startingRect = VirtualRectForAreaElementAndDirection(area, direction);
      }
    }

    krys_debug_assert(container);
    bool consumed = false;
    do
    {
      consumed = AdvanceFocusDirectionallyInContainer(*container, startingRect, direction);
      (void)document.UpdateLayoutIgnorePendingStylesheets();
      startingRect = NodeRectInAbsoluteCoordinates(*container, true /* ignore border */);
      container = ShareRefPtr(ScrollableEnclosingBoxOrParentFrameForNodeInDirection(direction, *container));
    } while (!consumed && container);

    return consumed;
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

      document.ScheduleContentRelevancyUpdate(dom::ContentRelevancy::Focused);
    }
    if (element)
    {
      document.SetSequentialFocusNavigationStartingPoint(element);
      element->SetFocus(true, visibility);

      document.ScheduleContentRelevancyUpdate(dom::ContentRelevancy::Focused);

      // The focus moved while a navigation is ongoing: its finish doesn't reset it.
      if (auto *window = document.Window())
      {
        if (auto *navigation = window->NavigationIfExists())
        {
          navigation->SetFocusChanged(FocusDidChange::Yes);
        }
      }
    }

    if (auto *editorClient = document.GetEditor().Client())
    {
      editorClient->SetInputMethodState(element);
    }
  }
}
