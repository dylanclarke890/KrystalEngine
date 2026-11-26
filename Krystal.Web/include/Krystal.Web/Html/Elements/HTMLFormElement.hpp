#pragma once

#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Web/Dom/Types/USVString.hpp"
#include "Krystal.Web/Html/Elements/HTMLElement.hpp"
#include "Krystal.Web/Html/Forms/RadioButtonGroups.hpp"
#include "Krystal.Web/Loader/FormSubmission.hpp"

namespace krys::web::html
{
  class DOMFormData;
  class FormAssociatedElement;
  class FormListedElement;
  class HTMLFormControlsCollection;
  class RadioNodeList;
  enum class RadioNodeListCandidates : uint8;
  class ValidatedFormListedElement;
}

namespace krys::web
{
  class Event;
  class HTMLFormControlElement;
  class HTMLImageElement;

  /// @see https://html.spec.whatwg.org/multipage/forms.html#htmlformelement
  class HTMLFormElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLFormElement);

  private:
    UniquePtr<DOMTokenList> _relList;
    WeakPtr<HTMLCollection> _elements;
    FormSubmission::Attributes _attributes;
    HashMap<DOMStringAtom, WeakPtr<HTMLElement>> _pastNamesMap;
    html::RadioButtonGroups _radioButtonGroups;
    mutable WeakPtr<HTMLFormControlElement> _defaultButton;
    List<WeakPtr<HTMLElement>> _listedElements;
    /// @brief The form's RadioNodeLists by name, which remove themselves as they're destroyed.
    HashMap<DOMString, WeakPtr<NodeList>> _radioNodeLists;
    /// @brief The same for images, which are candidates only if no listed element is.
    HashMap<DOMString, WeakPtr<NodeList>> _imageRadioNodeLists;
    /// @brief The images whose form owner this is, in no particular order.
    List<WeakPtr<HTMLImageElement>> _imageElements;
    List<WeakPtr<HTMLElement>> _invalidFormControls;
    WeakPtr<FormSubmission> _plannedFormSubmission;
    uint32 _listedElementsBeforeIndex {0};
    uint32 _listedElementsAfterIndex {0};
    bool _isSubmittingOrPreparingForSubmission {false};
    bool _shouldSubmit {false};
    bool _isInResetFunction {false};
    bool _isConstructingEntryList {false};

  public:
    HTMLFormElement(Document &document) noexcept;

    ~HTMLFormElement() noexcept override;

