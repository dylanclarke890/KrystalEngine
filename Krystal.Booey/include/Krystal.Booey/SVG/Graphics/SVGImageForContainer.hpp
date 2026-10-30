#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/Image.hpp"
#include "Krystal.Booey/SVG/Graphics/SVGImage.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::svg
{
  /// @brief An SVG image as one client draws it: at that client's container size and zoom, scrolled to the
  /// fragment of the client's URL.
  class SVGImageForContainer final : public Image
  {
  private:
    WeakPtr<SVGImage> _image;
    const FloatSize _containerSize;
    const float _containerZoom;
    const url::URL _initialFragmentURL;

  public:
    KRYS_NODISCARD static Ref<SVGImageForContainer> Create(SVGImage *image, const FloatSize &containerSize,
                                                           float containerZoom,
                                                           const url::URL &initialFragmentURL) noexcept
    {
      return AdoptRef(*new SVGImageForContainer(image, containerSize, containerZoom, initialFragmentURL));
    }

    KRYS_NODISCARD bool IsSVGImageForContainer() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD
    FloatSize Size(ImageOrientation = ImageOrientation::Orientation::FromImage) const noexcept final;

    KRYS_NODISCARD bool UsesContainerSize() const noexcept final
    {
      return _image->UsesContainerSize();
    }
    KRYS_NODISCARD bool HasRelativeWidth() const noexcept final
    {
      return _image->HasRelativeWidth();
    }
    KRYS_NODISCARD bool HasRelativeHeight() const noexcept final
    {
      return _image->HasRelativeHeight();
    }
    void ComputeIntrinsicDimensions(float &intrinsicWidth, float &intrinsicHeight,
                                    FloatSize &intrinsicRatio) noexcept final
    {
      RefPtr<SVGImage> image = ShareRefPtr(_image.get());
      image->ComputeIntrinsicDimensions(intrinsicWidth, intrinsicHeight, intrinsicRatio);
    }

    ImageDrawResult Draw(GraphicsContext &, const FloatRect &, const FloatRect &,
                         ImagePaintingOptions = {}) noexcept final;

    void DrawPattern(GraphicsContext &, const FloatRect &, const FloatRect &, const AffineTransform &,
                     const FloatPoint &, const FloatSize &, ImagePaintingOptions = {}) noexcept final;

    // FIXME: Implement this to be less conservative.
    KRYS_NODISCARD bool CurrentFrameKnownToBeOpaque() const noexcept final
    {
      return false;
    }
    KRYS_NODISCARD bool CurrentFrameIsComplete() const noexcept final
    {
      return static_cast<bool>(_image);
    }

    KRYS_NODISCARD RefPtr<NativeImage> CurrentNativeImage() noexcept final;

  private:
    SVGImageForContainer(SVGImage *, const FloatSize &containerSize, float containerZoom,
                         const url::URL &initialFragmentURL) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::svg::SVGImageForContainer)
  KRYS_NODISCARD static bool IsType(const krys::boo::Image &image) noexcept
  {
    return image.IsSVGImageForContainer();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
