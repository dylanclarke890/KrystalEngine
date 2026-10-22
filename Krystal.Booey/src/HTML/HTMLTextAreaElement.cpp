#include "Krystal.Booey/HTML/HTMLTextAreaElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/DOMFormData.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"

namespace krys::boo::html
{
  namespace
  {
    /// @brief The length of @p text as script measures it, with each CRLF counted once.
    KRYS_NODISCARD size_t ComputeLengthForAPIValue(dom::DOMStringView text) noexcept
    {
      size_t crlfCount = 0;
      for (size_t i = 0; i + 1 < text.size(); ++i)
      {
        if (text[i] == u8'\r' && text[i + 1] == u8'\n')
        {
          ++crlfCount;
        }
      }
      return HTMLTextAreaElement::LengthForAPIValue(text) - crlfCount;
    }

    /// @brief @p value with its CRLF and CR line breaks made LFs.
    KRYS_NODISCARD dom::DOMString SimplifyNewLines(dom::DOMStringView value) noexcept
    {
      dom::DOMString result;
      result.reserve(value.size());
      for (size_t i = 0; i < value.size(); ++i)
      {
        if (value[i] == u8'\r')
        {
          result += u8'\n';
          if (i + 1 < value.size() && value[i + 1] == u8'\n')
          {
            ++i;
          }
        }
        else
        {
          result += value[i];
        }
      }
      return result;
    }
  }

  HTMLTextAreaElement::HTMLTextAreaElement(dom::Document &document) noexcept
      : HTMLTextFormControlElement(document, HTMLElementInterface::TextArea)
  {
    SetFormControlValueMatchesRenderer(true);
  }

  void HTMLTextAreaElement::OnChildrenChanged() noexcept
  {
    HTMLTextFormControlElement::OnChildrenChanged();
    SetLastChangeWasNotUserEdit();
    if (!_isDirty)
    {
      SetNonDirtyValue(DefaultValue(), TextControlSetValueSelection::Clamp);
    }
    // FIXME: Setting the inner text value of a dirty text area, with its shadow tree.
  }

  void HTMLTextAreaElement::OnClone(dom::Node &copy, bool subtree) noexcept
  {
    HTMLTextFormControlElement::OnClone(copy, subtree);
    auto &clone = Downcast<HTMLTextAreaElement>(copy);
    clone.SetValueCommon(Value(), TextFieldEventBehavior::DispatchNoEvent,
                         TextControlSetValueSelection::DoNotSet);
    clone._isDirty = _isDirty;
    clone.UpdateValidity();
  }

  void HTMLTextAreaElement::OnAttributeChanged(dom::DOMStringAtom localName,
                                               Maybe<dom::DOMStringView> oldValue,
                                               Maybe<dom::DOMStringView> value,
                                               dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLTextFormControlElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      return;
    }

