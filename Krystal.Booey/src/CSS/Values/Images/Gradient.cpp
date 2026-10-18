#include "Krystal.Booey/CSS/Values/Images/Gradient.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  // MARK: - Gradient Color Stop

  namespace
  {
    template <typename C, typename P>
    void ColorStopSerialisationForCSS(StringBuilder &builder, const SerialisationContext &context,
                                      const GradientColorStop<C, P> &stop) noexcept
    {
      if (stop.Color && stop.Position)
      {
        Serialise(builder, context, *stop.Color);
        builder.Append(u8' ');
        Serialise(builder, context, *stop.Position);
      }
      else if (stop.Color)
      {
        Serialise(builder, context, *stop.Color);
      }
      else if (stop.Position)
      {
        Serialise(builder, context, *stop.Position);
      }
    }
  }

  void Serialiser<GradientAngularColorStop>::operator()(StringBuilder &builder,
                                                        const SerialisationContext &context,
                                                        const GradientAngularColorStop &stop) noexcept
  {
    ColorStopSerialisationForCSS(builder, context, stop);
  }

  void Serialiser<GradientLinearColorStop>::operator()(StringBuilder &builder,
                                                       const SerialisationContext &context,
                                                       const GradientLinearColorStop &stop) noexcept
  {
    ColorStopSerialisationForCSS(builder, context, stop);
  }

  void Serialiser<GradientDeprecatedColorStop>::operator()(StringBuilder &builder,
                                                           const SerialisationContext &context,
                                                           const GradientDeprecatedColorStop &stop) noexcept
  {
    auto appendRaw = [&](const auto &color, NumberRaw<> raw)
    {
      if (!raw.Value)
      {
        builder.Append(u8"from(");
        Serialise(builder, context, color);
        builder.Append(u8')');
      }
      else if (raw.Value == 1)
      {
        builder.Append(u8"to(");
        Serialise(builder, context, color);
        builder.Append(u8')');
      }
      else
      {
        builder.Append(u8"color-stop(");
        Serialise(builder, context, raw);
        builder.Append(u8", ");
        Serialise(builder, context, color);
        builder.Append(u8')');
      }
    };

    auto appendCalc = [&](const auto &color, const auto &calc)
    {
      builder.Append(u8"color-stop(");
      Serialise(builder, context, calc);
      builder.Append(u8", ");
      Serialise(builder, context, color);
      builder.Append(u8')');
    };

    stop.Position.SwitchOn(
      [&](const Number<> &number)
      {
        krys::SwitchOn(
          number, [&](const Number<>::Raw &raw) { appendRaw(stop.Color, raw); },
          [&](const Number<>::Calc &calc) { appendCalc(stop.Color, calc); });
      },
      [&](const Percentage<> &percentage)
      {
        krys::SwitchOn(
          percentage, [&](const Percentage<>::Raw &raw) { appendRaw(stop.Color, {raw.Value / 100.0}); },
          [&](const Percentage<>::Calc &calc) { appendCalc(stop.Color, calc); });
      });
  }

  // MARK: - Gradient Color Interpolation

  namespace
  {
    bool AppendColorInterpolationMethod(StringBuilder &builder,
                                        GradientColorInterpolationMethod colorInterpolationMethod,
                                        bool needsLeadingSpace) noexcept
    {
      return krys::SwitchOn(
        colorInterpolationMethod.Method.ColourSpace,
        [&](const ColourInterpolationMethod::OKLab &)
        {
          if (colorInterpolationMethod.DefaultMethod != GradientColorInterpolationMethod::Default::OKLab)
          {
            builder.Append(needsLeadingSpace ? u8" in oklab" : u8"in oklab");
            return true;
          }
          return false;
        },
        [&](const ColourInterpolationMethod::SRGB &)
        {
          if (colorInterpolationMethod.DefaultMethod != GradientColorInterpolationMethod::Default::SRGB)
          {
            builder.Append(needsLeadingSpace ? u8" in srgb" : u8"in srgb");
            return true;
          }
          return false;
        },
        [&]<typename MethodColourSpace>(const MethodColourSpace &methodColorSpace)
        {
          builder.Append(needsLeadingSpace ? u8" in " : u8"in ");
          builder.Append(SerialisationForCSS(methodColorSpace.InterpolationColourSpace));
          if constexpr (hasHueInterpolationMethod<MethodColourSpace>)
          {
            SerialisationForCSS(builder, methodColorSpace.HueInterpolationMethod);
          }
          return true;
        });
    }
  }

  // MARK: - LinearGradient

  void Serialiser<LinearGradient>::operator()(StringBuilder &builder, const SerialisationContext &context,
                                              const LinearGradient &gradient) noexcept
  {
    bool wroteSomething = false;

    krys::SwitchOn(
      gradient.GradientLine,
      [&](const Angle<> &angle)
      {
        krys::SwitchOn(
          angle,
          [&](const Angle<>::Raw &angleRaw)
          {
            if (ConvertToValueInUnitsOf<AngleUnit::Deg>(angleRaw) == 180)
            {
              return;
            }

            Serialise(builder, context, angleRaw);
            wroteSomething = true;
          },
          [&](const Angle<>::Calc &angleCalc)
          {
            Serialise(builder, context, angleCalc);
            wroteSomething = true;
          });
      },
      [&](const Horizontal &horizontal)
      {
        builder.Append(u8"to ");
        Serialise(builder, context, horizontal);
        wroteSomething = true;
      },
      [&](const Vertical &vertical)
      {
        if (std::holds_alternative<keywords::Bottom>(vertical))
        {
          return;
        }

        builder.Append(u8"to ");
        Serialise(builder, context, vertical);
        wroteSomething = true;
      },
      [&](const SpaceSeparatedTuple<Horizontal, Vertical> &pair)
      {
        builder.Append(u8"to ");
        Serialise(builder, context, pair);
        wroteSomething = true;
      });

    if (AppendColorInterpolationMethod(builder, gradient.ColourInterpolationMethod, wroteSomething))
    {
      wroteSomething = true;
    }

    if (wroteSomething)
    {
      builder.Append(u8", ");
    }

    Serialise(builder, context, gradient.Stops);
  }

  // MARK: - PrefixedLinearGradient

  void Serialiser<PrefixedLinearGradient>::operator()(StringBuilder &builder,
                                                      const SerialisationContext &context,
                                                      const PrefixedLinearGradient &gradient) noexcept
  {
    Serialise(builder, context, gradient.GradientLine);
    builder.Append(u8", ");
    Serialise(builder, context, gradient.Stops);
  }

  // MARK: - DeprecatedLinearGradient

  void Serialiser<DeprecatedLinearGradient>::operator()(StringBuilder &builder,
                                                        const SerialisationContext &context,
                                                        const DeprecatedLinearGradient &gradient) noexcept
  {
    builder.Append(u8"linear, ");

    Serialise(builder, context, gradient.GradientLine);

    if (!gradient.Stops.isEmpty())
    {
      builder.Append(u8", ");
      Serialise(builder, context, gradient.Stops);
    }
  }

  // MARK: - RadialGradient

  void Serialiser<RadialGradient::Ellipse>::operator()(StringBuilder &builder,
                                                       const SerialisationContext &context,
                                                       const RadialGradient::Ellipse &ellipse) noexcept
  {
    auto lengthBefore = builder.Length();

    krys::SwitchOn(
      ellipse.Size, [&](const RadialGradient::Ellipse::SizeType &size) { Serialise(builder, context, size); },
      [&](const RadialGradient::Extent &extent)
      {
        if (!std::holds_alternative<keywords::FarthestCorner>(extent))
        {
          Serialise(builder, context, extent);
        }
      });

    if (ellipse.Position)
    {
      if (!IsCenterPosition(*ellipse.Position))
      {
        bool wroteSomething = builder.Length() != lengthBefore;
        if (wroteSomething)
        {
          builder.Append(u8' ');
        }

        builder.Append(u8"at ");
        Serialise(builder, context, *ellipse.Position);
      }
    }
  }

  void Serialiser<RadialGradient::Circle>::operator()(StringBuilder &builder,
                                                      const SerialisationContext &context,
                                                      const RadialGradient::Circle &circle) noexcept
  {
    krys::SwitchOn(
      circle.Size, [&](const RadialGradient::Circle::Length &length) { Serialise(builder, context, length); },
      [&](const RadialGradient::Extent &extent)
      {
        if (!std::holds_alternative<keywords::FarthestCorner>(extent))
        {
          builder.Append(u8"circle ");
          Serialise(builder, context, extent);
        }
        else
        {
          builder.Append(u8"circle");
        }
      });

    if (circle.Position)
    {
      if (!IsCenterPosition(*circle.Position))
      {
        builder.Append(u8" at ");
        Serialise(builder, context, *circle.Position);
      }
    }
  }

  void Serialiser<RadialGradient>::operator()(StringBuilder &builder, const SerialisationContext &context,
                                              const RadialGradient &gradient) noexcept
  {
    auto lengthBefore = builder.Length();
    Serialise(builder, context, gradient.GradientBox);
    bool wroteSomething = builder.Length() != lengthBefore;

    if (AppendColorInterpolationMethod(builder, gradient.ColourInterpolationMethod, wroteSomething))
    {
      wroteSomething = true;
    }

    if (wroteSomething)
    {
      builder.Append(u8", ");
    }

    Serialise(builder, context, gradient.Stops);
  }

  // MARK: - PrefixedRadialGradient

  void Serialiser<PrefixedRadialGradient::Ellipse>::operator()(
    StringBuilder &builder, const SerialisationContext &context,
    const PrefixedRadialGradient::Ellipse &ellipse) noexcept
  {
    if (ellipse.Position)
    {
      Serialise(builder, context, *ellipse.Position);
    }
    else
    {
      builder.Append(u8"center");
    }

    if (ellipse.Size)
    {
      krys::SwitchOn(
        *ellipse.Size,
        [&](const PrefixedRadialGradient::Ellipse::SizeType &size)
        {
          builder.Append(u8", ");
          Serialise(builder, context, size);
        },
        [&](const PrefixedRadialGradient::Extent &extent)
        {
          builder.Append(u8", ellipse ");
          Serialise(builder, context, extent);
        });
    }
  }

  void Serialiser<PrefixedRadialGradient::Circle>::operator()(
    StringBuilder &builder, const SerialisationContext &context,
    const PrefixedRadialGradient::Circle &circle) noexcept
  {
    if (circle.Position)
    {
      Serialise(builder, context, *circle.Position);
    }
    else
    {
      builder.Append(u8"center");
    }

    builder.Append(u8", circle ");
    Serialise(builder, context,
              circle.Size ? *circle.Size : PrefixedRadialGradient::Extent {keywords::Cover {}});
  }

  void Serialiser<PrefixedRadialGradient>::operator()(StringBuilder &builder,
                                                      const SerialisationContext &context,
                                                      const PrefixedRadialGradient &gradient) noexcept
  {
    auto lengthBefore = builder.Length();
    Serialise(builder, context, gradient.GradientBox);
    bool wroteSomething = builder.Length() != lengthBefore;

    if (wroteSomething)
    {
      builder.Append(u8", ");
    }

    Serialise(builder, context, gradient.Stops);
  }

  // MARK: - DeprecatedRadialGradient

  void Serialiser<DeprecatedRadialGradient::GradientBoxType>::operator()(
    StringBuilder &builder, const SerialisationContext &context,
    const DeprecatedRadialGradient::GradientBoxType &gradientBox) noexcept
  {
    Serialise(builder, context, gradientBox.First);
    builder.Append(u8", ");
    Serialise(builder, context, gradientBox.FirstRadius);
    builder.Append(u8", ");
    Serialise(builder, context, gradientBox.Second);
    builder.Append(u8", ");
    Serialise(builder, context, gradientBox.SecondRadius);
  }

  void Serialiser<DeprecatedRadialGradient>::operator()(StringBuilder &builder,
                                                        const SerialisationContext &context,
                                                        const DeprecatedRadialGradient &gradient) noexcept
  {
    builder.Append(u8"radial, ");

    Serialise(builder, context, gradient.GradientBox);

    if (!gradient.Stops.isEmpty())
    {
      builder.Append(u8", ");
      Serialise(builder, context, gradient.Stops);
    }
  }

  // MARK: - ConicGradient

  void Serialiser<ConicGradient::GradientBoxType>::operator()(
    StringBuilder &builder, const SerialisationContext &context,
    const ConicGradient::GradientBoxType &gradientBox) noexcept
  {
    bool wroteSomething = false;

    if (gradientBox.Angle)
    {
      krys::SwitchOn(
        *gradientBox.Angle,
        [&](const Angle<>::Raw &angleRaw)
        {
          if (angleRaw.Value)
          {
            builder.Append(u8"from ");
            Serialise(builder, context, angleRaw);
            wroteSomething = true;
          }
        },
        [&](const Angle<>::Calc &angleCalc)
        {
          builder.Append(u8"from ");
          Serialise(builder, context, angleCalc);
          wroteSomething = true;
        });
    }

    if (gradientBox.Position && !IsCenterPosition(*gradientBox.Position))
    {
      if (wroteSomething)
      {
        builder.Append(u8' ');
      }
      builder.Append(u8"at ");
      Serialise(builder, context, *gradientBox.Position);
    }
  }

  void Serialiser<ConicGradient>::operator()(StringBuilder &builder, const SerialisationContext &context,
                                             const ConicGradient &gradient) noexcept
  {
    auto lengthBefore = builder.Length();
    Serialise(builder, context, gradient.GradientBox);
    bool wroteSomething = builder.Length() != lengthBefore;

    if (AppendColorInterpolationMethod(builder, gradient.ColourInterpolationMethod, wroteSomething))
    {
      wroteSomething = true;
    }

    if (wroteSomething)
    {
      builder.Append(u8", ");
    }

    Serialise(builder, context, gradient.Stops);
  }
}
