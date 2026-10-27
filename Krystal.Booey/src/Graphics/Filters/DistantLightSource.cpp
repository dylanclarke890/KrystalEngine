#include "Krystal.Booey/Graphics/Filters/DistantLightSource.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include <cmath>

namespace krys::boo
{
  Ref<DistantLightSource> DistantLightSource::Create(float azimuth, float elevation) noexcept
  {
    return AdoptRef(*new DistantLightSource(azimuth, elevation));
  }

  DistantLightSource::DistantLightSource(float azimuth, float elevation) noexcept
      : LightSource(LightType::LS_DISTANT), _azimuth(azimuth), _elevation(elevation)
  {
  }

  bool DistantLightSource::operator==(const DistantLightSource &other) const noexcept
  {
    return LightSource::operator==(other) && _azimuth == other._azimuth && _elevation == other._elevation;
  }

  void DistantLightSource::InitPaintingData(const Filter &, const FilterImage &,
                                            PaintingData &paintingData) const noexcept
  {
    float azimuth = deg2rad(_azimuth);
    float elevation = deg2rad(_elevation);
    paintingData.InitialLightingData.LightVector = {
      std::cos(azimuth) * std::cos(elevation), std::sin(azimuth) * std::cos(elevation), std::sin(elevation)};
    paintingData.InitialLightingData.LightVectorLength = 1;
  }

  LightSource::ComputedLightingData
    DistantLightSource::ComputePixelLightingData(const PaintingData &paintingData, int32, int32,
                                                 float) const noexcept
  {
    return paintingData.InitialLightingData;
  }

  bool DistantLightSource::SetAzimuth(float azimuth) noexcept
  {
    if (_azimuth == azimuth)
    {
      return false;
    }
    _azimuth = azimuth;
    return true;
  }

  bool DistantLightSource::SetElevation(float elevation) noexcept
  {
    if (_elevation == elevation)
    {
      return false;
    }
    _elevation = elevation;
    return true;
  }

  TextStream &DistantLightSource::ExternalRepresentation(TextStream &ts) const noexcept
  {
    ts << u8"[type=DISTANT-LIGHT] ";
    ts << u8"[azimuth=\"" << Azimuth() << u8"\"]";
    ts << u8"[elevation=\"" << Elevation() << u8"\"]";
    return ts;
  }
}
