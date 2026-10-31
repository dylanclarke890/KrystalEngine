#include "Krystal.Booey/HTML/ValidatedFormListedElement.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/HTMLDataListElement.hpp"
#include "Krystal.Booey/HTML/HTMLFieldSetElement.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/HTMLLegendElement.hpp"
#include "Krystal.Booey/Style/PseudoClassChangeInvalidation.hpp"

namespace krys::boo::html
{
  namespace
  {
    using PseudoClass = css::SelectorPseudoClass;

    void AddInvalidElementToAncestorFromInsertionPoint(const HTMLElement &element,
                                                       dom::ContainerNode *insertionPoint) noexcept
    {
      auto *insertionPointElement = DynamicDowncast<dom::Element>(insertionPoint);
      for (auto *ancestor = insertionPointElement; ancestor; ancestor = ancestor->ParentElement())
      {
        if (auto *fieldset = DynamicDowncast<HTMLFieldSetElement>(*ancestor))
        {
          fieldset->AddInvalidDescendant(element);
        }
      }
    }

    void RemoveInvalidElementToAncestorFromInsertionPoint(const HTMLElement &element,
                                                          dom::ContainerNode *insertionPoint) noexcept
    {
      auto *insertionPointElement = DynamicDowncast<dom::Element>(insertionPoint);
      for (auto *ancestor = insertionPointElement; ancestor; ancestor = ancestor->ParentElement())
      {
        if (auto *fieldset = DynamicDowncast<HTMLFieldSetElement>(*ancestor))
        {
          fieldset->RemoveInvalidDescendant(element);
        }
      }
    }
  }

  ValidatedFormListedElement::~ValidatedFormListedElement() noexcept = default;

  bool ValidatedFormListedElement::WillValidate() const noexcept
  {
    if (!_willValidateInitialized || _isInsideDataList == TriState::Indeterminate)
    {
      _willValidateInitialized = true;
      bool newWillValidate = ComputeWillValidate();
      if (_willValidate != newWillValidate)
      {
        _willValidate = newWillValidate;
      }
    }
    else
    {
      // If this fails, UpdateWillValidateAndValidity() isn't called when something that changes
      // ComputeWillValidate()'s result changes.
      krys_debug_assert(_willValidate == ComputeWillValidate());
    }
    return _willValidate;
  }

  bool ValidatedFormListedElement::ComputeWillValidate() const noexcept
  {
    if (_isInsideDataList == TriState::Indeterminate)
    {
      bool isInsideDataList = false;
      for (auto *ancestor = AsHTMLElement().ParentElement(); ancestor; ancestor = ancestor->ParentElement())
      {
        if (Is<HTMLDataListElement>(*ancestor))
        {
          isInsideDataList = true;
          break;
        }
      }
      _isInsideDataList = isInsideDataList ? TriState::True : TriState::False;
    }
    // readonly bars constraint validation for *all* <input> elements, regardless of the <input> type, for
    // compatibility.
    return _isInsideDataList == TriState::False && !IsDisabled()
           && !(_hasReadOnlyAttribute && ReadOnlyBarsFromConstraintValidation());
  }

  void ValidatedFormListedElement::UpdateVisibleValidationMessage(HTMLElement &) noexcept
  {
    // FIXME: The validation message bubble (ValidationMessage), shown by the host.
  }

  void ValidatedFormListedElement::HideVisibleValidationMessage() noexcept
  {
    // FIXME: The validation message bubble (ValidationMessage), shown by the host.
  }

