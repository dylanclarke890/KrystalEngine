#pragma once

#include "Krystal.Web/Html/Elements/HTMLFormControlElement.hpp"
#include "Krystal.Web/Platform/Scrolling/ScrollTypes.hpp"

namespace krys::web::dom
{
  struct SimpleRange;
}

namespace krys::web::html
{
  class TextControlInnerTextElement;

  enum class ForBindings : bool
  {
    No,
    Yes,
  };

  enum class TextFieldSelectionDirection : uint8
  {
    SelectionHasNoDirection,
    SelectionHasForwardDirection,
    SelectionHasBackwardDirection,
  };

  enum class TextFieldEventBehavior : uint8
  {
    DispatchNoEvent,
    DispatchChangeEvent,
    DispatchInputAndChangeEvent,
  };

  enum class TextControlSetValueSelection : uint8
  {
    SetSelectionToEnd,
    Clamp,
    DoNotSet,
  };
}

namespace krys::web
{
  class Position;
  class VisiblePosition;

  /// @brief A form control whose value is text: an input or a textarea. Its text is in an inner text element
  /// in its user agent shadow tree, which the user edits, and its selection is the frame's selection there
  /// while it's focused (and a cached one while it isn't).
  /// @note Selection offsets are UTF-8 byte offsets into the value, as offsets in the DOM are.
  // FIXME: Hiding the placeholder while a pen hovers (the hover's pointer type). (There are no spelling
  // markers for spellcheck to change; see DIFFERENCES.md.)
  class HTMLTextFormControlElement : public HTMLFormControlElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTextFormControlElement);

  private:
    html::TextFieldSelectionDirection _cachedSelectionDirection {
      html::TextFieldSelectionDirection::SelectionHasNoDirection};
    bool _lastChangeWasUserEdit : 1 {false};
    bool _isPlaceholderVisible : 1 {false};
    bool _hasCachedSelection : 1 {false};
    bool _hasScheduledSelectionChangeEvent : 1 {false};
    int32 _maxLength {-1};
    int32 _minLength {-1};
    uint32 _cachedSelectionStart {0};
    uint32 _cachedSelectionEnd {0};
    Maybe<DOMString> _textAsOfLastFormControlChangeEvent;

  public:
    /// @brief Whether TooLong() and TooShort() check the dirty value flag.
    enum NeedsToCheckDirtyFlag
    {
      CheckDirtyFlag,
      IgnoreDirtyFlag,
    };

    ~HTMLTextFormControlElement() noexcept override;

    KRYS_NODISCARD bool CanContainRangeEndPoint() const noexcept override
    {
      return false;
    }

    /// @brief The `maxlength` attribute's value, or -1 if it isn't a valid non-negative integer.
    KRYS_NODISCARD int32 MaxLength() const noexcept
    {
      return _maxLength;
    }

    /// @brief The `minlength` attribute's value, or -1 if it isn't a valid non-negative integer.
    KRYS_NODISCARD int32 MinLength() const noexcept
    {
      return _minLength;
    }

    /// @brief Called after the user (or an edit command) changed the inner text.
    void DidEditInnerTextValue(bool wasUserEdit) noexcept;

    /// @brief Hands an event to the inner text element's default event handler (mouse events, so that the
    /// user can select in the control from its border or padding).
    void ForwardEvent(Event &event) noexcept;

    /// @brief The shadow tree element that shows the placeholder, if there is one.
    KRYS_NODISCARD virtual HTMLElement *PlaceholderElement() const noexcept = 0;

    /// @brief Converts between a position in the inner text and an index in the value.
    KRYS_NODISCARD int32 IndexForVisiblePosition(const VisiblePosition &position) const noexcept;

    KRYS_NODISCARD VisiblePosition VisiblePositionForIndex(int32 index) const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea/input-selectionstart
    KRYS_NODISCARD uint32 SelectionStart() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea/input-selectionend
    KRYS_NODISCARD uint32 SelectionEnd() const noexcept;

    /// @brief "forward", "backward" or "none".
    /// @see https://html.spec.whatwg.org/#dom-textarea/input-selectiondirection
    KRYS_NODISCARD DOMStringView SelectionDirection() const noexcept;

    /// @brief Sets one end, or the direction, of the selection, keeping the rest.
    /// @see https://html.spec.whatwg.org/#dom-textarea/input-selectionstart
    void SetSelectionStart(uint32 start) noexcept;

    void SetSelectionEnd(uint32 end) noexcept;

    void SetSelectionDirection(DOMStringView direction) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea/input-select
    void Select(SelectionRevealMode revealMode = SelectionRevealMode::DoNotReveal) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea/input-setrangetext
    ExceptionOr<void> SetRangeText(DOMStringView replacement) noexcept;

    /// @param selectionMode "select", "start", "end" or "preserve".
    /// @see https://html.spec.whatwg.org/#dom-textarea/input-setrangetext
    virtual ExceptionOr<void> SetRangeText(DOMStringView replacement, uint32 start, uint32 end,
                                           DOMStringView selectionMode) noexcept;

    /// @param direction "forward", "backward", or anything else for none.
    /// @see https://html.spec.whatwg.org/#dom-textarea/input-setselectionrange
    void SetSelectionRange(uint32 start, uint32 end, DOMStringView direction,
                           html::ForBindings forBindings = html::ForBindings::No) noexcept;

    /// @brief Selects the text from @p start to @p end, clamped to the value (and to whole characters).
    /// @return Whether the cached selection changed.
    bool SetSelectionRange(uint32 start, uint32 end,
                           html::TextFieldSelectionDirection direction =
                             html::TextFieldSelectionDirection::SelectionHasNoDirection,
                           SelectionRevealMode revealMode = SelectionRevealMode::DoNotReveal,
                           html::ForBindings forBindings = html::ForBindings::No) noexcept;

    /// @brief Queues a `selectionchange` event at the control, unless one is queued already.
    void ScheduleSelectionChangeEvent() noexcept;

    /// @brief The direction of the frame's selection in the control, or the cached one.
    KRYS_NODISCARD html::TextFieldSelectionDirection ComputeSelectionDirection() const noexcept;

    /// @brief The cached selection as a range in the inner text, or nothing if there's none.
    KRYS_NODISCARD Maybe<dom::SimpleRange> Selection() const noexcept;

    /// @brief The selected part of the value.
    KRYS_NODISCARD DOMString SelectedText() const noexcept;

    void DispatchFormControlChangeEvent() noexcept final;

    /// @brief Queues `select` and `selectionchange` events at the control.
    void ScheduleSelectEvent() noexcept;

    /// @brief The control's value, and its setter.
    KRYS_NODISCARD virtual DOMString Value() const noexcept = 0;

    /// @brief Whether the control's auto directionality comes from its value rather than its text.
    /// @see https://html.spec.whatwg.org/multipage/dom.html#auto-directionality-form-associated-elements
    KRYS_NODISCARD virtual bool DirAutoUsesValue() const noexcept = 0;

    virtual ExceptionOr<void>
      SetValue(DOMStringView value,
               html::TextFieldEventBehavior eventBehavior = html::TextFieldEventBehavior::DispatchNoEvent,
               html::TextControlSetValueSelection selection =
                 html::TextControlSetValueSelection::SetSelectionToEnd) noexcept = 0;

    /// @brief The shadow tree element the value is edited in, if the shadow tree exists; the second form
    /// makes the shadow tree first.
    KRYS_NODISCARD virtual RefPtr<html::TextControlInnerTextElement> InnerTextElement() const noexcept = 0;

    virtual RefPtr<html::TextControlInnerTextElement>
      InnerTextElementCreatingShadowSubtreeIfNeeded() noexcept = 0;

    /// @brief The style of the inner text element, from the control's.
    KRYS_NODISCARD virtual RenderStyle CreateInnerTextStyle(const RenderStyle &style) noexcept = 0;

    /// @brief Caches the selection the frame has in the control, firing `select` if @p shouldFireSelectEvent
    /// and it isn't collapsed.
    /// @return Whether the cached selection changed.
    bool SelectionChanged(bool shouldFireSelectEvent) noexcept;

    /// @brief Whether the value last changed by the user's editing (rather than from the API).
    KRYS_NODISCARD bool LastChangeWasUserEdit() const noexcept;

    /// @brief Replaces the inner text with @p value (with a `<br>` after a trailing line break).
    void SetInnerTextValue(DOMString value) noexcept;

    /// @brief The inner text: its text, with a line break for each `<br>` but a trailing one.
    KRYS_NODISCARD DOMString InnerTextValue() const noexcept;

    /// @brief Whether the user can edit the value: the control isn't disabled or read-only.
    KRYS_NODISCARD virtual bool IsInnerTextElementEditable() const noexcept;

    /// @brief Whether the control can show a placeholder.
    KRYS_NODISCARD virtual bool SupportsPlaceholder() const noexcept = 0;

    /// @brief Whether the placeholder is shown, as `:placeholder-shown` matches.
    KRYS_NODISCARD bool IsPlaceholderVisible() const noexcept
    {
      return _isPlaceholderVisible;
    }

    /// @brief Shows or hides the placeholder, restyling :placeholder-shown.
    void UpdatePlaceholderVisibility() noexcept;

    /// @brief The directionality the `dirname` attribute submits: "ltr" or "rtl".
    KRYS_NODISCARD DOMString DirectionForFormData() const noexcept;

    /// @brief The length of @p text in UTF-16 code units, as script measures strings.
    KRYS_NODISCARD static size_t LengthForAPIValue(DOMStringView text) noexcept;

    /// @brief Records the value as of the last change event, to know whether the next one fires.
    void SetTextAsOfLastFormControlChangeEvent(DOMString text) noexcept
    {
      _textAsOfLastFormControlChangeEvent = krys::move(text);
    }

  private:
    /// @brief Whether the placeholder should show: the control supports one, its value is empty and its
    /// placeholder isn't (line breaks aside).
    KRYS_NODISCARD bool PlaceholderShouldBeVisible() const noexcept;

  protected:
    HTMLTextFormControlElement(Document &document, html::HTMLElementInterface interface) noexcept;

    /// @brief Whether the placeholder attribute is missing or only line breaks.
    KRYS_NODISCARD bool IsPlaceholderEmpty() const noexcept;

    /// @brief Updates the placeholder element's text from the attribute.
    virtual void UpdatePlaceholderText() noexcept = 0;

    void DisabledStateChanged() noexcept override;

    void ReadOnlyStateChanged() noexcept override;

    KRYS_NODISCARD bool ReadOnlyBarsFromConstraintValidation() const noexcept final
    {
      return true;
    }

    /// @brief Makes the inner text editable or not, as the control's disabled or read-only state changes.
    void UpdateInnerTextElementEditability() noexcept;

    /// @brief Keeps the selection, for the selection APIs while the control isn't focused.
    /// @return Whether it changed.
    bool CacheSelection(uint32 start, uint32 end, html::TextFieldSelectionDirection direction) noexcept;

    /// @brief Selects the kept selection again (as the control is focused), firing `select` if it changed.
    void RestoreCachedSelection(SelectionRevealMode revealMode) noexcept;

    /// @brief Whether a selection has been kept.
    KRYS_NODISCARD bool HasCachedSelection() const noexcept
    {
      return _hasCachedSelection;
    }

    /// @brief The start and end of the frame's selection in the control, or the kept ones.
    KRYS_NODISCARD uint32 ComputeSelectionStart() const noexcept;

    KRYS_NODISCARD uint32 ComputeSelectionEnd() const noexcept;

    /// @brief Called after the user edited the inner text.
    virtual void SubtreeHasChanged() noexcept = 0;

    /// @brief The value with a line break where each line wraps, for `wrap="hard"`.
    KRYS_NODISCARD DOMString ValueWithHardLineBreaks() const noexcept;

    /// @brief Gives the inner text's style what it takes from the control's: its direction, bidi, field
    /// sizing and so on.
    static void AdjustInnerTextStyle(const RenderStyle &parentStyle, RenderStyle &textBlockStyle) noexcept;

    void OnInsert() noexcept override;

    KRYS_NODISCARD bool ChildShouldCreateRenderer(const Node &child) const noexcept override;

    void DispatchFocusEvent(EventTarget *relatedTarget, const dom::FocusOptions &options) noexcept override;

    void DispatchBlurEvent(EventTarget *relatedTarget) noexcept override;

    /// @brief Whether the value the user edits is empty, which shows the placeholder.
    KRYS_NODISCARD virtual bool IsEmptyValue() const noexcept = 0;

    /// @brief Called as the control gains or loses the focus: after the placeholder's update, before the
    /// focus or blur event.
    virtual void HandleFocusEvent(Node * /* oldFocusedNode */) noexcept
    {
    }

    virtual void HandleBlurEvent() noexcept
    {
    }

    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept override;

    /// @brief Records that the value changed from the API.
    void SetLastChangeWasNotUserEdit() noexcept
    {
      _lastChangeWasUserEdit = false;
    }

    /// @brief Keeps the parsed `maxlength` and `minlength` attributes (-1 for none).
    void InternalSetMaxLength(int32 maxLength) noexcept
    {
      _maxLength = maxLength;
    }

    void InternalSetMinLength(int32 minLength) noexcept
    {
      _minLength = minLength;
    }

    /// @brief The value of a `maxlength` or `minlength` attribute: a valid non-negative integer, or -1.
    KRYS_NODISCARD static int32 ParseLengthAttribute(Maybe<DOMStringView> value) noexcept;

  private:
    /// @brief The index in the value of a position in the inner text.
    KRYS_NODISCARD uint32 IndexForPosition(const Position &position) const noexcept;
  };
}

namespace krys::web::html
{
  /// @brief The text control whose inner text @p position is in, or null.
  KRYS_NODISCARD HTMLTextFormControlElement *EnclosingTextFormControl(const Position &position) noexcept;
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLTextFormControlElement)
  KRYS_NODISCARD static bool IsType(const krys::web::Node &target) noexcept
  {
    if (!target.IsHTMLElement())
    {
      return false;
    }
    auto &element = Downcast<krys::web::HTMLElement>(target);
    return element.IsHTMLInputElement() || element.IsHTMLTextAreaElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLInputElement() || target.IsHTMLTextAreaElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::web::EventTarget &target) noexcept
  {
    auto *node = DynamicDowncast<krys::web::Node>(target);
    return node && IsType(*node);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
