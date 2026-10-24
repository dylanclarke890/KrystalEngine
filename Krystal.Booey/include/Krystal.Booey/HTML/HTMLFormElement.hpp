#pragma once

#include "Krystal.Booey/DOM/RadioButtonGroups.hpp"
#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Loader/FormSubmission.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Event;
}

namespace krys::boo::html
{
  class DOMFormData;
  class FormAssociatedElement;
  class FormListedElement;
  class HTMLFormControlElement;
  class HTMLFormControlsCollection;
  class RadioNodeList;
  class ValidatedFormListedElement;

  /// @see https://html.spec.whatwg.org/#htmlformelement
  class HTMLFormElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLFormElement);

  private:
    UniquePtr<dom::DOMTokenList> _relList;
    FormSubmission::Attributes _attributes;
    HashMap<dom::DOMStringAtom, WeakPtr<HTMLElement>> _pastNamesMap;
    dom::RadioButtonGroups _radioButtonGroups;
    mutable WeakPtr<HTMLFormControlElement> _defaultButton;
    List<WeakPtr<HTMLElement>> _listedElements;
    List<WeakPtr<HTMLElement>> _invalidFormControls;
    WeakPtr<FormSubmission> _plannedFormSubmission;
    uint32 _listedElementsBeforeIndex {0};
    uint32 _listedElementsAfterIndex {0};
    bool _wasUserSubmitted {false};
    bool _isSubmittingOrPreparingForSubmission {false};
    bool _shouldSubmit {false};
    bool _isInResetFunction {false};
    bool _isConstructingEntryList {false};

  public:
    HTMLFormElement(dom::Document &document) noexcept;
    ~HTMLFormElement() noexcept override;

    void FinishParsingChildren() noexcept final;

