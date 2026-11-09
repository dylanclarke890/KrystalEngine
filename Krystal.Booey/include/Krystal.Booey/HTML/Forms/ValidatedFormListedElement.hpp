/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004-2023 Apple Inc. All rights reserved.
 *           (C) 2006 Alexey Proskuryakov (ap@nypop.com)
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

#include "Krystal.Booey/HTML/Forms/FormListedElement.hpp"
#include "Krystal.Core/TriState.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::dom
{
  class ContainerNode;
}

namespace krys::boo::html
{
  /// @brief A listed element that takes part in constraint validation: it can be disabled (by its own
  /// attribute or a fieldset's), read-only, invalid, and reset.
  class ValidatedFormListedElement : public FormListedElement
  {
    friend class DelayedUpdateValidityScope;

  private:
    // Cache of Validity().Valid(), though being a candidate for constraint validation doesn't affect it.
    bool _isValid : 1 {true};
    // The initial value of WillValidate() depends on the derived class, so it can't be computed in the
    // constructor; it isn't settled while _willValidateInitialized is false.
    mutable bool _willValidate : 1 {true};
    mutable bool _willValidateInitialized : 1 {false};
    bool _disabled : 1 {false};
    bool _disabledByAncestorFieldset : 1 {false};
    bool _hasReadOnlyAttribute : 1 {false};
    bool _wasInteractedWithSinceLastFormSubmitEvent : 1 {false};
    bool _belongsToFormThatIsBeingDestroyed : 1 {false};
    bool _isFocusingWithValidationMessage {false};
    /// @brief The anchor the host was last asked to show the control's validation message at.
    WeakPtr<HTMLElement> _validationMessageAnchor;
    mutable TriState _isInsideDataList {TriState::Indeterminate};
    uint32 _delayedUpdateValidityCount {0};

  public:
    ~ValidatedFormListedElement() noexcept override;

    /// @brief Whether the element is a candidate for constraint validation.
    KRYS_NODISCARD bool WillValidate() const noexcept override;

    /// @brief Asks the host to show the control's validation message at @p validationAnchor, or to hide it if
    /// it's shown (as typing does) or there's none.
    void UpdateVisibleValidationMessage(HTMLElement &validationAnchor) noexcept;
    void HideVisibleValidationMessage() noexcept;

    /// @see https://html.spec.whatwg.org/#check-validity-steps
    /// @param unhandledInvalidControls Gets the element if it's invalid and its `invalid` event wasn't
    /// canceled.
    bool CheckValidity(List<Ref<HTMLElement>> *unhandledInvalidControls = nullptr) noexcept;

    /// @see https://html.spec.whatwg.org/#report-validity-steps
    bool ReportValidity() noexcept;

    KRYS_NODISCARD HTMLElement *FocusableValidationAnchorElement() noexcept;
    void ReportNonFocusableControlError() noexcept;
    void FocusAndShowValidationMessage(HTMLElement &validationAnchor) noexcept;
    KRYS_NODISCARD bool IsShowingValidationMessage() const noexcept;
    KRYS_NODISCARD bool IsFocusingWithValidationMessage() const noexcept;

    /// @brief Must be called when a validation constraint or the control's value changes.
    void UpdateValidity() noexcept;

    void SetCustomValidationMessage(dom::DOMStringView message) noexcept override;

    void SetDisabledByAncestorFieldset(bool isDisabled) noexcept;

    /// @see https://html.spec.whatwg.org/#concept-form-reset-control
    virtual void Reset() noexcept
    {
    }

    KRYS_NODISCARD virtual bool SupportsReadOnly() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD bool IsDisabled() const noexcept
    {
      return _disabled || _disabledByAncestorFieldset;
    }

    KRYS_NODISCARD bool IsReadOnly() const noexcept
    {
      return SupportsReadOnly() && _hasReadOnlyAttribute;
    }

    /// @see https://html.spec.whatwg.org/#concept-fe-mutable
    KRYS_NODISCARD bool IsMutable() const noexcept
    {
      return !IsDisabled() && !IsReadOnly();
    }