  bool ValidatedFormListedElement::CheckValidity(List<Ref<HTMLElement>> *unhandledInvalidControls) noexcept
  {
    if (!WillValidate() || IsValidFormControlElement())
    {
      return true;
    }

    // An event handler can deref this object.
    auto &element = AsHTMLElement();
    Ref<HTMLElement> protectedThis = ShareRef(element);
    Ref<dom::Document> originalDocument = ShareRef(element.NodeDocument());
    auto type = dom::DOMStringAtom(EventNames::Invalid);
    bool notCanceled =
      dom::EventDispatcher::FireEvent<dom::Event>(type, element, dom::EventInit {.Cancelable = true});
    if (notCanceled && unhandledInvalidControls && element.IsConnected()
        && originalDocument.get() == &element.NodeDocument())
    {
      unhandledInvalidControls->push_back(protectedThis);
    }
    return false;
  }

  bool ValidatedFormListedElement::ReportValidity() noexcept
  {
    List<Ref<HTMLElement>> elements;
    if (CheckValidity(&elements))
    {
      return true;
    }

    if (elements.empty())
    {
      return false;
    }

    // Lay out now, as whether the anchor is focusable depends on its renderer.
    (void)AsHTMLElement().NodeDocument().UpdateLayoutIgnorePendingStylesheets();

    if (auto *validationAnchor = FocusableValidationAnchorElement())
    {
      FocusAndShowValidationMessage(*validationAnchor);
    }
    else
    {
      ReportNonFocusableControlError();
    }

    return false;
  }

  HTMLElement *ValidatedFormListedElement::FocusableValidationAnchorElement() noexcept
  {
    if (auto *validationAnchor = ValidationAnchorElement())
    {
      if (validationAnchor->IsConnected() && FocusAlgorithms::IsFocusableArea(*validationAnchor))
      {
        return validationAnchor;
      }
    }
    return nullptr;
  }

  void ValidatedFormListedElement::FocusAndShowValidationMessage(HTMLElement &validationAnchor) noexcept
  {
    Ref<HTMLElement> protectedThis = ShareRef(AsHTMLElement());
    bool wasFocusingWithValidationMessage = std::exchange(_isFocusingWithValidationMessage, true);

    // Calling focus() will scroll the element into view.
    validationAnchor.Focus();

    // FIXME: Showing the validation message once focusing has scrolled the anchor into view
    // (Page::scheduleValidationMessageUpdate), with the validation message bubble.

    _isFocusingWithValidationMessage = wasFocusingWithValidationMessage;
  }

  void ValidatedFormListedElement::ReportNonFocusableControlError() noexcept
  {
    // FIXME: The console message ("An invalid form control with name='...' is not focusable."), once there's
    // a console.
  }

  bool ValidatedFormListedElement::IsShowingValidationMessage() const noexcept
  {
    // FIXME: The validation message bubble.
    return false;
  }

  bool ValidatedFormListedElement::IsFocusingWithValidationMessage() const noexcept
  {
    return _isFocusingWithValidationMessage;
  }

  void ValidatedFormListedElement::SetDisabledByAncestorFieldset(bool newDisabledByAncestorFieldset) noexcept
  {
    krys_debug_assert(ComputeIsDisabledByFieldsetAncestor() == newDisabledByAncestorFieldset);
    SetDisabledInternal(_disabled, newDisabledByAncestorFieldset);
  }

  void ValidatedFormListedElement::SetDisabledInternal(bool disabled,
                                                       bool disabledByAncestorFieldset) noexcept
  {
    bool newDisabledState = disabled || disabledByAncestorFieldset;
    bool changingDisabledState = newDisabledState != IsDisabled();

    Maybe<style::PseudoClassChangeInvalidation> styleInvalidation;
    if (changingDisabledState)
    {
      styleInvalidation.emplace(AsHTMLElement(), std::initializer_list<Pair<PseudoClass, bool>> {
                                                   {PseudoClass::Disabled, newDisabledState},
                                                   {PseudoClass::Enabled, !newDisabledState},
                                                 });
    }

    _disabled = disabled;
    _disabledByAncestorFieldset = disabledByAncestorFieldset;

    if (changingDisabledState)
    {
      DisabledStateChanged();
    }
  }

