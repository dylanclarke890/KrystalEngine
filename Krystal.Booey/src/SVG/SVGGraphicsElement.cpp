#include "Krystal.Booey/SVG/SVGGraphicsElement.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGPath.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGPathData.hpp"
#include "Krystal.Booey/Rendering/TransformOperationData.hpp"
#include "Krystal.Booey/Style/TransformResolver.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyOwnerRegistry.hpp"
#include "Krystal.Booey/SVG/SVGMatrix.hpp"
#include "Krystal.Booey/SVG/SVGNames.hpp"
#include "Krystal.Booey/SVG/SVGRect.hpp"

namespace krys::boo::svg
{
  SVGGraphicsElement::SVGGraphicsElement(dom::Document &document,
                                         UniquePtr<SVGPropertyRegistry> &&propertyRegistry,
                                         SVGElementInterface interface, SVGElementFlags flags,
                                         dom::NodeFlags nodeFlags) noexcept
      : SVGElement(document, krys::move(propertyRegistry), interface,
                   flags | SVGElementFlags::IsSvgGraphicsElement, nodeFlags),
        SVGTests(this), _transform(SVGAnimatedTransformList::Create(this))
  {
    static bool didRegistration = false;
    if (!didRegistration) KRYS_UNLIKELY
    {
      didRegistration = true;
      PropertyRegistry::RegisterProperty<&SVGGraphicsElement::_transform>(SVGNames::TransformAttr());
    }
  }

  SVGGraphicsElement::~SVGGraphicsElement() noexcept = default;

  Ref<SVGMatrix> SVGGraphicsElement::GetCTMForBindings() noexcept
  {
    return SVGMatrix::Create(GetCTM());
  }

  AffineTransform SVGGraphicsElement::GetCTM(StyleUpdateStrategy styleUpdateStrategy) noexcept
  {
    return SVGLocatable::ComputeCTM(this, CTMScope::NearestViewportScope, styleUpdateStrategy);
  }

  Ref<SVGMatrix> SVGGraphicsElement::GetScreenCTMForBindings() noexcept
  {
    return SVGMatrix::Create(GetScreenCTM());
  }

  AffineTransform SVGGraphicsElement::GetScreenCTM(StyleUpdateStrategy styleUpdateStrategy) noexcept
  {
    return SVGLocatable::ComputeCTM(this, CTMScope::ScreenScope, styleUpdateStrategy);
  }

  AffineTransform SVGGraphicsElement::AnimatedLocalTransform() const noexcept
  {
    AffineTransform matrix;

    auto *renderer = Renderer();
    const auto *style = renderer ? &renderer->Style() : nullptr;
    bool hasSpecifiedTransform = style && style->HasTransform();

    // Honor any of the transform-related CSS properties if set.
    if (hasSpecifiedTransform
        || (style && (style->HasTranslate() || style->HasScale() || style->HasRotate())))
    {
      // Note: objectBoundingBox is an emptyRect for elements like pattern or clipPath.
      // See the "Object bounding box units" section of http://dev.w3.org/csswg/css3-transforms/
      // FIXME: The motion path data of the renderer (TransformOperationData's renderer constructor).
      auto transform = style::TransformResolver::ComputeTransform(
        *style, TransformOperationData(renderer->TransformReferenceBoxRect()));

      // Flatten any 3D transform.
      matrix = transform.ToAffineTransform();
    }

    // If we didn't have the CSS "transform" property set, we must account for the "transform" attribute.
    if (!hasSpecifiedTransform && style && !Transform().IsEmpty())
    {
      auto t =
        style::TransformResolver::ComputeTransformOrigin(*style, renderer->TransformReferenceBoxRect()).XY();
      matrix.Translate(t);
      matrix *= Transform().Concatenate();
      matrix.Translate(-t.X(), -t.Y());
    }

    if (_supplementalTransform)
    {
      return *_supplementalTransform * matrix;
    }
    return matrix;
  }

  AffineTransform *SVGGraphicsElement::EnsureSupplementalTransform() noexcept
  {
    if (!_supplementalTransform)
    {
      _supplementalTransform = CreateUnique<AffineTransform>();
    }
    return _supplementalTransform.get();
  }

  void SVGGraphicsElement::AttributeChanged(const dom::QualifiedName &name,
                                            Maybe<dom::DOMStringView> oldValue,
                                            Maybe<dom::DOMStringView> value) noexcept
  {
    if (name == SVGNames::TransformAttr())
    {
      _transform->BaseVal()->Parse(value.value_or(dom::DOMStringView {}));
    }

    SVGTests::ParseAttribute(name, value);
    SVGElement::AttributeChanged(name, oldValue, value);
  }

  void SVGGraphicsElement::SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept
  {
    if (PropertyRegistry::IsKnownAttribute(attributeName))
    {
      krys_debug_assert(attributeName == SVGNames::TransformAttr());
      InstanceInvalidationGuard guard(*this);

      if (auto *renderer = Renderer())
      {
        renderer->SetNeedsTransformUpdate();
        UpdateSVGRendererForElementChange();
      }
      return;
    }

    SVGElement::SVGAttributeChanged(attributeName);
    SVGTests::SVGAttributeChanged(attributeName);
  }

  SVGElement *SVGGraphicsElement::NearestViewportElement() const noexcept
  {
    return SVGTransformable::NearestViewportElement(this);
  }

  SVGElement *SVGGraphicsElement::FarthestViewportElement() const noexcept
  {
    return SVGTransformable::FarthestViewportElement(this);
  }

  Ref<SVGRect> SVGGraphicsElement::GetBBoxForBindings() noexcept
  {
    return SVGRect::Create(GetBBox());
  }

  FloatRect SVGGraphicsElement::GetBBox(StyleUpdateStrategy styleUpdateStrategy) noexcept
  {
    return SVGTransformable::GetBBox(this, styleUpdateStrategy);
  }

  Path SVGGraphicsElement::ToClipPath() noexcept
  {
    Path path = PathFromGraphicsElement(*this);
    // FIXME: How do we know the element has done a layout?
    path.Transform(AnimatedLocalTransform());
    return path;
  }

  RenderPtr<RenderElement> SVGGraphicsElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<LegacyRenderSVGPath>(*this, krys::move(style));
  }
}
