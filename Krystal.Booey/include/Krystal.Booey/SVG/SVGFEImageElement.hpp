#pragma once

#include "Krystal.Booey/Loader/Cache/CachedImageClient.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceHandle.hpp"
#include "Krystal.Booey/SVG/SVGFilterPrimitiveStandardAttributes.hpp"
#include "Krystal.Booey/SVG/SVGPreserveAspectRatio.hpp"
#include "Krystal.Booey/SVG/SVGURIReference.hpp"

namespace krys::boo
{
  class CachedImage;
  class ImageBuffer;
}

namespace krys::boo::svg
{
  /// @brief The `<feImage>` element: an external image, or an element of the document, drawn into its
  /// subregion.
  /// @see https://drafts.fxtf.org/filter-effects/#InterfaceSVGFEImageElement
  class SVGFEImageElement final : public SVGFilterPrimitiveStandardAttributes,
                                  public SVGURIReference,
                                  private CachedImageClient
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(SVGFEImageElement);

  private:
    Ref<SVGAnimatedPreserveAspectRatio> _preserveAspectRatio;
    CachedResourceHandle<CachedImage> _cachedImage;

  public:
    explicit SVGFEImageElement(dom::Document &document) noexcept;
    ~SVGFEImageElement() noexcept override;

    KRYS_NODISCARD bool RenderingTaintsOrigin() const noexcept;

    KRYS_NODISCARD const SVGPreserveAspectRatioValue &PreserveAspectRatio() const noexcept
    {
      return _preserveAspectRatio->CurrentValue();
    }
    KRYS_NODISCARD SVGAnimatedPreserveAspectRatio &PreserveAspectRatioAnimated() noexcept
    {
      return *_preserveAspectRatio;
    }

    using PropertyRegistry =
      SVGPropertyOwnerRegistry<SVGFEImageElement, SVGFilterPrimitiveStandardAttributes, SVGURIReference>;

  private:
    void AttributeChanged(const dom::QualifiedName &name, Maybe<dom::DOMStringView> oldValue,
                          Maybe<dom::DOMStringView> value) noexcept final;
    void SVGAttributeChanged(const dom::QualifiedName &attributeName) noexcept final;

    void NotifyFinished(CachedResource &) noexcept final;

    void OnPostConnection() noexcept final;
    void OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept final;

    struct ImageBufferForEffect
    {
      RefPtr<ImageBuffer> Buffer;
      FloatRect ImageRect;
    };
    KRYS_NODISCARD ImageBufferForEffect
      GetImageBufferForEffect(const GraphicsContext &destinationContext) const noexcept;
    KRYS_NODISCARD RefPtr<FilterEffect>
      CreateFilterEffect(const FilterEffectVector &inputs,
                         const GraphicsContext &destinationContext) const noexcept final;

    void ClearResourceReferences() noexcept;
    void RequestImageResource() noexcept;
    void BuildPendingResource() noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGFEImageElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::svg::SVGElement &target) noexcept
  {
    return target.IsSVGFEImageElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
