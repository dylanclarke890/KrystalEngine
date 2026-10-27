#pragma once

#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Colour/BlendMode.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief The blend filter effect: its first input blended over its second.
  class FEBlend final : public FilterEffect
  {
  private:
    BlendMode _mode;

  public:
    KRYS_NODISCARD static Ref<FEBlend>
      Create(BlendMode mode,
             const DestinationColorSpace &colorSpace = DestinationColorSpace::SRGB()) noexcept;

    KRYS_NODISCARD bool operator==(const FEBlend &other) const noexcept;

    KRYS_NODISCARD BlendMode GetBlendMode() const noexcept
    {
      return _mode;
    }
    bool SetBlendMode(BlendMode mode) noexcept;

  private:
    FEBlend(BlendMode mode, const DestinationColorSpace &colorSpace) noexcept;

    KRYS_NODISCARD bool operator==(const FilterEffect &other) const noexcept override
    {
      return AreEqual<FEBlend>(*this, other);
    }

    KRYS_NODISCARD uint32 NumberOfEffectInputs() const noexcept override
    {
      return 2u;
    }

    TextStream &ExternalRepresentation(TextStream &ts,
                                       FilterRepresentation representation) const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_FUNCTION(FEBlend)
