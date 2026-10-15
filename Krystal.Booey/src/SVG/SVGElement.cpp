#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/SVG/SVGForeignObjectElement.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "Krystal.Booey/SVG/SVGUseElement.hpp"

namespace krys::boo::svg
{
  SVGElement::SVGElement(dom::Document &document, SVGElementInterface interface, SVGElementFlags flags,
                         dom::NodeFlags nodeFlags) noexcept
      : StyledElement(document, dom::NullQualifiedName(), dom::NodeFlags::IsSVGElement | nodeFlags),
        _flags(flags), _interface(interface)
  {
  }

  SVGElement::SVGElement(dom::Document &document) noexcept : SVGElement(document, SVGElementInterface::None)
  {
  }

  bool SVGElement::IsInnerSVGSVGElement() const noexcept
  {
    if (!IsSVGSvgElement())
    {
      return false;
    }

    // Element may not be in the document, pretend we're outermost for viewport(), getCTM(), etc.
    if (!ParentNode())
    {
      return false;
    }

    return Is<SVGElement>(*ParentNode());
  }

  bool SVGElement::IsOutermostSVGSVGElement() const noexcept
  {
    if (!IsSVGSvgElement())
    {
      return false;
    }

    // Element may not be in the document, pretend we're outermost for viewport(), getCTM(), etc.
    auto *parent = ParentNode();
    if (!parent)
    {
      return true;
    }

    // We act like an outermost SVG element, if we're a direct child of a <foreignObject> element.
    if (auto *svgParent = DynamicDowncast<SVGElement>(*parent);
        svgParent && Is<SVGForeignObjectElement>(*svgParent))
    {
      return true;
    }

    // If we're inside the shadow tree of a <use> element, we're always an inner <svg> element.
    if (IsInShadowTree())
    {
      auto *host = dom::TreeQueries::ShadowHost(*this);
      if (auto *svgHost = DynamicDowncast<SVGElement>(host); svgHost && Is<SVGUseElement>(*svgHost))
      {
        return false;
      }
    }

    // This is true whenever this is the outermost SVG, even if there are HTML elements outside it
    return !Is<SVGElement>(*parent);
  }
}
