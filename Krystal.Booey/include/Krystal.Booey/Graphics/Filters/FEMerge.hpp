#pragma once

#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief The merge filter effect: its inputs drawn over one another, the first lowest.
  class FEMerge final : public FilterEffect
  {
  private:
    uint32 _numberOfEffectInputs {0u};

  public:
    KRYS_NODISCARD static Ref<FEMerge>
      Create(uint32 numberOfEffectInputs,
             const DestinationColorSpace &colorSpace = DestinationColorSpace::SRGB()) noexcept;

    KRYS_NODISCARD bool operator==(const FEMerge &other) const noexcept;

    KRYS_NODISCARD uint32 NumberOfEffectInputs() const noexcept override
    {
      return _numberOfEffectInputs;
    }

  private:
    FEMerge(uint32 numberOfEffectInputs, const DestinationColorSpace &colorSpace) noexcept;

    KRYS_NODISCARD bool operator==(const FilterEffect &other) const noexcept override
    {
      return AreEqual<FEMerge>(*this, other);
    }

    TextStream &ExternalRepresentation(TextStream &ts,
                                       FilterRepresentation representation) const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_FUNCTION(FEMerge)
