#include "Krystal.Booey/SVG/Graphics/SVGImageForContainer.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Graphics/NativeImage.hpp"

namespace krys::boo::svg
{
  SVGImageForContainer::SVGImageForContainer(SVGImage *image, const FloatSize &containerSize,
                                             float containerZoom, const url::URL &initialFragmentURL) noexcept
      : _image(CreateWeakPtr(image)), _containerSize(containerSize), _containerZoom(containerZoom),
        _initialFragmentURL(initialFragmentURL)
  {
  }

  FloatSize SVGImageForContainer::Size(ImageOrientation) const noexcept
  {
    FloatSize scaledContainerSize(_containerSize);
    scaledContainerSize.Scale(_containerZoom);
    return FloatSize(RoundedIntSize(scaledContainerSize));
  }

  ImageDrawResult SVGImageForContainer::Draw(GraphicsContext &context, const FloatRect &dstRect,
                                             const FloatRect &srcRect, ImagePaintingOptions options) noexcept
  {
    RefPtr<SVGImage> image = ShareRefPtr(_image.get());
    return image->DrawForContainer(context, _containerSize, _containerZoom, _initialFragmentURL, dstRect,
                                   srcRect, options);
  }

  void SVGImageForContainer::DrawPattern(GraphicsContext &context, const FloatRect &dstRect,
                                         const FloatRect &srcRect, const AffineTransform &patternTransform,
                                         const FloatPoint &phase, const FloatSize &spacing,
                                         ImagePaintingOptions options) noexcept
  {
    RefPtr<SVGImage> image = ShareRefPtr(_image.get());
    image->DrawPatternForContainer(context, _containerSize, _containerZoom, _initialFragmentURL, srcRect,
                                   patternTransform, phase, spacing, dstRect, options);
  }

  RefPtr<NativeImage> SVGImageForContainer::CurrentNativeImage() noexcept
  {
    RefPtr<SVGImage> image = ShareRefPtr(_image.get());
    return image->CurrentNativeImage();
  }
}
