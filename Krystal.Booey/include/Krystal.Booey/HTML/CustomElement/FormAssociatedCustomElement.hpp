/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementFormValue.hpp"
#include "Krystal.Booey/HTML/CustomElement/ValidityStateFlags.hpp"
#include "Krystal.Booey/HTML/Forms/ValidatedFormListedElement.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::html
{
  class HTMLMaybeFormAssociatedCustomElement;

  /// @brief The form-associated half of a form-associated custom element: its form owner, submission value,
  /// validity and disabled state, set through its `ElementInternals`.
  /// @see https://html.spec.whatwg.org/#form-associated-custom-element
  class FormAssociatedCustomElement final : public ValidatedFormListedElement
  {
    KRYS_NON_COPY_MOVABLE(FormAssociatedCustomElement);

  private:
    HTMLMaybeFormAssociatedCustomElement &_element;
    ValidityStateFlags _validityStateFlags;
    WeakPtr<HTMLElement> _validationAnchor;
    CustomElementFormValue _submissionValue {nullptr};
    CustomElementFormValue _state {nullptr};

  public:
    explicit FormAssociatedCustomElement(HTMLMaybeFormAssociatedCustomElement &element) noexcept;
    ~FormAssociatedCustomElement() noexcept override;

    KRYS_NODISCARD bool IsFormListedElement() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool IsValidatedFormListedElement() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD FormAssociatedElement *AsFormAssociatedElement() noexcept final
    {
      return this;
    }

    KRYS_NODISCARD FormListedElement *AsFormListedElement() noexcept final
    {
      return this;
    }

    KRYS_NODISCARD ValidatedFormListedElement *AsValidatedFormListedElement() noexcept final
    {
      return this;
    }

    KRYS_NODISCARD HTMLElement &AsHTMLElement() noexcept final;
    KRYS_NODISCARD const HTMLElement &AsHTMLElement() const noexcept final;

    void Reset() noexcept final;
    KRYS_NODISCARD bool IsEnumeratable() const noexcept final;

    /// @see https://html.spec.whatwg.org/#dom-elementinternals-setformvalue
    void SetFormValue(CustomElementFormValue &&submissionValue,
                      Maybe<CustomElementFormValue> &&state) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-elementinternals-setvalidity
    dom::ExceptionOr<void> SetValidity(ValidityStateFlags flags, dom::DOMString message,
                                       HTMLElement *validationAnchor) noexcept;

    KRYS_NODISCARD dom::DOMString ValidationMessage() const noexcept final;

    KRYS_NODISCARD bool ComputeValidity() const noexcept final;
    bool AppendFormData(DOMFormData &formData) noexcept final;

    /// @brief The element is about to be upgraded by a form-associated definition.
    void WillUpgrade() noexcept;
    /// @brief The element was upgraded: it takes its form owner and disabled state from its attributes.
    void DidUpgrade() noexcept;

    KRYS_NODISCARD dom::DOMString FormControlType() const noexcept final;

    // The element's node hooks, which it forwards.
    using ValidatedFormListedElement::DidFinishInsertingNode;
    using ValidatedFormListedElement::DidMoveToNewDocument;
    using ValidatedFormListedElement::InsertedIntoAncestor;
    using ValidatedFormListedElement::ParseAttribute;
    using ValidatedFormListedElement::RemovedFromAncestor;

  protected:
    void DisabledStateChanged() noexcept final;

    KRYS_NODISCARD bool ReadOnlyBarsFromConstraintValidation() const noexcept final
    {
      return true;
    }

  private:
    KRYS_NODISCARD HTMLElement *ValidationAnchorElement() noexcept final;
    void DidChangeForm() noexcept final;

    KRYS_NODISCARD bool CustomError() const noexcept final
    {
      return _validityStateFlags.CustomError;
    }

    KRYS_NODISCARD bool HasBadInput() const noexcept final
    {
      return _validityStateFlags.BadInput;
    }

    KRYS_NODISCARD bool PatternMismatch() const noexcept final
    {
      return _validityStateFlags.PatternMismatch;
    }

    KRYS_NODISCARD bool RangeOverflow() const noexcept final
    {
      return _validityStateFlags.RangeOverflow;
    }

    KRYS_NODISCARD bool RangeUnderflow() const noexcept final
    {
      return _validityStateFlags.RangeUnderflow;
    }

    KRYS_NODISCARD bool StepMismatch() const noexcept final
    {
      return _validityStateFlags.StepMismatch;
    }

    KRYS_NODISCARD bool TooShort() const noexcept final
    {
      return _validityStateFlags.TooShort;
    }

    KRYS_NODISCARD bool TooLong() const noexcept final
    {
      return _validityStateFlags.TooLong;
    }

    KRYS_NODISCARD bool TypeMismatch() const noexcept final
    {
      return _validityStateFlags.TypeMismatch;
    }

    KRYS_NODISCARD bool ValueMissing() const noexcept final
    {
      return _validityStateFlags.ValueMissing;
    }
  };
}
