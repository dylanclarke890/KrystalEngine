/*
 * (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2007, 2008, 2013 Apple Inc. All rights reserved.
 * Copyright (C) 2013 Intel Corporation. All rights reserved.
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
 */

#include "Krystal.Booey/CSS/Properties/PropertyShorthand.hpp"
#include "Krystal.Core/Types/Array.hpp"

namespace krys::boo::css
{
  PropertyShorthand TransitionShorthandForParsing() noexcept
  {
    // Similar to animations, we have property after timing-function and delay after
    // duration.
    constexpr static Array<PropertyId, 5uz> transitionProperties = {{
      PropertyId::TransitionDuration,
      PropertyId::TransitionTimingFunction,
      PropertyId::TransitionDelay,
      PropertyId::TransitionBehavior,
      PropertyId::TransitionProperty,
    }};
    return PropertyShorthand(PropertyId::Transition, Span {transitionProperties});
  }

  uint32 IndexOfShorthandForLonghand(PropertyId shorthandId, const PropertyShorthandList &shorthands) noexcept
  {
    for (uint32 i = 0u, size = static_cast<uint32>(shorthands.size()); i < size; ++i)
    {
      if (shorthands[i].Id() == shorthandId)
      {
        return i;
      }
    }
    krys_debug_assert_msg(false, "The shorthand must be in the list.");
    return 0u;
  }
}
