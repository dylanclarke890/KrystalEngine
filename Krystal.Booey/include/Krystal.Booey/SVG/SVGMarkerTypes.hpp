#pragma once

#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyTraits.hpp"
#include "Krystal.Booey/SVG/SVGAngleValue.hpp"
#include "Krystal.Core/Enum.hpp"
#include <utility>

namespace krys::boo::svg
{
  enum class SVGMarkerUnitsType : uint8
  {
    Unknown = 0,
    UserSpaceOnUse,
    StrokeWidth,
  };

  enum SVGMarkerOrientType : uint8
  {
    SVGMarkerOrientUnknown = 0,
    SVGMarkerOrientAuto,
    SVGMarkerOrientAngle,
    SVGMarkerOrientAutoStartReverse
  };

  template <>
  struct SVGPropertyTraits<SVGMarkerUnitsType>
  {
    KRYS_NODISCARD static uint32 HighestEnumValue() noexcept
    {
      return krys::ToUnderlying(SVGMarkerUnitsType::StrokeWidth);
    }

    KRYS_NODISCARD static dom::DOMString ToString(SVGMarkerUnitsType type) noexcept
    {
      switch (type)
      {
        case SVGMarkerUnitsType::Unknown:
        {
          return {};
        }
        case SVGMarkerUnitsType::UserSpaceOnUse:
        {
          return u8"userSpaceOnUse";
        }
        case SVGMarkerUnitsType::StrokeWidth:
        {
          return u8"strokeWidth";
        }
      }

      krys_assert_msg(false, "Not reached");
      return {};
    }

    KRYS_NODISCARD static SVGMarkerUnitsType FromString(SVGElement &, dom::DOMStringView value) noexcept
    {
      if (value == u8"userSpaceOnUse")
      {
        return SVGMarkerUnitsType::UserSpaceOnUse;
      }
      if (value == u8"strokeWidth")
      {
        return SVGMarkerUnitsType::StrokeWidth;
      }
      return SVGMarkerUnitsType::Unknown;
    }
  };

  template <>
  struct SVGPropertyTraits<SVGMarkerOrientType>
  {
    KRYS_NODISCARD static dom::DOMStringView AutoStartReverseString() noexcept
    {
      return u8"auto-start-reverse";
    }

    KRYS_NODISCARD static uint32 HighestEnumValue() noexcept
    {
      return SVGMarkerOrientAutoStartReverse;
    }

    KRYS_NODISCARD static SVGMarkerOrientType FromString(SVGElement &, dom::DOMStringView string) noexcept
    {
      if (string == u8"auto")
      {
        return SVGMarkerOrientAuto;
      }
      if (string == AutoStartReverseString())
      {
        return SVGMarkerOrientAutoStartReverse;
      }
      return SVGMarkerOrientUnknown;
    }

    KRYS_NODISCARD static dom::DOMString ToString(SVGMarkerOrientType type) noexcept
    {
      if (type == SVGMarkerOrientAuto)
      {
        return u8"auto";
      }
      if (type == SVGMarkerOrientAutoStartReverse)
      {
        return dom::DOMString(AutoStartReverseString());
      }
      return {};
    }
  };

  template <>
  struct SVGPropertyTraits<std::pair<SVGAngleValue, SVGMarkerOrientType>>
  {
    KRYS_NODISCARD static std::pair<SVGAngleValue, SVGMarkerOrientType>
      FromString(SVGElement &targetElement, dom::DOMStringView string) noexcept
    {
      SVGAngleValue angle;
      SVGMarkerOrientType orientType =
        SVGPropertyTraits<SVGMarkerOrientType>::FromString(targetElement, string);
      if (orientType == SVGMarkerOrientUnknown)
      {
        auto result = angle.SetValueAsString(string);
        if (!result.HasException())
        {
          orientType = SVGMarkerOrientAngle;
        }
      }
      return std::make_pair(angle, orientType);
    }
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::svg::SVGMarkerUnitsType, 3uz);
