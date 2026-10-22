#include "Krystal.Booey/HTML/HTMLTextFormControlElement.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLParserIdioms.hpp"
#include "Krystal.Booey/Style/PseudoClassChangeInvalidation.hpp"
#include <algorithm>
#include <limits>

namespace krys::boo::html
{
  HTMLTextFormControlElement::HTMLTextFormControlElement(dom::Document &document,
                                                         HTMLElementInterface interface) noexcept
      : HTMLFormControlElement(document, interface)
  {
  }

  HTMLTextFormControlElement::~HTMLTextFormControlElement() noexcept = default;

  void HTMLTextFormControlElement::DispatchFormControlChangeEvent() noexcept
  {
    auto value = Value();
    if (_textAsOfLastFormControlChangeEvent != value)
    {
      DispatchChangeEvent();
      SetTextAsOfLastFormControlChangeEvent(krys::move(value));
    }
    SetChangedSinceLastFormControlChangeEvent(false);
  }

  bool HTMLTextFormControlElement::IsPlaceholderEmpty() const noexcept
  {
    auto attributeValue = GetAttribute(u8"placeholder").value_or(dom::DOMString());
    return std::ranges::all_of(attributeValue, IsHTMLLineBreak);
  }

  bool HTMLTextFormControlElement::PlaceholderShouldBeVisible() const noexcept
  {
    // This function is used by the style resolver to match the :placeholder-shown pseudo class. Since it is
    // used for styling, it must not use any value depending on the style.
    // FIXME: Hiding the placeholder while a pen hovers (canShowPlaceholder and the pointer type), with text
    // fields' placeholders.
    return SupportsPlaceholder() && IsEmptyValue() && !IsPlaceholderEmpty();
  }

  void HTMLTextFormControlElement::UpdatePlaceholderVisibility() noexcept
  {
    bool newIsPlaceholderVisible = PlaceholderShouldBeVisible();
    if (_isPlaceholderVisible == newIsPlaceholderVisible)
    {
      return;
    }

    style::PseudoClassChangeInvalidation styleInvalidation(*this, css::SelectorPseudoClass::PlaceholderShown,
                                                           newIsPlaceholderVisible);
    _isPlaceholderVisible = newIsPlaceholderVisible;
    // FIXME: Invalidating the placeholder element's style, with text fields' placeholders.
  }

  void HTMLTextFormControlElement::OnAttributeChanged(dom::DOMStringAtom localName,
                                                      Maybe<dom::DOMStringView> oldValue,
                                                      Maybe<dom::DOMStringView> value,
                                                      dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLFormControlElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI == dom::DOMStringAtom::Null() && localName == u8"placeholder")
    {
      // FIXME: Updating the placeholder text (updatePlaceholderText), with text fields' placeholders.
      UpdatePlaceholderVisibility();
    }
  }

  bool HTMLTextFormControlElement::LastChangeWasUserEdit() const noexcept
  {
    // FIXME: The inner text element's edits, once text fields have one.
    return _lastChangeWasUserEdit;
  }

  bool HTMLTextFormControlElement::WasEverChangedByUserEdit() const noexcept
  {
    return _wasEverChangedByUserEdit;
  }

  int32 HTMLTextFormControlElement::ParseLengthAttribute(Maybe<dom::DOMStringView> value) noexcept
  {
    if (!value)
    {
      return -1;
    }
    auto result = Numbers::ParseNonNegativeInteger(*value);
    if (!result.Success() || result.Value > static_cast<uint64>(std::numeric_limits<int32>::max()))
    {
      return -1;
    }
    return static_cast<int32>(result.Value);
  }

  size_t HTMLTextFormControlElement::LengthForAPIValue(dom::DOMStringView text) noexcept
  {
    size_t length = 0;
    for (char8 character : text)
    {
      auto byte = static_cast<uint8>(character);
      if ((byte & 0xC0) == 0x80)
      {
        continue;
      }
      // A code point beyond the Basic Multilingual Plane (four UTF-8 bytes) is a surrogate pair.
      length += byte >= 0xF0 ? 2 : 1;
    }
    return length;
  }

  dom::DOMString HTMLTextFormControlElement::DirectionForFormData() const noexcept
  {
    return css::EffectiveTextDirection(*this) == TextDirection::LTR ? u8"ltr" : u8"rtl";
  }
}
