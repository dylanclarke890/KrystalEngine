#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyTraits.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  class AffineTransform;
  class FloatRect;
}

namespace krys::boo::dom
{
  template <typename T>
  class ExceptionOr;
}

namespace krys::boo::svg
{
  class SVGPreserveAspectRatioValue
  {
  public:
    enum SVGPreserveAspectRatioType : uint8
    {
      SVG_PRESERVEASPECTRATIO_UNKNOWN = 0,
      SVG_PRESERVEASPECTRATIO_NONE = 1,
      SVG_PRESERVEASPECTRATIO_XMINYMIN = 2,
      SVG_PRESERVEASPECTRATIO_XMIDYMIN = 3,
      SVG_PRESERVEASPECTRATIO_XMAXYMIN = 4,
      SVG_PRESERVEASPECTRATIO_XMINYMID = 5,
      SVG_PRESERVEASPECTRATIO_XMIDYMID = 6,
      SVG_PRESERVEASPECTRATIO_XMAXYMID = 7,
      SVG_PRESERVEASPECTRATIO_XMINYMAX = 8,
      SVG_PRESERVEASPECTRATIO_XMIDYMAX = 9,
      SVG_PRESERVEASPECTRATIO_XMAXYMAX = 10
    };

    enum SVGMeetOrSliceType : uint8
    {
      SVG_MEETORSLICE_UNKNOWN = 0,
      SVG_MEETORSLICE_MEET = 1,
      SVG_MEETORSLICE_SLICE = 2
    };

  private:
    SVGPreserveAspectRatioType _align {SVGPreserveAspectRatioValue::SVG_PRESERVEASPECTRATIO_XMIDYMID};
    SVGMeetOrSliceType _meetOrSlice {SVGPreserveAspectRatioValue::SVG_MEETORSLICE_MEET};

  public:
    SVGPreserveAspectRatioValue() noexcept = default;
    explicit SVGPreserveAspectRatioValue(dom::DOMStringView) noexcept;
    SVGPreserveAspectRatioValue(SVGPreserveAspectRatioType, SVGMeetOrSliceType) noexcept;

    KRYS_NODISCARD bool operator==(const SVGPreserveAspectRatioValue &) const noexcept = default;

    dom::ExceptionOr<void> SetAlign(uint16) noexcept;
    KRYS_NODISCARD uint16 Align() const noexcept
    {
      return _align;
    }

    dom::ExceptionOr<void> SetMeetOrSlice(uint16) noexcept;
    KRYS_NODISCARD uint16 MeetOrSlice() const noexcept
    {
      return _meetOrSlice;
    }

    void TransformRect(FloatRect &destRect, FloatRect &srcRect) const noexcept;

    KRYS_NODISCARD AffineTransform GetCTM(float logicalX, float logicalY, float logicalWidth,
                                          float logicalHeight, float physicalWidth,
                                          float physicalHeight) const noexcept;

    bool Parse(dom::DOMStringView) noexcept;
    bool Parse(dom::DOMStringView &buffer, bool validate) noexcept;

    KRYS_NODISCARD dom::DOMString ValueAsString() const noexcept;

  private:
    bool ParseInternal(dom::DOMStringView &buffer, bool validate) noexcept;
  };

  template <>
  struct SVGPropertyTraits<SVGPreserveAspectRatioValue>
  {
    KRYS_NODISCARD static SVGPreserveAspectRatioValue InitialValue() noexcept
    {
      return SVGPreserveAspectRatioValue();
    }
    KRYS_NODISCARD static SVGPreserveAspectRatioValue FromString(dom::DOMStringView string) noexcept
    {
      return SVGPreserveAspectRatioValue(string);
    }
    KRYS_NODISCARD static Maybe<SVGPreserveAspectRatioValue> Parse(const dom::QualifiedName &,
                                                                   dom::DOMStringView) noexcept
    {
      krys_assert_msg(false, "Not reached");
      return InitialValue();
    }
    KRYS_NODISCARD static dom::DOMString ToString(const SVGPreserveAspectRatioValue &type) noexcept
    {
      return type.ValueAsString();
    }
  };
}
