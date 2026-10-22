#include "Krystal.Booey/HTML/TextFieldInputType.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/TextEvent.hpp"
#include "Krystal.Booey/HTML/DOMFormData.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLParserIdioms.hpp"
#include <algorithm>

namespace krys::boo::html
{
  namespace
  {
    /// @brief @p string truncated to @p maxLength UTF-16 code units, without splitting a code point.
    KRYS_NODISCARD dom::DOMString LimitLength(dom::DOMStringView string, size_t maxLength) noexcept
    {
      size_t length = 0;
      for (size_t i = 0; i < string.size(); ++i)
      {
        auto byte = static_cast<uint8>(string[i]);
        if ((byte & 0xC0) == 0x80)
        {
          continue;
        }
        size_t units = byte >= 0xF0 ? 2 : 1;
        if (length + units > maxLength)
        {
          return dom::DOMString(string.substr(0, i));
        }
        length += units;
      }
      return dom::DOMString(string);
    }
  }

  bool TextFieldInputType::ValueMissing(dom::DOMStringView value) const noexcept
  {
    krys_debug_assert(Element());
    auto &element = *Element();
    return element.IsMutable() && element.IsRequired() && value.empty();
  }

  void TextFieldInputType::SetValue(dom::DOMStringView sanitizedValue, bool valueChanged,
                                    TextFieldEventBehavior eventBehavior,
                                    TextControlSetValueSelection selection) noexcept
  {
    krys_debug_assert(Element());

    // Keep this input element even if an event handler changes the input type.
    Ref<HTMLInputElement> input = ShareRef(*Element());

    // InputType::SetValue doesn't dispatch the events, as text fields dispatch them differently.
    InputType::SetValue(sanitizedValue, valueChanged, TextFieldEventBehavior::DispatchNoEvent, selection);

    // FIXME: Updating the inner text and the selection, with text fields' shadow trees.
    if (!input->FormControlValueMatchesRenderer())
    {
      input->UpdatePlaceholderVisibility();
    }

    if (!valueChanged)
    {
      return;
    }

    switch (eventBehavior)
    {
      case TextFieldEventBehavior::DispatchChangeEvent:
        // If the user is still editing this field, dispatch an input event rather than a change event. The
        // change event will be dispatched when editing finishes.
        if (input->Focused())
        {
          input->DispatchFormControlInputEvent();
        }
        else
        {
          input->DispatchFormControlChangeEvent();
        }
        break;
      case TextFieldEventBehavior::DispatchInputAndChangeEvent:
        input->DispatchFormControlInputEvent();
        input->DispatchFormControlChangeEvent();
        break;
      case TextFieldEventBehavior::DispatchNoEvent: break;
    }

    if (!input->Focused())
    {
      input->SetTextAsOfLastFormControlChangeEvent(dom::DOMString(sanitizedValue));
    }
  }

  bool TextFieldInputType::ShouldSubmitImplicitly(dom::Event &event) noexcept
  {
    auto *textEvent = DynamicDowncast<dom::TextEvent>(event);
    return (event.Type() == EventNames::TextInput && textEvent && textEvent->Data() == u8"\n")
           || InputType::ShouldSubmitImplicitly(event);
  }

  bool TextFieldInputType::SupportsReadOnly() const noexcept
  {
    return true;
  }

  bool TextFieldInputType::IsEmptyValue() const noexcept
  {
    // FIXME: The inner text's value, with text fields' shadow trees.
    return VisibleValue().empty();
  }

  dom::DOMString TextFieldInputType::SanitizeValue(dom::DOMStringView proposedValue) const noexcept
  {
    if (std::ranges::none_of(proposedValue, IsHTMLLineBreak)) KRYS_LIKELY
    {
      return LimitLength(proposedValue, HTMLInputElement::MaxEffectiveLength);
    }

    dom::DOMString proposedValueWithoutLineBreaks;
    for (char8 character : proposedValue)
    {
      if (!IsHTMLLineBreak(character))
      {
        proposedValueWithoutLineBreaks += character;
      }
    }
    return LimitLength(proposedValueWithoutLineBreaks, HTMLInputElement::MaxEffectiveLength);
  }

  bool TextFieldInputType::AppendFormData(DOMFormData &formData) const noexcept
  {
    InputType::AppendFormData(formData);
    krys_debug_assert(Element());
    auto &element = *Element();
    // FIXME: should type=number be TextFieldInputType to begin with?
    if (element.IsNumberField())
    {
      return true;
    }
    if (auto dirname = element.GetAttribute(u8"dirname"))
    {
      formData.Append(*dirname, element.DirectionForFormData());
    }
    return true;
  }
}
