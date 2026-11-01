#include "Krystal.Booey/HTML/ElementInternals.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/NodeList.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomStateSet.hpp"
#include "Krystal.Booey/HTML/FormAssociatedCustomElement.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/HTMLMaybeFormAssociatedCustomElement.hpp"

namespace krys::boo::html
{
  ElementInternals::ElementInternals(HTMLElement &element) noexcept : _target(CreateWeakPtr(&element))
  {
  }

  ElementInternals::~ElementInternals() noexcept = default;

  // https://html.spec.whatwg.org/#dom-elementinternals-shadowroot
  RefPtr<dom::ShadowRoot> ElementInternals::ShadowRoot() const noexcept
  {
    // 1. Let target be this's target element.
    auto *element = _target.get();
    if (!element)
    {
      return nullptr;
    }

    // 2. If target is not a shadow host, then return null.
    // 3. Let shadow be target's shadow root.
    // 4. If shadow's available to element internals is false, then return null.
    auto *shadowRoot = dom::TreeQueries::GetShadowRoot(*element);
    if (!shadowRoot || !shadowRoot->IsAvailableToElementInternals())
    {
      return nullptr;
    }

    // 5. Return shadow.
    return ShareRefPtr(shadowRoot);
  }

  dom::ExceptionOr<RefPtr<HTMLFormElement>> ElementInternals::Form() const noexcept
  {
    if (auto *element = ElementAsFormAssociatedCustom())
    {
      return element->FormForBindings();
    }
    return dom::Exception {dom::ExceptionCode::NotSupportedError};
  }

  // https://html.spec.whatwg.org/#dom-elementinternals-setformvalue
  // 1. Let element be this's target element.
  // 2. If element is not a form-associated custom element, then throw a "NotSupportedError" DOMException.
  // (And so for the other form members.)
  dom::ExceptionOr<void> ElementInternals::SetFormValue(CustomElementFormValue &&value,
                                                        Maybe<CustomElementFormValue> &&state) noexcept
  {
    if (auto *element = ElementAsFormAssociatedCustom())
    {
      element->SetFormValue(krys::move(value), krys::move(state));
      return {};
    }
    return dom::Exception {dom::ExceptionCode::NotSupportedError};
  }

  dom::ExceptionOr<void> ElementInternals::SetValidity(ValidityStateFlags flags, dom::DOMString &&message,
                                                       HTMLElement *validationAnchor) noexcept
  {
    if (auto *element = ElementAsFormAssociatedCustom())
    {
      return element->SetValidity(flags, krys::move(message), validationAnchor);
    }
    return dom::Exception {dom::ExceptionCode::NotSupportedError};
  }

  dom::ExceptionOr<bool> ElementInternals::WillValidate() const noexcept
  {
    if (auto *element = ElementAsFormAssociatedCustom())
    {
      return element->WillValidate();
    }
    return dom::Exception {dom::ExceptionCode::NotSupportedError};
  }

  dom::ExceptionOr<ValidityState> ElementInternals::Validity() noexcept
  {
    if (auto *element = ElementAsFormAssociatedCustom())
    {
      return element->Validity();
    }
    return dom::Exception {dom::ExceptionCode::NotSupportedError};
  }

  dom::ExceptionOr<dom::DOMString> ElementInternals::ValidationMessage() const noexcept
  {
    if (auto *element = ElementAsFormAssociatedCustom())
    {
      return element->ValidationMessage();
    }
    return dom::Exception {dom::ExceptionCode::NotSupportedError};
  }

  dom::ExceptionOr<bool> ElementInternals::ReportValidity() noexcept
  {
    if (auto *element = ElementAsFormAssociatedCustom())
    {
      return element->ReportValidity();
    }
    return dom::Exception {dom::ExceptionCode::NotSupportedError};
  }

  dom::ExceptionOr<bool> ElementInternals::CheckValidity() noexcept
  {
    if (auto *element = ElementAsFormAssociatedCustom())
    {
      return element->CheckValidity();
    }
    return dom::Exception {dom::ExceptionCode::NotSupportedError};
  }

  dom::ExceptionOr<RefPtr<dom::NodeList>> ElementInternals::Labels() noexcept
  {
    if (auto *element = ElementAsFormAssociatedCustom())
    {
      return element->AsHTMLElement().Labels();
    }
    return dom::Exception {dom::ExceptionCode::NotSupportedError};
  }

  // https://html.spec.whatwg.org/#dom-elementinternals-states
  // (The states getter steps are to return this's target element's states set.)
  CustomStateSet &ElementInternals::States() noexcept
  {
    return _target->EnsureCustomStateSet();
  }

  FormAssociatedCustomElement *ElementInternals::ElementAsFormAssociatedCustom() const noexcept
  {
    if (auto *element = DynamicDowncast<HTMLMaybeFormAssociatedCustomElement>(_target.get()))
    {
      return element->FormAssociatedCustomElementForElementInternals();
    }
    return nullptr;
  }
}