    if (localName == u8"wrap")
    {
      // The virtual/physical values were a Netscape extension of HTML 3.0, now deprecated.
      // The soft/hard /off values are a recommendation for HTML 4 extension by IE and NS 4.
      auto newValue = value.value_or(dom::DOMStringView());
      WrapMethod wrap;
      if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(newValue, u8"physical")
          || infra::StringAlgorithms::ASCIICaseInsensitiveMatch(newValue, u8"hard")
          || infra::StringAlgorithms::ASCIICaseInsensitiveMatch(newValue, u8"on"))
      {
        wrap = HardWrap;
      }
      else if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(newValue, u8"off"))
      {
        wrap = NoWrap;
      }
      else
      {
        wrap = SoftWrap;
      }
      _wrap = wrap;
    }
    else if (localName == u8"maxlength")
    {
      InternalSetMaxLength(ParseLengthAttribute(value));
      UpdateValidity();
    }
    else if (localName == u8"minlength")
    {
      InternalSetMinLength(ParseLengthAttribute(value));
      UpdateValidity();
    }
  }

  bool HTMLTextAreaElement::AppendFormData(DOMFormData &formData) noexcept
  {
    auto name = Name();
    if (name.empty())
    {
      return false;
    }

    // FIXME: The value with the line breaks wrapping makes (valueWithHardLineBreaks) for wrap="hard", with
    // text fields' layout.
    formData.Append(name, Value());
    if (auto dirname = GetAttribute(u8"dirname"))
    {
      formData.Append(*dirname, DirectionForFormData());
    }
    return true;
  }

  void HTMLTextAreaElement::Reset() noexcept
  {
    SetInteractedWithSinceLastFormSubmitEvent(false);
    SetNonDirtyValue(DefaultValue(), TextControlSetValueSelection::SetSelectionToEnd);
  }

  dom::DOMString HTMLTextAreaElement::Value() const noexcept
  {
    // FIXME: Updating the value from the inner text the user edited (updateValue), with text fields.
    return _value;
  }

  dom::ExceptionOr<void> HTMLTextAreaElement::SetValue(dom::DOMStringView value,
                                                       TextFieldEventBehavior eventBehavior,
                                                       TextControlSetValueSelection selection) noexcept
  {
    SetValueCommon(value, eventBehavior, selection);
    _isDirty = true;
    UpdateValidity();
    return {};
  }

  void HTMLTextAreaElement::SetNonDirtyValue(dom::DOMStringView value,
                                             TextControlSetValueSelection selection) noexcept
  {
    SetValueCommon(value, TextFieldEventBehavior::DispatchNoEvent, selection);
    _isDirty = false;
    UpdateValidity();
  }

  void HTMLTextAreaElement::SetValueCommon(dom::DOMStringView newValue, TextFieldEventBehavior,
                                           TextControlSetValueSelection) noexcept
  {
    _wasModifiedByUser = false;
    // Line endings the user types or pastes are normalised elsewhere; these come from script.
    auto normalizedValue = SimplifyNewLines(newValue);

    // Return early because we don't want to move the caret or trigger other side effects when the value isn't
    // changing.
    if (normalizedValue == Value())
    {
      return;
    }

    _value = normalizedValue;
    SetLastChangeWasNotUserEdit();
    UpdatePlaceholderVisibility();
    InvalidateStyleForSubtree();
    SetFormControlValueMatchesRenderer(true);

    // FIXME: The inner text value, the selection (set to the end or clamped),
    // auto directionality, accessibility and the chrome client's didProgrammaticallyClearTextFormControl,
    // with text fields.

    SetTextAsOfLastFormControlChangeEvent(krys::move(normalizedValue));
  }

  dom::DOMString HTMLTextAreaElement::DefaultValue() const noexcept
  {
    return dom::TextAlgorithms::ChildTextContent(*this);
  }

  void HTMLTextAreaElement::DefaultValue(dom::DOMString &&defaultValue) noexcept
  {
    (void)TextContent(krys::move(defaultValue));
  }

  uint32 HTMLTextAreaElement::TextLength() const noexcept
  {
    return static_cast<uint32>(LengthForAPIValue(Value()));
  }

  bool HTMLTextAreaElement::ValueMissing() const noexcept
  {
    return ValueMissing(std::nullopt);
  }

  bool HTMLTextAreaElement::TooShort() const noexcept
  {
    return TooShort(std::nullopt, CheckDirtyFlag);
  }

  bool HTMLTextAreaElement::TooLong() const noexcept
  {
    return TooLong(std::nullopt, CheckDirtyFlag);
  }

  bool HTMLTextAreaElement::ValueMissing(Maybe<dom::DOMStringView> value) const noexcept
  {
    if (!(IsRequired() && IsMutable()))
    {
      return false;
    }
    if (!value)
    {
      return Value().empty();
    }
    return value->empty();
  }

  bool HTMLTextAreaElement::IsValidValue(dom::DOMStringView candidate) const noexcept
  {
    return !ValueMissing(candidate) && !TooShort(candidate, IgnoreDirtyFlag)
           && !TooLong(candidate, IgnoreDirtyFlag);
  }

  bool HTMLTextAreaElement::TooShort(Maybe<dom::DOMStringView> value,
                                     NeedsToCheckDirtyFlag check) const noexcept
  {
    // Return false for the default value or value set by script even if it is shorter than minLength.
    if (check == CheckDirtyFlag && !_wasModifiedByUser)
    {
      return false;
    }

    int32 min = MinLength();
    if (min <= 0)
    {
      return false;
    }

    // The empty string is excluded from tooShort validation.
    size_t length = !value ? LengthForAPIValue(Value()) : ComputeLengthForAPIValue(*value);
    return length > 0 && length < static_cast<size_t>(min);
  }

  bool HTMLTextAreaElement::TooLong(Maybe<dom::DOMStringView> value,
                                    NeedsToCheckDirtyFlag check) const noexcept
  {
    // Return false for the default value or value set by script even if it is longer than maxLength.
    if (check == CheckDirtyFlag && !_wasModifiedByUser)
    {
      return false;
    }

    int32 max = MaxLength();
    if (max < 0)
    {
      return false;
    }

    size_t length = !value ? LengthForAPIValue(Value()) : ComputeLengthForAPIValue(*value);
    return length > static_cast<size_t>(max);
  }

