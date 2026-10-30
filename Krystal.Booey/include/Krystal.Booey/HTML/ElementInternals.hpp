#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/HTML/CustomElementFormValue.hpp"
#include "Krystal.Booey/HTML/FormListedElement.hpp"
#include "Krystal.Booey/HTML/ValidityStateFlags.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class NodeList;
  class ShadowRoot;
}

namespace krys::boo::html
{
  class CustomStateSet;
  class FormAssociatedCustomElement;
  class HTMLElement;
  class HTMLFormElement;
  class Reflection;

  /// @brief What a custom element's behaviour reaches its element through: its closed shadow root, its
  /// custom states, and, for a form-associated custom element, its form value, validity and labels.
  /// @see https://html.spec.whatwg.org/#elementinternals
  class ElementInternals : public RefCounted<ElementInternals>
  {
    friend class Reflection;

  private:
    WeakPtr<HTMLElement> _target;

  public:
    explicit ElementInternals(HTMLElement &element) noexcept;
    ~ElementInternals() noexcept;

    KRYS_NODISCARD static Ref<ElementInternals> Create(HTMLElement &element) noexcept
    {
      return CreateRef<ElementInternals>(element);
    }

    /// @brief The element whose internals these are; null once it's gone.
    KRYS_NODISCARD HTMLElement *Element() const noexcept
    {
      return _target.get();
    }

    /// @brief The element's shadow root, open or closed, if it was attached while the element was a custom
    /// element.
    /// @see https://html.spec.whatwg.org/#dom-elementinternals-shadowroot
    KRYS_NODISCARD RefPtr<dom::ShadowRoot> ShadowRoot() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-elementinternals-form
    KRYS_NODISCARD dom::ExceptionOr<RefPtr<HTMLFormElement>> Form() const noexcept;

    /// @brief Sets the value the element submits with its form, and the state it restores (by default its
    /// value).
    /// @see https://html.spec.whatwg.org/#dom-elementinternals-setformvalue
    dom::ExceptionOr<void> SetFormValue(CustomElementFormValue &&value,
                                        Maybe<CustomElementFormValue> &&state = null) noexcept;

    /// @brief Sets the element's validity: invalid with @p message if any of @p flags is set.
    /// @param validationAnchor Where the validation message points (a shadow-including descendant of the
    /// element); the element if null.
    /// @see https://html.spec.whatwg.org/#dom-elementinternals-setvalidity
    dom::ExceptionOr<void> SetValidity(ValidityStateFlags flags = {}, dom::DOMString &&message = {},
                                       HTMLElement *validationAnchor = nullptr) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-elementinternals-willvalidate
    KRYS_NODISCARD dom::ExceptionOr<bool> WillValidate() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-elementinternals-validity
    KRYS_NODISCARD dom::ExceptionOr<ValidityState> Validity() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-elementinternals-validationmessage
    KRYS_NODISCARD dom::ExceptionOr<dom::DOMString> ValidationMessage() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-elementinternals-reportvalidity
    dom::ExceptionOr<bool> ReportValidity() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-elementinternals-checkvalidity
    dom::ExceptionOr<bool> CheckValidity() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-elementinternals-labels
    KRYS_NODISCARD dom::ExceptionOr<RefPtr<dom::NodeList>> Labels() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-elementinternals-states
    KRYS_NODISCARD CustomStateSet &States() noexcept;

  private:
    KRYS_NODISCARD HTMLElement *TargetElement() noexcept
    {
      return _target.get();
    }

    KRYS_NODISCARD FormAssociatedCustomElement *ElementAsFormAssociatedCustom() const noexcept;
  };
}
