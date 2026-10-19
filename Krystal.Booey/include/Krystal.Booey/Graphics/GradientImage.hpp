#pragma once

#include "Krystal.Booey/Graphics/GeneratedImage.hpp"
#include "Krystal.Booey/Graphics/Gradient.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"

namespace krys::boo
{
  /// @brief A gradient drawn as an image of its container's size.
  class GradientImage final : public GeneratedImage
  {
  private:
    const Ref<boo::Gradient> _gradient;
    RefPtr<ImageBuffer> _cachedImage;
    FloatSize _cachedAdjustedSize;
    uint32 _cachedGeneratorHash {0};
    FloatSize _cachedScaleFactor;

  public:
    KRYS_NODISCARD static Ref<GradientImage> Create(boo::Gradient &generator, const FloatSize &size) noexcept
    {
      return AdoptRef(*new GradientImage(generator, size));
    }

    ~GradientImage() noexcept override;

    KRYS_NODISCARD const boo::Gradient &GetGradient() const noexcept
    {
      return *_gradient;
    }

  private:
    GradientImage(boo::Gradient &, const FloatSize &) noexcept;

    ImageDrawResult Draw(GraphicsContext &, const FloatRect &dstRect, const FloatRect &srcRect,
                         ImagePaintingOptions = {}) noexcept override;
    void DrawPattern(GraphicsContext &, const FloatRect &destRect, const FloatRect &srcRect,
                     const AffineTransform &patternTransform, const FloatPoint &phase,
                     const FloatSize &spacing, ImagePaintingOptions = {}) noexcept override;

    KRYS_NODISCARD bool IsGradientImage() const noexcept override
    {
      return true;
    }
  };
}
