#pragma once

#include "Krystal.Booey/DOM/ToggleEventTask.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/HTML/CloseWatcher/CloseWatcher.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::html
{
  /// @brief A dialog box: shown with `Show()`, or `ShowModal()` in the top layer, where it blocks the rest of
  /// the document; closed with `Close()`, by a `method=dialog` form, or by the user through its close watcher
  /// (Escape, and a press outside it for `closedby=any`).
  /// @see https://html.spec.whatwg.org/#the-dialog-element
  class HTMLDialogElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLDialogElement);

  public:
    /// @see https://html.spec.whatwg.org/#attr-dialog-closedby
    enum class ClosedByState : uint8
    {
      Any,
      CloseRequest,
      None,
    };

  private:
    dom::DOMString _returnValue;
    /// @see https://html.spec.whatwg.org/#is-modal
    bool _isModal {false};
    /// @see https://html.spec.whatwg.org/#enable-close-watcher-for-requestclose()
    bool _enableCloseWatcherForRequestClose {false};
    /// @see https://html.spec.whatwg.org/#request-close-return-value
    Maybe<dom::DOMString> _requestCloseReturnValue;
    /// @see https://html.spec.whatwg.org/#request-close-source-element
    WeakPtr<dom::Element> _requestCloseSourceElement;
    /// @see https://html.spec.whatwg.org/#previously-focused-element
    WeakPtr<dom::Element> _previouslyFocusedElement;
    /// @see https://html.spec.whatwg.org/#dialog-close-watcher
    RefPtr<CloseWatcher> _closeWatcher;
    /// @see https://html.spec.whatwg.org/#dialog-toggle-task-tracker
    RefPtr<dom::ToggleEventTask> _toggleEventTask;

  public:
    HTMLDialogElement(dom::Document &document) noexcept;
    ~HTMLDialogElement() noexcept override;

#pragma region HTMLDialogElement - https://html.spec.whatwg.org/#htmldialogelement

    /// @see https://html.spec.whatwg.org/#dom-dialog-open
    KRYS_NODISCARD bool Open() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-dialog-open
    void Open(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-dialog-returnvalue
    KRYS_NODISCARD const dom::DOMString &ReturnValue() const noexcept
    {
      return _returnValue;
    }

    /// @see https://html.spec.whatwg.org/#dom-dialog-returnvalue
    void ReturnValue(dom::DOMString &&value) noexcept
    {
      _returnValue = krys::move(value);
    }

    /// @see https://html.spec.whatwg.org/#dom-dialog-closedby
    KRYS_NODISCARD dom::DOMString ClosedBy() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-dialog-closedby
    void ClosedBy(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-dialog-show
    dom::ExceptionOr<void> Show() noexcept;
    /// @see https://html.spec.whatwg.org/#dom-dialog-showmodal
    dom::ExceptionOr<void> ShowModal() noexcept;
    /// @see https://html.spec.whatwg.org/#dom-dialog-close
    void Close(Maybe<dom::DOMString> returnValue = null) noexcept;
    /// @see https://html.spec.whatwg.org/#dom-dialog-requestclose
    void RequestClose(Maybe<dom::DOMString> returnValue = null) noexcept;

#pragma endregion

    /// @see https://html.spec.whatwg.org/#is-modal
    KRYS_NODISCARD bool IsModal() const noexcept
    {
      return _isModal;
    }

    /// @brief The dialog's close watcher, while it's open and connected.
    KRYS_NODISCARD CloseWatcher *GetCloseWatcher() const noexcept
    {
      return _closeWatcher.get();
    }

    /// @see https://html.spec.whatwg.org/#computed-closed-by-state
    KRYS_NODISCARD ClosedByState ComputedClosedByState() const noexcept;

    /// @brief Shows the dialog modally, invoked by @p source.
    /// @see https://html.spec.whatwg.org/#show-a-modal-dialog
    dom::ExceptionOr<void> ShowModalDialog(dom::Element *source) noexcept;
    /// @brief Closes the dialog, setting its return value to @p result if there's one, invoked by @p source.
    /// @see https://html.spec.whatwg.org/#close-the-dialog
    void CloseTheDialog(Maybe<dom::DOMString> result, dom::Element *source) noexcept;
    /// @brief Asks the dialog to close (`cancel` can refuse), invoked by @p source.
    /// @see https://html.spec.whatwg.org/#dialog-request-close
    void RequestCloseTheDialog(Maybe<dom::DOMString> returnValue, dom::Element *source) noexcept;

    /// @see https://html.spec.whatwg.org/#dialog-focusing-steps
    void RunFocusingSteps() noexcept;

    /// @brief The popover commands, and the dialog commands.
    KRYS_NODISCARD bool IsValidCommandType(dom::CommandType command) const noexcept final;
    /// @brief Shows the dialog modally or closes it for the dialog commands (unless it's a showing popover),
    /// and shows or hides it for the popover commands.
    bool HandleCommandInternal(HTMLButtonElement &invoker, dom::CommandType command) noexcept final;

  protected:
    void OnInsert() noexcept override;
    void OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept override;
    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value,
                            dom::DOMStringAtom namespaceURI) noexcept override;

  private:
    void SetIsModal(bool value) noexcept;
    /// @see https://html.spec.whatwg.org/#queue-a-dialog-toggle-event-task
    void QueueDialogToggleEventTask(dom::ToggleState oldState, dom::ToggleState newState,
                                    dom::Element *source) noexcept;
    /// @brief Fires `beforetoggle` from @p oldState to @p newState.
    /// @return Whether it wasn't canceled.
    bool FireBeforeToggleEvent(dom::ToggleState oldState, dom::ToggleState newState, bool cancelable,
                               dom::Element *source) noexcept;
    /// @see https://html.spec.whatwg.org/#dialog-setup-steps
    void SetupSteps() noexcept;
    /// @see https://html.spec.whatwg.org/#dialog-cleanup-steps
    void CleanupSteps() noexcept;
    /// @see https://html.spec.whatwg.org/#set-the-dialog-close-watcher
    void SetCloseWatcher() noexcept;
    /// @brief Hides the popovers the dialog isn't in, as it shows.
    void HidePopoversOnShow() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLDialogElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLDialogElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLDialogElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