#pragma region HTMLFormElement - https://html.spec.whatwg.org/multipage/forms.html#htmlformelement

    /// @see https://html.spec.whatwg.org/multipage/forms.html#dom-form-acceptcharset
    KRYS_NODISCARD DOMString AcceptCharset() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/forms.html#dom-form-acceptcharset
    void AcceptCharset(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-action
    KRYS_NODISCARD dom::USVString Action() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-action
    void Action(dom::USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/forms.html#dom-form-autocomplete
    KRYS_NODISCARD DOMString Autocomplete() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/forms.html#dom-form-autocomplete
    void Autocomplete(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-enctype
    KRYS_NODISCARD DOMString EncType() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-enctype
    void EncType(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-encoding
    KRYS_NODISCARD DOMString Encoding() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-encoding
    void Encoding(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-method
    KRYS_NODISCARD DOMString Method() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-method
    void Method(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/forms.html#dom-form-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/forms.html#dom-form-name
    void Name(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-novalidate
    KRYS_NODISCARD bool NoValidate() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-novalidate
    void NoValidate(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-target
    KRYS_NODISCARD DOMString Target() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-target
    void Target(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/forms.html#dom-form-rel
    KRYS_NODISCARD DOMString Rel() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/forms.html#dom-form-rel
    void Rel(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/forms.html#dom-form-rellist
    KRYS_NODISCARD DOMTokenList &RelList() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/forms.html#dom-form-elements
    KRYS_NODISCARD Ref<html::HTMLFormControlsCollection> Elements() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/forms.html#dom-form-length
    KRYS_NODISCARD size_t Length() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/forms.html#dom-form-item
    KRYS_NODISCARD RefPtr<HTMLElement> Item(size_t index) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/forms.html#dom-form-nameditem
    KRYS_NODISCARD Maybe<Variant<RefPtr<html::RadioNodeList>, RefPtr<Element>>>
      NamedItem(DOMStringView name) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/forms.html#dom-form-submit
    void Submit() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/forms.html#dom-form-requestsubmit
    ExceptionOr<void> RequestSubmit(HTMLElement *submitter = nullptr) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/forms.html#dom-form-reset
    void Reset() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/forms.html#dom-form-checkvalidity
    bool CheckValidity() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/forms.html#dom-form-reportvalidity
    bool ReportValidity() noexcept;

#pragma endregion

    /// @brief The elements of the form named @p name, falling back to the one last found by that name.
    KRYS_NODISCARD List<Ref<Element>> NamedElements(DOMStringView name) noexcept;

    /// @brief Whether @p name names one of the form's elements or images, now or in its past names map.
    /// @see https://html.spec.whatwg.org/multipage/forms.html#the-form-element:supported-property-names
    KRYS_NODISCARD bool IsSupportedPropertyName(DOMStringView name) noexcept;

    /// @brief Whether the form's `autocomplete` isn't `off`.
    KRYS_NODISCARD bool ShouldAutocomplete() const noexcept;

    /// @brief Adds or removes an image whose form owner the form is, for named access.
    void RegisterImageElement(HTMLImageElement &element) noexcept;

    void UnregisterImageElement(HTMLImageElement &element) noexcept;

    /// @brief The images whose form owner this is with an ID or name of @p name, in tree order.
    KRYS_NODISCARD List<Ref<Element>> NamedImages(DOMStringView name) const noexcept;

    /// @brief Adds a listed element whose form owner the form becomes, at its place in tree order; or removes
    /// one.
    void RegisterFormListedElement(html::FormListedElement &element) noexcept;

    void UnregisterFormListedElement(html::FormListedElement &element) noexcept;

    /// @brief Records an invalid control of the form, or forgets one, restyling the form's :valid and
    /// :invalid as the first comes or the last goes.
    void AddInvalidFormControl(const HTMLElement &formControlElement) noexcept;

    void RemoveInvalidFormControlIfNeeded(const HTMLElement &formControlElement) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#form-submission-algorithm
    void SubmitIfPossible(
      Event *event, HTMLFormControlElement *submitter = nullptr,
      FormSubmissionTrigger trigger = FormSubmissionTrigger::NotSubmittedByJavaScript) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#implicit-submission
    void SubmitImplicitly(Event &event, bool fromImplicitSubmissionTrigger) noexcept;

    /// @brief The browsing context name the form submits to: the submitter's `formtarget`, the form's
    /// `target`, or the document's base target, with `_blank` for a target with dangling markup.
    /// @see https://html.spec.whatwg.org/#get-an-element's-target
    KRYS_NODISCARD DOMString EffectiveTarget(const Event *event,
                                             HTMLFormControlElement *submitter) const noexcept;

    /// @brief The form control the event that submits the form was targeted at, if any.
    KRYS_NODISCARD static HTMLFormControlElement *FindSubmitter(const Event *event) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#default-button
    KRYS_NODISCARD HTMLFormControlElement *DefaultButton() const noexcept;

    /// @brief Finds the default button again, as a submit button was added, removed or changed; restyles the
    /// old and new ones' :default.
    void ResetDefaultButton() noexcept;

    /// @brief The radio button groups of the form's radio buttons.
    KRYS_NODISCARD html::RadioButtonGroups &RadioButtonGroups() noexcept
    {
      return _radioButtonGroups;
    }

    /// @brief The form's listed elements (or only those that take part in validation), held for the caller,
    /// which may run events that change the list.
    KRYS_NODISCARD List<Ref<HTMLElement>> CopyListedElementsVector() const noexcept;

    KRYS_NODISCARD List<Ref<HTMLElement>> CopyValidatedListedElementsVector() const noexcept;

    /// @brief The nearest form ancestor of @p element, if any.
    KRYS_NODISCARD static HTMLFormElement *FindClosestFormAncestor(const Element &element) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#constructing-the-form-data-set
    /// @return The entry list, or null if the form is already constructing one.
    KRYS_NODISCARD RefPtr<html::DOMFormData>
      ConstructEntryList(const RefPtr<HTMLFormControlElement> &submitter,
                         const Ref<html::DOMFormData> &domFormData,
                         List<Pair<DOMString, DOMString>> *formValues) noexcept;

    /// @brief The form's submission attributes: action, method, enctype, target and the rest.
    KRYS_NODISCARD const FormSubmission::Attributes &Attributes() const noexcept
    {
      return _attributes;
    }

    /// @brief The form's listed elements, in tree order.
    KRYS_NODISCARD const List<WeakPtr<HTMLElement>> &ListedElements() const noexcept
    {
      return _listedElements;
    }

    /// @brief Forgets the RadioNodeList for @p name, as it's being destroyed.
    void RadioNodeListDestroyed(DOMStringView name, html::RadioNodeListCandidates candidates) noexcept;

    /// @brief The RadioNodeList of the form's listed elements (or images) named @p name, made if there's
    /// none.
    KRYS_NODISCARD Ref<html::RadioNodeList> RadioNodeListForName(DOMStringView name) noexcept;

    KRYS_NODISCARD Ref<html::RadioNodeList>
      RadioNodeListForName(DOMStringView name, html::RadioNodeListCandidates candidates) noexcept;

    KRYS_NODISCARD bool MatchesValidPseudoClass() const noexcept final;

    KRYS_NODISCARD bool MatchesInvalidPseudoClass() const noexcept final;

  protected:
    void OnInsert() noexcept override;

    void OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept override;

    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept override;

  private:
    /// @brief Submits the form, validating it interactively and firing submit first, unless script submits
    /// it.
    /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#form-submission-algorithm
    void Submit(Event *event, FormSubmissionTrigger trigger,
                HTMLFormControlElement *submitter = nullptr) noexcept;

    /// @brief Closes the form's open dialog ancestor, for a submission whose method is dialog.
    void SubmitDialog(const Ref<FormSubmission> &formSubmission) noexcept;

    /// @brief Where an element whose form attribute names the form goes in the listed elements, found by a
    /// binary search of [@p rangeStart, @p rangeEnd).
    KRYS_NODISCARD uint32 FormElementIndexWithFormAttribute(Element &element, uint32 rangeStart,
                                                            uint32 rangeEnd) noexcept;

    /// @brief Where @p listedElement goes in the listed elements, keeping them in tree order.
    KRYS_NODISCARD uint32 FormElementIndex(html::FormListedElement &listedElement) noexcept;

    /// @brief Validates the form, reporting the first invalid control to the user (focusing it).
    /// @return Whether the form is valid.
    /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#interactively-validate-the-constraints
    bool ValidateInteractively() noexcept;

    /// @brief Checks each control's validity, storing those whose `invalid` event wasn't canceled.
    /// @return Whether any control is invalid.
    bool CheckInvalidControlsAndCollectUnhandled(List<Ref<HTMLElement>> &unhandledInvalidControls) noexcept;

    /// @brief The past names map: the element last found by a name, kept while it's still the form's.
    /// @see https://html.spec.whatwg.org/multipage/forms.html#past-names-map
    KRYS_NODISCARD HTMLElement *ElementFromPastNamesMap(DOMStringView pastName) const noexcept;

    void AddToPastNamesMap(html::FormAssociatedElement &element, DOMStringView pastName) noexcept;

    void RemoveFromPastNamesMap(html::FormAssociatedElement &item) noexcept;

    /// @brief Resets each of the form's controls, as the form is reset.
    void ResetListedFormControlElements() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLFormElement)

  KRYS_NODISCARD static bool IsType(const krys::web::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::web::HTMLElement>(target).IsHTMLFormElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLFormElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
