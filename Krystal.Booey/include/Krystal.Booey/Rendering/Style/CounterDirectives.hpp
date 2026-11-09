/*
 * Copyright (C) 2018 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Types/HashMap.hpp"

namespace krys::boo
{
  /// @brief The `counter-reset`, `counter-increment` and `counter-set` values for one counter.
  struct CounterDirectives
  {
    Maybe<int32> ResetValue;
    Maybe<int32> IncrementValue;
    Maybe<int32> SetValue;

    KRYS_NODISCARD constexpr bool operator==(const CounterDirectives &) const noexcept = default;
  };

  /// @brief The counter directives of an element, by counter name.
  struct CounterDirectiveMap
  {
    HashMap<css::CSSOMStringAtom, CounterDirectives> Map;

    KRYS_NODISCARD bool operator==(const CounterDirectiveMap &) const noexcept = default;
  };
}
