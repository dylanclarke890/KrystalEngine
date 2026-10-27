#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterEffect.hpp"
#include "Krystal.Booey/Graphics/Filters/LightSource.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief A lighting filter effect: its input's alpha lit as a surface, by a light source.
  class FELighting : public FilterEffect
  {
  protected:
    Color _lightingColor;
    float _surfaceScale;
    float _diffuseConstant;
    float _specularConstant;
    float _specularExponent;
    float _kernelUnitLengthX;
    float _kernelUnitLengthY;
    const Ref<LightSource> _lightSource;

  public:
    KRYS_NODISCARD bool operator==(const FELighting &other) const noexcept;

    KRYS_NODISCARD const Color &LightingColor() const noexcept
    {
      return _lightingColor;
    }
    bool SetLightingColor(const Color &lightingColor) noexcept;

    KRYS_NODISCARD float SurfaceScale() const noexcept
    {
      return _surfaceScale;
    }
    bool SetSurfaceScale(float surfaceScale) noexcept;

    KRYS_NODISCARD float DiffuseConstant() const noexcept
    {
      return _diffuseConstant;
    }
    KRYS_NODISCARD float SpecularConstant() const noexcept
    {
      return _specularConstant;
    }
    KRYS_NODISCARD float SpecularExponent() const noexcept
    {
      return _specularExponent;
    }

    KRYS_NODISCARD float KernelUnitLengthX() const noexcept
    {
      return _kernelUnitLengthX;
    }
    bool SetKernelUnitLengthX(float kernelUnitLengthX) noexcept;

    KRYS_NODISCARD float KernelUnitLengthY() const noexcept
    {
      return _kernelUnitLengthY;
    }
    bool SetKernelUnitLengthY(float kernelUnitLengthY) noexcept;

    KRYS_NODISCARD LightSource &GetLightSource() const noexcept
    {
      return *_lightSource;
    }

  protected:
    FELighting(Type type, const Color &lightingColor, float surfaceScale, float diffuseConstant,
               float specularConstant, float specularExponent, float kernelUnitLengthX,
               float kernelUnitLengthY, Ref<LightSource> &&lightSource,
               const DestinationColorSpace &colorSpace) noexcept;

    using FilterEffect::operator==;

    KRYS_NODISCARD FloatRect CalculateImageRect(const Filter &filter, Span<const FloatRect> inputImageRects,
                                                const FloatRect &primitiveSubregion) const noexcept override;
  };
}