#pragma region HTMLTextAreaElement

  dom::DOMString HTMLTextAreaElement::Autocomplete() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"autocomplete").value_or(u8"");
  }

  void HTMLTextAreaElement::Autocomplete(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"autocomplete", krys::move(value));
  }

  uint32 HTMLTextAreaElement::Cols() const noexcept
  {
    return Reflection::Reflect<uint32>(*this, u8"cols");
  }

  void HTMLTextAreaElement::Cols(uint32 value) noexcept
  {
    Reflection::Reflect<uint32>(*this, u8"cols", krys::move(value));
  }

  dom::DOMString HTMLTextAreaElement::DirName() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"dirname").value_or(u8"");
  }

  void HTMLTextAreaElement::DirName(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"dirname", krys::move(value));
  }

  bool HTMLTextAreaElement::Disabled() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLTextAreaElement::Disabled(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"disabled", krys::move(value));
  }

  void HTMLTextAreaElement::MaxLength(int32 value) noexcept
  {
    Reflection::Reflect<int32>(*this, u8"maxlength", krys::move(value));
  }

  void HTMLTextAreaElement::MinLength(int32 value) noexcept
  {
    Reflection::Reflect<int32>(*this, u8"minlength", krys::move(value));
  }

  dom::DOMString HTMLTextAreaElement::Name() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLTextAreaElement::Name(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  dom::DOMString HTMLTextAreaElement::Placeholder() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"placeholder").value_or(u8"");
  }

  void HTMLTextAreaElement::Placeholder(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"placeholder", krys::move(value));
  }

  bool HTMLTextAreaElement::ReadOnly() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"readonly");
  }

  void HTMLTextAreaElement::ReadOnly(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"readonly", krys::move(value));
  }

  bool HTMLTextAreaElement::Required() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"required");
  }

  void HTMLTextAreaElement::Required(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"required", krys::move(value));
  }

  uint32 HTMLTextAreaElement::Rows() const noexcept
  {
    return Reflection::Reflect<uint32>(*this, u8"rows");
  }

  void HTMLTextAreaElement::Rows(uint32 value) noexcept
  {
    Reflection::Reflect<uint32>(*this, u8"rows", krys::move(value));
  }

  dom::DOMString HTMLTextAreaElement::Wrap() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"wrap").value_or(u8"");
  }

  void HTMLTextAreaElement::Wrap(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"wrap", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  bool HTMLTextAreaElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                               dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null())
    {
      if (localName == u8"align")
      {
        // Don't map 'align' attribute.  This matches what Firefox, Opera and IE do.
        return false;
      }
      if (localName == u8"wrap")
      {
        return true;
      }
    }
    return HTMLTextFormControlElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLTextAreaElement::CollectPresentationalHintsForAttribute(
    dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI, dom::DOMStringView value,
    css::MutableStyleProperties &style) noexcept
  {
    using namespace presentational_hints;
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
      return;
    }
    if (localName == u8"wrap")
    {
      if (!infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value, u8"off"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::WhiteSpaceCollapse,
                                             css::ValueId::Preserve);
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextWrapMode, css::ValueId::Wrap);
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::OverflowWrap, css::ValueId::BreakWord);
      }
      else
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::WhiteSpaceCollapse,
                                             css::ValueId::Preserve);
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextWrapMode, css::ValueId::Nowrap);
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::OverflowWrap, css::ValueId::Normal);
      }
      return;
    }
    HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

#pragma endregion
}
