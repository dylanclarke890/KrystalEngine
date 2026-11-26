/*
 * Copyright (C) 2004, 2005, 2006, 2007, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
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
 */

#pragma once

#include "Krystal.Booey/Dom/Types/DOMString.hpp"
#include "Krystal.Booey/Dom/Types/QualifiedName.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Svg/Internals/SVGParserUtilities.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Geometry/Point.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Core/Strings/StringBuilder.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <cmath>
#include <utility>

namespace krys::boo
{
  class SVGElement;

  template <typename TProperty>
  struct SVGPropertyTraits
  {
  };

  template <>
  struct SVGPropertyTraits<bool>
  {
    KRYS_NODISCARD static bool InitialValue() noexcept
    {
      return false;
    }

    KRYS_NODISCARD static bool FromString(SVGElement &, DOMStringView string) noexcept
    {
      return string == u8"true";
    }

    KRYS_NODISCARD static Maybe<bool> Parse(const QualifiedName &, DOMStringView) noexcept
    {
      krys_assert_msg(false, "Not reached");
      return InitialValue();
    }

    KRYS_NODISCARD static DOMString ToString(bool type) noexcept
    {
      return type ? u8"true" : u8"false";
    }
  };

  template <>
  struct SVGPropertyTraits<Color>
  {
    KRYS_NODISCARD static Color InitialValue() noexcept
    {
      return Color();
    }

    KRYS_NODISCARD static Color FromString(SVGElement &, DOMStringView) noexcept;

    KRYS_NODISCARD static DOMString ToString(const Color &) noexcept;
  };

  template <>
  struct SVGPropertyTraits<uint32>
  {
    KRYS_NODISCARD static uint32 InitialValue() noexcept
    {
      return 0;
    }

    KRYS_NODISCARD static Maybe<uint32> Parse(const QualifiedName &, DOMStringView) noexcept
    {
      krys_assert_msg(false, "Not reached");
      return InitialValue();
    }

    KRYS_NODISCARD static DOMString ToString(uint32 type) noexcept
    {
      StringBuilder builder;
      builder.Append(type);
      return krys::move(builder).ToString();
    }
  };

  template <>
  struct SVGPropertyTraits<int32>
  {
    KRYS_NODISCARD static int32 InitialValue() noexcept
    {
      return 0;
    }

    KRYS_NODISCARD static int32 FromString(SVGElement &, DOMStringView) noexcept;

    KRYS_NODISCARD static Maybe<int32> Parse(const QualifiedName &, DOMStringView) noexcept
    {
      krys_assert_msg(false, "Not reached");
      return InitialValue();
    }

    KRYS_NODISCARD static DOMString ToString(int32 type) noexcept
    {
      StringBuilder builder;
      builder.Append(type);
      return krys::move(builder).ToString();
    }
  };

  template <>
  struct SVGPropertyTraits<std::pair<int32, int32>>
  {
    KRYS_NODISCARD static std::pair<int32, int32> InitialValue() noexcept
    {
      return {};
    }

    KRYS_NODISCARD static std::pair<int32, int32> FromString(SVGElement &, DOMStringView string) noexcept
    {
      auto result = svg::ParseNumberOptionalNumber(string);
      if (!result)
      {
        return {};
      }
      return std::make_pair(static_cast<int32>(std::round(result->first)),
                            static_cast<int32>(std::round(result->second)));
    }

    KRYS_NODISCARD static Maybe<std::pair<int32, int32>> Parse(const QualifiedName &, DOMStringView) noexcept
    {
      krys_assert_msg(false, "Not reached");
      return InitialValue();
    }

    KRYS_NODISCARD static DOMString ToString(std::pair<int32, int32>) noexcept
    {
      krys_assert_msg(false, "Not reached");
      return {};
    }
  };

  template <>
  struct SVGPropertyTraits<float>
  {
    KRYS_NODISCARD static float InitialValue() noexcept
    {
      return 0;
    }

    KRYS_NODISCARD static float FromString(SVGElement &, DOMStringView string) noexcept
    {
      return svg::ParseNumberFromString(string).value_or(0);
    }

    KRYS_NODISCARD static Maybe<float> Parse(const QualifiedName &, DOMStringView string) noexcept
    {
      return svg::ParseNumberFromString(string);
    }

    KRYS_NODISCARD static DOMString ToString(float type) noexcept
    {
      StringBuilder builder;
      builder.Append(type);
      return krys::move(builder).ToString();
    }
  };

  template <>
  struct SVGPropertyTraits<std::pair<float, float>>
  {
    KRYS_NODISCARD static std::pair<float, float> InitialValue() noexcept
    {
      return {};
    }

    KRYS_NODISCARD static std::pair<float, float> FromString(SVGElement &, DOMStringView string) noexcept
    {
      return svg::ParseNumberOptionalNumber(string).value_or(std::pair<float, float> {});
    }

    KRYS_NODISCARD static Maybe<std::pair<float, float>> Parse(const QualifiedName &, DOMStringView) noexcept
    {
      krys_assert_msg(false, "Not reached");
      return InitialValue();
    }

    KRYS_NODISCARD static DOMString ToString(std::pair<float, float>) noexcept
    {
      krys_assert_msg(false, "Not reached");
      return {};
    }
  };

  template <>
  struct SVGPropertyTraits<PointF>
  {
    KRYS_NODISCARD static PointF InitialValue() noexcept
    {
      return PointF();
    }

    KRYS_NODISCARD static PointF FromString(SVGElement &, DOMStringView string) noexcept
    {
      return svg::ParsePoint(string).value_or(PointF());
    }

    KRYS_NODISCARD static Maybe<PointF> Parse(const QualifiedName &, DOMStringView string) noexcept
    {
      return svg::ParsePoint(string);
    }

    KRYS_NODISCARD static DOMString ToString(const PointF &type) noexcept
    {
      StringBuilder builder;
      builder.Append(type.X(), u8' ', type.Y());
      return krys::move(builder).ToString();
    }
  };

  template <>
  struct SVGPropertyTraits<RectF>
  {
    KRYS_NODISCARD static RectF InitialValue() noexcept
    {
      return RectF();
    }

    KRYS_NODISCARD static RectF FromString(SVGElement &, DOMStringView string) noexcept
    {
      return svg::ParseRect(string).value_or(RectF());
    }

    KRYS_NODISCARD static Maybe<RectF> Parse(const QualifiedName &, DOMStringView string) noexcept
    {
      return svg::ParseRect(string);
    }

    KRYS_NODISCARD static DOMString ToString(const RectF &type) noexcept
    {
      StringBuilder builder;
      builder.Append(type.X(), u8' ', type.Y(), u8' ', type.Width(), u8' ', type.Height());
      return krys::move(builder).ToString();
    }
  };

  template <>
  struct SVGPropertyTraits<DOMString>
  {
    KRYS_NODISCARD static DOMString InitialValue() noexcept
    {
      return DOMString();
    }

    KRYS_NODISCARD static DOMString FromString(SVGElement &, DOMStringView string) noexcept
    {
      return DOMString(string);
    }

    KRYS_NODISCARD static Maybe<DOMString> Parse(const QualifiedName &, DOMStringView string) noexcept
    {
      return DOMString(string);
    }

    KRYS_NODISCARD static DOMString ToString(DOMStringView string) noexcept
    {
      return DOMString(string);
    }
  };
}
