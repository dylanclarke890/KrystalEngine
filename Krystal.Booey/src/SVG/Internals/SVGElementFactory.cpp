#include "Krystal.Booey/SVG/Internals/SVGElementFactory.hpp"
#include "Krystal.Booey/SVG/SVGAElement.hpp"
#include "Krystal.Booey/SVG/SVGCircleElement.hpp"
#include "Krystal.Booey/SVG/SVGDefsElement.hpp"
#include "Krystal.Booey/SVG/SVGDescElement.hpp"
#include "Krystal.Booey/SVG/SVGEllipseElement.hpp"
#include "Krystal.Booey/SVG/SVGForeignObjectElement.hpp"
#include "Krystal.Booey/SVG/SVGGElement.hpp"
#include "Krystal.Booey/SVG/SVGImageElement.hpp"
#include "Krystal.Booey/SVG/SVGLineElement.hpp"
#include "Krystal.Booey/SVG/SVGMetadataElement.hpp"
#include "Krystal.Booey/SVG/SVGPathElement.hpp"
#include "Krystal.Booey/SVG/SVGPolygonElement.hpp"
#include "Krystal.Booey/SVG/SVGPolylineElement.hpp"
#include "Krystal.Booey/SVG/SVGRectElement.hpp"
#include "Krystal.Booey/SVG/SVGScriptElement.hpp"
#include "Krystal.Booey/SVG/SVGStyleElement.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "Krystal.Booey/SVG/SVGSwitchElement.hpp"
#include "Krystal.Booey/SVG/SVGSymbolElement.hpp"
#include "Krystal.Booey/SVG/SVGTitleElement.hpp"
#include "Krystal.Booey/SVG/SVGUnknownElement.hpp"
#include "Krystal.Booey/SVG/SVGUseElement.hpp"
#include "Krystal.Booey/SVG/SVGViewElement.hpp"

namespace krys::boo::svg
{
  RefPtr<SVGElement> SVGElementFactory::TryCreate(dom::Document &document, html::TagName elementName) noexcept
  {
    using html::TagName;

    switch (elementName)
    {
      case TagName::circle:
      {
        return CreateRefPtr<SVGCircleElement>(document);
      }
      case TagName::defs:
      {
        return CreateRefPtr<SVGDefsElement>(document);
      }
      case TagName::desc:
      {
        return CreateRefPtr<SVGDescElement>(document);
      }
      case TagName::ellipse:
      {
        return CreateRefPtr<SVGEllipseElement>(document);
      }
      case TagName::foreignObject:
      {
        return CreateRefPtr<SVGForeignObjectElement>(document);
      }
      case TagName::g:
      {
        return CreateRefPtr<SVGGElement>(document);
      }
      case TagName::image:
      {
        return CreateRefPtr<SVGImageElement>(document);
      }
      case TagName::line:
      {
        return CreateRefPtr<SVGLineElement>(document);
      }
      case TagName::metadata:
      {
        return CreateRefPtr<SVGMetadataElement>(document);
      }
      case TagName::path:
      {
        return CreateRefPtr<SVGPathElement>(document);
      }
      case TagName::polygon:
      {
        return CreateRefPtr<SVGPolygonElement>(document);
      }
      case TagName::polyline:
      {
        return CreateRefPtr<SVGPolylineElement>(document);
      }
      case TagName::rect:
      {
        return CreateRefPtr<SVGRectElement>(document);
      }
      case TagName::script:
      {
        return CreateRefPtr<SVGScriptElement>(document);
      }
      case TagName::style:
      {
        return CreateRefPtr<SVGStyleElement>(document);
      }
      case TagName::svg:
      {
        return CreateRefPtr<SVGSvgElement>(document);
      }
      case TagName::switch_:
      {
        return CreateRefPtr<SVGSwitchElement>(document);
      }
      case TagName::symbol:
      {
        return CreateRefPtr<SVGSymbolElement>(document);
      }
      case TagName::title:
      {
        return CreateRefPtr<SVGTitleElement>(document);
      }
      case TagName::use:
      {
        return CreateRefPtr<SVGUseElement>(document);
      }
      case TagName::a:
      {
        return CreateRefPtr<SVGAElement>(document);
      }
      case TagName::view:
      {
        return CreateRefPtr<SVGViewElement>(document);
      }
      // FIXME: The gradients, their stops and <pattern>, with paint servers (B11e).
      case TagName::linearGradient:
      case TagName::radialGradient:
      case TagName::stop:
      case TagName::pattern:
      // FIXME: <clipPath>, <mask> and <marker> (B11f).
      case TagName::clipPath:
      case TagName::mask:
      case TagName::marker:
      // FIXME: <text>, <tspan> and <textPath>, with text (B11g).
      case TagName::text:
      case TagName::tspan:
      case TagName::textPath:
      // FIXME: <filter> and the filter primitives, with filters (B11h).
      case TagName::filter:
      case TagName::feBlend:
      case TagName::feColorMatrix:
      case TagName::feComponentTransfer:
      case TagName::feComposite:
      case TagName::feConvolveMatrix:
      case TagName::feDiffuseLighting:
      case TagName::feDisplacementMap:
      case TagName::feDistantLight:
      case TagName::feDropShadow:
      case TagName::feFlood:
      case TagName::feFuncA:
      case TagName::feFuncB:
      case TagName::feFuncG:
      case TagName::feFuncR:
      case TagName::feGaussianBlur:
      case TagName::feImage:
      case TagName::feMerge:
      case TagName::feMergeNode:
      case TagName::feMorphology:
      case TagName::feOffset:
      case TagName::fePointLight:
      case TagName::feSpecularLighting:
      case TagName::feSpotLight:
      case TagName::feTile:
      case TagName::feTurbulence:
      // FIXME: The animation elements, with SMIL (B11i).
      case TagName::animate:
      case TagName::animateMotion:
      case TagName::animateTransform:
      case TagName::set:
      case TagName::mpath:
      {
        return CreateRefPtr<SVGUnknownElement>(document);
      }
    }

    return nullptr;
  }
}
