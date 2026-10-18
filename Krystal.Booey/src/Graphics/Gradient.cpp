#include "Krystal.Booey/Graphics/Gradient.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"

namespace krys::boo
{
  Ref<Gradient> Gradient::Create(Data &&data, ColourInterpolationMethod colorInterpolationMethod,
                                 GradientSpreadMethod spreadMethod, GradientColorStops &&stops,
                                 bool isTransient) noexcept
  {
    return AdoptRef(*new Gradient(krys::move(data), colorInterpolationMethod, spreadMethod, krys::move(stops),
                                  isTransient));
  }

  Gradient::Gradient(Data &&data, ColourInterpolationMethod colorInterpolationMethod,
                     GradientSpreadMethod spreadMethod, GradientColorStops &&stops, bool isTransient) noexcept
      : _data {krys::move(data)}, _colorInterpolationMethod {colorInterpolationMethod},
        _spreadMethod {spreadMethod}, _stops {krys::move(stops)}, _isTransient {isTransient}
  {
  }

  Gradient::~Gradient() noexcept = default;

  void Gradient::AdjustParametersForTiledDrawing(FloatSize &size, FloatRect &srcRect,
                                                 const FloatSize &spacing) noexcept
  {
    if (srcRect.IsEmpty())
    {
      return;
    }

    if (!spacing.IsZero())
    {
      return;
    }

    krys::SwitchOn(
      _data,
      [&](const LinearData &data)
      {
        if (data.Point0.X() == data.Point1.X())
        {
          size.SetWidth(1);
          srcRect.SetWidth(1);
          srcRect.SetX(0);
          return;
        }
        if (data.Point0.Y() != data.Point1.Y())
        {
          return;
        }

        size.SetHeight(1);
        srcRect.SetHeight(1);
        srcRect.SetY(0);
      },
      [](const RadialData &) {}, [](const ConicData &) {});
  }

  bool Gradient::IsZeroSize() const noexcept
  {
    return krys::SwitchOn(
      _data, [](const LinearData &data)
      { return data.Point0.X() == data.Point1.X() && data.Point0.Y() == data.Point1.Y(); },
      [](const RadialData &data)
      {
        return data.Point0.X() == data.Point1.X() && data.Point0.Y() == data.Point1.Y()
               && data.StartRadius == data.EndRadius;
      },
      [](const ConicData &) { return false; });
  }

  void Gradient::AddColorStop(GradientColorStop &&stop) noexcept
  {
    _stops.AddColorStop(krys::move(stop));
  }
}
