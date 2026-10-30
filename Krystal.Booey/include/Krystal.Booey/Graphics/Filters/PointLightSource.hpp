#pragma once

#include "Krystal.Booey/Graphics/Filters/LightSource.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief A light at a point, shining in every direction.
  class PointLightSource : public LightSource
  {
  private:
    FloatPoint3D _position;
    mutable FloatPoint3D _bufferPosition;

  public:
    KRYS_NODISCARD static Ref<PointLightSource> Create(const FloatPoint3D &position) noexcept;

    KRYS_NODISCARD bool operator==(const PointLightSource &) const noexcept;

    KRYS_NODISCARD const FloatPoint3D &Position() const noexcept
    {
      return _position;
    }
    bool SetX(float) noexcept override;
    bool SetY(float) noexcept override;
    bool SetZ(float) noexcept override;

    void InitPaintingData(const Filter &, const FilterImage &result, PaintingData &) const noexcept override;
    KRYS_NODISCARD ComputedLightingData ComputePixelLightingData(const PaintingData &, int32 x, int32 y,
                                                                 float z) const noexcept final;

    TextStream &ExternalRepresentation(TextStream &) const noexcept override;

  private:
    explicit PointLightSource(const FloatPoint3D &position) noexcept;

    KRYS_NODISCARD bool operator==(const LightSource &other) const noexcept override
    {
      return AreEqual<PointLightSource>(*this, other);
    }
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_LIGHTSOURCE(PointLightSource, LightType::LS_POINT)
