#pragma once

#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief The tile filter effect: its input's subregion repeated over its own.
  class FETile final : public FilterEffect
  {
  public:
    KRYS_NODISCARD static Ref<FETile>
      Create(const DestinationColorSpace &colorSpace = DestinationColorSpace::SRGB()) noexcept;

  private:
    explicit FETile(const DestinationColorSpace &colorSpace) noexcept;

    KRYS_NODISCARD FloatRect CalculateImageRect(const Filter &filter, Span<const FloatRect> inputImageRects,
                                                const FloatRect &primitiveSubregion) const noexcept override;

    KRYS_NODISCARD bool ResultIsAlphaImage(Span<const Ref<FilterImage>> inputs) const noexcept override;

    TextStream &ExternalRepresentation(TextStream &ts,
                                       FilterRepresentation representation) const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_FUNCTION(FETile)
