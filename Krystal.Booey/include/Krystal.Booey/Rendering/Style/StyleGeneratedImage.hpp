#pragma once

#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"

namespace krys::boo
{
  /// @brief An image generated from its CSS description rather than loaded.
  /// FIXME: The container size, the clients and the per-size cache of generated images (`cachedImageForSize`)
  /// come with the renderer and `GeneratedImage`.
  class StyleGeneratedImage : public StyleImage
  {
  private:
    bool _fixedSize;

  protected:
    StyleGeneratedImage(StyleImage::Type, bool fixedSize) noexcept;
    ~StyleGeneratedImage() noexcept override;

    KRYS_NODISCARD WrappedImagePtr Data() const noexcept final
    {
      return this;
    }

    KRYS_NODISCARD bool ImageHasRelativeWidth() const noexcept final
    {
      return !_fixedSize;
    }
    KRYS_NODISCARD bool ImageHasRelativeHeight() const noexcept final
    {
      return !_fixedSize;
    }
    KRYS_NODISCARD bool UsesImageContainerSize() const noexcept final
    {
      return !_fixedSize;
    }
    KRYS_NODISCARD bool ImageHasNaturalDimensions() const noexcept final
    {
      return !UsesImageContainerSize();
    }
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_IMAGE(StyleGeneratedImage, IsGeneratedImage)
