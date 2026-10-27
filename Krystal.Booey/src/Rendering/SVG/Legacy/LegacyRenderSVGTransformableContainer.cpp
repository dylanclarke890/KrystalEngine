#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGTransformableContainer.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderSupport.hpp"
#include "Krystal.Booey/SVG/SVGGElement.hpp"
#include "Krystal.Booey/SVG/SVGGraphicsElement.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"
#include "Krystal.Booey/SVG/SVGUseElement.hpp"

namespace krys::boo
{
  LegacyRenderSVGTransformableContainer::LegacyRenderSVGTransformableContainer(
    svg::SVGGraphicsElement &element, RenderStyle &&style) noexcept
      : LegacyRenderSVGContainer(Type::LegacySVGTransformableContainer, element, krys::move(style))
  {
    krys_debug_assert(IsLegacyRenderSVGTransformableContainer());
  }

  LegacyRenderSVGTransformableContainer::~LegacyRenderSVGTransformableContainer() noexcept = default;

  bool LegacyRenderSVGTransformableContainer::CalculateLocalTransform() noexcept
  {
    auto &element = GraphicsElement();

    // If we're either the renderer for a <use> element, or for any <g> element inside the shadow
    // tree, that was created during the use/symbol/svg expansion in SVGUseElement. These containers
    // need to respect the translations induced by their corresponding use elements x/y attributes.
    RefPtr<svg::SVGUseElement> useElement = ShareRefPtr(DynamicDowncast<svg::SVGUseElement>(element));
    if (!useElement && element.IsInShadowTree() && Is<svg::SVGGElement>(element))
    {
      if (auto *correspondingElement = element.CorrespondingElement())
      {
        useElement = ShareRefPtr(DynamicDowncast<svg::SVGUseElement>(*correspondingElement));
      }
    }

    if (useElement)
    {
      svg::SVGLengthContext lengthContext(&element);
      FloatSize translation(useElement->X().Value(lengthContext), useElement->Y().Value(lengthContext));
      if (translation != _additionalTranslation)
      {
        _needsTransformUpdate = true;
      }
      _additionalTranslation = translation;
    }

    auto referenceBoxRect = TransformReferenceBoxRect();
    if (referenceBoxRect != _lastTransformReferenceBoxRect)
    {
      _lastTransformReferenceBoxRect = referenceBoxRect;
      _needsTransformUpdate = true;
    }

    _didTransformToRootUpdate = _needsTransformUpdate || SVGRenderSupport::TransformToRootChanged(Parent());
    if (!_needsTransformUpdate)
    {
      return false;
    }

    _localTransform = element.AnimatedLocalTransform();
    _localTransform.Translate(_additionalTranslation);
    _needsTransformUpdate = false;
    return true;
  }

  svg::SVGGraphicsElement &LegacyRenderSVGTransformableContainer::GraphicsElement() noexcept
  {
    return Downcast<svg::SVGGraphicsElement>(LegacyRenderSVGContainer::GetElement());
  }
}
