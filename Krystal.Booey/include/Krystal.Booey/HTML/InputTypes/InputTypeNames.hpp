#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"

/// @brief The input types, as the `type` attribute names them.
/// @see https://html.spec.whatwg.org/multipage/input.html#attr-input-type
namespace krys::boo::html::input_type_names
{
  constexpr DOMStringView Button = u8"button";
  constexpr DOMStringView Checkbox = u8"checkbox";
  constexpr DOMStringView Color = u8"color";
  constexpr DOMStringView Date = u8"date";
  constexpr DOMStringView DateTimeLocal = u8"datetime-local";
  constexpr DOMStringView Email = u8"email";
  constexpr DOMStringView File = u8"file";
  constexpr DOMStringView Hidden = u8"hidden";
  constexpr DOMStringView Image = u8"image";
  constexpr DOMStringView Month = u8"month";
  constexpr DOMStringView Number = u8"number";
  constexpr DOMStringView Password = u8"password";
  constexpr DOMStringView Radio = u8"radio";
  constexpr DOMStringView Range = u8"range";
  constexpr DOMStringView Reset = u8"reset";
  constexpr DOMStringView Search = u8"search";
  constexpr DOMStringView Submit = u8"submit";
  constexpr DOMStringView Telephone = u8"tel";
  constexpr DOMStringView Text = u8"text";
  constexpr DOMStringView Time = u8"time";
  constexpr DOMStringView Url = u8"url";
  constexpr DOMStringView Week = u8"week";
}
