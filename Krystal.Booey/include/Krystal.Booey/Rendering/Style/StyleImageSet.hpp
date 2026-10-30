#pragma once

#include "Krystal.Booey/Rendering/Style/StyleMultiImage.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  /// @brief An `image-set()`: candidates at several resolutions, the best of which is used.
  class StyleImageSet final : public StyleMultiImage
  {
  private:
    bool _accessedBestFitImage {false};
    ImageWithScale _bestFitImage;
    float _deviceScaleFactor {1};
    List<ImageWithScale> _images;
    List<size_t> _sortedIndices;

  public:
    /// @param sortedIndices The indices of `images` from the lowest resolution to the highest.
    KRYS_NODISCARD static Ref<StyleImageSet> Create(List<ImageWithScale> &&images,
                                                    List<size_t> &&sortedIndices) noexcept;
    ~StyleImageSet() noexcept override;

    KRYS_NODISCARD bool Equals(const StyleImage &other) const noexcept final;
    KRYS_NODISCARD bool Equals(const StyleImageSet &) const noexcept;

    KRYS_NODISCARD ImageWithScale SelectBestFitImage(const dom::Document &) noexcept final;

  private:
    StyleImageSet(List<ImageWithScale> &&, List<size_t> &&) noexcept;

    KRYS_NODISCARD Ref<css::Value> ComputedStyleValue(const RenderStyle &) const noexcept final;

    KRYS_NODISCARD ImageWithScale BestImageForScaleFactor() noexcept;
    void UpdateDeviceScaleFactor(const dom::Document &) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_IMAGE(StyleImageSet, IsImageSet)
