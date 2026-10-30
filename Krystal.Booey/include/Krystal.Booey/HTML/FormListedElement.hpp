#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/HTML/FormAssociatedElement.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::dom
{
  class Node;
}

namespace krys::boo::html
{
  class DOMFormData;
  class FormAttributeTargetObserver;
  class ValidatedFormListedElement;
  class ValidityState;

  /// @brief A form-associated element listed in its form's `elements` and in its fieldsets'.
  /// @see https://html.spec.whatwg.org/#category-listed
  class FormListedElement : public FormAssociatedElement
  {
  private:
    UniquePtr<FormAttributeTargetObserver> _formAttributeTargetObserver;
    dom::DOMString _customValidationMessage;

  public:
    ~FormListedElement() noexcept override;

    /// @see https://html.spec.whatwg.org/#dom-cva-validity
    KRYS_NODISCARD ValidityState Validity() noexcept;

    KRYS_NODISCARD virtual bool IsValidatedFormListedElement() const noexcept = 0;
    KRYS_NODISCARD virtual bool IsEnumeratable() const noexcept = 0;

    /// @brief The `name` attribute's value, or the empty string if there isn't one (the `name` IDL attribute
    /// doesn't use this).
    KRYS_NODISCARD virtual dom::DOMString Name() const noexcept;

    /// @brief Appends the element's entries for submission, if it's a successful control.
    /// @return Whether it appended any.
    virtual bool AppendFormData(DOMFormData &formData) noexcept
    {
      (void)formData;
      return false;
    }

    void FormWillBeDestroyed() noexcept override;
    void ResetFormOwner() noexcept final;
    void FormOwnerRemovedFromTree(const dom::Node &formRoot) noexcept;

    // The ValidityState attributes. The implementations of patternMismatch, rangeOverflow, rangeUnderflow,
    // stepMismatch, tooShort, tooLong and valueMissing must call WillValidate.
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

    /// @see https://html.spec.whatwg.org/#dom-cva-validationmessage
    KRYS_NODISCARD virtual dom::DOMString ValidationMessage() const noexcept;

    KRYS_NODISCARD virtual bool ComputeValidity() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-cva-setcustomvalidity
    virtual void SetCustomValidity(dom::DOMStringView error) noexcept;

    void FormAttributeTargetChanged() noexcept;

    KRYS_NODISCARD virtual FormListedElement *AsFormListedElement() noexcept = 0;
    KRYS_NODISCARD virtual ValidatedFormListedElement *AsValidatedFormListedElement() noexcept = 0;

    /// @brief Whether the element is a candidate for constraint validation.
    /// @see https://html.spec.whatwg.org/#candidate-for-constraint-validation
    KRYS_NODISCARD virtual bool WillValidate() const noexcept = 0;

  protected:
    FormListedElement() noexcept;

    void ClearForm() noexcept
    {
      SetForm(nullptr);
    }

    void DidMoveToNewDocument() noexcept;
    void ElementInsertedIntoAncestor(dom::Element &element, bool connectedToDocument) noexcept override;
    void ElementRemovedFromAncestor(dom::Element &element, bool disconnectedFromDocument) noexcept override;
    void ParseAttribute(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> value) noexcept;
    void ParseFormAttribute(Maybe<dom::DOMStringView> value) noexcept;

    // A class that overrides WillChangeForm() or DidChangeForm() must clear its form in its destructor.
    virtual void WillChangeForm() noexcept;
    virtual void DidChangeForm() noexcept;

    KRYS_NODISCARD dom::DOMString CustomValidationMessage() const noexcept;

  private:
    void SetFormInternal(HTMLFormElement *newForm) noexcept final;

    void ResetFormAttributeTargetObserver() noexcept;
  };

  /// @brief The validity states of a listed element, as `validity` exposes them.
  /// @see https://html.spec.whatwg.org/#validitystate
  class ValidityState
  {
  private:
    FormListedElement &_element;

  public:
    explicit ValidityState(FormListedElement &element) noexcept : _element(element)
    {
    }

    KRYS_NODISCARD HTMLElement &Element() const noexcept
    {
      return _element.AsHTMLElement();
    }

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
