#pragma once

#include "Krystal.Booey/HTML/HTMLDivElement.hpp"

namespace krys::boo
{
  class DateComponents;
  class Locale;
  class RenderStyle;
}

namespace krys::boo::dom
{
  class KeyboardEvent;
}

namespace krys::boo::html
{
  class DateTimeFieldElement;
  struct DateTimeFieldsState;

  enum class DateTimePlaceholderIfNoValue : bool
  {
    No,
    Yes
  };

  /// @brief What a date or time field is part of: a date or time input's editable fields.
  class DateTimeFieldElementFieldOwner
  {
  public:
    virtual ~DateTimeFieldElementFieldOwner() noexcept = default;
    virtual void DidBlurFromField(dom::Event &event) noexcept = 0;
    virtual void FieldValueChanged() noexcept = 0;
    virtual bool FocusOnNextField(const DateTimeFieldElement &field) noexcept = 0;
    virtual bool FocusOnPreviousField(const DateTimeFieldElement &field) noexcept = 0;
    KRYS_NODISCARD virtual bool IsFieldOwnerDisabled() const noexcept = 0;
    KRYS_NODISCARD virtual bool IsFieldOwnerReadOnly() const noexcept = 0;
    KRYS_NODISCARD virtual bool IsFieldOwnerHorizontal() const noexcept = 0;
    KRYS_NODISCARD virtual dom::DOMString LocaleIdentifier() const noexcept = 0;
    /// @brief The date fields show (greyed) until they have values: today.
    KRYS_NODISCARD virtual const DateComponents &PlaceholderDate() const noexcept = 0;
  };

  /// @brief One of a date or time input's editable fields: a number or a name (a month, AM or PM) that the
  /// arrow keys step and typing sets, showing a placeholder until it has a value.
  class DateTimeFieldElement : public HTMLDivElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(DateTimeFieldElement);

  public:
    enum class EventBehavior : bool
    {
      DispatchNoEvent,
      DispatchInputAndChangeEvents
    };

  private:
    // The owner clears this when it goes (RemoveFieldOwner).
    DateTimeFieldElementFieldOwner *_fieldOwner;

  public:
    void DefaultEventHandler(dom::Event &event) noexcept override;

    KRYS_NODISCARD dom::DOMString VisibleValue() const noexcept;

    void RemoveFieldOwner() noexcept
    {
      _fieldOwner = nullptr;
    }

    KRYS_NODISCARD virtual bool HasValue() const noexcept = 0;
    virtual void PopulateDateTimeFieldsState(
      DateTimeFieldsState &state,
      DateTimePlaceholderIfNoValue placeholderIfNoValue = DateTimePlaceholderIfNoValue::No) noexcept = 0;
    virtual void SetEmptyValue(EventBehavior eventBehavior = EventBehavior::DispatchNoEvent) noexcept = 0;
    virtual void SetValueAsDate(const DateComponents &date) noexcept = 0;
    virtual void SetValueAsInteger(int value,
                                   EventBehavior eventBehavior = EventBehavior::DispatchNoEvent) noexcept = 0;
    virtual void StepDown() noexcept = 0;
    virtual void StepUp() noexcept = 0;
    KRYS_NODISCARD virtual dom::DOMString Value() const noexcept = 0;
    KRYS_NODISCARD virtual dom::DOMString PlaceholderValue() const noexcept = 0;

  protected:
    DateTimeFieldElement(dom::Document &document, DateTimeFieldElementFieldOwner &fieldOwner) noexcept;

    /// @brief Makes @p field the part named @p part, focusable (its parts are what a date input focuses).
    static void Initialize(DateTimeFieldElement &field, dom::DOMStringAtom part) noexcept;

    KRYS_NODISCARD Locale &LocaleForOwner() const noexcept;
    KRYS_NODISCARD dom::DOMString LocaleIdentifier() const noexcept;
    void UpdateVisibleValue(EventBehavior eventBehavior) noexcept;
    virtual void AdjustMinInlineSize(RenderStyle &style) const noexcept = 0;
    KRYS_NODISCARD virtual int ValueAsInteger() const noexcept = 0;
    KRYS_NODISCARD virtual int PlaceholderValueAsInteger() const noexcept = 0;
    virtual void HandleKeyboardEvent(dom::KeyboardEvent &event) noexcept = 0;
    virtual void HandleBlurEvent(dom::Event &event) noexcept;

    KRYS_NODISCARD DateTimeFieldElementFieldOwner *FieldOwner() const noexcept
    {
      return _fieldOwner;
    }

  private:
    KRYS_NODISCARD Maybe<style::UnadjustedStyle>
      ResolveCustomStyle(const style::ResolutionContext &resolutionContext,
                         const RenderStyle *shadowHostStyle) noexcept final;

    void DefaultKeyboardEventHandler(dom::KeyboardEvent &event) noexcept;
    KRYS_NODISCARD bool IsFieldOwnerDisabled() const noexcept;
    KRYS_NODISCARD bool IsFieldOwnerReadOnly() const noexcept;
    KRYS_NODISCARD bool IsFieldOwnerHorizontal() const noexcept;
  };
}
