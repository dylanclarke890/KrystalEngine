/*
 * (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004, 2005, 2006 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Properties/PropertyFlag.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Properties/Types/IsImplicit.hpp"
#include "Krystal.Booey/CSS/Properties/Types/IsImportant.hpp"
#include "Krystal.Booey/CSS/Properties/Types/IsSetFromShorthand.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/Graphics/Geometry/BoxSides.hpp"
#include "Krystal.Booey/Text/WritingMode.hpp"
#include "Krystal.Core/Containers/Span.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  /// @brief The average number of shorthands that can set a single property (max is currently 6). This is
  /// usually 'all' and one other. Logical properties are not counted as shorthands for physical properties
  /// and vice versa.
  /// @example `margin-left` has two shorthands: `margin` and `all`. `margin-inline` is not counted as a
  /// shorthand for `margin-left` because it is a logical property and `margin-left` is a physical property.
  /// `margin-inline-start` is also not counted as a shorthand for `margin-left` because it is a longhand
  /// property.
  constexpr uint8 MaxShorthandsPerProperty = 4u;

  struct ParserContext;

  struct PropertyMetadata
  {
    PropertyId Id : EnumBitWidth<PropertyId> {PropertyId::Invalid};
    EnumSet<PropertyFlag> Flags;

    /// @brief The index into the shorthands vector that this property was set from, if `SetFromShorthand`
    /// flag is set and there is ambiguity (i.e. multiple shorthands set the same property). If there is no
    /// ambiguity, this value will be 0.
    uint8 ShorthandIndex : std::bit_width(MaxShorthandsPerProperty) {0u};

    KRYS_NODISCARD PropertyId ShorthandId() const noexcept;

    KRYS_NODISCARD friend bool operator==(const PropertyMetadata &,
                                          const PropertyMetadata &) noexcept = default;
  };

  class Property
  {
  private:
    PropertyMetadata _metadata;
    Ref<css::Value> _value;

  public:
    Property(const PropertyMetadata &metadata, Ref<css::Value> &&value) noexcept
        : _metadata(metadata), _value(krys::move(value))
    {
      krys_debug_assert(_metadata.Id != PropertyId::Invalid);
      krys_debug_assert_msg(!IsShorthand(_metadata.Id), "unexpected shorthand property");
    }

    Property(PropertyId id, Ref<css::Value> &&value, IsImportant important = IsImportant::No,
             IsImplicit implicit = IsImplicit::No,
             IsSetFromShorthand setFromShorthand = IsSetFromShorthand::No, uint8 shorthandIndex = 0u) noexcept
        : _metadata({.Id = id, .ShorthandIndex = shorthandIndex}), _value(krys::move(value))
    {
      krys_debug_assert(id != PropertyId::Invalid);
      krys_debug_assert_msg(!IsShorthand(id), "unexpected shorthand property");

      if (important == IsImportant::Yes)
      {
        _metadata.Flags.Add(PropertyFlag::Important);
      }

      if (implicit == IsImplicit::Yes)
      {
        _metadata.Flags.Add(PropertyFlag::Implicit);
      }

      if (setFromShorthand == IsSetFromShorthand::Yes)
      {
        _metadata.Flags.Add(PropertyFlag::SetFromShorthand);
      }
    }

    KRYS_NODISCARD PropertyId Id() const noexcept
    {
      return _metadata.Id;
    }

    KRYS_NODISCARD PropertyId ShorthandId() const noexcept
    {
      return _metadata.ShorthandId();
    }

    KRYS_NODISCARD IsImportant Important() const noexcept
    {
      return IsImportant(_metadata.Flags.contains(PropertyFlag::Important));
    }

    KRYS_NODISCARD IsImplicit Implicit() const noexcept
    {
      return IsImplicit(_metadata.Flags.contains(PropertyFlag::Implicit));
    }

    KRYS_NODISCARD IsSetFromShorthand SetFromShorthand() const noexcept
    {
      return IsSetFromShorthand(_metadata.Flags.contains(PropertyFlag::SetFromShorthand));
    }

    KRYS_NODISCARD css::Value &Value() noexcept
    {
      return *_value;
    }

    KRYS_NODISCARD const css::Value &Value() const noexcept
    {
      return *_value;
    }

    KRYS_NODISCARD const PropertyMetadata &Metadata() const noexcept
    {
      return _metadata;
    }

    KRYS_NODISCARD bool operator==(const Property &other) const noexcept
    {
      if (!(_metadata == other._metadata))
      {
        return false;
      }
      return _value->Equals(*other._value);
    }
  };
}
