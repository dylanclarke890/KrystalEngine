#include "Krystal.Booey/Rendering/SVG/RenderSVGTextPath.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGPathData.hpp"
#include "Krystal.Booey/SVG/SVGGeometryElement.hpp"
#include "Krystal.Booey/SVG/SVGTextPathElement.hpp"

namespace krys::boo
{
  RenderSVGTextPath::RenderSVGTextPath(svg::SVGTextPathElement &element, RenderStyle &&style) noexcept
      : RenderSVGInline(Type::SVGTextPath, element, krys::move(style))
  {
    krys_debug_assert(IsRenderSVGTextPath());
  }

  RenderSVGTextPath::~RenderSVGTextPath() noexcept = default;

  svg::SVGTextPathElement &RenderSVGTextPath::TextPathElement() const noexcept
  {
    return Downcast<svg::SVGTextPathElement>(RenderSVGInline::GraphicsElement());
  }

  svg::SVGGeometryElement *RenderSVGTextPath::TargetElement() const noexcept
  {
    auto target = svg::SVGURIReference::TargetElementFromIRIString(
      TextPathElement().Href(), TextPathElement().TreeScopeForSVGReferences());
    auto *svgElement = DynamicDowncast<svg::SVGElement>(target.Element.get());
    return svgElement ? DynamicDowncast<svg::SVGGeometryElement>(*svgElement) : nullptr;
  }

  Path RenderSVGTextPath::LayoutPath() const noexcept
  {
    RefPtr element = ShareRefPtr(TargetElement());
    if (!element)
    {
      return {};
    }

    auto path = PathFromGraphicsElement(*element);

    // Spec:  The transform attribute on the referenced 'path' element represents a
    // supplemental transformation relative to the current user coordinate system for
    // the current 'text' element, including any adjustments to the current user coordinate
    // system due to a possible transform attribute on the current 'text' element.
    // http://www.w3.org/TR/SVG/text.html#TextPathElement
    path.Transform(element->AnimatedLocalTransform());
    return path;
  }

  const svg::SVGLengthValue &RenderSVGTextPath::StartOffset() const noexcept
  {
    return TextPathElement().StartOffset();
  }
}
