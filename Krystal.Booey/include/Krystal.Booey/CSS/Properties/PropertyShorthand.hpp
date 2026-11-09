/*
 * (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2008, 2013 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo::css
{
  /// @brief Represents a shorthand property and the longhand properties it expands into.
  /// @example `margin` expands into `margin-top`, `margin-left`, `margin-bottom`, `margin-right`.
  class PropertyShorthand
  {
  private:
    PropertyId _shorthandId {PropertyId::Invalid};
    Span<const PropertyId> _properties;

  public:
    PropertyShorthand() noexcept = default;

    template <size_t NumProperties>
    PropertyShorthand(PropertyId id, Span<const PropertyId, NumProperties> properties) noexcept
        : _shorthandId(id), _properties(properties)
    {
      static_assert(NumProperties != std::dynamic_extent);
    }

    KRYS_NODISCARD const PropertyId *begin() const noexcept
    {
      return std::to_address(_properties.begin());
    }

    KRYS_NODISCARD const PropertyId *end() const noexcept
    {
      return std::to_address(_properties.end());
    }

    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _properties.size();
    }

    KRYS_NODISCARD PropertyId Id() const noexcept
    {
      return _shorthandId;
    }

    KRYS_NODISCARD Span<const PropertyId> Properties() const noexcept
    {
      return _properties;
    }
  };

  /// @brief Custom PropertyShorthand function.
  KRYS_NODISCARD PropertyShorthand TransitionShorthandForParsing() noexcept;

  /// @brief Returns an empty value if the property is not a shorthand.
  /// @note The implementation is generated in PropertyShorthandFunctions.cpp.
  KRYS_NODISCARD PropertyShorthand ShorthandForProperty(PropertyId id) noexcept;

  using PropertyShorthandList = SmallList<PropertyShorthand, 4uz>;

  /// @brief Returns the list of shorthands for a given longhand.
  /// @note The implementation is generated in PropertyShorthandFunctions.cpp.
  KRYS_NODISCARD PropertyShorthandList MatchingShorthandsForLonghand(PropertyId id) noexcept;

  /// @brief Finds the index of `shorthandId` in `shorthands`, which must contain it.
  KRYS_NODISCARD uint32 IndexOfShorthandForLonghand(PropertyId shorthandId,
                                                    const PropertyShorthandList &shorthands) noexcept;
}