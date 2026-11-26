#include "Krystal.Booey/Svg/Internals/SVGElementFactory.hpp"
#include "Krystal.Booey/Svg/Elements/SVGAElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGAnimateElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGAnimateMotionElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGAnimateTransformElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGCircleElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGClipPathElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGDefsElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGDescElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGEllipseElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGFEBlendElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGFEColorMatrixElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGFEComponentTransferElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGFECompositeElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGFEConvolveMatrixElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGFEDiffuseLightingElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGFEDisplacementMapElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGFEDistantLightElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGFEDropShadowElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGFEFloodElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGFEFuncAElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGFEFuncBElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGFEFuncGElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGFEFuncRElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGFEGaussianBlurElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGFEImageElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGFEMergeElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGFEMergeNodeElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGFEMorphologyElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGFEOffsetElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGFEPointLightElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGFESpecularLightingElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGFESpotLightElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGFETileElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGFETurbulenceElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGFilterElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGForeignObjectElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGGElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGImageElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGLinearGradientElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGLineElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGMarkerElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGMaskElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGMetadataElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGMPathElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGPathElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGPatternElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGPolygonElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGPolylineElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGRadialGradientElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGRectElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGScriptElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGSetElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGStopElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGStyleElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGSvgElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGSwitchElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGSymbolElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGTextElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGTextPathElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGTitleElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGTSpanElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGUnknownElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGUseElement.hpp"
#include "Krystal.Booey/Svg/Elements/SVGViewElement.hpp"

namespace krys::boo
{
  RefPtr<SVGElement> SVGElementFactory::TryCreate(Document &document, html::TagName elementName) noexcept
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
