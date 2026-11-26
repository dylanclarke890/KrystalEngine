#pragma once

#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Web/Dom/Types/DOMString.hpp"
#include "Krystal.Web/Html/Forms/FormAssociatedElement.hpp"

namespace krys::web
{
  class Node;
}

namespace krys::web::html
{
  class DOMFormData;
  class FormAttributeTargetObserver;
  class ValidatedFormListedElement;
  class ValidityState;

  /// @brief A form-associated element listed in its form's `elements` and in its fieldsets'.
  /// @see https://html.spec.whatwg.org/multipage/forms.html#category-listed
  class FormListedElement : public FormAssociatedElement
  {
  private:
    UniquePtr<FormAttributeTargetObserver> _formAttributeTargetObserver;
    DOMString _customValidationMessage;

  public:
    ~FormListedElement() noexcept override;

    /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#dom-cva-validity
    KRYS_NODISCARD ValidityState Validity() noexcept;

    /// @brief Whether the element takes part in constraint validation (a ValidatedFormListedElement).
    KRYS_NODISCARD virtual bool IsValidatedFormListedElement() const noexcept = 0;

    /// @brief Whether the form's `elements` lists it (an image button isn't listed).
    /// @see https://html.spec.whatwg.org/multipage/forms.html#dom-form-elements
    KRYS_NODISCARD virtual bool IsEnumeratable() const noexcept = 0;

    /// @brief The `name` attribute's value, or the empty string if there isn't one (the `name` IDL attribute
    /// doesn't use this).
    KRYS_NODISCARD virtual DOMString Name() const noexcept;

    /// @brief Appends the element's entries for submission, if it's a successful control.
    /// @return Whether it appended any.
    virtual bool AppendFormData(DOMFormData &formData) noexcept
    {
      (void)formData;
      return false;
    }

    void FormWillBeDestroyed() noexcept override;

    void ResetFormOwner() noexcept final;

    /// @brief Drops the form owner if the element isn't in the tree the form left (rooted at @p formRoot).
    void FormOwnerRemovedFromTree(const Node &formRoot) noexcept;

    // The validity states (ValidityState's attributes). Overrides of patternMismatch, rangeOverflow,
    // rangeUnderflow, stepMismatch, tooShort, tooLong and valueMissing check WillValidate.
    KRYS_NODISCARD bool BadInput() const noexcept
    {
      return HasBadInput();
    }

    KRYS_NODISCARD virtual bool CustomError() const noexcept;

    KRYS_NODISCARD virtual bool HasBadInput() const noexcept;

    KRYS_NODISCARD virtual bool PatternMismatch() const noexcept;

    KRYS_NODISCARD virtual bool RangeOverflow() const noexcept;

    KRYS_NODISCARD virtual bool RangeUnderflow() const noexcept;

    KRYS_NODISCARD virtual bool StepMismatch() const noexcept;

    KRYS_NODISCARD virtual bool TooShort() const noexcept;

    KRYS_NODISCARD virtual bool TooLong() const noexcept;

    KRYS_NODISCARD virtual bool TypeMismatch() const noexcept;

    KRYS_NODISCARD virtual bool ValueMissing() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#dom-cva-validationmessage
    KRYS_NODISCARD virtual DOMString ValidationMessage() const noexcept;

    /// @brief Whether the element suffers from none of the validity states.
    KRYS_NODISCARD virtual bool ComputeValidity() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#dom-cva-setcustomvalidity
    virtual void SetCustomValidity(DOMStringView error) noexcept;

    /// @brief Resets the form owner, as the element the form attribute names changed.
    void FormAttributeTargetChanged() noexcept;

    /// @brief The element as each of its listed element bases.
    KRYS_NODISCARD virtual FormListedElement *AsFormListedElement() noexcept = 0;

    KRYS_NODISCARD virtual ValidatedFormListedElement *AsValidatedFormListedElement() noexcept = 0;

    /// @brief Whether the element is a candidate for constraint validation.
    /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#candidate-for-constraint-validation
    KRYS_NODISCARD virtual bool WillValidate() const noexcept = 0;

  protected:
    FormListedElement() noexcept;

    /// @brief Sets the custom validity error message as it is.
    virtual void SetCustomValidationMessage(DOMStringView message) noexcept;

    /// @brief Drops the form owner.
    void ClearForm() noexcept
    {
      SetForm(nullptr);
    }

    /// @brief Observes the form attribute's element in the new document.
    void DidMoveToNewDocument() noexcept;

    void ElementInsertedIntoAncestor(web::Element &element, bool connectedToDocument) noexcept override;

    void ElementRemovedFromAncestor(web::Element &element, bool disconnectedFromDocument) noexcept override;

    /// @brief Handles a change to the attributes listed elements share (`form`).
    void ParseAttribute(const DOMStringAtom &localName, Maybe<DOMStringView> value) noexcept;

    /// @brief Resets the form owner for a new `form` attribute, observing the element it names.
    void ParseFormAttribute(Maybe<DOMStringView> value) noexcept;

    // A class that overrides WillChangeForm() or DidChangeForm() must clear its form in its destructor.
    /// @brief Called before and after the form owner changes.
    virtual void WillChangeForm() noexcept;

    virtual void DidChangeForm() noexcept;

    /// @brief The custom validity error message, as `setCustomValidity()` set it.
    KRYS_NODISCARD DOMString CustomValidationMessage() const noexcept;

  private:
    void SetFormInternal(HTMLFormElement *newForm) noexcept final;

    /// @brief Observes the element the form attribute names, to reset the form owner when it changes.
    void ResetFormAttributeTargetObserver() noexcept;
  };

  /// @brief The validity states of a listed element, as `validity` exposes them.
  /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#validitystate
  class ValidityState
  {
  private:
    FormListedElement &_element;

  public:
    /// @brief The validity states of @p element.
    explicit ValidityState(FormListedElement &element) noexcept : _element(element)
    {
    }

    /// @brief The element whose validity this is.
    KRYS_NODISCARD HTMLElement &Element() const noexcept
    {
      return _element.AsHTMLElement();
    }

    /// @brief The validity states, each as its ValidityState attribute; `Valid` when none applies.
    /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#validitystate
    KRYS_NODISCARD bool ValueMissing() const noexcept
    {
      return _element.ValueMissing();
    }

    KRYS_NODISCARD bool TypeMismatch() const noexcept
    {
      return _element.TypeMismatch();
    }

    KRYS_NODISCARD bool PatternMismatch() const noexcept
    {
      return _element.PatternMismatch();
    }

    KRYS_NODISCARD bool TooShort() const noexcept
    {
      return _element.TooShort();
    }

    KRYS_NODISCARD bool TooLong() const noexcept
    {
      return _element.TooLong();
    }

    KRYS_NODISCARD bool RangeUnderflow() const noexcept
    {
      return _element.RangeUnderflow();
    }

    KRYS_NODISCARD bool RangeOverflow() const noexcept
    {
      return _element.RangeOverflow();
    }

    KRYS_NODISCARD bool StepMismatch() const noexcept
    {
      return _element.StepMismatch();
    }

    KRYS_NODISCARD bool BadInput() const noexcept
    {
      return _element.BadInput();
    }

    KRYS_NODISCARD bool CustomError() const noexcept
    {
      return _element.CustomError();
    }

    KRYS_NODISCARD bool Valid() const noexcept
    {
      return _element.ComputeValidity();
    }
  };

  inline ValidityState FormListedElement::Validity() noexcept
  {
    return ValidityState(*this);
  }
}
