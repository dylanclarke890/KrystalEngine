#include "Krystal.Booey/HTML/HTMLDialogElement.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorEnums.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/ToggleEvent.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/EnumeratedAttributes.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/CloseWatcher/CloseWatcher.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
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
    return Reflection::Reflect<bool>(*this, u8"open");
  }

  void HTMLDialogElement::Open(bool value) noexcept
  {
    CEReactionsScope ceReactions;
    Reflection::Reflect<bool>(*this, u8"open", krys::move(value));
  }

  dom::DOMString HTMLDialogElement::ClosedBy() const noexcept
  {
    auto value = Reflection::Reflect<dom::DOMString>(*this, u8"closedby");
    return EnumeratedAttribute<"closedby", HTMLDialogElement>::ResolveCanonicalKeyword<dom::DOMString>(
      krys::move(value));
  }

  void HTMLDialogElement::ClosedBy(dom::DOMString &&value) noexcept
  {
    CEReactionsScope ceReactions;
    Reflection::Reflect<dom::DOMString>(*this, u8"closedby", krys::move(value));
  }

  dom::ExceptionOr<void> HTMLDialogElement::Show() noexcept
  {
    CEReactionsScope ceReactions;

    // If this has an open attribute and is modal of this is false, then return.
    // If this has an open attribute, then throw an "InvalidStateError" DOMException.
    if (Open())
    {
      if (!IsModal())
      {
        return {};
      }
      return dom::Exception {dom::ExceptionCode::InvalidStateError,
                             u8"Cannot call show() on an open modal dialog."};
    }

    // If the result of firing an event named beforetoggle, using ToggleEvent, with the cancelable attribute
    // initialized to true, the oldState attribute initialized to "closed", and the newState attribute
    // initialized to "open" at this is false, then return.
    if (!FireBeforeToggleEvent(dom::ToggleState::Closed, dom::ToggleState::Open, true, nullptr))
    {
      return {};
    }

    // If this has an open attribute, then return.
    if (Open())
    {
      return {};
    }

    QueueDialogToggleEventTask(dom::ToggleState::Closed, dom::ToggleState::Open, nullptr);

    // Add an open attribute to this, whose value is the empty string (which runs the dialog setup steps).
    (void)SetAttribute(dom::DOMStringAtom(u8"open"), dom::DOMString());

    // Set this's previously focused element to the focused element.
    _previouslyFocusedElement = CreateWeakPtr(NodeDocument().FocusedElement());

    HidePopoversOnShow();
    RunFocusingSteps();
    return {};
  }

  dom::ExceptionOr<void> HTMLDialogElement::ShowModal() noexcept
  {
    CEReactionsScope ceReactions;
    return ShowModalDialog(nullptr);
  }

  void HTMLDialogElement::Close(Maybe<dom::DOMString> returnValue) noexcept
  {
    CEReactionsScope ceReactions;
    CloseTheDialog(krys::move(returnValue), nullptr);
  }

  void HTMLDialogElement::RequestClose(Maybe<dom::DOMString> returnValue) noexcept
  {
    CEReactionsScope ceReactions;
    RequestCloseTheDialog(krys::move(returnValue), nullptr);
  }

