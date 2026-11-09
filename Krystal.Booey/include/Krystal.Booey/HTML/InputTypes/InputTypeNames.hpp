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
