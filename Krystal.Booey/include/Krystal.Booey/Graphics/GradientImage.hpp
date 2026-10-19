#pragma once

#include "Krystal.Booey/Graphics/GeneratedImage.hpp"
#include "Krystal.Booey/Graphics/Gradient.hpp"

namespace krys::boo
{
  /// @brief A gradient drawn as an image of its container's size.
  /// FIXME: Tiles are filled one by one with the gradient; upstream renders a tile into an image buffer and
  /// fills with it as a pattern, which comes with image buffers.
  class GradientImage final : public GeneratedImage
  {
  private:
    const Ref<boo::Gradient> _gradient;

  public:
    /// @brief Beyond this many tiles, a tiled draw stops: the rest are too small to be worth filling one by
    /// one.
    static constexpr uint32 MaxTiles = 10'000u;

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
