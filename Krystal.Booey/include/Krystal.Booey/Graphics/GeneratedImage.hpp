#pragma once

#include "Krystal.Booey/Graphics/Image.hpp"

namespace krys::boo
{
  /// @brief An image generated at its container's size, such as a gradient.
  class GeneratedImage : public Image
  {
  private:
    FloatSize _size;

  public:
    void SetContainerSize(const FloatSize &size) noexcept override
    {
      _size = size;
    }
    KRYS_NODISCARD bool UsesContainerSize() const noexcept override
    {
      return true;
    }
    KRYS_NODISCARD bool HasRelativeWidth() const noexcept override
    {
      return true;
    }
    KRYS_NODISCARD bool HasRelativeHeight() const noexcept override
    {
      return true;
    }

    void ComputeIntrinsicDimensions(float &intrinsicWidth, float &intrinsicHeight,
                                    FloatSize &intrinsicRatio) noexcept override;

    KRYS_NODISCARD FloatSize
      Size(ImageOrientation = ImageOrientation::Orientation::FromImage) const noexcept override
    {
      return _size;
    }

  protected:
    GeneratedImage() noexcept = default;

    ImageDrawResult Draw(GraphicsContext &, const FloatRect &dstRect, const FloatRect &srcRect,
                         ImagePaintingOptions = {}) noexcept override = 0;
    void DrawPattern(GraphicsContext &, const FloatRect &destRect, const FloatRect &srcRect,
                     const AffineTransform &patternTransform, const FloatPoint &phase,
                     const FloatSize &spacing, ImagePaintingOptions = {}) noexcept override = 0;

    // FIXME: Implement this to be less conservative.
    KRYS_NODISCARD bool CurrentFrameKnownToBeOpaque() const noexcept override
    {
      return false;
    }
    KRYS_NODISCARD bool CurrentFrameIsComplete() const noexcept override
    {
      return true;
    }

  private:
    KRYS_NODISCARD bool IsGeneratedImage() const noexcept override
    {
      return true;
    }
  };
}
