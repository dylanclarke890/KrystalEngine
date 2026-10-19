#pragma once

#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"

namespace krys::boo
{
  /// @brief An image generated from its CSS description rather than loaded.
  /// FIXME: The clients come with the resource loader; generated images are made again for each paint rather
  /// than kept per size for a few seconds (`cachedImageForSize`).
  class StyleGeneratedImage : public StyleImage
  {
  protected:
    FloatSize _containerSize;

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

    KRYS_NODISCARD FloatSize ImageSize(const RenderElement *, float multiplier) const noexcept final;
    void ComputeIntrinsicDimensions(const RenderElement *, float &intrinsicWidth, float &intrinsicHeight,
                                    FloatSize &intrinsicRatio) noexcept final;
    void SetContainerContextForRenderer(const RenderElement &, const FloatSize &containerSize,
                                        float) noexcept final
    {
      _containerSize = containerSize;
    }

    // All generated images must be able to compute their fixed size.
    // FIXME: Pure upstream; the generated images other than gradients implement it with their drawing.
    KRYS_NODISCARD virtual FloatSize FixedSize(const RenderElement &) const noexcept
    {
      return {};
    }
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_IMAGE(StyleGeneratedImage, IsGeneratedImage)
