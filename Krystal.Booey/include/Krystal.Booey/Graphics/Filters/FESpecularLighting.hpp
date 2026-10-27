#pragma once

#include "Krystal.Booey/Graphics/Filters/FELighting.hpp"

namespace krys::boo
{
  /// @brief The specular lighting filter effect: a shiny surface lit by its light source.
  class FESpecularLighting final : public FELighting
  {
  public:
    KRYS_NODISCARD static Ref<FESpecularLighting>
      Create(const Color &lightingColor, float surfaceScale, float specularConstant, float specularExponent,
             float kernelUnitLengthX, float kernelUnitLengthY, Ref<LightSource> &&lightSource,
             const DestinationColorSpace &colorSpace = DestinationColorSpace::SRGB()) noexcept;

    KRYS_NODISCARD bool operator==(const FESpecularLighting &other) const noexcept
    {
      return FELighting::operator==(other);
    }

    KRYS_NODISCARD float SpecularConstant() const noexcept
    {
      return _specularConstant;
    }
    bool SetSpecularConstant(float specularConstant) noexcept;

    KRYS_NODISCARD float SpecularExponent() const noexcept
    {
      return _specularExponent;
    }
    bool SetSpecularExponent(float specularExponent) noexcept;

    TextStream &ExternalRepresentation(TextStream &ts,
                                       FilterRepresentation representation) const noexcept override;

  private:
    FESpecularLighting(const Color &lightingColor, float surfaceScale, float specularConstant,
                       float specularExponent, float kernelUnitLengthX, float kernelUnitLengthY,
                       Ref<LightSource> &&lightSource, const DestinationColorSpace &colorSpace) noexcept;

    KRYS_NODISCARD bool operator==(const FilterEffect &other) const noexcept override
    {
      return AreEqual<FESpecularLighting>(*this, other);
    }
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_FUNCTION(FESpecularLighting)
