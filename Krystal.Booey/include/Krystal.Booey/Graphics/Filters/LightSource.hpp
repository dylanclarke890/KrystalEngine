#pragma once

#include "Krystal.Booey/Graphics/FloatPoint3D.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  enum class LightType : uint8
  {
    LS_DISTANT,
    LS_POINT,
    LS_SPOT
  };

  class Filter;
  class FilterImage;

  /// @brief The light of a lighting filter effect.
  class LightSource : public RefCounted<LightSource>
  {
  public:
    struct ComputedLightingData
    {
      FloatPoint3D LightVector;
      FloatPoint3D ColorVector;
      float LightVectorLength {0.f};
    };

    struct PaintingData
    {
      ComputedLightingData InitialLightingData;
      FloatPoint3D DirectionVector;
      float ConeCutOffLimit {0.f};
      float ConeFullLight {0.f};
    };

  private:
    LightType _type;

  public:
    explicit LightSource(LightType type) noexcept : _type(type)
    {
    }

    virtual ~LightSource() noexcept = default;

    KRYS_NODISCARD virtual bool operator==(const LightSource &other) const noexcept
    {
      return _type == other._type;
    }

    KRYS_NODISCARD LightType Type() const noexcept
    {
      return _type;
    }

    virtual TextStream &ExternalRepresentation(TextStream &) const noexcept = 0;

    virtual void InitPaintingData(const Filter &, const FilterImage &result,
                                  PaintingData &) const noexcept = 0;
    // z is a float number, since it is the alpha value scaled by a user
    // specified "surfaceScale" constant, which type is <number> in the SVG standard.
    // x and y are in the coordinates of the FilterEffect's buffer.
    KRYS_NODISCARD virtual ComputedLightingData ComputePixelLightingData(const PaintingData &, int32 x,
                                                                         int32 y, float z) const noexcept = 0;

    virtual bool SetAzimuth(float) noexcept
    {
      return false;
    }
    virtual bool SetElevation(float) noexcept
    {
      return false;
    }

    // These are in user space coordinates.
    virtual bool SetX(float) noexcept
    {
      return false;
    }
    virtual bool SetY(float) noexcept
    {
      return false;
    }
    virtual bool SetZ(float) noexcept
    {
      return false;
    }
    virtual bool SetPointsAtX(float) noexcept
    {
      return false;
    }
    virtual bool SetPointsAtY(float) noexcept
    {
      return false;
    }
    virtual bool SetPointsAtZ(float) noexcept
    {
      return false;
    }

    virtual bool SetSpecularExponent(float) noexcept
    {
      return false;
    }
    virtual bool SetLimitingConeAngle(float) noexcept
    {
      return false;
    }

  protected:
    template <typename TLightSource>
    KRYS_NODISCARD static bool AreEqual(const TLightSource &a, const LightSource &b) noexcept
    {
      auto *bType = DynamicDowncast<TLightSource>(b);
      return bType && a.operator==(*bType);
    }
  };
}

#define KRYS_SPECIALIZE_TYPE_TRAITS_LIGHTSOURCE(ClassName, LightSourceType)                                  \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ClassName)                                               \
    KRYS_NODISCARD static bool IsType(const krys::boo::LightSource &source) noexcept                         \
    {                                                                                                        \
      return source.Type() == krys::boo::LightSourceType;                                                    \
    }                                                                                                        \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
