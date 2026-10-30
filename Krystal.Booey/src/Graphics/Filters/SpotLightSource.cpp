#include "Krystal.Booey/Graphics/Filters/SpotLightSource.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterImage.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo
{
  namespace
  {
    // spot-light edge darkening depends on an absolute treshold
    // according to the SVG 1.1 SE light regression tests
    constexpr float AntialiasThreshold = 0.016f;
  }

  Ref<SpotLightSource> SpotLightSource::Create(const FloatPoint3D &position, const FloatPoint3D &direction,
                                               float specularExponent, float limitingConeAngle) noexcept
  {
    return AdoptRef(*new SpotLightSource(position, direction, specularExponent, limitingConeAngle));
  }

  SpotLightSource::SpotLightSource(const FloatPoint3D &position, const FloatPoint3D &pointsAt,
                                   float specularExponent, float limitingConeAngle) noexcept
      : LightSource(LightType::LS_SPOT), _position(position), _pointsAt(pointsAt),
        _specularExponent(std::clamp(specularExponent, 1.0f, 128.0f)), _limitingConeAngle(limitingConeAngle)
  {
  }

  bool SpotLightSource::operator==(const SpotLightSource &other) const noexcept
  {
    return LightSource::operator==(other) && _position == other._position && _pointsAt == other._pointsAt
           && _specularExponent == other._specularExponent && _limitingConeAngle == other._limitingConeAngle;
  }

  void SpotLightSource::InitPaintingData(const Filter &filter, const FilterImage &result,
                                         PaintingData &paintingData) const noexcept
  {
    auto position = filter.ResolvedPoint3D(_position);
    auto pointsAt = filter.ResolvedPoint3D(_pointsAt);

    auto absolutePosition = filter.ScaledByFilterScale(position.XY());
    _bufferPosition.SetXY(result.MappedAbsolutePoint(absolutePosition));

    // To scale Z, map a point offset from position in the x direction by z.
    auto absoluteMappedZ = filter.ScaledByFilterScale(FloatPoint {position.X() + position.Z(), position.Y()});
    _bufferPosition.SetZ(result.MappedAbsolutePoint(absoluteMappedZ).X() - _bufferPosition.X());

    paintingData.DirectionVector = pointsAt - position;
    paintingData.DirectionVector.Normalise();

    if (!_limitingConeAngle)
    {
      paintingData.ConeCutOffLimit = 0.0f;
      paintingData.ConeFullLight = -AntialiasThreshold;
    }
    else
    {
      float limitingConeAngle = _limitingConeAngle;
      if (limitingConeAngle < 0.0f)
      {
        limitingConeAngle = -limitingConeAngle;
      }
      if (limitingConeAngle > 90.0f)
      {
        limitingConeAngle = 90.0f;
      }
      paintingData.ConeCutOffLimit = std::cos(deg2rad(180.0f - limitingConeAngle));
      paintingData.ConeFullLight = paintingData.ConeCutOffLimit - AntialiasThreshold;
    }
  }

  LightSource::ComputedLightingData
    SpotLightSource::ComputePixelLightingData(const PaintingData &paintingData, int32 x, int32 y,
                                              float z) const noexcept
  {
    FloatPoint3D lightVector = {_bufferPosition.X() - static_cast<float>(x),
                                _bufferPosition.Y() - static_cast<float>(y), _bufferPosition.Z() - z};
    float lightVectorLength = lightVector.Length();

    float cosineOfAngle = (lightVector * paintingData.DirectionVector) / lightVectorLength;
    if (cosineOfAngle > paintingData.ConeCutOffLimit)
    {
      // No light is produced, scanlines are not updated
      return {lightVector, {}, lightVectorLength};
    }

    // Set the color of the pixel
    float lightStrength;
    if (1.0f == _specularExponent)
    {
      lightStrength = -cosineOfAngle; // -cosineOfAngle ^ 1 == -cosineOfAngle
    }
    else
    {
      lightStrength = std::pow(-cosineOfAngle, _specularExponent);
    }

    if (cosineOfAngle > paintingData.ConeFullLight)
    {
      lightStrength *= (paintingData.ConeCutOffLimit - cosineOfAngle)
                       / (paintingData.ConeCutOffLimit - paintingData.ConeFullLight);
    }

    if (lightStrength > 1.0f)
    {
      lightStrength = 1.0f;
    }

    return {lightVector, paintingData.InitialLightingData.ColorVector * lightStrength, lightVectorLength};
  }

  bool SpotLightSource::SetX(float x) noexcept
  {
    if (_position.X() == x)
    {
      return false;
    }
    _position.SetX(x);
    return true;
  }

  bool SpotLightSource::SetY(float y) noexcept
  {
    if (_position.Y() == y)
    {
      return false;
    }
    _position.SetY(y);
    return true;
  }

  bool SpotLightSource::SetZ(float z) noexcept
  {
    if (_position.Z() == z)
    {
      return false;
    }
    _position.SetZ(z);
    return true;
  }

  bool SpotLightSource::SetPointsAtX(float pointsAtX) noexcept
  {
    if (_pointsAt.X() == pointsAtX)
    {
      return false;
    }
    _pointsAt.SetX(pointsAtX);
    return true;
  }

  bool SpotLightSource::SetPointsAtY(float pointsAtY) noexcept
  {
    if (_pointsAt.Y() == pointsAtY)
    {
      return false;
    }
    _pointsAt.SetY(pointsAtY);
    return true;
  }

  bool SpotLightSource::SetPointsAtZ(float pointsAtZ) noexcept
  {
    if (_pointsAt.Z() == pointsAtZ)
    {
      return false;
    }
    _pointsAt.SetZ(pointsAtZ);
    return true;
  }

  bool SpotLightSource::SetSpecularExponent(float specularExponent) noexcept
  {
    specularExponent = std::clamp(specularExponent, 1.0f, 128.0f);
    if (_specularExponent == specularExponent)
    {
      return false;
    }
    _specularExponent = specularExponent;
    return true;
  }

  bool SpotLightSource::SetLimitingConeAngle(float limitingConeAngle) noexcept
  {
    if (_limitingConeAngle == limitingConeAngle)
    {
      return false;
    }
    _limitingConeAngle = limitingConeAngle;
    return true;
  }

  TextStream &SpotLightSource::ExternalRepresentation(TextStream &ts) const noexcept
  {
    ts << u8"[type=SPOT-LIGHT] ";
    ts << u8"[position=\"" << Position() << u8"\"]";
    ts << u8"[direction=\"" << Direction() << u8"\"]";
    ts << u8"[specularExponent=\"" << SpecularExponent() << u8"\"]";
    ts << u8"[limitingConeAngle=\"" << LimitingConeAngle() << u8"\"]";
    return ts;
  }
}
