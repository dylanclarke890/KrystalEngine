#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief The flood filter effect: a colour filling its subregion.
  class FEFlood final : public FilterEffect
  {
  private:
    Color _floodColor;
    float _floodOpacity;

  public:
    KRYS_NODISCARD static Ref<FEFlood>
      Create(const Color &floodColor, float floodOpacity,
             const DestinationColorSpace &colorSpace = DestinationColorSpace::SRGB()) noexcept;

    KRYS_NODISCARD bool operator==(const FEFlood &other) const noexcept;

    KRYS_NODISCARD const Color &FloodColor() const noexcept
    {
      return _floodColor;
    }
    bool SetFloodColor(const Color &color) noexcept;

    KRYS_NODISCARD float FloodOpacity() const noexcept
    {
      return _floodOpacity;
    }
    bool SetFloodOpacity(float floodOpacity) noexcept;

    // feFlood does not perform color interpolation of any kind, so the result is always in the current
    // color space regardless of the value of color-interpolation-filters.
    void SetOperatingColorSpace(const DestinationColorSpace &) noexcept override
    {
    }

  private:
    FEFlood(const Color &floodColor, float floodOpacity, const DestinationColorSpace &colorSpace) noexcept;

    KRYS_NODISCARD bool operator==(const FilterEffect &other) const noexcept override
    {
      return AreEqual<FEFlood>(*this, other);
    }

    KRYS_NODISCARD uint32 NumberOfEffectInputs() const noexcept override
    {
      return 0u;
    }

    KRYS_NODISCARD FloatRect CalculateImageRect(const Filter &filter, Span<const FloatRect> inputImageRects,
                                                const FloatRect &primitiveSubregion) const noexcept override;

    TextStream &ExternalRepresentation(TextStream &ts,
                                       FilterRepresentation representation) const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_FUNCTION(FEFlood)
