/*
 * Copyright (C) 2018-2019 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Web/Dom/Types/ExceptionOr.hpp"
#include "Krystal.Web/Svg/Properties/SVGPropertyTraits.hpp"
#include "Krystal.Web/Svg/SVGAngleValue.hpp"
#include <utility>

namespace krys::web
{
  enum class SVGMarkerUnitsType : uint8
  {
    Unknown = 0,
    UserSpaceOnUse,
    StrokeWidth,
  };

  consteval SVGMarkerUnitsType MaxEnumValue(SVGMarkerUnitsType) noexcept
  {
    return SVGMarkerUnitsType::StrokeWidth;
  }

  enum SVGMarkerOrientType : uint8
  {
    SVGMarkerOrientUnknown = 0,
    SVGMarkerOrientAuto,
    SVGMarkerOrientAngle,
    SVGMarkerOrientAutoStartReverse,
  };

  template <>
  struct SVGPropertyTraits<SVGMarkerUnitsType>
  {
    KRYS_NODISCARD static uint32 HighestEnumValue() noexcept
    {
      return krys::ToUnderlying(SVGMarkerUnitsType::StrokeWidth);
    }

    KRYS_NODISCARD static DOMString ToString(SVGMarkerUnitsType type) noexcept
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

    KRYS_NODISCARD static SVGMarkerUnitsType FromString(SVGElement &, DOMStringView value) noexcept
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
    KRYS_NODISCARD static DOMStringView AutoStartReverseString() noexcept
    {
      return u8"auto-start-reverse";
    }

    KRYS_NODISCARD static uint32 HighestEnumValue() noexcept
    {
      return SVGMarkerOrientAutoStartReverse;
    }

    KRYS_NODISCARD static SVGMarkerOrientType FromString(SVGElement &, DOMStringView string) noexcept
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

    KRYS_NODISCARD static DOMString ToString(SVGMarkerOrientType type) noexcept
    {
      if (type == SVGMarkerOrientAuto)
      {
        return u8"auto";
      }
      if (type == SVGMarkerOrientAutoStartReverse)
      {
        return DOMString(AutoStartReverseString());
      }
      return {};
    }
  };

  template <>
  struct SVGPropertyTraits<std::pair<SVGAngleValue, SVGMarkerOrientType>>
  {
    KRYS_NODISCARD static std::pair<SVGAngleValue, SVGMarkerOrientType>
      FromString(SVGElement &targetElement, DOMStringView string) noexcept
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
