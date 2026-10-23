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

  RefPtr<dom::ShadowRoot> ElementInternals::ShadowRoot() const noexcept
  {
    auto *element = _target.get();
    if (!element)
    {
      return nullptr;
    }
    auto *shadowRoot = dom::TreeQueries::GetShadowRoot(*element);
    if (!shadowRoot || !shadowRoot->IsAvailableToElementInternals())
    {
      return nullptr;
    }
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
