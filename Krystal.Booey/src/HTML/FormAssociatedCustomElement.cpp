#include "Krystal.Booey/HTML/FormAssociatedCustomElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/DOMFormData.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/HTMLMaybeFormAssociatedCustomElement.hpp"

namespace krys::boo::html
{
  namespace
  {
    CustomElementFormValue CloneIfIsFormData(CustomElementFormValue &&value) noexcept
    {
      if (auto *formData = std::get_if<RefPtr<DOMFormData>>(&value); formData && *formData)
      {
        return RefPtr<DOMFormData>((*formData)->Clone());
      }
      return krys::move(value);
    }
  }

  FormAssociatedCustomElement::FormAssociatedCustomElement(
    HTMLMaybeFormAssociatedCustomElement &element) noexcept
      : _element(element)
  {
  }

  FormAssociatedCustomElement::~FormAssociatedCustomElement() noexcept
  {
    ClearForm();
  }

  HTMLElement &FormAssociatedCustomElement::AsHTMLElement() noexcept
  {
    return _element;
  }

  const HTMLElement &FormAssociatedCustomElement::AsHTMLElement() const noexcept
  {
    return _element;
  }

  dom::ExceptionOr<void> FormAssociatedCustomElement::SetValidity(ValidityStateFlags flags,
                                                                  dom::DOMString &&message,
                                                                  HTMLElement *validationAnchor) noexcept
  {
    krys_debug_assert(_element.IsPrecustomizedOrDefinedCustomElement());

    if (!flags.IsValid() && message.empty())
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    _validityStateFlags = flags;
    SetCustomValidity(flags.IsValid() ? dom::DOMStringView() : dom::DOMStringView(message));

    if (validationAnchor
        && !dom::TreeQueries::IsShadowIncludingInclusiveAncestor(_element, *validationAnchor))
    {
      return dom::Exception {dom::ExceptionCode::NotFoundError};
    }

    _validationAnchor = CreateWeakPtr(validationAnchor);

    return {};
  }

  dom::DOMString FormAssociatedCustomElement::ValidationMessage() const noexcept
  {
    krys_debug_assert(_element.IsPrecustomizedOrDefinedCustomElement());
    return CustomValidationMessage();
  }

  void FormAssociatedCustomElement::SetFormValue(CustomElementFormValue &&submissionValue,
                                                 Maybe<CustomElementFormValue> &&state) noexcept
  {
    krys_debug_assert(_element.IsPrecustomizedOrDefinedCustomElement());

    _submissionValue = CloneIfIsFormData(krys::move(submissionValue));
    _state = state.has_value() ? CloneIfIsFormData(krys::move(*state)) : _submissionValue;
  }

  HTMLElement *FormAssociatedCustomElement::ValidationAnchorElement() noexcept
  {
    krys_debug_assert(_element.IsDefinedCustomElement());
    auto *anchor = _validationAnchor.get();
    return anchor ? anchor : &_element;
  }

  bool FormAssociatedCustomElement::ComputeValidity() const noexcept
  {
    krys_debug_assert(_element.IsPrecustomizedOrDefinedCustomElement());
    return _validityStateFlags.IsValid();
  }

  bool FormAssociatedCustomElement::AppendFormData(DOMFormData &formData) noexcept
  {
    krys_debug_assert(_element.IsDefinedCustomElement());

    if (auto *entries = std::get_if<RefPtr<DOMFormData>>(&_submissionValue))
    {
      if (*entries)
      {
        for (auto &item : (*entries)->Items())
        {
          formData.Append(item.Name, item.Data);
        }
      }
    }
    else if (auto *value = std::get_if<dom::DOMString>(&_submissionValue))
    {
      if (auto name = Name(); !name.empty())
      {
        formData.Append(name, *value);
      }
    }

    return true;
  }

  bool FormAssociatedCustomElement::IsEnumeratable() const noexcept
  {
    krys_debug_assert(_element.IsDefinedCustomElement());
    return true;
  }

  void FormAssociatedCustomElement::Reset() noexcept
  {
    krys_debug_assert(_element.IsDefinedCustomElement());
    SetInteractedWithSinceLastFormSubmitEvent(false);
    CustomElementReactionQueue::EnqueueFormResetCallbackIfNeeded(_element);
  }

  void FormAssociatedCustomElement::DisabledStateChanged() noexcept
  {
    krys_debug_assert(_element.IsDefinedCustomElement());
    ValidatedFormListedElement::DisabledStateChanged();
    CustomElementReactionQueue::EnqueueFormDisabledCallbackIfNeeded(_element, IsDisabled());
  }

  void FormAssociatedCustomElement::DidChangeForm() noexcept
  {
    krys_debug_assert(_element.IsDefinedCustomElement());
    ValidatedFormListedElement::DidChangeForm();
    if (!BelongsToFormThatIsBeingDestroyed())
    {
      CustomElementReactionQueue::EnqueueFormAssociatedCallbackIfNeeded(_element, Form());
    }
  }

  void FormAssociatedCustomElement::WillUpgrade() noexcept
  {
    SetDataListAncestorState(TriState::False);
  }

  void FormAssociatedCustomElement::DidUpgrade() noexcept
  {
    krys_debug_assert(!Form());

    ParseFormAttribute(_element.GetAttribute(u8"form"));
    ParseDisabledAttribute(_element.GetAttribute(u8"disabled"));
    ParseReadOnlyAttribute(_element.GetAttribute(u8"readonly"));

    SetDataListAncestorState(TriState::Indeterminate);
    UpdateWillValidateAndValidity();
    SyncWithFieldsetAncestors(_element.ParentNode());
    // FIXME: Restore the control's state, once there's form state restoration.
  }

  dom::DOMString FormAssociatedCustomElement::FormControlType() const noexcept
  {
    return dom::DOMString(_element.LocalName().View());
  }
}
