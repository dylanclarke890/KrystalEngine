#include "Krystal.Booey/HTML/HTMLDialogElement.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorEnums.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/ToggleEvent.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/EnumeratedAttributes.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/CloseWatcher/CloseWatcher.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLButtonElement.hpp"
#include "Krystal.Booey/Style/PseudoClassChangeInvalidation.hpp"

namespace krys::boo::html
{
  template <>
  struct EnumeratedAttributeTraits<"closedby", HTMLDialogElement>
  {
    static inline KeywordStateMappingArray<3> StateMappings = {
      KeywordStateMapping {u8"any", AttributeState::Any},
      KeywordStateMapping {u8"closerequest", AttributeState::CloseRequest},
      KeywordStateMapping {u8"none", AttributeState::None}};

    constexpr static AttributeState MissingValueDefault = AttributeState::Auto;
    constexpr static AttributeState InvalidValueDefault = AttributeState::Auto;
  };

  HTMLDialogElement::HTMLDialogElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Dialog)
  {
  }

  HTMLDialogElement::~HTMLDialogElement() noexcept = default;

#pragma region HTMLDialogElement

  bool HTMLDialogElement::Open() const noexcept
  {
    return Reflect<bool>(*this, u8"open");
  }

  void HTMLDialogElement::Open(bool value) noexcept
  {
    CEReactionsScope ceReactions;
    Reflect<bool>(*this, u8"open", krys::move(value));
  }

  dom::DOMString HTMLDialogElement::ClosedBy() const noexcept
  {
    auto value = Reflect<dom::DOMString>(*this, u8"closedby");
    return EnumeratedAttribute<"closedby", HTMLDialogElement>::ResolveCanonicalKeyword<dom::DOMString>(
      krys::move(value));
  }

  void HTMLDialogElement::ClosedBy(dom::DOMString value) noexcept
  {
    CEReactionsScope ceReactions;
    Reflect<dom::DOMString>(*this, u8"closedby", krys::move(value));
  }

  // https://html.spec.whatwg.org/#dom-dialog-show
  dom::ExceptionOr<void> HTMLDialogElement::Show() noexcept
  {
    CEReactionsScope ceReactions;

    // 1. If this has an open attribute and is modal of this is false, then return.
    // 2. If this has an open attribute, then throw an "InvalidStateError" DOMException.
    if (Open())
    {
      if (!IsModal())
      {
        return {};
      }
      return dom::Exception {dom::ExceptionCode::InvalidStateError,
                             u8"Cannot call show() on an open modal dialog."};
    }

    // 3. If the result of firing an event named beforetoggle, using ToggleEvent, with the cancelable
    // attribute
    //    initialized to true, the oldState attribute initialized to "closed", and the newState attribute
    //    initialized to "open" at this is false, then return.
    if (!FireBeforeToggleEvent(dom::ToggleState::Closed, dom::ToggleState::Open, true, nullptr))
    {
      return {};
    }

    // 4. If this has an open attribute, then return.
    if (Open())
    {
      return {};
    }

    // 5. Queue a dialog toggle event task given this, "closed", "open", and null.
    QueueDialogToggleEventTask(dom::ToggleState::Closed, dom::ToggleState::Open, nullptr);

    // 6. Add an open attribute to this, whose value is the empty string.
    (void)SetAttribute(dom::DOMStringAtom(u8"open"), dom::DOMString());

    // 7. Set this's previously focused element to the focused element.
    _previouslyFocusedElement = CreateWeakPtr(NodeDocument().FocusedElement());

    // 8. Let document be this's node document.
    // 9. Let hideUntil be the result of running topmost popover ancestor given this, null, and false.
    // 10. Run hide popovers until given document, hideUntil, false, and true.
    HidePopoversOnShow();

    // 11. Run the dialog focusing steps given this.
    RunFocusingSteps();
    return {};
  }

  // https://html.spec.whatwg.org/#dom-dialog-showmodal
  dom::ExceptionOr<void> HTMLDialogElement::ShowModal() noexcept
  {
    CEReactionsScope ceReactions;
    return ShowModalDialog(nullptr);
  }

  // https://html.spec.whatwg.org/#dom-dialog-close
  void HTMLDialogElement::Close(Maybe<dom::DOMString> returnValue) noexcept
  {
    CEReactionsScope ceReactions;
    CloseTheDialog(krys::move(returnValue), nullptr);
  }

  // https://html.spec.whatwg.org/#dom-dialog-requestclose
  void HTMLDialogElement::RequestClose(Maybe<dom::DOMString> returnValue) noexcept
  {
    CEReactionsScope ceReactions;
    RequestCloseTheDialog(krys::move(returnValue), nullptr);
  }

