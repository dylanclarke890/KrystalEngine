#pragma once

#include "Krystal.Booey/HTML/HTMLFormControlElement.hpp"

namespace krys::boo::html
{
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

  /// @brief A form control whose value is text: an input or a textarea.
  // FIXME: The inner text element, the placeholder, the selection APIs and the editing they drive come with
  // text fields' shadow trees.
  class HTMLTextFormControlElement : public HTMLFormControlElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTextFormControlElement);

  private:
    bool _lastChangeWasUserEdit : 1 {false};
    bool _wasEverChangedByUserEdit : 1 {false};
    bool _isPlaceholderVisible : 1 {false};
    int32 _maxLength {-1};
    int32 _minLength {-1};
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

    void DispatchFormControlChangeEvent() noexcept final;

    KRYS_NODISCARD virtual dom::DOMString Value() const noexcept = 0;
    virtual dom::ExceptionOr<void> SetValue(
      dom::DOMStringView value,
      TextFieldEventBehavior eventBehavior = TextFieldEventBehavior::DispatchNoEvent,
      TextControlSetValueSelection selection = TextControlSetValueSelection::SetSelectionToEnd) noexcept = 0;

    KRYS_NODISCARD bool LastChangeWasUserEdit() const noexcept;
    KRYS_NODISCARD bool WasEverChangedByUserEdit() const noexcept;

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
    KRYS_NODISCARD bool IsPlaceholderEmpty() const noexcept;
    KRYS_NODISCARD bool PlaceholderShouldBeVisible() const noexcept;

  public:
  protected:
    HTMLTextFormControlElement(dom::Document &document, HTMLElementInterface interface) noexcept;

    KRYS_NODISCARD bool ReadOnlyBarsFromConstraintValidation() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD virtual bool IsEmptyValue() const noexcept = 0;

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
  };
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
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