#pragma region HTMLFormElement - https://html.spec.whatwg.org/#htmlformelement

    /// @see https://html.spec.whatwg.org/#dom-form-acceptcharset
    KRYS_NODISCARD dom::DOMString AcceptCharset() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-acceptcharset
    void AcceptCharset(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-action
    KRYS_NODISCARD dom::USVString Action() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-action
    void Action(dom::USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-autocomplete
    KRYS_NODISCARD dom::DOMString Autocomplete() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-autocomplete
    void Autocomplete(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-enctype
    KRYS_NODISCARD dom::DOMString EncType() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-enctype
    void EncType(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-encoding
    KRYS_NODISCARD dom::DOMString Encoding() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-encoding
    void Encoding(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-method
    KRYS_NODISCARD dom::DOMString Method() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-method
    void Method(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-name
    KRYS_NODISCARD dom::DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-name
    void Name(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-novalidate
    KRYS_NODISCARD bool NoValidate() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-novalidate
    void NoValidate(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-target
    KRYS_NODISCARD dom::DOMString Target() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-target
    void Target(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-rel
    KRYS_NODISCARD dom::DOMString Rel() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-rel
    void Rel(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-rellist
    KRYS_NODISCARD dom::DOMTokenList &RelList() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-elements
    KRYS_NODISCARD Ref<HTMLFormControlsCollection> Elements() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-length
    KRYS_NODISCARD size_t Length() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-item
    KRYS_NODISCARD RefPtr<HTMLElement> Item(size_t index) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-nameditem
    KRYS_NODISCARD Maybe<Variant<RefPtr<RadioNodeList>, RefPtr<dom::Element>>>
      NamedItem(dom::DOMStringView name) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-submit
    void Submit() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-requestsubmit
    dom::ExceptionOr<void> RequestSubmit(HTMLElement *submitter = nullptr) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-reset
    void Reset() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-checkvalidity
    bool CheckValidity() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-reportvalidity
    bool ReportValidity() noexcept;

#pragma endregion

    /// @brief The elements of the form named @p name, falling back to the one last found by that name.
    KRYS_NODISCARD List<Ref<dom::Element>> NamedElements(dom::DOMStringView name) noexcept;

    KRYS_NODISCARD bool IsSupportedPropertyName(dom::DOMStringView name) noexcept;

    KRYS_NODISCARD bool ShouldAutocomplete() const noexcept;

    void RegisterFormListedElement(FormListedElement &element) noexcept;
    void UnregisterFormListedElement(FormListedElement &element) noexcept;

    void AddInvalidFormControl(const HTMLElement &formControlElement) noexcept;
    void RemoveInvalidFormControlIfNeeded(const HTMLElement &formControlElement) noexcept;

    /// @see https://html.spec.whatwg.org/#form-submission-algorithm
    void SubmitIfPossible(
      dom::Event *event, HTMLFormControlElement *submitter = nullptr,
      FormSubmissionTrigger trigger = FormSubmissionTrigger::NotSubmittedByJavaScript) noexcept;

    /// @brief Submits the form as script's `submit()` does.
    void SubmitFromJavaScript() noexcept;

    /// @see https://html.spec.whatwg.org/#implicit-submission
    void SubmitImplicitly(dom::Event &event, bool fromImplicitSubmissionTrigger) noexcept;

    KRYS_NODISCARD bool IsMethodPost() const noexcept
    {
      return _attributes.GetMethod() == FormSubmission::Method::Post;
    }

    /// @brief The browsing context name the form submits to: the submitter's `formtarget`, the form's
    /// `target`, or the document's base target.
    KRYS_NODISCARD dom::DOMString EffectiveTarget(const dom::Event *event,
                                                  HTMLFormControlElement *submitter) const noexcept;

    KRYS_NODISCARD bool WasUserSubmitted() const noexcept;

    /// @brief The form control the event that submits the form was targeted at, if any.
    KRYS_NODISCARD HTMLFormControlElement *FindSubmitter(const dom::Event *event) const noexcept;

    /// @see https://html.spec.whatwg.org/#default-button
    KRYS_NODISCARD HTMLFormControlElement *DefaultButton() const noexcept;
    void ResetDefaultButton() noexcept;

    KRYS_NODISCARD dom::RadioButtonGroups &RadioButtonGroups() noexcept
    {
      return _radioButtonGroups;
    }

    KRYS_NODISCARD List<Ref<HTMLElement>> CopyListedElementsVector() const noexcept;
    KRYS_NODISCARD List<Ref<HTMLElement>> CopyValidatedListedElementsVector() const noexcept;

    /// @brief The names and values of the form's text fields.
    KRYS_NODISCARD List<Pair<dom::DOMString, dom::DOMString>> TextFieldValues() const noexcept;

    KRYS_NODISCARD static HTMLFormElement *FindClosestFormAncestor(const dom::Element &element) noexcept;

    /// @see https://html.spec.whatwg.org/#constructing-the-form-data-set
    /// @return The entry list, or null if the form is already constructing one.
    KRYS_NODISCARD RefPtr<DOMFormData>
      ConstructEntryList(RefPtr<HTMLFormControlElement> &&submitter, Ref<DOMFormData> &&domFormData,
                         List<Pair<dom::DOMString, dom::DOMString>> *formValues) noexcept;

    KRYS_NODISCARD const FormSubmission::Attributes &Attributes() const noexcept
    {
      return _attributes;
    }

    /// @brief The form's elements with the ID or name @p name.
    KRYS_NODISCARD Ref<RadioNodeList> RadioNodeListForName(dom::DOMStringView name) noexcept;

    KRYS_NODISCARD bool MatchesValidPseudoClass() const noexcept final;
    KRYS_NODISCARD bool MatchesInvalidPseudoClass() const noexcept final;

  protected:
    void OnInsert() noexcept override;
    void OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept override;
    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value,
                            dom::DOMStringAtom namespaceURI) noexcept override;

  private:
    void Submit(dom::Event *event, bool processingUserGesture, FormSubmissionTrigger trigger,
                HTMLFormControlElement *submitter = nullptr) noexcept;
    void SubmitDialog(Ref<FormSubmission> &&formSubmission) noexcept;

    KRYS_NODISCARD uint32 FormElementIndexWithFormAttribute(dom::Element &element, uint32 rangeStart,
                                                            uint32 rangeEnd) noexcept;
    KRYS_NODISCARD uint32 FormElementIndex(FormListedElement &listedElement) noexcept;

    bool ValidateInteractively() noexcept;

    /// @brief Checks each control's validity, storing those whose `invalid` event wasn't canceled.
    /// @return Whether any control is invalid.
    bool CheckInvalidControlsAndCollectUnhandled(List<Ref<HTMLElement>> &unhandledInvalidControls) noexcept;

    KRYS_NODISCARD HTMLElement *ElementFromPastNamesMap(dom::DOMStringView pastName) const noexcept;
    void AddToPastNamesMap(FormAssociatedElement &element, dom::DOMStringView pastName) noexcept;
    void RemoveFromPastNamesMap(FormAssociatedElement &element) noexcept;

    void ResetListedFormControlElements() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLFormElement)

  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLFormElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLFormElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