#pragma endregion

  // https://html.spec.whatwg.org/#computed-closed-by-state
  HTMLDialogElement::ClosedByState HTMLDialogElement::ComputedClosedByState() const noexcept
  {
    // 1. If the state of dialog's closedby attribute is Auto:
    // 1a. If dialog's is modal is true, then return Close Request.
    // 1b. Return None.
    // 2. Return the state of dialog's closedby attribute.
    auto value = Reflect<dom::DOMString>(*this, u8"closedby");
    switch (EnumeratedAttribute<"closedby", HTMLDialogElement>::ResolveState(value))
    {
      case AttributeState::Any:          return ClosedByState::Any;
      case AttributeState::CloseRequest: return ClosedByState::CloseRequest;
      case AttributeState::None:         return ClosedByState::None;
      default:
        // The auto state: close requests close a modal dialog.
        return IsModal() ? ClosedByState::CloseRequest : ClosedByState::None;
    }
  }

  // https://html.spec.whatwg.org/#show-a-modal-dialog
  dom::ExceptionOr<void> HTMLDialogElement::ShowModalDialog(dom::Element *source) noexcept
  {
    // 1. If subject has an open attribute and is modal of subject is true, then return.
    // 2. If subject has an open attribute, then throw an "InvalidStateError" DOMException.
    if (Open())
    {
      if (IsModal())
      {
        return {};
      }
      return dom::Exception {dom::ExceptionCode::InvalidStateError,
                             u8"Cannot call showModal() on an open non-modal dialog."};
    }

    // 3. If subject's node document is not fully active, then throw an "InvalidStateError" DOMException.
    // 4. If subject is not connected, then throw an "InvalidStateError" DOMException.
    // 5. If subject is in the popover showing state, then throw an "InvalidStateError" DOMException.
    if (!IsConnected())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError, u8"Element is not connected."};
    }
    if (IsPopoverShowing())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError, u8"Element is already an open popover."};
    }
    Ref<dom::Document> document = ShareRef(NodeDocument());
    if (!document->IsFullyActive())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError,
                             u8"Invalid for dialogs within documents that are not fully active."};
    }

    // 6. If the result of firing an event named beforetoggle, using ToggleEvent, with the cancelable
    // attribute
    //    initialized to true, the oldState attribute initialized to "closed", the newState attribute
    //    initialized to "open", and the source attribute initialized to source at subject is false, then
    //    return.
    if (!FireBeforeToggleEvent(dom::ToggleState::Closed, dom::ToggleState::Open, true, source))
    {
      return {};
    }

    // 7. If subject has an open attribute, then return.
    // 8. If subject is not connected, then return.
    // 9. If subject is in the popover showing state, then return.
    if (Open() || !IsConnected() || IsPopoverShowing())
    {
      return {};
    }

    // 10. Queue a dialog toggle event task given subject, "closed", "open", and source.
    QueueDialogToggleEventTask(dom::ToggleState::Closed, dom::ToggleState::Open, source);

    // 11. Add an open attribute to subject, whose value is the empty string.
    (void)SetAttribute(dom::DOMStringAtom(u8"open"), dom::DOMString());

    // 12. Assert: subject's close watcher is not null.
    // (Null in a document without a window.)
    // 13. Set is modal of subject to true.
    // 14. Set subject's node document to be blocked by the modal dialog subject.
    // (The document's active modal dialog is its topmost modal dialog.)
    SetIsModal(true);

    // 15. If subject's node document's top layer does not already contain subject, then add an element to the
    //     top layer given subject.
    if (!IsInTopLayer() || document->IsPendingTopLayerRemoval(*this))
    {
      AddToTopLayer();
    }

    // 16. Set subject's previously focused element to the focused element.
    _previouslyFocusedElement = CreateWeakPtr(document->FocusedElement());

    // 17. Let document be subject's node document.
    // 18. Let hideUntil be the result of running topmost popover ancestor given subject, null, and false.
    // 19. Run hide popovers until given document, hideUntil, false, and true.
    HidePopoversOnShow();

    // 20. Run the dialog focusing steps given subject.
    RunFocusingSteps();
    return {};
  }

  // https://html.spec.whatwg.org/#close-the-dialog
  void HTMLDialogElement::CloseTheDialog(Maybe<dom::DOMString> result, dom::Element *source) noexcept
  {
    // 1. If subject does not have an open attribute, then return.
    if (!Open())
    {
      return;
    }

    // 2. Fire an event named beforetoggle, using ToggleEvent, with the oldState attribute initialized to
    // "open",
    //    the newState attribute initialized to "closed", and the source attribute initialized to source at
    //    subject.
    Ref<HTMLDialogElement> protectedThis = ShareRef(*this);
    (void)FireBeforeToggleEvent(dom::ToggleState::Open, dom::ToggleState::Closed, false, source);

    // 3. If subject does not have an open attribute, then return.
    if (!Open())
    {
      return;
    }

    // 4. Queue a dialog toggle event task given subject, "open", "closed", and source.
    QueueDialogToggleEventTask(dom::ToggleState::Open, dom::ToggleState::Closed, source);

    // 5. Remove subject's open attribute.
    RemoveAttribute(dom::DOMStringAtom(u8"open"));

    // 6. If is modal of subject is true, then request an element to be removed from the top layer given
    //    subject.
    if (IsModal() && IsInTopLayer())
    {
      RequestRemoveFromTopLayer();
    }

    // 7. Let wasModal be the value of subject's is modal flag.
    bool wasModal = IsModal();

    // 8. Set is modal of subject to false.
    SetIsModal(false);

    // 9. If result is not null, then set subject's returnValue attribute to result.
    if (result)
    {
      _returnValue = krys::move(*result);
    }

    // 10. Set subject's request close return value to null.
    _requestCloseReturnValue = null;

    // 11. Set subject's request close source element to null.
    _requestCloseSourceElement = nullptr;

    // 12. If subject's previously focused element is not null:
    // 12a. Let element be subject's previously focused element.
    // 12b. Set subject's previously focused element to null.
    if (RefPtr<dom::Element> element = std::exchange(_previouslyFocusedElement, nullptr).lock())
    {
      // 12c. If subject's node document's focused area of the document's DOM anchor is a shadow-including
      //      inclusive descendant of subject, or wasModal is true, then run the focusing steps for element;
      //      the viewport should not be scrolled by doing this step.
      auto *focusedArea = NodeDocument().FocusedElement();
      if (wasModal
          || (focusedArea && dom::TreeQueries::IsShadowIncludingInclusiveDescendant(*focusedArea, *this)))
      {
        FocusAlgorithms::FocusingSteps(*element, nullptr, {.PreventScroll = true});
      }
    }

    // 13. Queue an element task on the user interaction task source given the subject element to fire an
    // event
    //     named close at subject.
    EventLoop::Current().QueueTask(
      TaskSource::UserInteraction, NodeDocument(),
      [protectedThis]
      {
        auto event = dom::Event::Create(dom::DOMStringAtom(EventNames::Close), false, false);
        (void)dom::EventDispatcher::DispatchToTarget(*event, protectedThis.get());
      });
  }

  // https://html.spec.whatwg.org/#dialog-request-close
  void HTMLDialogElement::RequestCloseTheDialog(Maybe<dom::DOMString> returnValue,
                                                dom::Element *source) noexcept
  {
    // 1. If subject does not have an open attribute, then return.
    // 2. If subject is not connected or subject's node document is not fully active, then return.
    // 3. Assert: subject's close watcher is not null.
    // (Null in a document without a window, which has no close watchers.)
    if (!Open() || !IsConnected() || !NodeDocument().IsFullyActive() || !_closeWatcher)
    {
      return;
    }

    Ref<HTMLDialogElement> protectedThis = ShareRef(*this);
    Ref<CloseWatcher> closeWatcher = ShareRef(*_closeWatcher);

    // 4. Set subject's enable close watcher for request close to true.
    _enableCloseWatcherForRequestClose = true;

    // 5. Set subject's request close return value to returnValue.
    _requestCloseReturnValue = krys::move(returnValue);

    // 6. Set subject's request close source element to source.
    _requestCloseSourceElement = CreateWeakPtr(source);

    // 7. Request to close subject's close watcher with false.
    (void)closeWatcher->RequestToClose(false);

    // 8. Set subject's enable close watcher for request close to false.
    _enableCloseWatcherForRequestClose = false;
  }

  // https://html.spec.whatwg.org/#is-valid-command-steps
  bool HTMLDialogElement::IsValidCommandType(dom::CommandType command) const noexcept
  {
    // 1. If command is in the Close state, the Request Close state, or the Show Modal state, then return
    // true.
    // 2. Return false.
    // (Or a popover command, valid for every HTML element.)
    return HTMLElement::IsValidCommandType(command) || command == dom::CommandType::ShowModal
           || command == dom::CommandType::Close || command == dom::CommandType::RequestClose;
  }

  bool HTMLDialogElement::HandleCommandInternal(HTMLButtonElement &invoker, dom::CommandType command) noexcept
  {
    // (The popover commands.)
    if (HTMLElement::HandleCommandInternal(invoker, command))
    {
      return true;
    }

    // https://html.spec.whatwg.org/#command-steps
    // 1. If element is in the popover showing state, then return.
    if (IsPopoverShowing())
    {
      return false;
    }

    // 2. If command is in the Close state and element has an open attribute, then close the dialog element
    //    with source's optional value and source.
    // 3. If command is in the Request Close state and element has an open attribute, then request to close
    // the
    //    dialog element with source's optional value and source.
    // 4. If command is the Show Modal state and element does not have an open attribute, then show a modal
    //    dialog given element and source.
    if (Open())
    {
      if (command == dom::CommandType::Close)
      {
        CloseTheDialog(invoker.GetAttribute(u8"value"), &invoker);
        return true;
      }
      if (command == dom::CommandType::RequestClose)
      {
        RequestCloseTheDialog(invoker.GetAttribute(u8"value"), &invoker);
        return true;
      }
    }
    else
    {
      if (command == dom::CommandType::ShowModal)
      {
        (void)ShowModalDialog(&invoker);
        return true;
      }
    }

    return false;
  }

  // https://html.spec.whatwg.org/#dialog-focusing-steps
  void HTMLDialogElement::RunFocusingSteps() noexcept
  {
    // 1. FIXME: If the allow focus steps given subject's node document return false, then return, with
    //    permissions policy.

    // 2. Let control be null.
    // 3. If subject has the autofocus attribute, then set control to subject.
    // 4. If control is null, then set control to the focus delegate of subject.
    // 5. If control is null, then set control to subject.
    RefPtr<dom::Node> control;
    if (HasAttribute(u8"autofocus"))
    {
      control = ShareRefPtr<dom::Node>(this);
    }
    if (!control)
    {
      control = ShareRefPtr(FocusAlgorithms::FocusDelegate(*this));
    }
    if (!control)
    {
      control = ShareRefPtr<dom::Node>(this);
    }

    Ref<dom::Document> controlDocument = ShareRef(control->NodeDocument());
    if (!controlDocument->GetPage())
    {
      return;
    }

    // 6. Run the focusing steps for control.
    // (A modal dialog that can't focus it moves the focus to the viewport.)
    auto *controlElement = DynamicDowncast<dom::Element>(*control);
    if (controlElement && FocusAlgorithms::IsFocusableArea(*controlElement))
    {
      FocusAlgorithms::FocusingSteps(*controlElement);
    }
    else if (IsModal())
    {
      FocusAlgorithms::FocusingSteps(NodeDocument());
    }

    // 7. Let topDocument be control's node navigable's top-level traversable's active document.
    // 8. If control's node document's origin is not the same as the origin of topDocument, then return.
    // (There are no subframes.)
    // 9. Empty topDocument's autofocus candidates.
    // 10. Set topDocument's autofocus processed flag to true.
    controlDocument->AutofocusCandidates().clear();
    controlDocument->SetAutofocusProcessed();
  }

  void HTMLDialogElement::OnInsert() noexcept
  {
    HTMLElement::OnInsert();

    // https://html.spec.whatwg.org/#the-dialog-element:html-element-insertion-steps
    // 1. If insertedNode's node document is not fully active, then return.
    // 2. If insertedNode has an open attribute and is connected, then run the dialog setup steps given
    //    insertedNode.
    if (NodeDocument().IsFullyActive() && Open() && IsConnected())
    {
      SetupSteps();
    }
  }

  void HTMLDialogElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnRemove(isSubtreeRoot, oldAncestor);

    // https://html.spec.whatwg.org/#the-dialog-element:html-element-removing-steps
    // 1. If removedNode has an open attribute, then run the dialog cleanup steps given removedNode.
    if (Open())
    {
      CleanupSteps();
    }

    // 2. If removedNode's node document's top layer contains removedNode, then remove an element from the top
    //    layer immediately given removedNode.
    if (IsInTopLayer())
    {
      RemoveFromTopLayer();
    }

    // 3. Set is modal of removedNode to false.
    SetIsModal(false);
  }

  void HTMLDialogElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                             Maybe<dom::DOMStringView> value,
                                             dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);

    // https://html.spec.whatwg.org/#the-dialog-element:concept-element-attributes-change-ext
    // 1. If namespace is not null, then return.
    // 2. If localName is not open, then return.
    if (namespaceURI != dom::DOMStringAtom::Null() || localName != u8"open")
    {
      return;
    }

    // 3. If value is null and oldValue is not null, then run the dialog cleanup steps given element.
    // 4. If element's node document is not fully active, then return.
    // 5. If element is not connected, then return.
    // 6. If value is not null and oldValue is null, then run the dialog setup steps given element.
    if (!value && oldValue)
    {
      CleanupSteps();
    }
    else if (value && !oldValue && IsConnected() && NodeDocument().IsFullyActive())
    {
      SetupSteps();
    }
  }

  void HTMLDialogElement::SetIsModal(bool value) noexcept
  {
    if (_isModal == value)
    {
      return;
    }
    style::PseudoClassChangeInvalidation styleInvalidation(*this, css::SelectorPseudoClass::Modal, value);
    _isModal = value;

    // Which dialog blocks the document changes: the document element is the inert subtree root.
    InvalidateStyleInternal();
    if (RefPtr<dom::Element> documentElement = NodeDocument().DocumentElement())
    {
      documentElement->InvalidateStyleInternal();
    }
  }

  // https://html.spec.whatwg.org/#queue-a-dialog-toggle-event-task
  void HTMLDialogElement::QueueDialogToggleEventTask(dom::ToggleState oldState, dom::ToggleState newState,
                                                     dom::Element *source) noexcept
  {
    // (The task tracker is a ToggleEventTask, which coalesces the queued events.)
    if (!_toggleEventTask)
    {
      _toggleEventTask = dom::ToggleEventTask::Create(*this);
    }
    _toggleEventTask->Queue(oldState, newState, source);
  }

  bool HTMLDialogElement::FireBeforeToggleEvent(dom::ToggleState oldState, dom::ToggleState newState,
                                                bool cancelable, dom::Element *source) noexcept
  {
    auto stringForState = [](dom::ToggleState state)
    {
      return dom::DOMString(state == dom::ToggleState::Closed ? u8"closed" : u8"open");
    };

    dom::ToggleEventInit init;
    init.OldState = stringForState(oldState);
    init.NewState = stringForState(newState);
    init.Source = ShareRefPtr(source);
    auto event = dom::ToggleEvent::Create(dom::DOMStringAtom(u8"beforetoggle"), init, cancelable);
    (void)dom::EventDispatcher::DispatchToTarget(*event, this);
    return !event->DefaultPrevented();
  }

  // https://html.spec.whatwg.org/#dialog-setup-steps
  void HTMLDialogElement::SetupSteps() noexcept
  {
    // 1. Assert: subject has an open attribute.
    // 2. Assert: subject is connected.
    krys_debug_assert(Open() && IsConnected());

    // 3. Assert: subject's node document's open dialogs list does not contain subject.
    // (It can, when a dialog moved within its document is inserted again.)
    auto &document = NodeDocument();
    if (document.OpenDialogs().contains(ShareRef(*this)))
    {
      return;
    }

    // 4. Add subject to subject's node document's open dialogs list.
    document.AddOpenDialog(*this);

    // 5. Set the dialog close watcher with subject.
    SetCloseWatcher();
  }

  // https://html.spec.whatwg.org/#dialog-cleanup-steps
  void HTMLDialogElement::CleanupSteps() noexcept
  {
    // 1. Remove subject from subject's node document's open dialogs list.
    NodeDocument().RemoveOpenDialog(*this);

    // 2. If subject's close watcher is not null:
    // 2a. Destroy subject's close watcher.
    // 2b. Set subject's close watcher to null.
    if (RefPtr<CloseWatcher> closeWatcher = std::exchange(_closeWatcher, nullptr))
    {
      closeWatcher->Destroy();
    }
  }

  // https://html.spec.whatwg.org/#set-the-dialog-close-watcher
  void HTMLDialogElement::SetCloseWatcher() noexcept
  {
    // 1. Assert: dialog's close watcher is null.
    // 2. Assert: dialog has an open attribute and dialog's node document is fully active.
    krys_debug_assert(!_closeWatcher && Open());
    if (!NodeDocument().Window())
    {
      return;
    }

    // 3. Set dialog's close watcher to the result of establishing a close watcher given dialog's relevant
    // global
    //    object, with: cancelAction ...; closeAction ...; getEnabledState ...

    // The watcher holds the dialog weakly: the dialog holds the watcher.
    auto weakThis = CreateWeakPtr(this);
    _closeWatcher = CloseWatcher::Establish(
      NodeDocument(),
      [weakThis](bool canPreventClose)
      {
        RefPtr<HTMLDialogElement> dialog = weakThis.lock();
        if (!dialog)
        {
          return true;
        }
        auto event = dom::Event::Create(dom::DOMStringAtom(EventNames::Cancel), false, canPreventClose);
        (void)dom::EventDispatcher::DispatchToTarget(*event, dialog.get());
        return !event->DefaultPrevented();
      },
      [weakThis]
      {
        if (RefPtr<HTMLDialogElement> dialog = weakThis.lock())
        {
          dialog->CloseTheDialog(dialog->_requestCloseReturnValue, dialog->_requestCloseSourceElement.get());
        }
      },
      [weakThis]
      {
        RefPtr<HTMLDialogElement> dialog = weakThis.lock();
        return dialog
               && (dialog->_enableCloseWatcherForRequestClose
                   || dialog->ComputedClosedByState() != ClosedByState::None);
      });
  }

  void HTMLDialogElement::HidePopoversOnShow() noexcept
  {
    // Hide the popovers above the topmost popover the dialog is in, or all of them.
    RefPtr<HTMLElement> hideUntil = TopmostPopoverAncestor(nullptr, false);
    NodeDocument().HideAllPopoversUntil(hideUntil.get(), dom::FocusPreviousElement::No, dom::FireEvents::Yes);
  }
}
