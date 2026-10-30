#pragma once

#include "Krystal.Booey/HTML/FormController.hpp"
#include "Krystal.Booey/HTML/FormListedElement.hpp"
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
    mutable TriState _isInsideDataList {TriState::Indeterminate};
    uint32 _delayedUpdateValidityCount {0};

  public:
    ~ValidatedFormListedElement() noexcept override;

    /// @brief Whether the element is a candidate for constraint validation.
    KRYS_NODISCARD bool WillValidate() const noexcept override;

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

    void SetCustomValidity(dom::DOMStringView error) noexcept override;

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

    /// @brief Whether the control's state is saved with its document's history item and restored to it: if
    /// the control saves state, is connected, and isn't in a form with `autocomplete=off`.
    KRYS_NODISCARD bool IsCandidateForSavingAndRestoringState() const noexcept;

    KRYS_NODISCARD virtual bool ShouldSaveAndRestoreFormControlState() const noexcept
    {
      return false;
    }

    /// @brief The control's state, to restore it later; empty if there's nothing to restore.
    KRYS_NODISCARD virtual FormControlState SaveFormControlState() const noexcept;

    /// @brief Gives the control the @p state it saved.
    virtual void RestoreFormControlState(const FormControlState &state) noexcept
    {
      (void)state;
    }

    /// @brief The control's type, as `type` exposes it.
    KRYS_NODISCARD virtual dom::DOMString FormControlType() const noexcept = 0;

  protected:
    ValidatedFormListedElement() noexcept = default;

    /// @brief Gives the control its saved state, if its document has some for it.
    void RestoreFormControlStateIfNecessary() noexcept;

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