#pragma endregion

  HTMLDialogElement::ClosedByState HTMLDialogElement::ComputedClosedByState() const noexcept
  {
    auto value = Reflection::Reflect<dom::DOMString>(*this, u8"closedby");
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

  dom::ExceptionOr<void> HTMLDialogElement::ShowModalDialog(dom::Element *source) noexcept
  {
    // If subject has an open attribute and is modal of subject is true, then return.
    // If subject has an open attribute, then throw an "InvalidStateError" DOMException.
    if (Open())
    {
      if (IsModal())
      {
        return {};
      }
      return dom::Exception {dom::ExceptionCode::InvalidStateError,
                             u8"Cannot call showModal() on an open non-modal dialog."};
    }

    // If subject's node document is not fully active, or subject is not connected, then throw an
    // "InvalidStateError" DOMException.
    if (!IsConnected())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError, u8"Element is not connected."};
    }
    // FIXME: If subject's popover visibility state is showing, then throw, with popovers (B4d).
    Ref<dom::Document> document = ShareRef(NodeDocument());
    if (!document->IsFullyActive())
    {
      return dom::Exception {dom::ExceptionCode::InvalidStateError,
                             u8"Invalid for dialogs within documents that are not fully active."};
    }

    if (!FireBeforeToggleEvent(dom::ToggleState::Closed, dom::ToggleState::Open, true, source))
    {
      return {};
    }

    // The beforetoggle listeners could have opened, disconnected or shown the dialog as a popover.
    if (Open() || !IsConnected())
    {
      return {};
    }

    QueueDialogToggleEventTask(dom::ToggleState::Closed, dom::ToggleState::Open, source);

    // Add an open attribute to subject, whose value is the empty string (which runs the dialog setup steps).
    (void)SetAttribute(dom::DOMStringAtom(u8"open"), dom::DOMString());

    // Set is modal of subject to true, which blocks its node document by it.
    SetIsModal(true);

    // If subject's node document's top layer does not already contain subject, then add an element to the
    // top layer given subject.
    if (!IsInTopLayer() || document->IsPendingTopLayerRemoval(*this))
    {
      AddToTopLayer();
    }

    _previouslyFocusedElement = CreateWeakPtr(document->FocusedElement());

    HidePopoversOnShow();
    RunFocusingSteps();
    return {};
  }

  void HTMLDialogElement::CloseTheDialog(Maybe<dom::DOMString> result, dom::Element *source) noexcept
  {
    // If subject does not have an open attribute, then return.
    if (!Open())
    {
      return;
    }

    Ref<HTMLDialogElement> protectedThis = ShareRef(*this);
    (void)FireBeforeToggleEvent(dom::ToggleState::Open, dom::ToggleState::Closed, false, source);

    // If subject does not have an open attribute, then return.
    if (!Open())
    {
      return;
    }

    QueueDialogToggleEventTask(dom::ToggleState::Open, dom::ToggleState::Closed, source);

    // Remove subject's open attribute (which runs the dialog cleanup steps).
    RemoveAttribute(dom::DOMStringAtom(u8"open"));

    // If is modal of subject is true, then request an element to be removed from the top layer given subject.
    if (IsModal() && IsInTopLayer())
    {
      RequestRemoveFromTopLayer();
    }

    bool wasModal = IsModal();
    SetIsModal(false);

    if (result)
    {
      _returnValue = krys::move(*result);
    }
    _requestCloseReturnValue = null;
    _requestCloseSourceElement = nullptr;

    // If subject's previously focused element is not null, then focus it if the focus is in subject or it
    // was modal, without scrolling.
    if (RefPtr<dom::Element> element = std::exchange(_previouslyFocusedElement, nullptr).lock())
    {
      auto *focusedArea = NodeDocument().FocusedElement();
      if (wasModal
          || (focusedArea && dom::TreeQueries::IsShadowIncludingInclusiveDescendant(*focusedArea, *this)))
      {
        FocusAlgorithms::FocusingSteps(*element, nullptr, {.PreventScroll = true});
      }
    }

    // Queue an element task on the user interaction task source given subject to fire an event named close
    // at subject.
    EventLoop::Current().QueueTask(
      TaskSource::UserInteraction, NodeDocument(),
      [protectedThis]
      {
        auto event = dom::Event::Create(dom::DOMStringAtom(EventNames::Close), false, false);
        (void)dom::EventDispatcher::DispatchToTarget(*event, protectedThis.get());
      });
  }

  void HTMLDialogElement::RequestCloseTheDialog(Maybe<dom::DOMString> returnValue,
                                                dom::Element *source) noexcept
  {
    // If subject does not have an open attribute, or isn't connected, or its node document isn't fully
    // active, then return.
    if (!Open() || !IsConnected() || !NodeDocument().IsFullyActive() || !_closeWatcher)
    {
      return;
    }

    Ref<HTMLDialogElement> protectedThis = ShareRef(*this);
    Ref<CloseWatcher> closeWatcher = ShareRef(*_closeWatcher);
    _enableCloseWatcherForRequestClose = true;
    _requestCloseReturnValue = krys::move(returnValue);
    _requestCloseSourceElement = CreateWeakPtr(source);
    (void)closeWatcher->RequestToClose(false);
    _enableCloseWatcherForRequestClose = false;
  }

  void HTMLDialogElement::RunFocusingSteps() noexcept
  {
    // FIXME: If the allow focus steps given subject's node document return false, then return, with
    // permissions policy.

    // Let control be subject if it has the autofocus attribute, else its focus delegate, else subject.
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

    // Run the focusing steps for control; a modal dialog that can't focus it moves the focus to the viewport.
    auto *controlElement = DynamicDowncast<dom::Element>(*control);
    if (controlElement && FocusAlgorithms::IsFocusableArea(*controlElement))
    {
      FocusAlgorithms::FocusingSteps(*controlElement);
    }
    else if (IsModal())
    {
      FocusAlgorithms::FocusingSteps(NodeDocument());
    }

    // Empty the top document's autofocus candidates, and set its autofocus processed flag.
    controlDocument->AutofocusCandidates().clear();
    controlDocument->SetAutofocusProcessed();
  }

  void HTMLDialogElement::OnInsert() noexcept
  {
    HTMLElement::OnInsert();

    // If insertedNode's node document is fully active and insertedNode has an open attribute and is
    // connected, then run the dialog setup steps given insertedNode.
    if (NodeDocument().IsFullyActive() && Open() && IsConnected())
    {
      SetupSteps();
    }
  }

  void HTMLDialogElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnRemove(isSubtreeRoot, oldAncestor);

    if (RefPtr<CloseWatcher> closeWatcher = std::exchange(_closeWatcher, nullptr))
    {
      closeWatcher->Destroy();
    }
    if (IsInTopLayer())
    {
      RemoveFromTopLayer();
    }
    SetIsModal(false);
    NodeDocument().RemoveOpenDialog(*this);
  }

  void HTMLDialogElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                             Maybe<dom::DOMStringView> value,
                                             dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);

    if (namespaceURI != dom::DOMStringAtom::Null() || localName != u8"open")
    {
      return;
    }
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

  void HTMLDialogElement::QueueDialogToggleEventTask(dom::ToggleState oldState, dom::ToggleState newState,
                                                     dom::Element *source) noexcept
  {
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

  void HTMLDialogElement::SetupSteps() noexcept
  {
    krys_debug_assert(Open() && IsConnected());
    auto &document = NodeDocument();
    if (document.OpenDialogs().contains(ShareRef(*this)))
    {
      return;
    }
    document.AddOpenDialog(*this);
    SetCloseWatcher();
  }

  void HTMLDialogElement::CleanupSteps() noexcept
  {
    NodeDocument().RemoveOpenDialog(*this);
    if (RefPtr<CloseWatcher> closeWatcher = std::exchange(_closeWatcher, nullptr))
    {
      closeWatcher->Destroy();
    }
  }

  void HTMLDialogElement::SetCloseWatcher() noexcept
  {
    krys_debug_assert(!_closeWatcher && Open());
    if (!NodeDocument().Window())
    {
      return;
    }

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
    // FIXME: Hide all popovers until the topmost popover ancestor of the dialog (or the document), with
    // popovers (B4d).
  }
}
