#pragma once

#include "Krystal.Booey/Graphics/Images/Image.hpp"

namespace krys::boo
{
  /// @brief The icon drawn for an image that failed to load: a 16 by 16 picture torn in two, drawn as shapes
  /// so it's sharp at any scale.
  class BrokenImageIcon final : public Image
  {
  public:
    static constexpr float IconSize = 16.f;

    /// @brief The one icon, shared by every broken image.
    KRYS_NODISCARD static BrokenImageIcon &Singleton() noexcept;

    KRYS_NODISCARD FloatSize
      Size(ImageOrientation = ImageOrientation::Orientation::FromImage) const noexcept override
    {
      return FloatSize(IconSize, IconSize);
    }

  private:
    BrokenImageIcon() noexcept = default;

    ImageDrawResult Draw(GraphicsContext &, const FloatRect &dstRect, const FloatRect &srcRect,
                         ImagePaintingOptions = {}) noexcept override;

    KRYS_NODISCARD bool CurrentFrameKnownToBeOpaque() const noexcept override
    {
      return false;
    }
    KRYS_NODISCARD bool CurrentFrameIsComplete() const noexcept override
    {
      return true;
    }
  };
}