  void ValidatedFormListedElement::UpdateValidity() noexcept
  {
    if (_delayedUpdateValidityCount)
    {
      return;
    }

    bool willValidate = WillValidate();
    bool newIsValid = ComputeValidity();

    if (newIsValid != _isValid)
    {
      auto &element = AsHTMLElement();
      style::PseudoClassChangeInvalidation styleInvalidation(
        element, {
                   {PseudoClass::Valid, newIsValid},
                   {PseudoClass::Invalid, !newIsValid},
                   {PseudoClass::UserValid, _wasInteractedWithSinceLastFormSubmitEvent && newIsValid},
                   {PseudoClass::UserInvalid, _wasInteractedWithSinceLastFormSubmitEvent && !newIsValid},
                 });

      _isValid = newIsValid;

      if (willValidate)
      {
        if (!newIsValid)
        {
          if (!BelongsToFormThatIsBeingDestroyed())
          {
            AddInvalidElementToAncestorFromInsertionPoint(element, element.ParentNode());
          }
          if (auto *form = Form())
          {
            form->AddInvalidFormControl(element);
          }
        }
        else
        {
          if (!BelongsToFormThatIsBeingDestroyed())
          {
            RemoveInvalidElementToAncestorFromInsertionPoint(element, element.ParentNode());
          }
          if (auto *form = Form())
          {
            form->RemoveInvalidFormControlIfNeeded(element);
          }
        }
      }

      // FIXME: Telling accessibility (AXObjectCache::onValidityChange), once there's accessibility.
    }

    // Updates only if this control already has a validation message.
    if (IsShowingValidationMessage())
    {
      // Calls UpdateVisibleValidationMessage() even if _isValid didn't change, as the validation message can.
      if (auto *validationAnchor = FocusableValidationAnchorElement())
      {
        UpdateVisibleValidationMessage(*validationAnchor);
      }
    }
  }

  void ValidatedFormListedElement::ParseAttribute(dom::DOMStringAtom localName,
                                                  Maybe<dom::DOMStringView> value) noexcept
  {
    if (localName == u8"disabled" && AsHTMLElement().CanBeActuallyDisabled())
    {
      ParseDisabledAttribute(value);
    }
    else if (localName == u8"readonly" && ReadOnlyBarsFromConstraintValidation())
    {
      ParseReadOnlyAttribute(value);
    }
    else
    {
      FormListedElement::ParseAttribute(localName, value);
    }
  }

  void ValidatedFormListedElement::ParseDisabledAttribute(Maybe<dom::DOMStringView> value) noexcept
  {
    krys_debug_assert(AsHTMLElement().CanBeActuallyDisabled());
    bool newDisabled = value.has_value();
    SetDisabledInternal(newDisabled, _disabledByAncestorFieldset);
  }

  void ValidatedFormListedElement::ParseReadOnlyAttribute(Maybe<dom::DOMStringView> value) noexcept
  {
    krys_debug_assert(ReadOnlyBarsFromConstraintValidation());
    bool newHasReadOnlyAttribute = value.has_value();
    if (_hasReadOnlyAttribute != newHasReadOnlyAttribute)
    {
      bool newMatchesReadWrite = SupportsReadOnly() && !newHasReadOnlyAttribute;
      style::PseudoClassChangeInvalidation readWriteInvalidation(
        AsHTMLElement(), {
                           {PseudoClass::ReadWrite, newMatchesReadWrite},
                           {PseudoClass::ReadOnly, !newMatchesReadWrite},
                         });
      _hasReadOnlyAttribute = newHasReadOnlyAttribute;
      ReadOnlyStateChanged();
    }
  }

  void ValidatedFormListedElement::InsertedIntoAncestor(bool connectedToDocument,
                                                        dom::ContainerNode &parentOfInsertedTree) noexcept
  {
    _isInsideDataList = TriState::Indeterminate;
    UpdateWillValidateAndValidity();
    SyncWithFieldsetAncestors(&parentOfInsertedTree);

    FormListedElement::ElementInsertedIntoAncestor(AsHTMLElement(), connectedToDocument);
    if (!connectedToDocument)
    {
      ResetFormOwner();
    }
    // An element inserted into a document waits for DidFinishInsertingNode to reset its form, as the
    // element a form attribute names may not be in the tree yet.
  }

