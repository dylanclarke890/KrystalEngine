#pragma once

#include "Krystal.Booey/Graphics/Controls/ControlPart.hpp"

namespace krys::boo
{
  /// @brief A meter: its value in its range, and which region of the gauge it's in.
  class MeterPart : public ControlPart
  {
  public:
    enum class GaugeRegion : uint8
    {
      Optimum,
      Suboptimal,
      EvenLessGood
    };

  private:
    GaugeRegion _gaugeRegion;
    double _value;
    double _minimum;
    double _maximum;

  public:
    KRYS_NODISCARD static Ref<MeterPart> Create() noexcept;
    KRYS_NODISCARD static Ref<MeterPart> Create(GaugeRegion, double value, double minimum,
                                                double maximum) noexcept;

    KRYS_NODISCARD GaugeRegion GetGaugeRegion() const noexcept
    {
      return _gaugeRegion;
    }

    void SetGaugeRegion(GaugeRegion gaugeRegion) noexcept
    {
      _gaugeRegion = gaugeRegion;
    }

    KRYS_NODISCARD double Value() const noexcept
    {
      return _value;
    }

    void SetValue(double value) noexcept
    {
      _value = value;
    }

    KRYS_NODISCARD double Minimum() const noexcept
    {
      return _minimum;
    }

    void SetMinimum(double minimum) noexcept
    {
      _minimum = minimum;
    }

    KRYS_NODISCARD double Maximum() const noexcept
    {
      return _maximum;
    }

    void SetMaximum(double maximum) noexcept
    {
      _maximum = maximum;
    }

  private:
    MeterPart(GaugeRegion, double value, double minimum, double maximum) noexcept;

    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformControl() noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_CONTROL_PART(Meter)
