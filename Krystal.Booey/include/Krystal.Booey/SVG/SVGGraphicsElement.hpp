#pragma once

#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyImpl.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGTests.hpp"
#include "Krystal.Booey/SVG/SVGTransformable.hpp"
#include "Krystal.Booey/SVG/SVGTransformList.hpp"

namespace krys::boo::svg
{
  class SVGMatrix;
  class SVGRect;

  /// @brief An element that renders graphics: it has a transform, and takes part in conditional processing.
  /// @see https://svgwg.org/svg2-draft/types.html#InterfaceSVGGraphicsElement
  class SVGGraphicsElement : public SVGElement, public SVGTransformable, public SVGTests
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGGraphicsElement);

  private:
    // Used by <animateMotion>
    UniquePtr<AffineTransform> _supplementalTransform;

    // Used to isolate blend operations caused by masking.
    bool _shouldIsolateBlending {false};

    Ref<SVGAnimatedTransformList> _transform;

  public:
    ~SVGGraphicsElement() noexcept override;

    /// @see https://svgwg.org/svg2-draft/types.html#__svg__SVGGraphicsElement__getCTM
    KRYS_NODISCARD Ref<SVGMatrix> GetCTMForBindings() noexcept;
    KRYS_NODISCARD AffineTransform GetCTM(StyleUpdateStrategy = AllowStyleUpdate) noexcept override;

    /// @see https://svgwg.org/svg2-draft/types.html#__svg__SVGGraphicsElement__getScreenCTM
    KRYS_NODISCARD Ref<SVGMatrix> GetScreenCTMForBindings() noexcept;
    KRYS_NODISCARD AffineTransform GetScreenCTM(StyleUpdateStrategy = AllowStyleUpdate) noexcept override;

    KRYS_NODISCARD SVGElement *NearestViewportElement() const noexcept override;
    KRYS_NODISCARD SVGElement *FarthestViewportElement() const noexcept override;

    KRYS_NODISCARD AffineTransform LocalCoordinateSpaceTransform(CTMScope mode) const noexcept override
    {
      return SVGTransformable::LocalCoordinateSpaceTransform(mode);
    }

    KRYS_NODISCARD AffineTransform AnimatedLocalTransform() const noexcept override;
    KRYS_NODISCARD AffineTransform *EnsureSupplementalTransform() noexcept override;

    KRYS_NODISCARD AffineTransform *SupplementalTransform() const noexcept override
    {
      return _supplementalTransform.get();
    }

    KRYS_NODISCARD virtual bool HasTransformRelatedAttributes() const noexcept
    {
      return !Transform().Concatenate().IsIdentity() || _supplementalTransform;
    }

    /// @see https://svgwg.org/svg2-draft/types.html#__svg__SVGGraphicsElement__getBBox
    KRYS_NODISCARD Ref<SVGRect> GetBBoxForBindings() noexcept;
    KRYS_NODISCARD FloatRect GetBBox(StyleUpdateStrategy = AllowStyleUpdate) noexcept override;

    KRYS_NODISCARD bool ShouldIsolateBlending() const noexcept
    {
      return _shouldIsolateBlending;
    }

    void SetShouldIsolateBlending(bool isolate) noexcept
    {
      _shouldIsolateBlending = isolate;
    }

    // "base class" methods for all the elements which render as paths
    KRYS_NODISCARD virtual Path ToClipPath() noexcept;
    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept override;

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGGraphicsElement, SVGElement, SVGTests>;

    KRYS_NODISCARD const SVGTransformList &Transform() const noexcept
    {
      return _transform->CurrentValue();
    }

    KRYS_NODISCARD SVGAnimatedTransformList &TransformAnimated() noexcept
    {
      return *_transform;
    }

  protected:
    SVGGraphicsElement(dom::Document &document, UniquePtr<SVGPropertyRegistry> &&propertyRegistry,
                       SVGElementInterface interface, SVGElementFlags flags = SVGElementFlags::None,
                       dom::NodeFlags nodeFlags = dom::NodeFlags::None) noexcept;

    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept override;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGGraphicsElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGGraphicsElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
