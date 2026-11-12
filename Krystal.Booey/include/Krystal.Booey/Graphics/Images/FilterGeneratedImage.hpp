#pragma once

#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/Images/GeneratedImage.hpp"

namespace krys::boo
{
  /// @brief A `filter()` image drawn: its input image drawn through the filter as it's painted, rather than
  /// filtered into a bitmap up front, so it works wherever the destination can apply the filter.
  class FilterGeneratedImage final : public GeneratedImage
  {
  private:
    const Ref<Image> _image;
    const Ref<Filter> _filter;

  public:
    /// @brief @p image filtered by @p filter, whose filter region is the image's rect at @p size.
    KRYS_NODISCARD static Ref<FilterGeneratedImage> Create(Image &image, Filter &filter,
                                                           const FloatSize &size) noexcept
    {
      return AdoptRef(*new FilterGeneratedImage(image, filter, size));
    }

  private:
    FilterGeneratedImage(Image &, Filter &, const FloatSize &) noexcept;

    ImageDrawResult Draw(GraphicsContext &, const FloatRect &dstRect, const FloatRect &srcRect,
                         ImagePaintingOptions = {}) noexcept override;
    void DrawPattern(GraphicsContext &, const FloatRect &dstRect, const FloatRect &srcRect,
                     const AffineTransform &patternTransform, const FloatPoint &phase,
                     const FloatSize &spacing, ImagePaintingOptions = {}) noexcept override;

    void DrawFiltered(GraphicsContext &) noexcept;
  };
}
