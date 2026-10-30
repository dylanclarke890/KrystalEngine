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
