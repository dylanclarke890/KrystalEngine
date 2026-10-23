#pragma once

#include "Krystal.Booey/Core/ScrollTypes.hpp"
#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/HTML/HTMLFormControlElement.hpp"

namespace krys::boo
{
  class VisiblePosition;
}

namespace krys::boo::dom
{
  class Position;
}

namespace krys::boo::html
{
  class TextControlInnerTextElement;

  enum class ForBindings : bool
  {
    No,
    Yes
  };

  enum class TextFieldSelectionDirection : uint8
  {
    SelectionHasNoDirection,
    SelectionHasForwardDirection,
    SelectionHasBackwardDirection
  };

  enum class TextFieldEventBehavior : uint8
  {
    DispatchNoEvent,
    DispatchChangeEvent,
    DispatchInputAndChangeEvent
  };

  enum class TextControlSetValueSelection : uint8
  {
    SetSelectionToEnd,
    Clamp,
    DoNotSet
  };

  /// @brief A form control whose value is text: an input or a textarea. Its text is in an inner text element
  /// in its user agent shadow tree, which the user edits, and its selection is the frame's selection there
  /// while it's focused (and a cached one while it isn't).
  /// @note Selection offsets are UTF-8 byte offsets into the value, as offsets in the DOM are.
  // FIXME: Hiding the placeholder while a pen hovers (the hover's pointer type), with pointer events; the
  // spelling markers (effectiveSpellcheckAttributeChanged), with spelling.
  class HTMLTextFormControlElement : public HTMLFormControlElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTextFormControlElement);

  private:
    TextFieldSelectionDirection _cachedSelectionDirection {
      TextFieldSelectionDirection::SelectionHasNoDirection};
    bool _lastChangeWasUserEdit : 1 {false};
    bool _isPlaceholderVisible : 1 {false};
    bool _canShowPlaceholder : 1 {true};
    bool _wasEverChangedByUserEdit : 1 {false};
    bool _hasCachedSelection : 1 {false};
    bool _hasScheduledSelectionChangeEvent : 1 {false};
    int32 _maxLength {-1};
    int32 _minLength {-1};
    uint32 _cachedSelectionStart {0};
    uint32 _cachedSelectionEnd {0};
    Maybe<dom::DOMString> _textAsOfLastFormControlChangeEvent;

  public:
    /// @brief Whether TooLong() and TooShort() check the dirty value flag.
    enum NeedsToCheckDirtyFlag
    {
      CheckDirtyFlag,
      IgnoreDirtyFlag
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
    void ForwardEvent(dom::Event &event) noexcept;

    KRYS_NODISCARD virtual html::HTMLElement *PlaceholderElement() const noexcept = 0;

    void SetCanShowPlaceholder(bool canShowPlaceholder) noexcept;

    KRYS_NODISCARD bool CanShowPlaceholder() const noexcept
    {
      return _canShowPlaceholder;
    }

    KRYS_NODISCARD int32 IndexForVisiblePosition(const VisiblePosition &position) const noexcept;
    KRYS_NODISCARD VisiblePosition VisiblePositionForIndex(int32 index) const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea/input-selectionstart
    KRYS_NODISCARD uint32 SelectionStart() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea/input-selectionend
    KRYS_NODISCARD uint32 SelectionEnd() const noexcept;

    /// @brief "forward", "backward" or "none".
    /// @see https://html.spec.whatwg.org/#dom-textarea/input-selectiondirection
    KRYS_NODISCARD dom::DOMStringView SelectionDirection() const noexcept;

    void SetSelectionStart(uint32 start) noexcept;
    void SetSelectionEnd(uint32 end) noexcept;
    void SetSelectionDirection(dom::DOMStringView direction) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea/input-select
    void Select(SelectionRevealMode revealMode = SelectionRevealMode::DoNotReveal) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea/input-setrangetext
    dom::ExceptionOr<void> SetRangeText(dom::DOMStringView replacement) noexcept;

    /// @param selectionMode "select", "start", "end" or "preserve".
    /// @see https://html.spec.whatwg.org/#dom-textarea/input-setrangetext
    virtual dom::ExceptionOr<void> SetRangeText(dom::DOMStringView replacement, uint32 start, uint32 end,
                                                dom::DOMStringView selectionMode) noexcept;

    /// @param direction "forward", "backward", or anything else for none.
    /// @see https://html.spec.whatwg.org/#dom-textarea/input-setselectionrange
    void SetSelectionRange(uint32 start, uint32 end, dom::DOMStringView direction,
                           ForBindings forBindings = ForBindings::No) noexcept;

    /// @brief Selects the text from @p start to @p end, clamped to the value (and to whole characters).
    /// @return Whether the cached selection changed.
    bool SetSelectionRange(
      uint32 start, uint32 end,
      TextFieldSelectionDirection direction = TextFieldSelectionDirection::SelectionHasNoDirection,
      SelectionRevealMode revealMode = SelectionRevealMode::DoNotReveal,
      ForBindings forBindings = ForBindings::No) noexcept;

    void ScheduleSelectionChangeEvent() noexcept;

    KRYS_NODISCARD TextFieldSelectionDirection ComputeSelectionDirection() const noexcept;

    /// @brief The cached selection as a range in the inner text, or nothing if there's none.
    KRYS_NODISCARD Maybe<dom::SimpleRange> Selection() const noexcept;
    KRYS_NODISCARD dom::DOMString SelectedText() const noexcept;

    void DispatchFormControlChangeEvent() noexcept final;
    void ScheduleSelectEvent() noexcept;

    KRYS_NODISCARD virtual dom::DOMString Value() const noexcept = 0;
    virtual dom::ExceptionOr<void> SetValue(
      dom::DOMStringView value,
      TextFieldEventBehavior eventBehavior = TextFieldEventBehavior::DispatchNoEvent,
      TextControlSetValueSelection selection = TextControlSetValueSelection::SetSelectionToEnd) noexcept = 0;

    KRYS_NODISCARD virtual RefPtr<TextControlInnerTextElement> InnerTextElement() const noexcept = 0;
    virtual RefPtr<TextControlInnerTextElement> InnerTextElementCreatingShadowSubtreeIfNeeded() noexcept = 0;

    /// @brief The style of the inner text element, from the control's.
    KRYS_NODISCARD virtual RenderStyle CreateInnerTextStyle(const RenderStyle &style) noexcept = 0;

    /// @brief Caches the selection the frame has in the control, firing `select` if @p shouldFireSelectEvent
    /// and it isn't collapsed.
    /// @return Whether the cached selection changed.
    bool SelectionChanged(bool shouldFireSelectEvent) noexcept;

    KRYS_NODISCARD bool LastChangeWasUserEdit() const noexcept;
    KRYS_NODISCARD bool WasEverChangedByUserEdit() const noexcept;

    /// @brief Replaces the inner text with @p value (with a `<br>` after a trailing line break).
    void SetInnerTextValue(dom::DOMString &&value) noexcept;

    /// @brief The inner text: its text, with a line break for each `<br>` but a trailing one.
    KRYS_NODISCARD dom::DOMString InnerTextValue() const noexcept;

    KRYS_NODISCARD virtual bool IsInnerTextElementEditable() const noexcept;

    KRYS_NODISCARD virtual bool SupportsPlaceholder() const noexcept = 0;

    /// @brief Whether the placeholder is shown, as `:placeholder-shown` matches.
    KRYS_NODISCARD bool IsPlaceholderVisible() const noexcept
    {
      return _isPlaceholderVisible;
    }

    void UpdatePlaceholderVisibility() noexcept;

    /// @brief The directionality the `dirname` attribute submits: "ltr" or "rtl".
    KRYS_NODISCARD dom::DOMString DirectionForFormData() const noexcept;

    /// @brief The length of @p text in UTF-16 code units, as script measures strings.
    KRYS_NODISCARD static size_t LengthForAPIValue(dom::DOMStringView text) noexcept;

    void SetTextAsOfLastFormControlChangeEvent(dom::DOMString &&text) noexcept
    {
      _textAsOfLastFormControlChangeEvent = krys::move(text);
    }

  private:
    KRYS_NODISCARD bool PlaceholderShouldBeVisible() const noexcept;

  protected:
    HTMLTextFormControlElement(dom::Document &document, HTMLElementInterface interface) noexcept;

    KRYS_NODISCARD bool IsPlaceholderEmpty() const noexcept;
    virtual void UpdatePlaceholderText() noexcept = 0;

    void DisabledStateChanged() noexcept override;
    void ReadOnlyStateChanged() noexcept override;

    KRYS_NODISCARD bool ReadOnlyBarsFromConstraintValidation() const noexcept final
    {
      return true;
    }

    void UpdateInnerTextElementEditability() noexcept;

    bool CacheSelection(uint32 start, uint32 end, TextFieldSelectionDirection direction) noexcept;

    void RestoreCachedSelection(SelectionRevealMode revealMode) noexcept;

    KRYS_NODISCARD bool HasCachedSelection() const noexcept
    {
      return _hasCachedSelection;
    }

    KRYS_NODISCARD uint32 ComputeSelectionStart() const noexcept;
    KRYS_NODISCARD uint32 ComputeSelectionEnd() const noexcept;

    virtual void SubtreeHasChanged() noexcept = 0;

    /// @brief The value with a line break where each line wraps, for `wrap="hard"`.
    KRYS_NODISCARD dom::DOMString ValueWithHardLineBreaks() const noexcept;

    void AdjustInnerTextStyle(const RenderStyle &parentStyle, RenderStyle &textBlockStyle) const noexcept;

    void OnInsert() noexcept override;

    KRYS_NODISCARD bool ChildShouldCreateRenderer(const dom::Node &child) const noexcept override;

    void DispatchFocusEvent(dom::EventTarget *relatedTarget,
                            const dom::FocusOptions &options) noexcept override;
    void DispatchBlurEvent(dom::EventTarget *relatedTarget) noexcept override;

    // Returns true if user-editable value is empty. Used to check placeholder visibility.
    KRYS_NODISCARD virtual bool IsEmptyValue() const noexcept = 0;
    // Called in DispatchFocusEvent(), after placeholder process, before calling parent's
    // DispatchFocusEvent().
    virtual void HandleFocusEvent(dom::Node * /* oldFocusedNode */) noexcept
    {
    }
    // Called in DispatchBlurEvent(), after placeholder process, before calling parent's DispatchBlurEvent().
    virtual void HandleBlurEvent() noexcept
    {
    }

    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value,
                            dom::DOMStringAtom namespaceURI) noexcept override;

    void SetLastChangeWasNotUserEdit() noexcept
    {
      _lastChangeWasUserEdit = false;
    }

    void InternalSetMaxLength(int32 maxLength) noexcept
    {
      _maxLength = maxLength;
    }

    void InternalSetMinLength(int32 minLength) noexcept
    {
      _minLength = minLength;
    }

    /// @brief The value of a `maxlength` or `minlength` attribute: a valid non-negative integer, or -1.
    KRYS_NODISCARD static int32 ParseLengthAttribute(Maybe<dom::DOMStringView> value) noexcept;

  private:
    KRYS_NODISCARD uint32 IndexForPosition(const dom::Position &position) const noexcept;
  };

  /// @brief The text control whose inner text @p position is in, or null.
  KRYS_NODISCARD HTMLTextFormControlElement *EnclosingTextFormControl(const dom::Position &position) noexcept;
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLTextFormControlElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    if (!target.IsHTMLElement())
    {
      return false;
    }
    auto &element = Downcast<krys::boo::html::HTMLElement>(target);
    return element.IsHTMLInputElement() || element.IsHTMLTextAreaElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLInputElement() || target.IsHTMLTextAreaElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::dom::EventTarget &target) noexcept
  {
    auto *node = DynamicDowncast<krys::boo::dom::Node>(target);
    return node && IsType(*node);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
