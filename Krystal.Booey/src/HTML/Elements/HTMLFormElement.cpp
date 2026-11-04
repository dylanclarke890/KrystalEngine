#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
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
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Events/SubmitEvent.hpp"
#include "Krystal.Booey/HTML/Forms/DOMFormData.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Style/Invalidation/PseudoClassChangeInvalidation.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <algorithm>
#include <numeric>

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#attr-form-autocomplete
  template <>
  struct EnumeratedAttributeTraits<"autocomplete", HTMLFormElement>
  {
    using State = AutocompleteAttributeState;
    static inline const KeywordStateMapping<State> StateMappings[] = {{u8"on", State::On},
                                                                      {u8"off", State::Off}};

    constexpr static State MissingValueDefault = State::On;
    constexpr static State InvalidValueDefault = State::On;
  };

  /// @see https://html.spec.whatwg.org/#attr-fs-method
  template <>
  struct EnumeratedAttributeTraits<"method", HTMLFormElement>
  {
    using State = MethodAttributeState;
    static inline const KeywordStateMapping<State> StateMappings[] = {
      {u8"get", State::Get},
      {u8"post", State::Post},
      {u8"dialog", State::Dialog},
    };

    constexpr static State MissingValueDefault = State::Get;
    constexpr static State InvalidValueDefault = State::Get;
  };

  /// @see https://html.spec.whatwg.org/#attr-fs-enctype
  template <>
  struct EnumeratedAttributeTraits<"enctype", HTMLFormElement>
  {
    using State = EnctypeAttributeState;
    static inline const KeywordStateMapping<State> StateMappings[] = {
      {u8"application/x-www-form-urlencoded", State::URLEncoded},
      {u8"multipart/form-data", State::FormData},
      {u8"text/plain", State::TextPlain},
    };

    constexpr static State MissingValueDefault = State::URLEncoded;
    constexpr static State InvalidValueDefault = State::URLEncoded;
  };

  namespace
  {
    /// @brief `_blank` in place of a target that looks like the start of dangling markup.
    KRYS_NODISCARD dom::DOMString MakeTargetBlankIfHasDanglingMarkup(dom::DOMStringView target) noexcept
    {
      auto contains = [&](char8 character)
      {
        return target.find(character) != dom::DOMStringView::npos;
      };
      if ((contains(u8'\n') || contains(u8'\r') || contains(u8'\t')) && contains(u8'<'))
      {
        return u8"_blank";
      }
      return dom::DOMString(target);
    }

    struct FormRelAttributes
    {
      bool Noopener {false};
      bool Noreferrer {false};
      bool Opener {false};
    };
  }

  HTMLFormElement::HTMLFormElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Form)
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
  }

