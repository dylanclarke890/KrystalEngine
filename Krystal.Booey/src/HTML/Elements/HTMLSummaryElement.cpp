#include "Krystal.Booey/HTML/Elements/HTMLSummaryElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Events/KeyboardEvent.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDetailsElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGAElement.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD bool IsInSummaryInteractiveContent(EventTarget *target) noexcept
    {
      // The target may have been retargeted out of the summary (the default summary's details), in which case
      // no summary is met on the way up.
      auto isInInteractiveContent = false;
      auto *node = DynamicDowncast<Node>(target);
      for (auto *element = node ? DynamicDowncast<Element>(*node) : nullptr; element;
           element = dom::TreeQueries::ParentOrShadowHostElement(*element))
      {
        if (Is<HTMLSummaryElement>(*element))
        {
          return isInInteractiveContent;
        }
        auto *svgElement = DynamicDowncast<SVGElement>(*element);
        if (auto *htmlElement = DynamicDowncast<HTMLElement>(*element);
            (htmlElement && htmlElement->IsInteractiveContent())
            || (svgElement && Is<SVGAElement>(*svgElement)))
        {
          isInInteractiveContent = true;
        }
      }
      return false;
    }
  }

  HTMLSummaryElement::HTMLSummaryElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Summary)
  {
    _eventTargetFlags.Add(dom::EventTargetFlag::HasActivationBehavior);
  }

  RefPtr<HTMLDetailsElement> HTMLSummaryElement::DetailsElement() const noexcept
  {
    if (auto *parent = DynamicDowncast<HTMLDetailsElement>(ParentElement()))
    {
      return ShareRefPtr(parent);
    }
    // The default summary is in the details' shadow tree.
    if (auto *details = DynamicDowncast<HTMLDetailsElement>(dom::TreeQueries::ShadowHost(*this)))
    {
      return ShareRefPtr(details);
    }
    return nullptr;
  }

  bool HTMLSummaryElement::IsActiveSummary() const noexcept
  {
    auto details = DetailsElement();
    if (!details)
    {
      return false;
    }
    return details->IsActiveSummary(*this);
  }

  // https://html.spec.whatwg.org/multipage/interactive-elements.html#the-summary-element:activation-behaviour
  void HTMLSummaryElement::ActivationBehavior(Event &event) noexcept
  {
    // 1. If this summary element is not the summary for its parent details, then return.
    // (Nor when the event targets interactive content within it, as in browsers.)
    if (!IsActiveSummary() || IsInSummaryInteractiveContent(event.Target().get()))
    {
      return;
    }

    // 2. Let parent be this summary element's parent.
    // 3. If the open attribute is present on parent, then remove it. Otherwise, set parent's open attribute
    //    to the empty string.
    if (auto details = DetailsElement())
    {
      details->ToggleOpen();
    }
    event.SetDefaultHandled();
  }

  void HTMLSummaryElement::DefaultEventHandler(Event &event) noexcept
  {
    if (IsActiveSummary())
    {
      if (auto *keyboardEvent = DynamicDowncast<KeyboardEvent>(event))
      {
        if (keyboardEvent->Type() == event_names::KeyDown && keyboardEvent->Key() == u8" ")
        {
          SetActive(true);
          // No SetDefaultHandled(): a keypress follows.
          return;
        }
        if (keyboardEvent->Type() == event_names::KeyPress)
        {
          switch (keyboardEvent->CharCode())
          {
            case '\r':
              (void)DispatchSimulatedClick(keyboardEvent);
              keyboardEvent->SetDefaultHandled();
              return;
            case ' ':
              // The space bar's keypress doesn't scroll the page: the keyup activates the summary.
              keyboardEvent->SetDefaultHandled();
              return;
            default: break;
          }
        }
        if (keyboardEvent->Type() == event_names::KeyUp && keyboardEvent->Key() == u8" ")
        {
          if (Active())
          {
            (void)DispatchSimulatedClick(keyboardEvent);
          }
          keyboardEvent->SetDefaultHandled();
          return;
        }
      }
    }

    HTMLElement::DefaultEventHandler(event);
  }

#pragma region Extensibility Hooks

  void HTMLSummaryElement::OnInsert() noexcept
  {
    HTMLElement::OnInsert();
    if (auto *details = DynamicDowncast<HTMLDetailsElement>(ParentElement()))
    {
      details->SummaryChildChanged();
    }
  }

  void HTMLSummaryElement::OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnRemove(isSubtreeRoot, oldAncestor);
    if (auto *details = DynamicDowncast<HTMLDetailsElement>(oldAncestor); details && isSubtreeRoot)
    {
      details->SummaryChildChanged();
    }
  }

  void HTMLSummaryElement::OnMove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnMove(isSubtreeRoot, oldAncestor);
    if (!isSubtreeRoot)
    {
      return;
    }
    auto *oldDetails = DynamicDowncast<HTMLDetailsElement>(oldAncestor);
    if (oldDetails)
    {
      oldDetails->SummaryChildChanged();
    }
    if (auto *details = DynamicDowncast<HTMLDetailsElement>(ParentElement());
        details && details != oldDetails)
    {
      details->SummaryChildChanged();
    }
  }

#pragma endregion
}
