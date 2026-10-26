#include "Krystal.Booey/SVG/SVGLocatable.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGForeignObjectElement.hpp"
#include "Krystal.Booey/SVG/SVGImageElement.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "Krystal.Booey/SVG/SVGSymbolElement.hpp"

namespace krys::boo::svg
{
  namespace
  {
    // FIXME: This doesn't match SVGElement::viewportElement() as it has an extra check for
    // foreign object.
    KRYS_NODISCARD bool IsViewportElement(const SVGElement *element) noexcept
    {
      if (!element)
      {
        return false;
      }

      return Is<SVGSvgElement>(*element) || Is<SVGSymbolElement>(*element)
             || Is<SVGForeignObjectElement>(*element) || Is<SVGImageElement>(*element);
    }

    constexpr EnumSet<dom::LayoutOptions> LayoutOptionsForQueries {
      dom::LayoutOptions::TreatContentVisibilityHiddenAsVisible,
      dom::LayoutOptions::TreatContentVisibilityAutoAsVisible};
  }

  SVGElement *SVGLocatable::NearestViewportElement(const SVGElement *element) noexcept
  {
    krys_debug_assert(element);
    for (RefPtr<dom::Element> current = ShareRefPtr(dom::TreeQueries::ParentOrShadowHostElement(*element));
         current; current = ShareRefPtr(dom::TreeQueries::ParentOrShadowHostElement(*current)))
    {
      auto *svgElement = DynamicDowncast<SVGElement>(*current);
      if (IsViewportElement(svgElement))
      {
        return svgElement;
      }
    }

    return nullptr;
  }

  SVGElement *SVGLocatable::FarthestViewportElement(const SVGElement *element) noexcept
  {
    krys_debug_assert(element);
    SVGElement *farthest = nullptr;
    for (RefPtr<dom::Element> current = ShareRefPtr(dom::TreeQueries::ParentOrShadowHostElement(*element));
         current; current = ShareRefPtr(dom::TreeQueries::ParentOrShadowHostElement(*current)))
    {
      auto *svgElement = DynamicDowncast<SVGElement>(*current);
      if (IsViewportElement(svgElement))
      {
        farthest = svgElement;
      }
    }
    return farthest;
  }

  FloatRect SVGLocatable::GetBBox(SVGElement *element, StyleUpdateStrategy styleUpdateStrategy) noexcept
  {
    krys_debug_assert(element);
    if (styleUpdateStrategy == AllowStyleUpdate)
    {
      (void)element->NodeDocument().UpdateLayoutIgnorePendingStylesheets(LayoutOptionsForQueries, element);
    }

    // FIXME: Eventually we should support getBBox for detached elements.
    if (!element->Renderer())
    {
      return FloatRect();
    }

    // FIXME: The renderer's object bounding box, once there are SVG renderers (B11b).
    return FloatRect();
  }

  AffineTransform SVGLocatable::ComputeCTM(SVGElement *element, CTMScope mode,
                                           StyleUpdateStrategy styleUpdateStrategy) noexcept
  {
    krys_debug_assert(element);
    if (styleUpdateStrategy == AllowStyleUpdate)
    {
      (void)element->NodeDocument().UpdateLayoutIgnorePendingStylesheets(LayoutOptionsForQueries, element);
    }

    RefPtr<SVGElement> stopAtElement =
      ShareRefPtr(mode == CTMScope::NearestViewportScope ? NearestViewportElement(element) : nullptr);

    AffineTransform ctm;

    for (dom::Element *currentElement = element; currentElement;
         currentElement = dom::TreeQueries::ParentOrShadowHostElement(*currentElement))
    {
      RefPtr<SVGElement> svgElement = ShareRefPtr(DynamicDowncast<SVGElement>(*currentElement));
      if (!svgElement)
      {
        break;
      }

      ctm = svgElement->LocalCoordinateSpaceTransform(mode).Multiply(ctm);

      // For getCTM() computation, stop at the nearest viewport element
      if (currentElement == stopAtElement.get())
      {
        break;
      }
    }

    return ctm;
  }
}