#pragma region HTMLFormElement

  dom::DOMString HTMLFormElement::AcceptCharset() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"accept-charset").value_or(u8"");
  }

  void HTMLFormElement::AcceptCharset(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"accept-charset", krys::move(value));
  }

  dom::USVString HTMLFormElement::Action() const noexcept
  {
    // https://html.spec.whatwg.org/#dom-fs-action
    // Let attributeValue be the result of running get an attribute value given this and action. If
    // attributeValue is null or the empty string, return this's node document's URL, serialized.
    auto attributeValue = GetAttribute(u8"action");
    if (!attributeValue || attributeValue->empty())
    {
      return dom::USVString(NodeDocument().GetURL().String());
    }
    // Let urlString be the result of encoding-parsing-and-serializing a URL given attributeValue, relative to
    // this's node document. If urlString is not failure, return urlString.
    if (auto url = NodeDocument().CompleteURL(*attributeValue); url.IsValid())
    {
      return dom::USVString(url.String());
    }
    // Return attributeValue, converted to a scalar value string.
    return dom::USVString(krys::move(*attributeValue));
  }

  void HTMLFormElement::Action(dom::USVString value) noexcept
  {
    return ReflectSetter<dom::USVString>(*this, u8"action", krys::move(value));
  }

  dom::DOMString HTMLFormElement::Autocomplete() const noexcept
  {
    return ReflectEnumerated<"autocomplete", HTMLFormElement>(*this);
  }

  void HTMLFormElement::Autocomplete(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"autocomplete", krys::move(value));
  }

  dom::DOMString HTMLFormElement::EncType() const noexcept
  {
    return ReflectEnumerated<"enctype", HTMLFormElement>(*this);
  }

  void HTMLFormElement::EncType(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"enctype", krys::move(value));
  }

  // https://html.spec.whatwg.org/#dom-fs-encoding
  dom::DOMString HTMLFormElement::Encoding() const noexcept
  {
    // The encoding IDL attribute must reflect the enctype content attribute, limited to only known values.
    return EncType();
  }

  void HTMLFormElement::Encoding(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"enctype", krys::move(value));
  }

  dom::DOMString HTMLFormElement::Method() const noexcept
  {
    return ReflectEnumerated<"method", HTMLFormElement>(*this);
  }

  void HTMLFormElement::Method(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"method", krys::move(value));
  }

  dom::DOMString HTMLFormElement::Name() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLFormElement::Name(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  bool HTMLFormElement::NoValidate() const noexcept
  {
    return Reflect<bool>(*this, u8"novalidate");
  }

  void HTMLFormElement::NoValidate(bool value) noexcept
  {
    Reflect<bool>(*this, u8"novalidate", krys::move(value));
  }

  dom::DOMString HTMLFormElement::Target() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"target").value_or(u8"");
  }

  void HTMLFormElement::Target(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"target", krys::move(value));
  }

  dom::DOMString HTMLFormElement::Rel() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"rel").value_or(u8"");
  }

  void HTMLFormElement::Rel(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"rel", krys::move(value));
  }

  dom::DOMTokenList &HTMLFormElement::RelList() noexcept
  {
    if (!_relList)
    {
      _relList = dom::ElementAlgorithms::CreateDOMTokenList(
        *this, u8"rel",
        [](dom::Document &, dom::DOMStringView token)
        {
          return text::IsASCIICaselessEqual(token, utf8_stringview(u8"noreferrer"))
                 || text::IsASCIICaselessEqual(token, utf8_stringview(u8"noopener"))
                 || text::IsASCIICaselessEqual(token, utf8_stringview(u8"opener"));
        });
    }

    return *_relList;
  }

  Ref<HTMLFormControlsCollection> HTMLFormElement::Elements() noexcept
  {
    if (auto *elements = _elements.get())
    {
      // Only ever an HTMLFormControlsCollection.
      return ShareRef(static_cast<HTMLFormControlsCollection &>(*elements));
    }
    auto elements = CreateRef<HTMLFormControlsCollection>(*this);
    _elements = CreateWeakPtr<dom::HTMLCollection>(elements.get());
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
    RefPtr<dom::Element> item = Elements()->Item(index);
    return ShareRefPtr(DynamicDowncast<HTMLElement>(item.get()));
  }

  Maybe<Variant<RefPtr<RadioNodeList>, RefPtr<dom::Element>>>
    HTMLFormElement::NamedItem(dom::DOMStringView name) noexcept
  {
    auto namedItems = NamedElements(name);
    if (namedItems.empty())
    {
      return std::nullopt;
    }
    if (namedItems.size() == 1)
    {
      return Variant<RefPtr<RadioNodeList>, RefPtr<dom::Element>> {RefPtr<dom::Element>(namedItems[0])};
    }
    return Variant<RefPtr<RadioNodeList>, RefPtr<dom::Element>> {
      RefPtr<RadioNodeList>(RadioNodeListForName(name))};
  }

  void HTMLFormElement::Submit() noexcept
  {
    Submit(nullptr, true, FormSubmissionTrigger::NotSubmittedByJavaScript);
  }

  void HTMLFormElement::SubmitFromJavaScript() noexcept
  {
    // FIXME: Whether a user gesture is being processed (UserGestureIndicator).
    Submit(nullptr, false, FormSubmissionTrigger::SubmittedByJavaScript);
  }

  dom::ExceptionOr<void> HTMLFormElement::RequestSubmit(HTMLElement *submitter) noexcept
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
        return dom::Exception {dom::ExceptionCode::TypeError,
                               u8"The specified element is not a submit button."};
      }

      // 1b. If submitter's form owner is not this form element, then throw a "NotFoundError" DOMException.
      if (control->Form() != this)
      {
        return dom::Exception {dom::ExceptionCode::NotFoundError,
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
    CEReactionsScope ceReactions;

    // A reset during a reset (from its event) does nothing, nor does one in a document without a frame.
    if (_isInResetFunction || !NodeDocument().Frame())
    {
      return;
    }

    Ref<HTMLFormElement> protectedThis = ShareRef(*this);
    bool wasInResetFunction = std::exchange(_isInResetFunction, true);

    // https://html.spec.whatwg.org/#concept-form-reset
    // 1. Let reset be the result of firing an event named reset at form, with the bubbles and cancelable
    //    attributes initialized to true.
    auto event = dom::Event::Create(dom::DOMStringAtom(EventNames::Reset), true, true);
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

  // https://html.spec.whatwg.org/#statically-validate-the-constraints
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

  // https://html.spec.whatwg.org/#interactively-validate-the-constraints
  bool HTMLFormElement::ValidateInteractively() noexcept
  {
    for (auto &element : CopyValidatedListedElementsVector())
    {
      element->AsValidatedFormListedElement()->HideVisibleValidationMessage();
    }

    // 1. Statically validate the constraints of form, and let unhandled invalid controls be the list of
    // elements
    //    returned if the result was negative.
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

    // Focus on the first focusable control and show a validation message.
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

  void HTMLFormElement::SubmitImplicitly(dom::Event &event, bool fromImplicitSubmissionTrigger) noexcept
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

    // FIXME: Submitting forms with several fields that block implicit submission
    // (Settings::allowMultiElementImplicitSubmission), for older apps.
    if (fromImplicitSubmissionTrigger && submissionTriggerCount == 1)
    {
      SubmitIfPossible(&event);
    }
  }

  void HTMLFormElement::SubmitIfPossible(dom::Event *event, HTMLFormControlElement *submitter,
                                         FormSubmissionTrigger trigger) noexcept
  {
    // https://html.spec.whatwg.org/#form-submission-algorithm
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

    // FIXME: Turning interactive validation off (Settings::interactiveFormValidationEnabled), with settings.
    bool shouldValidate = NodeDocument().GetPage() && !NoValidate();
    if (shouldValidate)
    {
      auto *submitElement = submitter ? submitter : FindSubmitter(event);
      if (submitElement && submitElement->FormNoValidate())
      {
        shouldValidate = false;
      }
    }

    // Interactive validation must be done before dispatching the submit event.
    if (shouldValidate && !ValidateInteractively())
    {
      _isSubmittingOrPreparingForSubmission = false;
      return;
    }

    // FIXME: Finding the frame the target names and telling its loader client the submit event is coming
    // (FrameLoaderClient::dispatchWillSendSubmitEvent), with subframes and navigation.

    Ref<HTMLFormElement> protectedThis = ShareRef(*this);

    auto submitEvent = SubmitEvent::Create(ShareRefPtr(static_cast<HTMLElement *>(submitter)));
    (void)dom::EventDispatcher::DispatchToTarget(*submitEvent, this);

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

    Submit(event, !submitter, trigger, submitter);
  }

  List<Pair<dom::DOMString, dom::DOMString>> HTMLFormElement::TextFieldValues() const noexcept
  {
    List<Pair<dom::DOMString, dom::DOMString>> values;
    for (auto &element : CopyListedElementsVector())
    {
      auto *input = DynamicDowncast<HTMLInputElement>(*element);
      if (!input || !input->IsTextField())
      {
        continue;
      }
      values.push_back({input->Name(), input->Value()});
    }
    return values;
  }

  void HTMLFormElement::Submit(dom::Event *event, bool processingUserGesture, FormSubmissionTrigger trigger,
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
    _wasUserSubmitted = processingUserGesture;

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

    // FIXME: The rel attribute's noopener, noreferrer and opener, which set the submission's opener and
    // referrer policies.

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

  // https://html.spec.whatwg.org/#submit-dialog
  void HTMLFormElement::SubmitDialog(Ref<FormSubmission> &&formSubmission) noexcept
  {
    // Let subject be the nearest ancestor dialog element of form, if any.
    RefPtr<HTMLDialogElement> dialog;
    for (auto *ancestor = ParentElement(); ancestor; ancestor = ancestor->ParentElement())
    {
      if (auto *candidate = DynamicDowncast<HTMLDialogElement>(*ancestor))
      {
        dialog = ShareRefPtr(candidate);
        break;
      }
    }

    // If there isn't one, or if it does not have an open attribute, do nothing.
    if (!dialog || !dialog->Open())
    {
      return;
    }

    // Then, close the dialog subject. If there is a result, let that be the return value.
    dialog->CloseTheDialog(formSubmission->ReturnValue(), nullptr);
  }

  void HTMLFormElement::OnInsert() noexcept
  {
    HTMLElement::OnInsert();
    // FIXME: Telling the host about the form (Document::didAssociateFormControl), for autofill.
  }

  void HTMLFormElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    // Not the root node, as the tree scope flags are out of date.
    auto &root = dom::TreeQueries::Root(*this);
    for (auto &element : CopyListedElementsVector())
    {
      element->AsFormListedElement()->FormOwnerRemovedFromTree(root);
    }

    // FIXME: The form's image elements, once images register with forms.

    HTMLElement::OnRemove(isSubtreeRoot, oldAncestor);
  }

  void HTMLFormElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                           Maybe<dom::DOMStringView> value,
                                           dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      return;
    }

    // The token lists follow their attributes.
    if (localName == u8"rel" && _relList)
    {
      _relList->AssociatedAttributeValueChanged();
    }

    auto newValue = value.value_or(dom::DOMStringView());
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
    // FIXME: Resetting the controls when the document is restored from the back/forward cache with
    // autocomplete off (registerForDocumentSuspensionCallbacks), once there's a back/forward cache.
  }

  uint32 HTMLFormElement::FormElementIndexWithFormAttribute(dom::Element &element, uint32 rangeStart,
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

  uint32 HTMLFormElement::FormElementIndex(FormListedElement &listedElement) noexcept
  {
    auto &listedHTMLElement = listedElement.AsHTMLElement();

    // The element with a form attribute is a separate case, for speed.
    if (listedHTMLElement.HasAttribute(u8"form") && listedHTMLElement.IsConnected())
    {
      auto order = dom::TreeOrder(listedHTMLElement, *this);
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

  void HTMLFormElement::RegisterFormListedElement(FormListedElement &element) noexcept
  {
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

  void HTMLFormElement::UnregisterFormListedElement(FormListedElement &element) noexcept
  {
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

  dom::DOMString HTMLFormElement::EffectiveTarget(const dom::Event *event,
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

  bool HTMLFormElement::WasUserSubmitted() const noexcept
  {
    return _wasUserSubmitted;
  }

  HTMLFormControlElement *HTMLFormElement::FindSubmitter(const dom::Event *event) const noexcept
  {
    if (!event)
    {
      return nullptr;
    }

    auto target = event->Target();
    auto *node = DynamicDowncast<dom::Node>(target.get());
    if (!node)
    {
      return nullptr;
    }

    auto *element = DynamicDowncast<dom::Element>(*node);
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

  HTMLElement *HTMLFormElement::ElementFromPastNamesMap(dom::DOMStringView pastName) const noexcept
  {
    if (pastName.empty() || _pastNamesMap.empty())
    {
      return nullptr;
    }

    auto entry = _pastNamesMap.find(dom::DOMStringAtom(pastName));
    return entry != _pastNamesMap.end() ? entry->second.get() : nullptr;
  }

  void HTMLFormElement::AddToPastNamesMap(FormAssociatedElement &item, dom::DOMStringView pastName) noexcept
  {
    krys_debug_assert(item.Form() == this);
    if (pastName.empty())
    {
      return;
    }
    _pastNamesMap[dom::DOMStringAtom(pastName)] = CreateWeakPtr(&item.AsHTMLElement());
  }

  void HTMLFormElement::RemoveFromPastNamesMap(FormAssociatedElement &item) noexcept
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

  List<Ref<dom::Element>> HTMLFormElement::NamedElements(dom::DOMStringView name) noexcept
  {
    if (name.empty())
    {
      return {};
    }

    // https://html.spec.whatwg.org/#dom-form-nameditem
    auto namedItems = Elements()->NamedItems(name);

    auto *elementFromPast = ElementFromPastNamesMap(name);
    if (namedItems.size() == 1 && namedItems.front().get() != elementFromPast)
    {
      AddToPastNamesMap(*Downcast<HTMLElement>(*namedItems.front()).AsFormAssociatedElement(), name);
    }
    else if (elementFromPast && namedItems.empty())
    {
      namedItems.push_back(ShareRef(*elementFromPast));
    }

    return namedItems;
  }

  bool HTMLFormElement::IsSupportedPropertyName(dom::DOMStringView name) noexcept
  {
    return !name.empty() && Elements()->NamedItem(name);
  }

  bool HTMLFormElement::ShouldAutocomplete() const noexcept
  {
    auto autocomplete = GetAttribute(u8"autocomplete");
    return !autocomplete
           || !text::IsASCIICaselessEqual(dom::DOMStringView(*autocomplete), dom::DOMStringView(u8"off"));
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

  HTMLFormElement *HTMLFormElement::FindClosestFormAncestor(const dom::Element &startElement) noexcept
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

  Ref<RadioNodeList> HTMLFormElement::RadioNodeListForName(dom::DOMStringView name) noexcept
  {
    return CreateRef<RadioNodeList>(*this, name);
  }

  RefPtr<DOMFormData>
    HTMLFormElement::ConstructEntryList(RefPtr<HTMLFormControlElement> &&submitter,
                                        Ref<DOMFormData> &&domFormData,
                                        List<Pair<dom::DOMString, dom::DOMString>> *formValues) noexcept
  {
    // https://html.spec.whatwg.org/#constructing-the-form-data-set
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
          formValues->push_back({input->Name(), input->Value()});
          // FIXME: Adding the value to the search field's recent searches (addSearchResult), with search
          // fields' results.
        }
      }
    }

    // 6. Let form data be a new FormData object associated with entry list.
    // 7. Fire an event named formdata at form using FormDataEvent, with the formData attribute initialized to
    //    form data and the bubbles attribute initialized to true.
    auto event = dom::FormDataEvent::Create(dom::DOMStringAtom(EventNames::FormData), true, false, false,
                                            Ref<DOMFormData>(domFormData));
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