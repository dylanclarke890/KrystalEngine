#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/SVG/Internals/SVGParserUtilities.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <cmath>
#include <utility>

namespace krys::boo::svg
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
    KRYS_NODISCARD static bool FromString(SVGElement &, dom::DOMStringView string) noexcept
    {
      return string == u8"true";
    }
    KRYS_NODISCARD static Maybe<bool> Parse(const dom::QualifiedName &, dom::DOMStringView) noexcept
    {
      krys_assert_msg(false, "Not reached");
      return InitialValue();
    }
    KRYS_NODISCARD static dom::DOMString ToString(bool type) noexcept
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
    KRYS_NODISCARD static Color FromString(SVGElement &, dom::DOMStringView) noexcept;
    KRYS_NODISCARD static dom::DOMString ToString(const Color &) noexcept;
  };

  template <>
  struct SVGPropertyTraits<uint32>
  {
    KRYS_NODISCARD static uint32 InitialValue() noexcept
    {
      return 0;
    }
    KRYS_NODISCARD static Maybe<uint32> Parse(const dom::QualifiedName &, dom::DOMStringView) noexcept
    {
      krys_assert_msg(false, "Not reached");
      return InitialValue();
    }
    KRYS_NODISCARD static dom::DOMString ToString(uint32 type) noexcept
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
    KRYS_NODISCARD static int32 FromString(SVGElement &, dom::DOMStringView) noexcept;
    KRYS_NODISCARD static Maybe<int32> Parse(const dom::QualifiedName &, dom::DOMStringView) noexcept
    {
      krys_assert_msg(false, "Not reached");
      return InitialValue();
    }
    KRYS_NODISCARD static dom::DOMString ToString(int32 type) noexcept
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
    KRYS_NODISCARD static std::pair<int32, int32> FromString(SVGElement &, dom::DOMStringView string) noexcept
    {
      auto result = ParseNumberOptionalNumber(string);
      if (!result)
      {
        return {};
      }
      return std::make_pair(static_cast<int32>(std::round(result->first)),
                            static_cast<int32>(std::round(result->second)));
    }
    KRYS_NODISCARD static Maybe<std::pair<int32, int32>> Parse(const dom::QualifiedName &,
                                                               dom::DOMStringView) noexcept
    {
      krys_assert_msg(false, "Not reached");
      return InitialValue();
    }
    KRYS_NODISCARD static dom::DOMString ToString(std::pair<int32, int32>) noexcept
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
    KRYS_NODISCARD static float FromString(SVGElement &, dom::DOMStringView string) noexcept
    {
      return ParseNumberFromString(string).value_or(0);
    }
    KRYS_NODISCARD static Maybe<float> Parse(const dom::QualifiedName &, dom::DOMStringView string) noexcept
    {
      return ParseNumberFromString(string);
    }
    KRYS_NODISCARD static dom::DOMString ToString(float type) noexcept
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
    KRYS_NODISCARD static std::pair<float, float> FromString(SVGElement &, dom::DOMStringView string) noexcept
    {
      return ParseNumberOptionalNumber(string).value_or(std::pair<float, float> {});
    }
    KRYS_NODISCARD static Maybe<std::pair<float, float>> Parse(const dom::QualifiedName &,
                                                               dom::DOMStringView) noexcept
    {
      krys_assert_msg(false, "Not reached");
      return InitialValue();
    }
    KRYS_NODISCARD static dom::DOMString ToString(std::pair<float, float>) noexcept
    {
      krys_assert_msg(false, "Not reached");
      return {};
    }
  };

  template <>
  struct SVGPropertyTraits<FloatPoint>
  {
    KRYS_NODISCARD static FloatPoint InitialValue() noexcept
    {
      return FloatPoint();
    }
    KRYS_NODISCARD static FloatPoint FromString(SVGElement &, dom::DOMStringView string) noexcept
    {
      return ParsePoint(string).value_or(FloatPoint());
    }
    KRYS_NODISCARD static Maybe<FloatPoint> Parse(const dom::QualifiedName &,
                                                  dom::DOMStringView string) noexcept
    {
      return ParsePoint(string);
    }
    KRYS_NODISCARD static dom::DOMString ToString(const FloatPoint &type) noexcept
    {
      StringBuilder builder;
      builder.Append(type.X(), u8' ', type.Y());
      return krys::move(builder).ToString();
    }
  };

  template <>
  struct SVGPropertyTraits<FloatRect>
  {
    KRYS_NODISCARD static FloatRect InitialValue() noexcept
    {
      return FloatRect();
    }
    KRYS_NODISCARD static FloatRect FromString(SVGElement &, dom::DOMStringView string) noexcept
    {
      return ParseRect(string).value_or(FloatRect());
    }
    KRYS_NODISCARD static Maybe<FloatRect> Parse(const dom::QualifiedName &,
                                                 dom::DOMStringView string) noexcept
    {
      return ParseRect(string);
    }
    KRYS_NODISCARD static dom::DOMString ToString(const FloatRect &type) noexcept
    {
      StringBuilder builder;
      builder.Append(type.X(), u8' ', type.Y(), u8' ', type.Width(), u8' ', type.Height());
      return krys::move(builder).ToString();
    }
  };

  template <>
  struct SVGPropertyTraits<dom::DOMString>
  {
    KRYS_NODISCARD static dom::DOMString InitialValue() noexcept
    {
      return dom::DOMString();
    }
    KRYS_NODISCARD static dom::DOMString FromString(SVGElement &, dom::DOMStringView string) noexcept
    {
      return dom::DOMString(string);
    }
    KRYS_NODISCARD static Maybe<dom::DOMString> Parse(const dom::QualifiedName &,
                                                      dom::DOMStringView string) noexcept
    {
      return dom::DOMString(string);
    }
    KRYS_NODISCARD static dom::DOMString ToString(dom::DOMStringView string) noexcept
    {
      return dom::DOMString(string);
    }
  };
}
