#pragma once

#include "Krystal.Booey/Graphics/GeneratedImage.hpp"
#include "Krystal.Booey/Style/Values/Primitives/Url.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class LegacyRenderSVGResourceContainer;
}

namespace krys::boo::svg
{
  /// @brief An image drawn by an SVG resource a CSS image refers to (a `mask-image: url()` to a `<mask>`).
  class SVGResourceImage final : public GeneratedImage
  {
  private:
    WeakPtr<LegacyRenderSVGResourceContainer> _legacyRenderResource;
    style::URL _resourceURL;

  public:
    KRYS_NODISCARD static Ref<SVGResourceImage> Create(LegacyRenderSVGResourceContainer &,
                                                       const style::URL &) noexcept;
    ~SVGResourceImage() noexcept override;

  private:
    SVGResourceImage(LegacyRenderSVGResourceContainer &, const style::URL &) noexcept;

    ImageDrawResult Draw(GraphicsContext &, const FloatRect &destinationRect, const FloatRect &sourceRect,
                         ImagePaintingOptions = {}) noexcept final;
    void DrawPattern(GraphicsContext &, const FloatRect &destRect, const FloatRect &srcRect,
                     const AffineTransform &patternTransform, const FloatPoint &phase,
                     const FloatSize &spacing, ImagePaintingOptions = {}) noexcept final;

    KRYS_NODISCARD bool IsSVGResourceImage() const noexcept final
    {
      return true;
    }

    void Dump(TextStream &) const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGResourceImage)
  KRYS_NODISCARD static bool IsType(const krys::boo::Image &image) noexcept
  {
    return image.IsSVGResourceImage();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