  void ValidatedFormListedElement::SetDataListAncestorState(TriState isInsideDataList) noexcept
  {
    _isInsideDataList = isInsideDataList;
  }

  void ValidatedFormListedElement::SyncWithFieldsetAncestors(dom::ContainerNode *insertionPoint) noexcept
  {
    auto &element = AsHTMLElement();
    if (MatchesInvalidPseudoClass())
    {
      AddInvalidElementToAncestorFromInsertionPoint(element, insertionPoint);
    }
    // FIXME: Only when the document has a disabled fieldset (Document::hasDisabledFieldsetElement), which
    // saves walking the ancestors.
    SetDisabledInternal(_disabled, ComputeIsDisabledByFieldsetAncestor());
  }

  void ValidatedFormListedElement::RemovedFromAncestor(bool disconnectedFromDocument,
                                                       dom::ContainerNode &oldParentOfRemovedTree) noexcept
  {
    auto &element = AsHTMLElement();
    bool wasMatchingInvalidPseudoClass = MatchesInvalidPseudoClass();

    HideVisibleValidationMessage();

    if (_disabledByAncestorFieldset)
    {
      SetDisabledInternal(_disabled, ComputeIsDisabledByFieldsetAncestor());
    }

    bool wasInsideDataList = _isInsideDataList == TriState::True;
    if (wasInsideDataList)
    {
      _isInsideDataList = TriState::Indeterminate;
    }

    FormListedElement::ElementRemovedFromAncestor(element, disconnectedFromDocument);

    if (wasMatchingInvalidPseudoClass)
    {
      RemoveInvalidElementToAncestorFromInsertionPoint(element, &oldParentOfRemovedTree);
    }

    if (wasInsideDataList)
    {
      UpdateWillValidateAndValidity();
    }
  }

  bool ValidatedFormListedElement::ComputeIsDisabledByFieldsetAncestor() const noexcept
  {
    const dom::Element *previousAncestor = nullptr;
    for (auto *ancestor = AsHTMLElement().ParentElement(); ancestor; ancestor = ancestor->ParentElement())
    {
      if (auto *fieldset = DynamicDowncast<HTMLFieldSetElement>(*ancestor);
          fieldset && ancestor->HasAttribute(u8"disabled"))
      {
        bool isInFirstLegend = previousAncestor && Is<HTMLLegendElement>(*previousAncestor)
                               && previousAncestor == fieldset->Legend();
        return !isInFirstLegend;
      }
      previousAncestor = ancestor;
    }
    return false;
  }

  void ValidatedFormListedElement::WillChangeForm() noexcept
  {
    if (auto *form = Form())
    {
      form->RemoveInvalidFormControlIfNeeded(AsHTMLElement());
    }
    FormListedElement::WillChangeForm();
  }

  void ValidatedFormListedElement::DidChangeForm() noexcept
  {
    FormListedElement::DidChangeForm();
    if (auto *form = Form())
    {
      if (_willValidateInitialized && _willValidate && !IsValidFormControlElement())
      {
        form->AddInvalidFormControl(AsHTMLElement());
      }
    }
  }

  void ValidatedFormListedElement::FormWillBeDestroyed() noexcept
  {
    _belongsToFormThatIsBeingDestroyed = true;
    FormListedElement::FormWillBeDestroyed();
    _belongsToFormThatIsBeingDestroyed = false;
  }

  void ValidatedFormListedElement::DisabledStateChanged() noexcept
  {
    UpdateWillValidateAndValidity();
  }

  void ValidatedFormListedElement::ReadOnlyStateChanged() noexcept
  {
    UpdateWillValidateAndValidity();
  }

