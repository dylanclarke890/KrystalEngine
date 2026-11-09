/*
 * Copyright (C) 2006 Nikolas Zimmermann <zimmermann@kde.org>
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

#include "Krystal.Booey/SVG/Properties/SVGPropertyTraits.hpp"
#include "Krystal.Core/RefCounted.hpp"

namespace krys::boo::svg
{
  class SVGUnitTypes final : public RefCounted<SVGUnitTypes>
  {
  public:
    enum SVGUnitType : uint8
    {
      SVG_UNIT_TYPE_UNKNOWN = 0,
      SVG_UNIT_TYPE_USERSPACEONUSE = 1,
      SVG_UNIT_TYPE_OBJECTBOUNDINGBOX = 2
    };

  private:
    SVGUnitTypes() noexcept = default;
  };

  template <>
  struct SVGPropertyTraits<SVGUnitTypes::SVGUnitType>
  {
    KRYS_NODISCARD static uint32 HighestEnumValue() noexcept
    {
      return SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX;
    }

    KRYS_NODISCARD static dom::DOMString ToString(SVGUnitTypes::SVGUnitType type) noexcept
    {
      switch (type)
      {
        case SVGUnitTypes::SVG_UNIT_TYPE_UNKNOWN:
        {
          return {};
        }
        case SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE:
        {
          return u8"userSpaceOnUse";
        }
        case SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX:
        {
          return u8"objectBoundingBox";
        }
      }

      krys_assert_msg(false, "Not reached");
      return {};
    }

    KRYS_NODISCARD static SVGUnitTypes::SVGUnitType FromString(SVGElement &,
                                                               dom::DOMStringView value) noexcept
    {
      if (value == u8"userSpaceOnUse")
      {
        return SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE;
      }
      if (value == u8"objectBoundingBox")
      {
        return SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX;
      }
      return SVGUnitTypes::SVG_UNIT_TYPE_UNKNOWN;
    }
  };
}
