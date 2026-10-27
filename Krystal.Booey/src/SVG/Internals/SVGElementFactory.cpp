#include "Krystal.Booey/SVG/Internals/SVGElementFactory.hpp"
#include "Krystal.Booey/SVG/SVGAElement.hpp"
#include "Krystal.Booey/SVG/SVGAnimateElement.hpp"
#include "Krystal.Booey/SVG/SVGAnimateMotionElement.hpp"
#include "Krystal.Booey/SVG/SVGAnimateTransformElement.hpp"
#include "Krystal.Booey/SVG/SVGCircleElement.hpp"
#include "Krystal.Booey/SVG/SVGClipPathElement.hpp"
#include "Krystal.Booey/SVG/SVGDefsElement.hpp"
#include "Krystal.Booey/SVG/SVGDescElement.hpp"
#include "Krystal.Booey/SVG/SVGEllipseElement.hpp"
#include "Krystal.Booey/SVG/SVGFEBlendElement.hpp"
#include "Krystal.Booey/SVG/SVGFEColorMatrixElement.hpp"
#include "Krystal.Booey/SVG/SVGFEComponentTransferElement.hpp"
#include "Krystal.Booey/SVG/SVGFECompositeElement.hpp"
#include "Krystal.Booey/SVG/SVGFEConvolveMatrixElement.hpp"
#include "Krystal.Booey/SVG/SVGFEDiffuseLightingElement.hpp"
#include "Krystal.Booey/SVG/SVGFEDisplacementMapElement.hpp"
#include "Krystal.Booey/SVG/SVGFEDistantLightElement.hpp"
#include "Krystal.Booey/SVG/SVGFEDropShadowElement.hpp"
#include "Krystal.Booey/SVG/SVGFEFloodElement.hpp"
#include "Krystal.Booey/SVG/SVGFEFuncAElement.hpp"
#include "Krystal.Booey/SVG/SVGFEFuncBElement.hpp"
#include "Krystal.Booey/SVG/SVGFEFuncGElement.hpp"
#include "Krystal.Booey/SVG/SVGFEFuncRElement.hpp"
#include "Krystal.Booey/SVG/SVGFEGaussianBlurElement.hpp"
#include "Krystal.Booey/SVG/SVGFEImageElement.hpp"
#include "Krystal.Booey/SVG/SVGFEMergeElement.hpp"
#include "Krystal.Booey/SVG/SVGFEMergeNodeElement.hpp"
#include "Krystal.Booey/SVG/SVGFEMorphologyElement.hpp"
#include "Krystal.Booey/SVG/SVGFEOffsetElement.hpp"
#include "Krystal.Booey/SVG/SVGFEPointLightElement.hpp"
#include "Krystal.Booey/SVG/SVGFESpecularLightingElement.hpp"
#include "Krystal.Booey/SVG/SVGFESpotLightElement.hpp"
#include "Krystal.Booey/SVG/SVGFETileElement.hpp"
#include "Krystal.Booey/SVG/SVGFETurbulenceElement.hpp"
#include "Krystal.Booey/SVG/SVGFilterElement.hpp"
#include "Krystal.Booey/SVG/SVGForeignObjectElement.hpp"
#include "Krystal.Booey/SVG/SVGGElement.hpp"
#include "Krystal.Booey/SVG/SVGImageElement.hpp"
#include "Krystal.Booey/SVG/SVGLinearGradientElement.hpp"
#include "Krystal.Booey/SVG/SVGLineElement.hpp"
#include "Krystal.Booey/SVG/SVGMarkerElement.hpp"
#include "Krystal.Booey/SVG/SVGMaskElement.hpp"
#include "Krystal.Booey/SVG/SVGMetadataElement.hpp"
#include "Krystal.Booey/SVG/SVGMPathElement.hpp"
#include "Krystal.Booey/SVG/SVGPathElement.hpp"
#include "Krystal.Booey/SVG/SVGPatternElement.hpp"
#include "Krystal.Booey/SVG/SVGPolygonElement.hpp"
#include "Krystal.Booey/SVG/SVGPolylineElement.hpp"
#include "Krystal.Booey/SVG/SVGRadialGradientElement.hpp"
#include "Krystal.Booey/SVG/SVGRectElement.hpp"
#include "Krystal.Booey/SVG/SVGScriptElement.hpp"
#include "Krystal.Booey/SVG/SVGSetElement.hpp"
#include "Krystal.Booey/SVG/SVGStopElement.hpp"
#include "Krystal.Booey/SVG/SVGStyleElement.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "Krystal.Booey/SVG/SVGSwitchElement.hpp"
#include "Krystal.Booey/SVG/SVGSymbolElement.hpp"
#include "Krystal.Booey/SVG/SVGTextElement.hpp"
#include "Krystal.Booey/SVG/SVGTextPathElement.hpp"
#include "Krystal.Booey/SVG/SVGTitleElement.hpp"
#include "Krystal.Booey/SVG/SVGTSpanElement.hpp"
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
      case TagName::linearGradient:
      {
        return CreateRefPtr<SVGLinearGradientElement>(document);
      }
      case TagName::radialGradient:
      {
        return CreateRefPtr<SVGRadialGradientElement>(document);
      }
      case TagName::stop:
      {
        return CreateRefPtr<SVGStopElement>(document);
      }
      case TagName::pattern:
      {
        return CreateRefPtr<SVGPatternElement>(document);
      }
      case TagName::clipPath:
      {
        return CreateRefPtr<SVGClipPathElement>(document);
      }
      case TagName::mask:
      {
        return CreateRefPtr<SVGMaskElement>(document);
      }
      case TagName::marker:
      {
        return CreateRefPtr<SVGMarkerElement>(document);
      }
      case TagName::text:
      {
        return CreateRefPtr<SVGTextElement>(document);
      }
      case TagName::tspan:
      {
        return CreateRefPtr<SVGTSpanElement>(document);
      }
      case TagName::textPath:
      {
        return CreateRefPtr<SVGTextPathElement>(document);
      }
      case TagName::filter:
      {
        return CreateRefPtr<SVGFilterElement>(document);
      }
      case TagName::feBlend:
      {
        return CreateRefPtr<SVGFEBlendElement>(document);
      }
      case TagName::feColorMatrix:
      {
        return CreateRefPtr<SVGFEColorMatrixElement>(document);
      }
      case TagName::feComponentTransfer:
      {
        return CreateRefPtr<SVGFEComponentTransferElement>(document);
      }
      case TagName::feComposite:
      {
        return CreateRefPtr<SVGFECompositeElement>(document);
      }
      case TagName::feConvolveMatrix:
      {
        return CreateRefPtr<SVGFEConvolveMatrixElement>(document);
      }
      case TagName::feDiffuseLighting:
      {
        return CreateRefPtr<SVGFEDiffuseLightingElement>(document);
      }
      case TagName::feDisplacementMap:
      {
        return CreateRefPtr<SVGFEDisplacementMapElement>(document);
      }
      case TagName::feDistantLight:
      {
        return CreateRefPtr<SVGFEDistantLightElement>(document);
      }
      case TagName::feDropShadow:
      {
        return CreateRefPtr<SVGFEDropShadowElement>(document);
      }
      case TagName::feFlood:
      {
        return CreateRefPtr<SVGFEFloodElement>(document);
      }
      case TagName::feFuncA:
      {
        return CreateRefPtr<SVGFEFuncAElement>(document);
      }
      case TagName::feFuncB:
      {
        return CreateRefPtr<SVGFEFuncBElement>(document);
      }
      case TagName::feFuncG:
      {
        return CreateRefPtr<SVGFEFuncGElement>(document);
      }
      case TagName::feFuncR:
      {
        return CreateRefPtr<SVGFEFuncRElement>(document);
      }
      case TagName::feGaussianBlur:
      {
        return CreateRefPtr<SVGFEGaussianBlurElement>(document);
      }
      case TagName::feImage:
      {
        return CreateRefPtr<SVGFEImageElement>(document);
      }
      case TagName::feMerge:
      {
        return CreateRefPtr<SVGFEMergeElement>(document);
      }
      case TagName::feMergeNode:
      {
        return CreateRefPtr<SVGFEMergeNodeElement>(document);
      }
      case TagName::feMorphology:
      {
        return CreateRefPtr<SVGFEMorphologyElement>(document);
      }
      case TagName::feOffset:
      {
        return CreateRefPtr<SVGFEOffsetElement>(document);
      }
      case TagName::fePointLight:
      {
        return CreateRefPtr<SVGFEPointLightElement>(document);
      }
      case TagName::feSpecularLighting:
      {
        return CreateRefPtr<SVGFESpecularLightingElement>(document);
      }
      case TagName::feSpotLight:
      {
        return CreateRefPtr<SVGFESpotLightElement>(document);
      }
      case TagName::feTile:
      {
        return CreateRefPtr<SVGFETileElement>(document);
      }
      case TagName::feTurbulence:
      {
        return CreateRefPtr<SVGFETurbulenceElement>(document);
      }
      case TagName::animate:
      {
        return CreateRefPtr<SVGAnimateElement>(document);
      }
      case TagName::animateMotion:
      {
        return CreateRefPtr<SVGAnimateMotionElement>(document);
      }
      case TagName::animateTransform:
      {
        return CreateRefPtr<SVGAnimateTransformElement>(document);
      }
      case TagName::set:
      {
        return CreateRefPtr<SVGSetElement>(document);
      }
      case TagName::mpath:
      {
        return CreateRefPtr<SVGMPathElement>(document);
      }
    }

    return nullptr;
  }
}