    KRYS_NODISCARD bool IsValidFormControlElement() const noexcept
    {
      return _isValid;
    }

    KRYS_NODISCARD bool IsEnumeratable() const noexcept override
    {
      return false;
    }

    KRYS_NODISCARD bool WasInteractedWithSinceLastFormSubmitEvent() const noexcept
    {
      return _wasInteractedWithSinceLastFormSubmitEvent;
    }

    void SetInteractedWithSinceLastFormSubmitEvent(bool interactedWith) noexcept;

    KRYS_NODISCARD bool MatchesValidPseudoClass() const noexcept;
    KRYS_NODISCARD bool MatchesInvalidPseudoClass() const noexcept;
    KRYS_NODISCARD bool MatchesUserInvalidPseudoClass() const noexcept;
    KRYS_NODISCARD bool MatchesUserValidPseudoClass() const noexcept;

    KRYS_NODISCARD virtual bool ShouldAutocomplete() const noexcept;

    /// @brief The control's type, as `type` exposes it.
    KRYS_NODISCARD virtual dom::DOMString FormControlType() const noexcept = 0;

  protected:
    ValidatedFormListedElement() noexcept = default;

    KRYS_NODISCARD bool HasDisabledAttribute() const noexcept
    {
      return _disabled;
    }

    KRYS_NODISCARD virtual bool ComputeWillValidate() const noexcept;

    KRYS_NODISCARD virtual bool ReadOnlyBarsFromConstraintValidation() const noexcept
    {
      return false;
    }

    /// @brief Must be called whenever the result of WillValidate() may have changed.
    void UpdateWillValidateAndValidity() noexcept;

    KRYS_NODISCARD bool DisabledByAncestorFieldset() const noexcept
    {
      return _disabledByAncestorFieldset;
    }

    void InsertedIntoAncestor(bool connectedToDocument, dom::ContainerNode &parentOfInsertedTree) noexcept;
    void DidFinishInsertingNode() noexcept;
    void RemovedFromAncestor(bool disconnectedFromDocument,
                             dom::ContainerNode &oldParentOfRemovedTree) noexcept;
    void ParseAttribute(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> value) noexcept;
    void ParseDisabledAttribute(Maybe<dom::DOMStringView> value) noexcept;
    void ParseReadOnlyAttribute(Maybe<dom::DOMStringView> value) noexcept;

    virtual void DisabledStateChanged() noexcept;
    virtual void ReadOnlyStateChanged() noexcept;

    void WillChangeForm() noexcept override;
    void DidChangeForm() noexcept override;
    void FormWillBeDestroyed() noexcept final;

    KRYS_NODISCARD bool BelongsToFormThatIsBeingDestroyed() const noexcept
    {
      return _belongsToFormThatIsBeingDestroyed;
    }

    void SetDataListAncestorState(TriState isInsideDataList) noexcept;
    void SyncWithFieldsetAncestors(dom::ContainerNode *insertionNode) noexcept;

  private:
    KRYS_NODISCARD bool ComputeIsDisabledByFieldsetAncestor() const noexcept;
    void SetDisabledInternal(bool disabled, bool disabledByAncestorFieldset) noexcept;

    KRYS_NODISCARD virtual HTMLElement *ValidationAnchorElement() noexcept = 0;

    void StartDelayingUpdateValidity() noexcept
    {
      ++_delayedUpdateValidityCount;
    }

    void EndDelayingUpdateValidity() noexcept;
  };

  /// @brief Delays the validity updates of an element while it lives, updating it once at its end.
  class DelayedUpdateValidityScope
  {
    KRYS_NON_COPY_MOVABLE(DelayedUpdateValidityScope);

  private:
    ValidatedFormListedElement &_element;
    Ref<HTMLElement> _protectedElement;

  public:
    explicit DelayedUpdateValidityScope(ValidatedFormListedElement &element) noexcept;

    ~DelayedUpdateValidityScope() noexcept
    {
      _element.EndDelayingUpdateValidity();
    }
  };
}
