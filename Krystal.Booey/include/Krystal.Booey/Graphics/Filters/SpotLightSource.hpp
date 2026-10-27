#pragma once

#include "Krystal.Booey/Graphics/Filters/LightSource.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief A light at a point, shining at another, within a cone.
  class SpotLightSource : public LightSource
  {
  private:
    FloatPoint3D _position;
    FloatPoint3D _pointsAt;
    mutable FloatPoint3D _bufferPosition;
    float _specularExponent;
    float _limitingConeAngle;

  public:
    KRYS_NODISCARD static Ref<SpotLightSource> Create(const FloatPoint3D &position,
                                                      const FloatPoint3D &pointsAt, float specularExponent,
                                                      float limitingConeAngle) noexcept;

    KRYS_NODISCARD bool operator==(const SpotLightSource &) const noexcept;

    KRYS_NODISCARD const FloatPoint3D &Position() const noexcept
    {
      return _position;
    }
    KRYS_NODISCARD const FloatPoint3D &Direction() const noexcept
    {
      return _pointsAt;
    }
    KRYS_NODISCARD float SpecularExponent() const noexcept
    {
      return _specularExponent;
    }
    KRYS_NODISCARD float LimitingConeAngle() const noexcept
    {
      return _limitingConeAngle;
    }

    bool SetX(float) noexcept override;
    bool SetY(float) noexcept override;
    bool SetZ(float) noexcept override;
    bool SetPointsAtX(float) noexcept override;
    bool SetPointsAtY(float) noexcept override;
    bool SetPointsAtZ(float) noexcept override;

    bool SetSpecularExponent(float) noexcept override;
    bool SetLimitingConeAngle(float) noexcept override;

    void InitPaintingData(const Filter &, const FilterImage &result, PaintingData &) const noexcept override;
    KRYS_NODISCARD ComputedLightingData ComputePixelLightingData(const PaintingData &, int32 x, int32 y,
                                                                 float z) const noexcept final;

    TextStream &ExternalRepresentation(TextStream &) const noexcept override;

  private:
    SpotLightSource(const FloatPoint3D &position, const FloatPoint3D &direction, float specularExponent,
                    float limitingConeAngle) noexcept;

    KRYS_NODISCARD bool operator==(const LightSource &other) const noexcept override
    {
      return AreEqual<SpotLightSource>(*this, other);
    }
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_LIGHTSOURCE(SpotLightSource, LightType::LS_SPOT)
