#include "Krystal.Booey/SVG/SVGPolyElement.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGPath.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"

namespace krys::boo::svg
{
  SVGPolyElement::SVGPolyElement(dom::Document &document, SVGElementInterface interface) noexcept
      : SVGGeometryElement(document, CreateUnique<PropertyRegistry>(*this), interface),
        _points(SVGAnimatedPointList::Create(this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGPolyElement::_points>(SVGNames::PointsAttr());
    }
  }

  void SVGPolyElement::AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                                        Maybe<dom::DOMStringView> value) noexcept
  {
    if (name == SVGNames::PointsAttr())
    {
      if (!_points->BaseVal()->Parse(value.value_or(dom::DOMStringView {})))
      {
        // FIXME: Report "Problem parsing points="value"" to the console, once there's one.
      }
    }

    SVGGeometryElement::AttributeChanged(name, oldValue, value);
  }

  void SVGPolyElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      krys_debug_assert(attributeName == SVGNames::PointsAttr());
      InstanceInvalidationGuard guard(*this);

      if (auto *path = DynamicDowncast<LegacyRenderSVGPath>(Renderer()))
      {
        path->SetNeedsShapeUpdate();
      }

      UpdateSVGRendererForElementChange();
      return;
    }

    SVGGeometryElement::SVGAttributeChanged(attributeName);
  }
}
