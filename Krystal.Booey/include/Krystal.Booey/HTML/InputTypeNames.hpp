#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"

/// @brief The input types, as the `type` attribute names them.
/// @see https://html.spec.whatwg.org/#attr-input-type
namespace krys::boo::html::InputTypeNames
{
  constexpr dom::DOMStringView Button = u8"button";
  constexpr dom::DOMStringView Checkbox = u8"checkbox";
  constexpr dom::DOMStringView Color = u8"color";
  constexpr dom::DOMStringView Date = u8"date";
  constexpr dom::DOMStringView DateTimeLocal = u8"datetime-local";
  constexpr dom::DOMStringView Email = u8"email";
  constexpr dom::DOMStringView File = u8"file";
  constexpr dom::DOMStringView Hidden = u8"hidden";
  constexpr dom::DOMStringView Image = u8"image";
  constexpr dom::DOMStringView Month = u8"month";
  constexpr dom::DOMStringView Number = u8"number";
  constexpr dom::DOMStringView Password = u8"password";
  constexpr dom::DOMStringView Radio = u8"radio";
  constexpr dom::DOMStringView Range = u8"range";
  constexpr dom::DOMStringView Reset = u8"reset";
  constexpr dom::DOMStringView Search = u8"search";
  constexpr dom::DOMStringView Submit = u8"submit";
  constexpr dom::DOMStringView Telephone = u8"tel";
  constexpr dom::DOMStringView Text = u8"text";
  constexpr dom::DOMStringView Time = u8"time";
  constexpr dom::DOMStringView URL = u8"url";
  constexpr dom::DOMStringView Week = u8"week";
}
