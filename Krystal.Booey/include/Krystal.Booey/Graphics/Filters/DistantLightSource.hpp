#pragma once

#include "Krystal.Booey/Graphics/Filters/LightSource.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief A light infinitely far away, in a direction.
  class DistantLightSource : public LightSource
  {
  private:
    float _azimuth;
    float _elevation;

  public:
    KRYS_NODISCARD static Ref<DistantLightSource> Create(float azimuth, float elevation) noexcept;

    KRYS_NODISCARD bool operator==(const DistantLightSource &) const noexcept;

    // These are in degrees.
    KRYS_NODISCARD float Azimuth() const noexcept
    {
      return _azimuth;
    }
    KRYS_NODISCARD float Elevation() const noexcept
    {
      return _elevation;
    }

    bool SetAzimuth(float) noexcept override;
    bool SetElevation(float) noexcept override;

    void InitPaintingData(const Filter &, const FilterImage &result, PaintingData &) const noexcept override;
    KRYS_NODISCARD ComputedLightingData ComputePixelLightingData(const PaintingData &, int32 x, int32 y,
                                                                 float z) const noexcept final;

    TextStream &ExternalRepresentation(TextStream &) const noexcept override;

  private:
    DistantLightSource(float azimuth, float elevation) noexcept;

    KRYS_NODISCARD bool operator==(const LightSource &other) const noexcept override
    {
      return AreEqual<DistantLightSource>(*this, other);
    }
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_LIGHTSOURCE(DistantLightSource, LightType::LS_DISTANT)
