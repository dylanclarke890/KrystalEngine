#pragma once

#include "Krystal.Booey/Graphics/Filters/FELighting.hpp"

namespace krys::boo
{
  /// @brief The diffuse lighting filter effect: a matte surface lit by its light source.
  class FEDiffuseLighting final : public FELighting
  {
  public:
    KRYS_NODISCARD static Ref<FEDiffuseLighting>
      Create(const Color &lightingColor, float surfaceScale, float diffuseConstant, float kernelUnitLengthX,
             float kernelUnitLengthY, Ref<LightSource> &&lightSource,
             const DestinationColorSpace &colorSpace = DestinationColorSpace::SRGB()) noexcept;

    KRYS_NODISCARD bool operator==(const FEDiffuseLighting &other) const noexcept
    {
      return FELighting::operator==(other);
    }

    KRYS_NODISCARD float DiffuseConstant() const noexcept
    {
      return _diffuseConstant;
    }
    bool SetDiffuseConstant(float diffuseConstant) noexcept;

    TextStream &ExternalRepresentation(TextStream &ts,
                                       FilterRepresentation representation) const noexcept override;

  private:
    FEDiffuseLighting(const Color &lightingColor, float surfaceScale, float diffuseConstant,
                      float kernelUnitLengthX, float kernelUnitLengthY, Ref<LightSource> &&lightSource,
                      const DestinationColorSpace &colorSpace) noexcept;

    KRYS_NODISCARD bool operator==(const FilterEffect &other) const noexcept override
    {
      return AreEqual<FEDiffuseLighting>(*this, other);
    }
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_FUNCTION(FEDiffuseLighting)
