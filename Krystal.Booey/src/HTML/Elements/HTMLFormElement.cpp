#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventTargetAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Collections/DOMTokenList.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Events/FormDataEvent.hpp"
#include "Krystal.Booey/DOM/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/Attributes/EnumeratedAttributes.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/Collections/HTMLFormControlsCollection.hpp"
#include "Krystal.Booey/HTML/Collections/RadioNodeList.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDataListElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDialogElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormControlElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Events/SubmitEvent.hpp"
#include "Krystal.Booey/HTML/Forms/DOMFormData.hpp"
#include "Krystal.Booey/Page/Frame.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Style/Invalidation/PseudoClassChangeInvalidation.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"
#include <algorithm>
#include <numeric>

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/multipage/forms.html#attr-form-autocomplete
  template <>
  struct html::EnumeratedAttributeTraits<"autocomplete", HTMLFormElement>
  {
    using State = html::AutocompleteAttributeState;
    static inline const html::KeywordStateMapping<State> StateMappings[] = {{u8"on", State::On},
                                                                            {u8"off", State::Off}};

    constexpr static State MissingValueDefault = State::On;
    constexpr static State InvalidValueDefault = State::On;
  };

  /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#attr-fs-method
  template <>
  struct html::EnumeratedAttributeTraits<"method", HTMLFormElement>
  {
    using State = html::MethodAttributeState;
    static inline const html::KeywordStateMapping<State> StateMappings[] = {
      {u8"get", State::Get},
      {u8"post", State::Post},
      {u8"dialog", State::Dialog},
    };

    constexpr static State MissingValueDefault = State::Get;
    constexpr static State InvalidValueDefault = State::Get;
  };

  /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#attr-fs-enctype
  template <>
  struct html::EnumeratedAttributeTraits<"enctype", HTMLFormElement>
  {
    using State = html::EnctypeAttributeState;
    static inline const html::KeywordStateMapping<State> StateMappings[] = {
      {u8"application/x-www-form-urlencoded", State::UrlEncoded},
      {u8"multipart/form-data", State::FormData},
      {u8"text/plain", State::TextPlain},
    };

    constexpr static State MissingValueDefault = State::UrlEncoded;
    constexpr static State InvalidValueDefault = State::UrlEncoded;
  };

  namespace
  {
    /// @brief `_blank` in place of a target that looks like the start of dangling markup.
    KRYS_NODISCARD DOMString MakeTargetBlankIfHasDanglingMarkup(DOMStringView target) noexcept
    {
      auto contains = [&](char8 character)
      {
        return target.contains(character);
      };
      if ((contains(u8'\n') || contains(u8'\r') || contains(u8'\t')) && contains(u8'<'))
      {
        return u8"_blank";
      }
      return DOMString(target);
    }
  }

  HTMLFormElement::HTMLFormElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Form)
  {
  }

  HTMLFormElement::~HTMLFormElement() noexcept
  {
    // FormWillBeDestroyed below would try to update the validity of every radio button in a group.
    _radioButtonGroups.Clear();
    _defaultButton = nullptr;
    for (auto &weakElement : _listedElements)
    {
      auto *element = weakElement.get();
      krys_debug_assert(element);
      if (element)
      {
        element->AsFormListedElement()->FormWillBeDestroyed();
      }
    }
    for (auto &image : _imageElements)
    {
      if (auto *element = image.get())
      {
        element->FormWillBeDestroyed();
      }
    }
  }

