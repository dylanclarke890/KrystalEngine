#pragma once

#include "Krystal.Booey/Core/BoxExtents.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief The offset filter effect: its input moved.
  class FEOffset final : public FilterEffect
  {
  private:
    float _dx;
    float _dy;

  public:
    KRYS_NODISCARD static Ref<FEOffset>
      Create(float dx, float dy,
             const DestinationColorSpace &colorSpace = DestinationColorSpace::SRGB()) noexcept;

    KRYS_NODISCARD bool operator==(const FEOffset &other) const noexcept;

    KRYS_NODISCARD float Dx() const noexcept
    {
      return _dx;
    }
    bool SetDx(float dx) noexcept;

    KRYS_NODISCARD float Dy() const noexcept
    {
      return _dy;
    }
    bool SetDy(float dy) noexcept;

    KRYS_NODISCARD static IntOutsets CalculateOutsets(const FloatSize &offset) noexcept;

  private:
    FEOffset(float dx, float dy, const DestinationColorSpace &colorSpace) noexcept;

    KRYS_NODISCARD bool operator==(const FilterEffect &other) const noexcept override
    {
      return AreEqual<FEOffset>(*this, other);
    }

    KRYS_NODISCARD FloatRect CalculateImageRect(const Filter &filter, Span<const FloatRect> inputImageRects,
                                                const FloatRect &primitiveSubregion) const noexcept override;

    KRYS_NODISCARD bool ResultIsAlphaImage(Span<const Ref<FilterImage>> inputs) const noexcept override;

    TextStream &ExternalRepresentation(TextStream &ts,
                                       FilterRepresentation representation) const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_FUNCTION(FEOffset)
