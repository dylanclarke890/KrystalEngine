/*
 * Copyright (C) 2010, 2012 Google Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"

/// @brief The input types, as the `type` attribute names them.
/// @see https://html.spec.whatwg.org/#attr-input-type
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
