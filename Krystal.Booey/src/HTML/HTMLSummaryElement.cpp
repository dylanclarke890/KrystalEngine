#include "Krystal.Booey/HTML/HTMLSummaryElement.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/HTML/HTMLDetailsElement.hpp"
#include "Krystal.Booey/SVG/SVGAElement.hpp"

namespace krys::boo::html
{
  namespace
  {
    KRYS_NODISCARD bool IsInSummaryInteractiveContent(dom::EventTarget *target) noexcept
    {
      // The target may have been retargeted out of the summary (the default summary's details), in which case
      // no summary is met on the way up.
      auto isInInteractiveContent = false;
      auto *node = DynamicDowncast<dom::Node>(target);
      for (auto *element = node ? DynamicDowncast<dom::Element>(*node) : nullptr; element;
           element = dom::TreeQueries::ParentOrShadowHostElement(*element))
      {
        if (Is<HTMLSummaryElement>(*element))
        {
          return isInInteractiveContent;
        }
        auto *svgElement = DynamicDowncast<svg::SVGElement>(*element);
        if (auto *htmlElement = DynamicDowncast<HTMLElement>(*element);
            (htmlElement && htmlElement->IsInteractiveContent())
            || (svgElement && Is<svg::SVGAElement>(*svgElement)))
        {
          isInInteractiveContent = true;
        }
      }
      return false;
    }
  }

  HTMLSummaryElement::HTMLSummaryElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Summary)
  {
    SetEventTargetFlag(dom::EventTargetFlags::HasActivationBehavior);
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

  void HTMLSummaryElement::ActivationBehavior(dom::Event &event) noexcept
  {
    if (!IsActiveSummary() || IsInSummaryInteractiveContent(event.Target().get()))
    {
      return;
    }
    if (auto details = DetailsElement())
    {
      details->ToggleOpen();
    }
    event.SetDefaultHandled();
  }

  void HTMLSummaryElement::DefaultEventHandler(dom::Event &event) noexcept
  {
    if (IsActiveSummary())
    {
      if (auto *keyboardEvent = DynamicDowncast<dom::KeyboardEvent>(event))
      {
        if (keyboardEvent->Type() == EventNames::KeyDown && keyboardEvent->KeyIdentifier() == u8"U+0020")
        {
          SetActive(true);
          // No SetDefaultHandled(): a keypress follows.
          return;
        }
        if (keyboardEvent->Type() == EventNames::KeyPress)
        {
          switch (keyboardEvent->CharCode())
          {
            case '\r':
              (void)DispatchSimulatedClick(keyboardEvent);
              keyboardEvent->SetDefaultHandled();
              return;
            case ' ':
              // Prevent scrolling down the page.
              keyboardEvent->SetDefaultHandled();
              return;
            default: break;
          }
        }
        if (keyboardEvent->Type() == EventNames::KeyUp && keyboardEvent->KeyIdentifier() == u8"U+0020")
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

  void HTMLSummaryElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnRemove(isSubtreeRoot, oldAncestor);
    if (auto *details = DynamicDowncast<HTMLDetailsElement>(oldAncestor); details && isSubtreeRoot)
    {
      details->SummaryChildChanged();
    }
  }

  void HTMLSummaryElement::OnMove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
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
