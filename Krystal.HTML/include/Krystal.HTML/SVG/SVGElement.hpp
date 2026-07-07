#pragma once

#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/SVG/Enums/SVGElementFlags.hpp"
#include "Krystal.HTML/SVG/Enums/SVGElementInterface.hpp"

namespace Krys::HTML
{
  class SVGElement : public Element
  {
    KRYS_TYPE_CAST_TRAITS_ACCESS();
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGElement);

  protected:
    SVGElementFlags _flags : BitCount<SVGElementFlags>() {SVGElementFlags::None};
    SVGElementInterface _interface : BitCount<SVGElementInterface>() {SVGElementInterface::None};

    SVGElement(Document &document, SVGElementInterface interface,
               SVGElementFlags flags = SVGElementFlags::None, NodeFlags nodeFlags = NodeFlags::None) noexcept;

  public:
    SVGElement(Document &document) noexcept;

  protected:
#pragma region Type Checks

    KRYS_NODISCARD bool IsSVGCircleElement() const noexcept
    {
     return _interface == SVGElementInterface::Circle;
    }

    KRYS_NODISCARD bool IsSVGDefsElement() const noexcept
    {
      return _interface == SVGElementInterface::Defs;
    }

    KRYS_NODISCARD bool IsSVGDescElement() const noexcept
    {
      return _interface == SVGElementInterface::Desc;
    }

    KRYS_NODISCARD bool IsSVGEllipseElement() const noexcept
    {
      return _interface == SVGElementInterface::Ellipse;
    }

    KRYS_NODISCARD bool IsSVGForeignObjectElement() const noexcept
    {
      return _interface == SVGElementInterface::ForeignObject;
    }

    KRYS_NODISCARD bool IsSVGGElement() const noexcept
    {
      return _interface == SVGElementInterface::G;
    }

    KRYS_NODISCARD bool IsSVGGeometryElement() const noexcept
    {
      return HasFlag(_flags, SVGElementFlags::IsSvgGeometryElement);
    }

    KRYS_NODISCARD bool IsSVGGraphicsElement() const noexcept
    {
      return HasFlag(_flags, SVGElementFlags::IsSvgGraphicsElement);
    }

    KRYS_NODISCARD bool IsSVGImageElement() const noexcept
    {
      return _interface == SVGElementInterface::Image;
    }

    KRYS_NODISCARD bool IsSVGLineElement() const noexcept
    {
      return _interface == SVGElementInterface::Line;
    }

    KRYS_NODISCARD bool IsSVGMetadataElement() const noexcept
    {
      return _interface == SVGElementInterface::Metadata;
    }

    KRYS_NODISCARD bool IsSVGPolygonElement() const noexcept
    {
      return _interface == SVGElementInterface::Polygon;
    }

    KRYS_NODISCARD bool IsSVGPolylineElement() const noexcept
    {
      return _interface == SVGElementInterface::Polyline;
    }

    KRYS_NODISCARD bool IsSVGRectElement() const noexcept
    {
      return _interface == SVGElementInterface::Rect;
    }

    KRYS_NODISCARD bool IsSVGScriptElement() const noexcept
    {
      return _interface == SVGElementInterface::Script;
    }

    KRYS_NODISCARD bool IsSVGStyleElement() const noexcept
    {
      return _interface == SVGElementInterface::Style;
    }

    KRYS_NODISCARD bool IsSVGSvgElement() const noexcept
    {
      return _interface == SVGElementInterface::Svg;
    }

    KRYS_NODISCARD bool IsSVGSwitchElement() const noexcept
    {
      return _interface == SVGElementInterface::Switch;
    }

    KRYS_NODISCARD bool IsSVGSymbolElement() const noexcept
    {
      return _interface == SVGElementInterface::Symbol;
    }

    KRYS_NODISCARD bool IsSVGTitleElement() const noexcept
    {
      return _interface == SVGElementInterface::Title;
    }

    // NOTE: SVGUnknownElement type check is not needed here as Node already has it.

    KRYS_NODISCARD bool IsSVGUseElement() const noexcept
    {
      return _interface == SVGElementInterface::Use;
    }

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::SVGElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::Node &target) noexcept
  {
    return target.IsSVGElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();