  void ValidatedFormListedElement::UpdateWillValidateAndValidity() noexcept
  {
    // WillValidate is recomputed at once, as its changes can change style.
    bool newWillValidate = ComputeWillValidate();
    if (_willValidateInitialized && _willValidate == newWillValidate)
    {
      return;
    }

    bool wasValid = _isValid;

    _willValidateInitialized = true;
    _willValidate = newWillValidate;

    UpdateValidity();

    if (!_willValidate && !wasValid)
    {
      auto &element = AsHTMLElement();
      RemoveInvalidElementToAncestorFromInsertionPoint(element, element.ParentNode());
      if (auto *form = Form())
      {
        form->RemoveInvalidFormControlIfNeeded(element);
      }
    }

    if (!_willValidate)
    {
      HideVisibleValidationMessage();
    }
  }

  void ValidatedFormListedElement::DidFinishInsertingNode() noexcept
  {
    ResetFormOwner();
  }

  void ValidatedFormListedElement::SetCustomValidity(dom::DOMStringView error) noexcept
  {
    FormListedElement::SetCustomValidity(error);
    UpdateValidity();
  }

  void ValidatedFormListedElement::EndDelayingUpdateValidity() noexcept
  {
    krys_debug_assert(_delayedUpdateValidityCount);
    if (!--_delayedUpdateValidityCount)
    {
      UpdateValidity();
    }
  }

  bool ValidatedFormListedElement::IsCandidateForSavingAndRestoringState() const noexcept
  {
    // We don't save/restore control state in a form with autocomplete=off.
    return ShouldSaveAndRestoreFormControlState() && AsHTMLElement().IsConnected() && ShouldAutocomplete();
  }

  FormControlState ValidatedFormListedElement::SaveFormControlState() const noexcept
  {
    return {};
  }

  void ValidatedFormListedElement::RestoreFormControlStateIfNecessary() noexcept
  {
    AsHTMLElement().NodeDocument().GetFormController().RestoreControlStateFor(*this);
  }

  bool ValidatedFormListedElement::ShouldAutocomplete() const noexcept
  {
    if (!Form())
    {
      return true;
    }
    return Form()->ShouldAutocomplete();
  }

  bool ValidatedFormListedElement::MatchesValidPseudoClass() const noexcept
  {
    return WillValidate() && IsValidFormControlElement();
  }

  bool ValidatedFormListedElement::MatchesInvalidPseudoClass() const noexcept
  {
    return WillValidate() && !IsValidFormControlElement();
  }

  bool ValidatedFormListedElement::MatchesUserInvalidPseudoClass() const noexcept
  {
    return _wasInteractedWithSinceLastFormSubmitEvent && MatchesInvalidPseudoClass();
  }

  bool ValidatedFormListedElement::MatchesUserValidPseudoClass() const noexcept
  {
    return _wasInteractedWithSinceLastFormSubmitEvent && MatchesValidPseudoClass();
  }

  void ValidatedFormListedElement::SetInteractedWithSinceLastFormSubmitEvent(bool interactedWith) noexcept
  {
    if (_wasInteractedWithSinceLastFormSubmitEvent == interactedWith)
    {
      return;
    }

    style::PseudoClassChangeInvalidation styleInvalidation(
      AsHTMLElement(), {
                         {PseudoClass::UserValid, interactedWith && MatchesValidPseudoClass()},
                         {PseudoClass::UserInvalid, interactedWith && MatchesInvalidPseudoClass()},
                       });

    _wasInteractedWithSinceLastFormSubmitEvent = interactedWith;
  }

  DelayedUpdateValidityScope::DelayedUpdateValidityScope(ValidatedFormListedElement &element) noexcept
      : _element(element), _protectedElement(ShareRef(element.AsHTMLElement()))
  {
    _element.StartDelayingUpdateValidity();
  }
}
