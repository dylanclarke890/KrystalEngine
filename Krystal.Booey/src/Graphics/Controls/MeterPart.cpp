#include "Krystal.Booey/Graphics/Controls/MeterPart.hpp"

namespace krys::boo
{
  Ref<MeterPart> MeterPart::Create() noexcept
  {
    return AdoptRef(*new MeterPart(GaugeRegion::EvenLessGood, 0, 0, 0));
  }

  Ref<MeterPart> MeterPart::Create(GaugeRegion gaugeRegion, double value, double minimum,
                                   double maximum) noexcept
  {
    return AdoptRef(*new MeterPart(gaugeRegion, value, minimum, maximum));
  }

  MeterPart::MeterPart(GaugeRegion gaugeRegion, double value, double minimum, double maximum) noexcept
      : ControlPart(StyleAppearance::Meter), _gaugeRegion(gaugeRegion), _value(value), _minimum(minimum),
        _maximum(maximum)
  {
  }

  UniquePtr<PlatformControl> MeterPart::CreatePlatformControl() noexcept
  {
    return GetControlFactory().CreatePlatformMeter(*this);
  }
}
