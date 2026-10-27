#pragma once

#include "Krystal.Booey/SVG/SVGGraphicsElement.hpp"
#include "Krystal.Booey/SVG/SVGPreserveAspectRatio.hpp"
#include "Krystal.Booey/SVG/SVGURIReference.hpp"

namespace krys::boo
{
  class CachedImage;
}

namespace krys::boo::svg
{
  class SVGImageLoader;

  /// @brief The `<image>` element: a raster image drawn into a rectangle.
  /// @see https://svgwg.org/svg2-draft/embedded.html#InterfaceSVGImageElement
  class SVGImageElement final : public SVGGraphicsElement, public SVGURIReference
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGImageElement);

  private:
    Ref<SVGAnimatedLength> _x;
    Ref<SVGAnimatedLength> _y;
    Ref<SVGAnimatedLength> _width;
    Ref<SVGAnimatedLength> _height;
    Ref<SVGAnimatedPreserveAspectRatio> _preserveAspectRatio;
    const UniquePtr<SVGImageLoader> _imageLoader;

  public:
    explicit SVGImageElement(dom::Document &document) noexcept;
    ~SVGImageElement() noexcept override;

    KRYS_NODISCARD CachedImage *GetCachedImage() const noexcept;
    KRYS_NODISCARD Maybe<dom::DOMString> ImageSourceURL() const noexcept final;

    KRYS_NODISCARD const SVGLengthValue &X() const noexcept
    {
      return _x->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthValue &Y() const noexcept
    {
      return _y->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthValue &Width() const noexcept
    {
      return _width->CurrentValue();
    }
    KRYS_NODISCARD const SVGLengthValue &Height() const noexcept
    {
      return _height->CurrentValue();
    }
    KRYS_NODISCARD const SVGPreserveAspectRatioValue &PreserveAspectRatio() const noexcept
    {
      return _preserveAspectRatio->CurrentValue();
    }

    /// @see https://svgwg.org/svg2-draft/embedded.html#__svg__SVGImageElement__crossOrigin
    KRYS_NODISCARD Maybe<dom::DOMString> CrossOrigin() const noexcept;
    /// @see https://svgwg.org/svg2-draft/embedded.html#__svg__SVGImageElement__crossOrigin
    void CrossOrigin(Maybe<dom::DOMString> &&value) noexcept;

    KRYS_NODISCARD SVGAnimatedLength &XAnimated() noexcept
    {
      return *_x;
    }
    KRYS_NODISCARD SVGAnimatedLength &YAnimated() noexcept
    {
      return *_y;
    }
    KRYS_NODISCARD SVGAnimatedLength &WidthAnimated() noexcept
    {
      return *_width;
    }
    KRYS_NODISCARD SVGAnimatedLength &HeightAnimated() noexcept
    {
      return *_height;
    }
    KRYS_NODISCARD SVGAnimatedPreserveAspectRatio &PreserveAspectRatioAnimated() noexcept
    {
      return *_preserveAspectRatio;
    }

    using PropertyRegistry = SVGPropertyOwnerRegistry<SVGImageElement, SVGGraphicsElement, SVGURIReference>;

    // FIXME: decode(), once image loaders have decode() promises.

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept final;

    void DidAttachRenderers() noexcept final;
    void OnInsert() noexcept final;
    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&) noexcept final;

    KRYS_NODISCARD bool HaveLoadedRequiredResources() noexcept final;

    KRYS_NODISCARD bool IsValid() const noexcept final
    {
      return SVGTests::IsValid();
    }
    KRYS_NODISCARD bool SelfHasRelativeLengths() const noexcept final
    {
      return true;
    }
    void OnAdopt(dom::Document &oldDocument) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGImageElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGImageElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