#pragma region HTMLFormElement

  DOMString HTMLFormElement::AcceptCharset() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"accept-charset").value_or(u8"");
  }

  void HTMLFormElement::AcceptCharset(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"accept-charset", krys::move(value));
  }

  dom::USVString HTMLFormElement::Action() const noexcept
  {
    // https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#dom-fs-action
    // Let attributeValue be the result of running get an attribute value given this and action. If
    // attributeValue is null or the empty string, return this's node document's URL, serialized.
    auto attributeValue = GetAttribute(u8"action");
    if (!attributeValue || attributeValue->empty())
    {
      return dom::USVString(NodeDocument().GetUrl().String());
    }
    // Let urlString be the result of encoding-parsing-and-serializing a URL given attributeValue, relative to
    // this's node document. If urlString is not failure, return urlString.
    if (auto url = NodeDocument().CompleteUrl(*attributeValue); url.IsValid())
    {
      return dom::USVString(url.String());
    }
    // Return attributeValue, converted to a scalar value string.
    return dom::USVString(krys::move(*attributeValue));
  }

  void HTMLFormElement::Action(dom::USVString value) noexcept
  {
    return html::ReflectSetter<dom::USVString>(*this, u8"action", krys::move(value));
  }

  DOMString HTMLFormElement::Autocomplete() const noexcept
  {
    return html::ReflectEnumerated<"autocomplete", HTMLFormElement>(*this);
  }

  void HTMLFormElement::Autocomplete(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"autocomplete", krys::move(value));
  }

  DOMString HTMLFormElement::EncType() const noexcept
  {
    return html::ReflectEnumerated<"enctype", HTMLFormElement>(*this);
  }

  void HTMLFormElement::EncType(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"enctype", krys::move(value));
  }

  // https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#dom-fs-encoding
  DOMString HTMLFormElement::Encoding() const noexcept
  {
    // The encoding IDL attribute must reflect the enctype content attribute, limited to only known values.
    return EncType();
  }

  void HTMLFormElement::Encoding(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"enctype", krys::move(value));
  }

  DOMString HTMLFormElement::Method() const noexcept
  {
    return html::ReflectEnumerated<"method", HTMLFormElement>(*this);
  }

  void HTMLFormElement::Method(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"method", krys::move(value));
  }

  DOMString HTMLFormElement::Name() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLFormElement::Name(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"name", krys::move(value));
  }

  bool HTMLFormElement::NoValidate() const noexcept
  {
    return html::Reflect<bool>(*this, u8"novalidate");
  }

  void HTMLFormElement::NoValidate(bool value) noexcept
  {
    html::Reflect<bool>(*this, u8"novalidate", krys::move(value));
  }

  DOMString HTMLFormElement::Target() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"target").value_or(u8"");
  }

  void HTMLFormElement::Target(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"target", krys::move(value));
  }

  DOMString HTMLFormElement::Rel() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"rel").value_or(u8"");
  }

  void HTMLFormElement::Rel(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"rel", krys::move(value));
  }

  DOMTokenList &HTMLFormElement::RelList() noexcept
  {
    if (!_relList)
    {
      _relList = DOMTokenList::Create(*this, u8"rel",
                                      [](Document &, DOMStringView token)
                                      {
                                        return text::IsASCIICaselessEqual(token, u8"noreferrer")
                                               || text::IsASCIICaselessEqual(token, u8"noopener")
                                               || text::IsASCIICaselessEqual(token, u8"opener");
                                      });
    }

    return *_relList;
  }

  Ref<html::HTMLFormControlsCollection> HTMLFormElement::Elements() noexcept
  {
    if (auto *elements = _elements.get())
    {
      // Only ever an HTMLFormControlsCollection.
      return ShareRef(static_cast<html::HTMLFormControlsCollection &>(*elements));
    }
    auto elements = CreateRef<html::HTMLFormControlsCollection>(*this);
    _elements = CreateWeakPtr<HTMLCollection>(elements.get());
    return elements;
  }

  size_t HTMLFormElement::Length() const noexcept
  {
    size_t length = 0;
    for (auto &weakElement : _listedElements)
    {
      auto *element = weakElement.get();
      krys_debug_assert(element);
      if (element && element->AsFormListedElement()->IsEnumeratable())
      {
        ++length;
      }
    }
    return length;
  }

  RefPtr<HTMLElement> HTMLFormElement::Item(size_t index) noexcept
  {
    RefPtr<Element> item = Elements()->Item(index);
    return DynamicDowncast<HTMLElement>(krys::move(item));
  }

  Maybe<Variant<RefPtr<html::RadioNodeList>, RefPtr<Element>>>
    HTMLFormElement::NamedItem(DOMStringView name) noexcept
  {
    auto namedItems = NamedElements(name);
    if (namedItems.empty())
    {
      return std::nullopt;
    }
    if (namedItems.size() == 1)
    {
      return Variant<RefPtr<html::RadioNodeList>, RefPtr<Element>> {RefPtr<Element>(namedItems[0])};
    }
    // Images are candidates only if no listed element is.
    return Variant<RefPtr<html::RadioNodeList>, RefPtr<Element>> {RefPtr<html::RadioNodeList>(
      RadioNodeListForName(name, Elements()->NamedItems(name).empty()
                                   ? html::RadioNodeListCandidates::Images
                                   : html::RadioNodeListCandidates::ListedElements))};
  }

  void HTMLFormElement::Submit() noexcept
  {
    Submit(nullptr, FormSubmissionTrigger::NotSubmittedByJavaScript);
  }

  ExceptionOr<void> HTMLFormElement::RequestSubmit(HTMLElement *submitter) noexcept
  {
    // Update layout before processing form actions in case the style changes the form or button
    // relationships.
    (void)NodeDocument().UpdateLayoutIgnorePendingStylesheets();

    // 1. If submitter is not null:
    RefPtr<HTMLFormControlElement> control;
    if (submitter)
    {
      // 1a. If submitter is not a submit button, then throw a TypeError.
      control = ShareRefPtr(DynamicDowncast<HTMLFormControlElement>(*submitter));
      if (!control || !control->IsSubmitButton())
      {
        return Exception {ExceptionCode::TypeError, u8"The specified element is not a submit button."};
      }

      // 1b. If submitter's form owner is not this form element, then throw a "NotFoundError" DOMException.
      if (control->Form() != this)
      {
        return Exception {ExceptionCode::NotFoundError,
                          u8"The specified element is not owned by this form element."};
      }
    }

    // 2. Otherwise, set submitter to this form element.
    // (A null submitter is the form.)
    // 3. Submit this form element, from submitter.
    SubmitIfPossible(nullptr, control.get(), FormSubmissionTrigger::SubmittedByJavaScript);
    return {};
  }

  void HTMLFormElement::Reset() noexcept
  {
    html::CEReactionsScope ceReactions;

    // A reset during a reset (from its event) does nothing, nor does one in a document without a frame.
    if (_isInResetFunction || !NodeDocument().Frame())
    {
      return;
    }

    Ref<HTMLFormElement> protectedThis = ShareRef(*this);
    bool wasInResetFunction = std::exchange(_isInResetFunction, true);

    // https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#concept-form-reset
    // 1. Let reset be the result of firing an event named reset at form, with the bubbles and cancelable
    //    attributes initialized to true.
    auto event = Event::Create(event_names::Reset, true, true);
    (void)dom::EventDispatcher::DispatchToTarget(*event, this);

    // 2. If reset is true, then invoke the reset algorithm of each resettable element whose form owner is
    // form.
    if (!event->DefaultPrevented())
    {
      ResetListedFormControlElements();
    }

    _isInResetFunction = wasInResetFunction;
  }

  void HTMLFormElement::ResetListedFormControlElements() noexcept
  {
    // Event handling can add or remove listed elements while they're being reset, so the copy protects them
    // until all are told.
    for (auto &control : CopyValidatedListedElementsVector())
    {
      control->AsValidatedFormListedElement()->Reset();
    }
  }

  bool HTMLFormElement::CheckValidity() noexcept
  {
    List<Ref<HTMLElement>> controls;
    return !CheckInvalidControlsAndCollectUnhandled(controls);
  }

  // https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#statically-validate-the-constraints
  bool HTMLFormElement::CheckInvalidControlsAndCollectUnhandled(
    List<Ref<HTMLElement>> &unhandledInvalidControls) noexcept
  {
    // 1. Let controls be a list of all the submittable elements whose form owner is form, in tree order.
    // 2. Let invalid controls be an initially empty list of elements.
    // 3. For each element field in controls, in tree order: ...
    // 6. For each element field in invalid controls, if any, in tree order: ...
    // (Firing invalid at each invalid control in tree order as it's found.)
    Ref<HTMLFormElement> protectedThis = ShareRef(*this);
    // The listed elements are copied, as the `invalid` event handlers can change them.
    bool hasInvalidControls = false;
    for (auto &element : CopyValidatedListedElementsVector())
    {
      auto *control = element->AsValidatedFormListedElement();
      // CheckValidity() can fire events that change the DOM, hence checking the form twice.
      if (control->Form() == this && !control->CheckValidity(&unhandledInvalidControls)
          && control->Form() == this)
      {
        hasInvalidControls = true;
      }
    }
    return hasInvalidControls;
  }

  bool HTMLFormElement::ReportValidity() noexcept
  {
    Ref<HTMLFormElement> protectedThis = ShareRef(*this);

    // Update layout before processing form actions in case the style changes the form or button
    // relationships.
    (void)NodeDocument().UpdateLayoutIgnorePendingStylesheets();

    return ValidateInteractively();
  }

  // https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#interactively-validate-the-constraints
  bool HTMLFormElement::ValidateInteractively() noexcept
  {
    for (auto &element : CopyValidatedListedElementsVector())
    {
      element->AsValidatedFormListedElement()->HideVisibleValidationMessage();
    }

    // 1. Statically validate the constraints of form, and let unhandled invalid controls be the list of
    //    elements returned if the result was negative.
    // 2. If the result was positive, then return that result.
    List<Ref<HTMLElement>> unhandledInvalidControls;
    if (!CheckInvalidControlsAndCollectUnhandled(unhandledInvalidControls))
    {
      return true;
    }

    // 3. Report the problems with the constraints of at least one of the elements given in unhandled invalid
    //    controls to the user.

    // The form has invalid controls, so the submission is aborted and a validation message is shown on a
    // focusable control.
    Ref<HTMLFormElement> protectedThis = ShareRef(*this);

    // The first focusable invalid control is focused, with its validation message.
    bool shouldFocus = true;
    for (auto &element : unhandledInvalidControls)
    {
      auto *control = element->AsValidatedFormListedElement();
      if (auto *validationAnchor = control->FocusableValidationAnchorElement())
      {
        if (shouldFocus)
        {
          shouldFocus = false;
          control->FocusAndShowValidationMessage(*validationAnchor);
        }
      }
      else
      {
        control->ReportNonFocusableControlError();
      }
    }

    // 4. Return a negative result.
    return false;
  }

  void HTMLFormElement::SubmitImplicitly(Event &event, bool fromImplicitSubmissionTrigger) noexcept
  {
    uint32 submissionTriggerCount = 0;
    for (auto &element : CopyListedElementsVector())
    {
      auto *formElement = DynamicDowncast<HTMLFormControlElement>(*element);
      if (!formElement)
      {
        continue;
      }
      if (formElement->IsSuccessfulSubmitButton())
      {
        (void)formElement->DispatchSimulatedClick(&event);
        return;
      }
      if (formElement->CanTriggerImplicitSubmission())
      {
        ++submissionTriggerCount;
      }
    }

    if (!submissionTriggerCount)
    {
      return;
    }

    // Without a submit button, only a form with a single field that blocks implicit submission submits.
    if (fromImplicitSubmissionTrigger && submissionTriggerCount == 1)
    {
      SubmitIfPossible(&event);
    }
  }

  void HTMLFormElement::SubmitIfPossible(Event *event, HTMLFormControlElement *submitter,
                                         FormSubmissionTrigger trigger) noexcept
  {
    // https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#form-submission-algorithm
    if (!IsConnected())
    {
      return;
    }

    auto *frame = NodeDocument().Frame();
    if (_isSubmittingOrPreparingForSubmission || !frame)
    {
      return;
    }

    _isSubmittingOrPreparingForSubmission = true;
    _shouldSubmit = false;

    for (auto &element : CopyListedElementsVector())
    {
      if (auto *formControlElement = DynamicDowncast<HTMLFormControlElement>(*element))
      {
        formControlElement->SetInteractedWithSinceLastFormSubmitEvent(true);
      }
    }

    bool shouldValidate = NodeDocument().Frame()
                          && NodeDocument().GetSettings().InteractiveFormValidationEnabled && !NoValidate();
    if (shouldValidate)
    {
      auto *submitElement = submitter ? submitter : FindSubmitter(event);
      if (submitElement && submitElement->FormNoValidate())
      {
        shouldValidate = false;
      }
    }

    // Interactive validation comes before the submit event.
    if (shouldValidate && !ValidateInteractively())
    {
      _isSubmittingOrPreparingForSubmission = false;
      return;
    }

    Ref<HTMLFormElement> protectedThis = ShareRef(*this);

    // Let relatedTarget be form if the result of retargeting submitterButton against form is form; otherwise
    // submitterButton. Fire submit, composed, with relatedTarget set to relatedTarget.
    auto submitEvent = SubmitEvent::Create(ShareRefPtr(static_cast<HTMLElement *>(submitter)));
    (void)dom::EventDispatcher::DispatchFromSource(*submitEvent, *this, submitter);

    // Event handling could have resulted in _shouldSubmit becoming true as a side effect, too.
    if (!submitEvent->DefaultPrevented())
    {
      _shouldSubmit = true;
    }

    _isSubmittingOrPreparingForSubmission = false;

    if (!_shouldSubmit)
    {
      return;
    }

    if (auto plannedFormSubmission = std::exchange(_plannedFormSubmission, nullptr))
    {
      plannedFormSubmission->Cancel();
    }

    Submit(event, trigger, submitter);
  }

  void HTMLFormElement::Submit(Event *event, FormSubmissionTrigger trigger,
                               HTMLFormControlElement *submitter) noexcept
  {
    // SubmitIfPossible also checks this, but some paths bypass it.
    if (!IsConnected())
    {
      return;
    }

    if (_isConstructingEntryList)
    {
      return;
    }

    auto *frame = NodeDocument().Frame();
    if (!frame || !frame->View())
    {
      return;
    }

    if (trigger != FormSubmissionTrigger::SubmittedByJavaScript && _isSubmittingOrPreparingForSubmission)
    {
      _shouldSubmit = true;
      return;
    }

    _isSubmittingOrPreparingForSubmission = true;

    if (event && !submitter)
    {
      // A submit event handler might have added a submit button to a form submitted implicitly without one,
      // so it's searched for again.
      for (auto &element : CopyListedElementsVector())
      {
        if (auto *control = DynamicDowncast<HTMLFormControlElement>(*element);
            control && control->IsSuccessfulSubmitButton())
        {
          submitter = control;
          break;
        }
      }
    }

    Ref<HTMLFormElement> protectedThis = ShareRef(*this); // Form submission can execute arbitrary JavaScript.

    auto formSubmission = FormSubmission::Create(*this, submitter, _attributes, event, trigger);

    if (!IsConnected())
    {
      return;
    }

    _plannedFormSubmission = CreateWeakPtr(formSubmission.get());

    if (formSubmission->GetMethod() == FormSubmission::Method::Dialog)
    {
      SubmitDialog(krys::move(formSubmission));
    }
    else
    {
      frame->Loader().SubmitForm(krys::move(formSubmission));
    }

    _shouldSubmit = false;
    _isSubmittingOrPreparingForSubmission = false;
  }

  // A dialog method submission closes the form's nearest dialog ancestor, if it's open, with the
  // submission's return value.
  // https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#form-submission-algorithm
  void HTMLFormElement::SubmitDialog(const Ref<FormSubmission> &formSubmission) noexcept
  {
    RefPtr<HTMLDialogElement> dialog;
    for (auto *ancestor = ParentElement(); ancestor; ancestor = ancestor->ParentElement())
    {
      if (auto *candidate = DynamicDowncast<HTMLDialogElement>(*ancestor))
      {
        dialog = ShareRefPtr(candidate);
        break;
      }
    }

    if (!dialog || !dialog->Open())
    {
      return;
    }

    dialog->CloseTheDialog(formSubmission->ReturnValue(), nullptr);
  }

  void HTMLFormElement::OnInsert() noexcept
  {
    HTMLElement::OnInsert();
  }

  void HTMLFormElement::OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
  {
    // Not the root node, as the tree scope flags are out of date.
    auto &root = dom::TreeQueries::Root(*this);
    for (auto &element : CopyListedElementsVector())
    {
      element->AsFormListedElement()->FormOwnerRemovedFromTree(root);
    }

    auto images = _imageElements;
    for (auto &image : images)
    {
      if (RefPtr<HTMLImageElement> element = ShareRefPtr(image.get()))
      {
        element->FormOwnerRemovedFromTree(root);
      }
    }

    HTMLElement::OnRemove(isSubtreeRoot, oldAncestor);
  }

  void HTMLFormElement::OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                                           Maybe<DOMStringView> value,
                                           const DOMStringAtom &namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != null)
    {
      return;
    }

    // The token lists follow their attributes.
    if (localName == u8"rel" && _relList)
    {
      _relList->AssociatedAttributeValueChanged();
    }

    auto newValue = value.value_or(DOMStringView());
    if (localName == u8"action")
    {
      _attributes.ParseAction(newValue);
    }
    else if (localName == u8"target")
    {
      _attributes.SetTarget(newValue);
    }
    else if (localName == u8"method")
    {
      _attributes.UpdateMethodType(newValue);
    }
    else if (localName == u8"enctype")
    {
      _attributes.UpdateEncodingType(newValue);
    }
    else if (localName == u8"accept-charset")
    {
      _attributes.SetAcceptCharset(newValue);
    }
  }

  uint32 HTMLFormElement::FormElementIndexWithFormAttribute(Element &element, uint32 rangeStart,
                                                            uint32 rangeEnd) noexcept
  {
    if (_listedElements.empty())
    {
      return 0;
    }

    krys_debug_assert(rangeStart <= rangeEnd);

    if (rangeStart == rangeEnd)
    {
      return rangeStart;
    }

    uint32 left = rangeStart;
    uint32 right = rangeEnd - 1;

    // Does binary search on _listedElements in order to find the index to be inserted.
    while (left != right)
    {
      uint32 middle = std::midpoint(left, right);
      krys_debug_assert(middle < _listedElementsBeforeIndex || middle >= _listedElementsAfterIndex);
      if (std::is_lt(dom::TreeOrder(element, *_listedElements[middle])))
      {
        right = middle;
      }
      else
      {
        left = middle + 1;
      }
    }

    krys_debug_assert(left < _listedElementsBeforeIndex || left >= _listedElementsAfterIndex);
    if (std::is_lt(dom::TreeOrder(element, *_listedElements[left])))
    {
      return left;
    }
    return left + 1;
  }

  uint32 HTMLFormElement::FormElementIndex(html::FormListedElement &listedElement) noexcept
  {
    auto &listedHTMLElement = listedElement.AsHTMLElement();

    // The element with a form attribute is a separate case, for speed.
    if (listedHTMLElement.HasAttribute(u8"form") && listedHTMLElement.IsConnected())
    {
      // A form in a shadow tree the control's form attribute reaches through its host's reference target is
      // placed where the host is.
      auto &formInControlTree =
        Downcast<Node>(*dom::EventTargetAlgorithms::Retarget(this, listedHTMLElement));
      auto order = dom::TreeOrder(listedHTMLElement, formInControlTree);
      krys_debug_assert(order != std::partial_ordering::unordered);
      if (std::is_lt(order))
      {
        ++_listedElementsBeforeIndex;
        ++_listedElementsAfterIndex;
        return FormElementIndexWithFormAttribute(listedHTMLElement, 0, _listedElementsBeforeIndex - 1);
      }
      if (std::is_gt(order) && !dom::TreeQueries::IsDescendant(listedHTMLElement, *this))
      {
        return FormElementIndexWithFormAttribute(listedHTMLElement, _listedElementsAfterIndex,
                                                 static_cast<uint32>(_listedElements.size()));
      }
    }

    uint32 currentListedElementsAfterIndex = _listedElementsAfterIndex;
    ++_listedElementsAfterIndex;

    if (!dom::TreeQueries::IsDescendant(listedHTMLElement, *this))
    {
      return currentListedElementsAfterIndex;
    }

    // Common during parsing: the element is the last thing in the form, so it goes at the end without a walk.
    if (!dom::TreeQueries::Next(listedHTMLElement, this))
    {
      return currentListedElementsAfterIndex;
    }

    uint32 i = _listedElementsBeforeIndex;
    for (auto &node : dom::DescendantRange(*this))
    {
      auto *element = DynamicDowncast<HTMLElement>(node);
      if (!element)
      {
        continue;
      }
      if (element == &listedHTMLElement)
      {
        return i;
      }
      if (!element->IsFormListedElement())
      {
        continue;
      }
      if (element->AsFormListedElement()->Form() != this)
      {
        continue;
      }
      ++i;
    }
    return currentListedElementsAfterIndex;
  }

  void HTMLFormElement::RegisterFormListedElement(html::FormListedElement &element) noexcept
  {
    InvalidateLiveCollectionCaches();
    _listedElements.insert(_listedElements.begin() + FormElementIndex(element),
                           CreateWeakPtr(&element.AsHTMLElement()));

    auto *control = DynamicDowncast<HTMLFormControlElement>(element.AsHTMLElement());
    if (!control || !control->IsSuccessfulSubmitButton())
    {
      return;
    }

    if (!_defaultButton)
    {
      control->InvalidateStyleForSubtree();
    }
    else
    {
      ResetDefaultButton();
    }
  }

  void HTMLFormElement::UnregisterFormListedElement(html::FormListedElement &element) noexcept
  {
    InvalidateLiveCollectionCaches();
    auto *htmlElement = &element.AsHTMLElement();
    auto position =
      std::ranges::find_if(_listedElements, [&](auto &listed) { return listed.get() == htmlElement; });
    krys_debug_assert(position != _listedElements.end());
    if (position == _listedElements.end())
    {
      return;
    }

    auto index = static_cast<uint32>(position - _listedElements.begin());
    if (index < _listedElementsBeforeIndex)
    {
      --_listedElementsBeforeIndex;
    }
    if (index < _listedElementsAfterIndex)
    {
      --_listedElementsAfterIndex;
    }
    RemoveFromPastNamesMap(element);
    _listedElements.erase(position);

    if (htmlElement == static_cast<HTMLElement *>(_defaultButton.get()))
    {
      ResetDefaultButton();
    }
  }

  void HTMLFormElement::AddInvalidFormControl(const HTMLElement &formControlElement) noexcept
  {
    krys_debug_assert(formControlElement.MatchesInvalidPseudoClass());

    if (std::ranges::any_of(_invalidFormControls,
                            [&](auto &control) { return control.get() == &formControlElement; }))
    {
      return;
    }

    Maybe<style::PseudoClassChangeInvalidation> styleInvalidation;
    if (!MatchesInvalidPseudoClass())
    {
      styleInvalidation.emplace(*this, std::initializer_list<Pair<css::SelectorPseudoClass, bool>> {
                                         {css::SelectorPseudoClass::Valid, false},
                                         {css::SelectorPseudoClass::Invalid, true},
                                       });
    }

    _invalidFormControls.push_back(CreateWeakPtr(&formControlElement));
  }

  void HTMLFormElement::RemoveInvalidFormControlIfNeeded(const HTMLElement &formControlElement) noexcept
  {
    auto position = std::ranges::find_if(_invalidFormControls,
                                         [&](auto &control) { return control.get() == &formControlElement; });
    if (position == _invalidFormControls.end())
    {
      return;
    }

    std::erase_if(_invalidFormControls, [](auto &control) { return !control.get(); });
    position = std::ranges::find_if(_invalidFormControls,
                                    [&](auto &control) { return control.get() == &formControlElement; });

    Maybe<style::PseudoClassChangeInvalidation> styleInvalidation;
    if (_invalidFormControls.size() == 1)
    {
      styleInvalidation.emplace(*this, std::initializer_list<Pair<css::SelectorPseudoClass, bool>> {
                                         {css::SelectorPseudoClass::Valid, true},
                                         {css::SelectorPseudoClass::Invalid, false},
                                       });
    }

    _invalidFormControls.erase(position);
  }

  DOMString HTMLFormElement::EffectiveTarget(const Event *event,
                                             HTMLFormControlElement *overrideSubmitter) const noexcept
  {
    // The submitter's formtarget is the target given to get an element's target:
    // https://html.spec.whatwg.org/#get-an-element's-target
    // 1. If target is null:
    // 1a. If element has a target attribute, then set target to that attribute's value.
    // 1b. Otherwise, if element's node document contains a base element with a target attribute, set target
    //     to the value of the target attribute of the first such base element.
    // 2. If target is not null, and contains an ASCII tab or newline and a U+003C (<), then set target to
    //    "_blank".
    // 3. Return target.
    if (auto *submitter = overrideSubmitter ? overrideSubmitter : FindSubmitter(event))
    {
      if (auto targetValue = submitter->GetAttribute(u8"formtarget"))
      {
        return MakeTargetBlankIfHasDanglingMarkup(*targetValue);
      }
    }

    if (auto targetValue = GetAttribute(u8"target"))
    {
      return MakeTargetBlankIfHasDanglingMarkup(*targetValue);
    }

    return MakeTargetBlankIfHasDanglingMarkup(NodeDocument().BaseTarget());
  }

  HTMLFormControlElement *HTMLFormElement::FindSubmitter(const Event *event) noexcept
  {
    if (!event)
    {
      return nullptr;
    }

    auto target = event->Target();
    auto *node = DynamicDowncast<Node>(target.get());
    if (!node)
    {
      return nullptr;
    }

    auto *element = DynamicDowncast<Element>(*node);
    if (!element)
    {
      element = node->ParentElement();
    }

    for (auto *ancestor = element; ancestor; ancestor = ancestor->ParentElement())
    {
      if (auto *control = DynamicDowncast<HTMLFormControlElement>(*ancestor))
      {
        return control;
      }
    }
    return nullptr;
  }

  HTMLFormControlElement *HTMLFormElement::DefaultButton() const noexcept
  {
    if (_defaultButton)
    {
      return _defaultButton.get();
    }

    for (auto &listedElement : _listedElements)
    {
      if (auto *control = DynamicDowncast<HTMLFormControlElement>(listedElement.get());
          control && control->IsSuccessfulSubmitButton())
      {
        _defaultButton = CreateWeakPtr(control);
        return control;
      }
    }
    return nullptr;
  }

  void HTMLFormElement::ResetDefaultButton() noexcept
  {
    if (!_defaultButton)
    {
      // Computing the default button is not cheap, so it isn't done unless needed. Without a default button,
      // the only style to invalidate is the element's being added to the form, which
      // RegisterFormListedElement does.
      return;
    }

    RefPtr<HTMLFormControlElement> oldDefault = ShareRefPtr(_defaultButton.get());
    _defaultButton = nullptr;
    (void)DefaultButton();
    if (_defaultButton.get() != oldDefault.get())
    {
      if (oldDefault)
      {
        oldDefault->InvalidateStyleForSubtree();
      }
      if (auto *defaultButton = _defaultButton.get())
      {
        defaultButton->InvalidateStyleForSubtree();
      }
    }
  }

  HTMLElement *HTMLFormElement::ElementFromPastNamesMap(DOMStringView pastName) const noexcept
  {
    if (pastName.empty() || _pastNamesMap.empty())
    {
      return nullptr;
    }

    auto entry = _pastNamesMap.find(DOMStringAtom(pastName));
    return entry != _pastNamesMap.end() ? entry->second.get() : nullptr;
  }

  void HTMLFormElement::AddToPastNamesMap(html::FormAssociatedElement &element,
                                          DOMStringView pastName) noexcept
  {
    krys_debug_assert(element.Form() == this);
    if (pastName.empty())
    {
      return;
    }
    _pastNamesMap[DOMStringAtom(pastName)] = CreateWeakPtr(&element.AsHTMLElement());
  }

  void HTMLFormElement::RemoveFromPastNamesMap(html::FormAssociatedElement &item) noexcept
  {
    if (_pastNamesMap.empty())
    {
      return;
    }

    auto *element = &item.AsHTMLElement();
    std::erase_if(_pastNamesMap, [&](auto &entry) { return entry.second.get() == element; });
  }

  bool HTMLFormElement::MatchesValidPseudoClass() const noexcept
  {
    return std::ranges::none_of(_invalidFormControls, [](auto &control) { return control.get() != nullptr; });
  }

  bool HTMLFormElement::MatchesInvalidPseudoClass() const noexcept
  {
    return !MatchesValidPseudoClass();
  }

  List<Ref<Element>> HTMLFormElement::NamedElements(DOMStringView name) noexcept
  {
    if (name.empty())
    {
      return {};
    }

    // https://html.spec.whatwg.org/multipage/forms.html#dom-form-nameditem
    auto namedItems = Elements()->NamedItems(name);
    if (namedItems.empty())
    {
      namedItems = NamedImages(name);
    }

    auto *elementFromPast = ElementFromPastNamesMap(name);
    if (namedItems.size() == 1 && namedItems.front().get() != elementFromPast)
    {
      AddToPastNamesMap(*Downcast<HTMLElement>(*namedItems.front()).AsFormAssociatedElement(), name);
    }
    else if (elementFromPast && namedItems.empty())
    {
      namedItems.emplace_back(ShareRef(*elementFromPast));
    }

    return namedItems;
  }

  bool HTMLFormElement::IsSupportedPropertyName(DOMStringView name) noexcept
  {
    return !name.empty()
           && (Elements()->NamedItem(name) || !NamedImages(name).empty() || ElementFromPastNamesMap(name));
  }

  bool HTMLFormElement::ShouldAutocomplete() const noexcept
  {
    auto autocomplete = GetAttribute(u8"autocomplete");
    return !autocomplete || !text::IsASCIICaselessEqual(DOMStringView(*autocomplete), u8"off");
  }

  List<Ref<HTMLElement>> HTMLFormElement::CopyListedElementsVector() const noexcept
  {
    List<Ref<HTMLElement>> elements;
    for (auto &weakElement : _listedElements)
    {
      if (auto *element = weakElement.get())
      {
        elements.push_back(ShareRef(*element));
      }
    }
    return elements;
  }

  List<Ref<HTMLElement>> HTMLFormElement::CopyValidatedListedElementsVector() const noexcept
  {
    List<Ref<HTMLElement>> elements;
    for (auto &weakElement : _listedElements)
    {
      if (auto *element = weakElement.get(); element && element->AsValidatedFormListedElement())
      {
        elements.push_back(ShareRef(*element));
      }
    }
    return elements;
  }

  HTMLFormElement *HTMLFormElement::FindClosestFormAncestor(const Element &startElement) noexcept
  {
    for (auto *ancestor = startElement.ParentElement(); ancestor; ancestor = ancestor->ParentElement())
    {
      if (auto *form = DynamicDowncast<HTMLFormElement>(*ancestor))
      {
        return form;
      }
    }
    return nullptr;
  }

  Ref<html::RadioNodeList> HTMLFormElement::RadioNodeListForName(DOMStringView name) noexcept
  {
    return RadioNodeListForName(name, html::RadioNodeListCandidates::ListedElements);
  }

  Ref<html::RadioNodeList>
    HTMLFormElement::RadioNodeListForName(DOMStringView name,
                                          html::RadioNodeListCandidates candidates) noexcept
  {
    auto &lists =
      candidates == html::RadioNodeListCandidates::Images ? _imageRadioNodeLists : _radioNodeLists;
    auto &cached = lists[DOMString(name)];
    if (auto *list = cached.get())
    {
      return ShareRef(static_cast<html::RadioNodeList &>(*list));
    }

    auto list = CreateRef<html::RadioNodeList>(*this, name, candidates);
    cached = CreateWeakPtr<NodeList>(list.get());
    return list;
  }

  void HTMLFormElement::RadioNodeListDestroyed(DOMStringView name,
                                               html::RadioNodeListCandidates candidates) noexcept
  {
    auto &lists =
      candidates == html::RadioNodeListCandidates::Images ? _imageRadioNodeLists : _radioNodeLists;
    lists.erase(DOMString(name));
  }

  void HTMLFormElement::RegisterImageElement(HTMLImageElement &element) noexcept
  {
    InvalidateLiveCollectionCaches();
    _imageElements.push_back(CreateWeakPtr<HTMLImageElement>(&element));
  }

  void HTMLFormElement::UnregisterImageElement(HTMLImageElement &element) noexcept
  {
    InvalidateLiveCollectionCaches();
    RemoveFromPastNamesMap(element);
    auto position =
      std::ranges::find_if(_imageElements, [&](auto &image) { return image.get() == &element; });
    krys_debug_assert(position != _imageElements.end());
    if (position != _imageElements.end())
    {
      _imageElements.erase(position);
    }
  }

  List<Ref<Element>> HTMLFormElement::NamedImages(DOMStringView name) const noexcept
  {
    List<Ref<Element>> images;
    if (name.empty())
    {
      return images;
    }
    for (auto &image : _imageElements)
    {
      auto *element = image.get();
      if (element && (element->GetAttribute(u8"id") == name || element->GetAttribute(u8"name") == name))
      {
        images.push_back(ShareRef<Element>(*element));
      }
    }
    std::ranges::sort(images,
                      [](const auto &a, const auto &b) { return std::is_lt(dom::TreeOrder(*a, *b)); });
    return images;
  }

  RefPtr<html::DOMFormData>
    HTMLFormElement::ConstructEntryList(const RefPtr<HTMLFormControlElement> &submitter,
                                        const Ref<html::DOMFormData> &domFormData,
                                        List<Pair<DOMString, DOMString>> *formValues) noexcept
  {
    // https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#constructing-the-form-data-set
    // 1. If form's constructing entry list is true, then return null.
    if (_isConstructingEntryList)
    {
      return nullptr;
    }

    // 2. Set form's constructing entry list to true.
    bool wasConstructingEntryList = std::exchange(_isConstructingEntryList, true);

    // (The submitter is the only button that appends an entry.)
    if (submitter)
    {
      submitter->SetActivatedSubmit(true);
    }

    // 3. Let controls be a list of all the submittable elements whose form owner is form, in tree order.
    // 4. Let entry list be a new empty entry list.
    // 5. For each element field in controls, in tree order:
    for (auto &element : CopyListedElementsVector())
    {
      // 5a. If any of the following are true: field has a datalist element ancestor; field is disabled; ...
      //     then continue.
      // 5b. to 5k. (Each control's AppendFormData.)
      bool hasDataListAncestor = false;
      for (auto *ancestor = element->ParentElement(); ancestor; ancestor = ancestor->ParentElement())
      {
        if (Is<HTMLDataListElement>(*ancestor))
        {
          hasDataListAncestor = true;
          break;
        }
      }
      if (!hasDataListAncestor && !element->IsDisabledFormControl())
      {
        element->AsFormListedElement()->AppendFormData(*domFormData);
      }
      if (auto *input = DynamicDowncast<HTMLInputElement>(*element); formValues && input)
      {
        if (input->IsTextField())
        {
          formValues->emplace_back(input->Name(), input->Value());
        }
      }
    }

    // 6. Let form data be a new FormData object associated with entry list.
    // 7. Fire an event named formdata at form using FormDataEvent, with the formData attribute initialized to
    //    form data and the bubbles attribute initialized to true.
    auto event =
      FormDataEvent::Create(event_names::FormData, true, false, false, Ref<html::DOMFormData>(domFormData));
    (void)dom::EventDispatcher::DispatchToTarget(*event, this);

    if (submitter)
    {
      submitter->SetActivatedSubmit(false);
    }

    // 8. Set form's constructing entry list to false.
    _isConstructingEntryList = wasConstructingEntryList;

    // 9. Return a clone of entry list.
    return domFormData->Clone();
  }

#pragma endregion
}
