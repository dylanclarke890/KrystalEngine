#pragma once

#include "Krystal.Booey/Rendering/RenderImageResource.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief The image of generated content (`content: url(...)`): a style image, whatever kind it is.
  class RenderImageResourceStyleImage final : public RenderImageResource
  {
  private:
    Ref<StyleImage> _styleImage;

  public:
    explicit RenderImageResourceStyleImage(StyleImage &styleImage) noexcept;

  private:
    void Initialize(RenderElement &renderer) noexcept override;
    void Shutdown() noexcept override;

    KRYS_NODISCARD RefPtr<Image> GetImage(const IntSize &size = {}) const noexcept override;
    KRYS_NODISCARD bool ErrorOccurred() const noexcept override
    {
      return _styleImage->ErrorOccurred();
    }

    void SetContainerContext(const IntSize &size, const url::URL &) noexcept override;

    KRYS_NODISCARD bool ImageHasRelativeWidth() const noexcept override
    {
      return _styleImage->ImageHasRelativeWidth();
    }
    KRYS_NODISCARD bool ImageHasRelativeHeight() const noexcept override
    {
      return _styleImage->ImageHasRelativeHeight();
    }

    KRYS_NODISCARD WrappedImagePtr ImagePtr() const noexcept override
    {
      return _styleImage->Data();
    }
    KRYS_NODISCARD LayoutSize ImageSize(float multiplier, CachedImage::SizeType) const noexcept override
    {
      return LayoutSize(_styleImage->ImageSize(Renderer(), multiplier));
    }
  };
}
