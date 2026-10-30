#pragma once

#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"
#include "Krystal.Booey/Graphics/SourceImage.hpp"
#include "Krystal.Booey/SVG/SVGPreserveAspectRatioValue.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief The image filter effect: an image, or an element drawn into an image buffer.
  class FEImage final : public FilterEffect
  {
  private:
    SourceImage _sourceImage;
    FloatRect _sourceImageRect;
    svg::SVGPreserveAspectRatioValue _preserveAspectRatio;

  public:
    KRYS_NODISCARD static Ref<FEImage>
      Create(SourceImage &&sourceImage, const FloatRect &sourceImageRect,
             const svg::SVGPreserveAspectRatioValue &preserveAspectRatio) noexcept;

    KRYS_NODISCARD bool operator==(const FEImage &other) const noexcept;

    KRYS_NODISCARD const SourceImage &GetSourceImage() const noexcept
    {
      return _sourceImage;
    }
    void SetImageSource(SourceImage &&sourceImage) noexcept
    {
      _sourceImage = krys::move(sourceImage);
    }

    KRYS_NODISCARD FloatRect SourceImageRect() const noexcept
    {
      return _sourceImageRect;
    }
    KRYS_NODISCARD const svg::SVGPreserveAspectRatioValue &PreserveAspectRatio() const noexcept
    {
      return _preserveAspectRatio;
    }

  private:
    FEImage(SourceImage &&sourceImage, const FloatRect &sourceImageRect,
            const svg::SVGPreserveAspectRatioValue &preserveAspectRatio) noexcept;

    KRYS_NODISCARD bool operator==(const FilterEffect &other) const noexcept override
    {
      return AreEqual<FEImage>(*this, other);
    }

    KRYS_NODISCARD uint32 NumberOfEffectInputs() const noexcept override
    {
      return 0u;
    }

    // FEImage results are always in DestinationColorSpace::SRGB()
    void SetOperatingColorSpace(const DestinationColorSpace &) noexcept override
    {
    }

    KRYS_NODISCARD FloatRect CalculateImageRect(const Filter &filter, Span<const FloatRect> inputImageRects,
                                                const FloatRect &primitiveSubregion) const noexcept override;

    TextStream &ExternalRepresentation(TextStream &ts,
                                       FilterRepresentation representation) const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_FUNCTION(FEImage)
