#include "Krystal.Booey/Graphics/Filters/PointLightSource.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterImage.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  Ref<PointLightSource> PointLightSource::Create(const FloatPoint3D &position) noexcept
  {
    return AdoptRef(*new PointLightSource(position));
  }

  PointLightSource::PointLightSource(const FloatPoint3D &position) noexcept
      : LightSource(LightType::LS_POINT), _position(position)
  {
  }

  bool PointLightSource::operator==(const PointLightSource &other) const noexcept
  {
    return LightSource::operator==(other) && _position == other._position;
  }

  void PointLightSource::InitPaintingData(const Filter &filter, const FilterImage &result,
                                          PaintingData &) const noexcept
  {
    auto position = filter.ResolvedPoint3D(_position);
    auto absolutePosition = filter.ScaledByFilterScale(position.XY());
    _bufferPosition.SetXY(result.MappedAbsolutePoint(absolutePosition));

    // To scale Z, map a point offset from position in the x direction by z.
    auto absoluteMappedZ = filter.ScaledByFilterScale(FloatPoint {position.X() + position.Z(), position.Y()});
    _bufferPosition.SetZ(result.MappedAbsolutePoint(absoluteMappedZ).X() - _bufferPosition.X());
  }

  LightSource::ComputedLightingData
    PointLightSource::ComputePixelLightingData(const PaintingData &paintingData, int32 x, int32 y,
                                               float z) const noexcept
  {
    FloatPoint3D lightVector = {_bufferPosition.X() - static_cast<float>(x),
                                _bufferPosition.Y() - static_cast<float>(y), _bufferPosition.Z() - z};

    return {lightVector, paintingData.InitialLightingData.ColorVector, lightVector.Length()};
  }

  bool PointLightSource::SetX(float x) noexcept
  {
    if (_position.X() == x)
    {
      return false;
    }
    _position.SetX(x);
    return true;
  }

  bool PointLightSource::SetY(float y) noexcept
  {
    if (_position.Y() == y)
    {
      return false;
    }
    _position.SetY(y);
    return true;
  }

  bool PointLightSource::SetZ(float z) noexcept
  {
    if (_position.Z() == z)
    {
      return false;
    }
    _position.SetZ(z);
    return true;
  }

  TextStream &PointLightSource::ExternalRepresentation(TextStream &ts) const noexcept
  {
    ts << u8"[type=POINT-LIGHT] ";
    ts << u8"[position=\"" << Position() << u8"\"]";
    return ts;
  }
}
