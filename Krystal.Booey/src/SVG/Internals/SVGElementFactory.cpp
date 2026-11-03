#include "Krystal.Booey/SVG/Internals/SVGElementFactory.hpp"
#include "Krystal.Booey/SVG/Elements/SVGAElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGAnimateElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGAnimateMotionElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGAnimateTransformElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGCircleElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGClipPathElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGDefsElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGDescElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGEllipseElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEBlendElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEColorMatrixElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEComponentTransferElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFECompositeElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEConvolveMatrixElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEDiffuseLightingElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEDisplacementMapElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEDistantLightElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEDropShadowElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEFloodElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEFuncAElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEFuncBElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEFuncGElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEFuncRElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEGaussianBlurElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEImageElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEMergeElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEMergeNodeElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEMorphologyElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEOffsetElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFEPointLightElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFESpecularLightingElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFESpotLightElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFETileElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFETurbulenceElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGFilterElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGForeignObjectElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGGElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGImageElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGLinearGradientElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGLineElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGMarkerElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGMaskElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGMetadataElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGMPathElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGPathElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGPatternElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGPolygonElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGPolylineElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGRadialGradientElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGRectElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGScriptElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGSetElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGStopElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGStyleElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGSvgElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGSwitchElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGSymbolElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGTextElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGTextPathElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGTitleElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGTSpanElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGUnknownElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGUseElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGViewElement.hpp"

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
