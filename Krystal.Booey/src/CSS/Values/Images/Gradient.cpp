#include "Krystal.Booey/CSS/Values/Images/Gradient.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::css
{
  // MARK: - Gradient Color Stop

  namespace
  {
    template <typename C, typename P>
    void ColorStopSerialisationForCSS(CSSOMString &builder, const SerialisationContext &context,
                                      const GradientColorStop<C, P> &stop) noexcept
    {
      if (stop.Color && stop.Position)
      {
        Serialise(builder, context, *stop.Color);
        builder.push_back(u8' ');
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

  void Serialiser<GradientAngularColorStop>::operator()(CSSOMString &builder,
                                                        const SerialisationContext &context,
                                                        const GradientAngularColorStop &stop) noexcept
  {
    ColorStopSerialisationForCSS(builder, context, stop);
  }

  void Serialiser<GradientLinearColorStop>::operator()(CSSOMString &builder,
                                                       const SerialisationContext &context,
                                                       const GradientLinearColorStop &stop) noexcept
  {
    ColorStopSerialisationForCSS(builder, context, stop);
  }

  void Serialiser<GradientDeprecatedColorStop>::operator()(CSSOMString &builder,
                                                           const SerialisationContext &context,
                                                           const GradientDeprecatedColorStop &stop) noexcept
  {
    auto appendRaw = [&](const auto &color, NumberRaw<> raw)
    {
      if (!raw.Value)
      {
        builder.append(u8"from(");
        Serialise(builder, context, color);
        builder.push_back(u8')');
      }
      else if (raw.Value == 1)
      {
        builder.append(u8"to(");
        Serialise(builder, context, color);
        builder.push_back(u8')');
      }
      else
      {
        builder.append(u8"color-stop(");
        Serialise(builder, context, raw);
        builder.append(u8", ");
        Serialise(builder, context, color);
        builder.push_back(u8')');
      }
    };

    auto appendCalc = [&](const auto &color, const auto &calc)
    {
      builder.append(u8"color-stop(");
      Serialise(builder, context, calc);
      builder.append(u8", ");
      Serialise(builder, context, color);
      builder.push_back(u8')');
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
    bool AppendColorInterpolationMethod(CSSOMString &builder,
                                        GradientColorInterpolationMethod colorInterpolationMethod,
                                        bool needsLeadingSpace) noexcept
    {
      return krys::SwitchOn(
        colorInterpolationMethod.Method.ColorSpace,
        [&](const ColorInterpolationMethod::OKLab &)
        {
          if (colorInterpolationMethod.DefaultMethod != GradientColorInterpolationMethod::Default::OKLab)
          {
            builder.append(needsLeadingSpace ? u8" in oklab" : u8"in oklab");
            return true;
          }
          return false;
        },
        [&](const ColorInterpolationMethod::SRGB &)
        {
          if (colorInterpolationMethod.DefaultMethod != GradientColorInterpolationMethod::Default::SRGB)
          {
            builder.append(needsLeadingSpace ? u8" in srgb" : u8"in srgb");
            return true;
          }
          return false;
        },
        [&]<typename MethodColorSpace>(const MethodColorSpace &methodColorSpace)
        {
          builder.append(needsLeadingSpace ? u8" in " : u8"in ");
          builder.append(SerialisationForCSS(methodColorSpace.InterpolationColorSpace));
          if constexpr (hasHueInterpolationMethod<MethodColorSpace>)
          {
            SerialisationForCSS(builder, methodColorSpace.HueInterpolationMethod);
          }
          return true;
        });
    }
  }

  // MARK: - LinearGradient

  void Serialiser<LinearGradient>::operator()(CSSOMString &builder, const SerialisationContext &context,
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
        builder.append(u8"to ");
        Serialise(builder, context, horizontal);
        wroteSomething = true;
      },
      [&](const Vertical &vertical)
      {
        if (std::holds_alternative<keywords::Bottom>(vertical))
        {
          return;
        }

        builder.append(u8"to ");
        Serialise(builder, context, vertical);
        wroteSomething = true;
      },
      [&](const SpaceSeparatedTuple<Horizontal, Vertical> &pair)
      {
        builder.append(u8"to ");
        Serialise(builder, context, pair);
        wroteSomething = true;
      });

    if (AppendColorInterpolationMethod(builder, gradient.ColorInterpolationMethod, wroteSomething))
    {
      wroteSomething = true;
    }

    if (wroteSomething)
    {
      builder.append(u8", ");
    }

    Serialise(builder, context, gradient.Stops);
  }

  // MARK: - PrefixedLinearGradient

  void Serialiser<PrefixedLinearGradient>::operator()(CSSOMString &builder,
                                                      const SerialisationContext &context,
                                                      const PrefixedLinearGradient &gradient) noexcept
  {
    Serialise(builder, context, gradient.GradientLine);
    builder.append(u8", ");
    Serialise(builder, context, gradient.Stops);
  }

  // MARK: - DeprecatedLinearGradient

  void Serialiser<DeprecatedLinearGradient>::operator()(CSSOMString &builder,
                                                        const SerialisationContext &context,
                                                        const DeprecatedLinearGradient &gradient) noexcept
  {
    builder.append(u8"linear, ");

    Serialise(builder, context, gradient.GradientLine);

    if (!gradient.Stops.isEmpty())
    {
      builder.append(u8", ");
      Serialise(builder, context, gradient.Stops);
    }
  }

  // MARK: - RadialGradient

  void Serialiser<RadialGradient::Ellipse>::operator()(CSSOMString &builder,
                                                       const SerialisationContext &context,
                                                       const RadialGradient::Ellipse &ellipse) noexcept
  {
    auto lengthBefore = builder.size();

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
        bool wroteSomething = builder.size() != lengthBefore;
        if (wroteSomething)
        {
          builder.push_back(u8' ');
        }

        builder.append(u8"at ");
        Serialise(builder, context, *ellipse.Position);
      }
    }
  }

  void Serialiser<RadialGradient::Circle>::operator()(CSSOMString &builder,
                                                      const SerialisationContext &context,
                                                      const RadialGradient::Circle &circle) noexcept
  {
    krys::SwitchOn(
      circle.Size, [&](const RadialGradient::Circle::Length &length) { Serialise(builder, context, length); },
      [&](const RadialGradient::Extent &extent)
      {
        if (!std::holds_alternative<keywords::FarthestCorner>(extent))
        {
          builder.append(u8"circle ");
          Serialise(builder, context, extent);
        }
        else
        {
          builder.append(u8"circle");
        }
      });

    if (circle.Position)
    {
      if (!IsCenterPosition(*circle.Position))
      {
        builder.append(u8" at ");
        Serialise(builder, context, *circle.Position);
      }
    }
  }

  void Serialiser<RadialGradient>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                              const RadialGradient &gradient) noexcept
  {
    auto lengthBefore = builder.size();
    Serialise(builder, context, gradient.GradientBox);
    bool wroteSomething = builder.size() != lengthBefore;

    if (AppendColorInterpolationMethod(builder, gradient.ColorInterpolationMethod, wroteSomething))
    {
      wroteSomething = true;
    }

    if (wroteSomething)
    {
      builder.append(u8", ");
    }

    Serialise(builder, context, gradient.Stops);
  }

  // MARK: - PrefixedRadialGradient

  void Serialiser<PrefixedRadialGradient::Ellipse>::operator()(
    CSSOMString &builder, const SerialisationContext &context,
    const PrefixedRadialGradient::Ellipse &ellipse) noexcept
  {
    if (ellipse.Position)
    {
      Serialise(builder, context, *ellipse.Position);
    }
    else
    {
      builder.append(u8"center");
    }

    if (ellipse.Size)
    {
      krys::SwitchOn(
        *ellipse.Size,
        [&](const PrefixedRadialGradient::Ellipse::SizeType &size)
        {
          builder.append(u8", ");
          Serialise(builder, context, size);
        },
        [&](const PrefixedRadialGradient::Extent &extent)
        {
          builder.append(u8", ellipse ");
          Serialise(builder, context, extent);
        });
    }
  }

  void Serialiser<PrefixedRadialGradient::Circle>::operator()(
    CSSOMString &builder, const SerialisationContext &context,
    const PrefixedRadialGradient::Circle &circle) noexcept
  {
    if (circle.Position)
    {
      Serialise(builder, context, *circle.Position);
    }
    else
    {
      builder.append(u8"center");
    }

    builder.append(u8", circle ");
    Serialise(builder, context,
              circle.Size ? *circle.Size : PrefixedRadialGradient::Extent {keywords::Cover {}});
  }

  void Serialiser<PrefixedRadialGradient>::operator()(CSSOMString &builder,
                                                      const SerialisationContext &context,
                                                      const PrefixedRadialGradient &gradient) noexcept
  {
    auto lengthBefore = builder.size();
    Serialise(builder, context, gradient.GradientBox);
    bool wroteSomething = builder.size() != lengthBefore;

    if (wroteSomething)
    {
      builder.append(u8", ");
    }

    Serialise(builder, context, gradient.Stops);
  }

  // MARK: - DeprecatedRadialGradient

  void Serialiser<DeprecatedRadialGradient::GradientBoxType>::operator()(
    CSSOMString &builder, const SerialisationContext &context,
    const DeprecatedRadialGradient::GradientBoxType &gradientBox) noexcept
  {
    Serialise(builder, context, gradientBox.First);
    builder.append(u8", ");
    Serialise(builder, context, gradientBox.FirstRadius);
    builder.append(u8", ");
    Serialise(builder, context, gradientBox.Second);
    builder.append(u8", ");
    Serialise(builder, context, gradientBox.SecondRadius);
  }

  void Serialiser<DeprecatedRadialGradient>::operator()(CSSOMString &builder,
                                                        const SerialisationContext &context,
                                                        const DeprecatedRadialGradient &gradient) noexcept
  {
    builder.append(u8"radial, ");

    Serialise(builder, context, gradient.GradientBox);

    if (!gradient.Stops.isEmpty())
    {
      builder.append(u8", ");
      Serialise(builder, context, gradient.Stops);
    }
  }

  // MARK: - ConicGradient

  void Serialiser<ConicGradient::GradientBoxType>::operator()(
    CSSOMString &builder, const SerialisationContext &context,
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
            builder.append(u8"from ");
            Serialise(builder, context, angleRaw);
            wroteSomething = true;
          }
        },
        [&](const Angle<>::Calc &angleCalc)
        {
          builder.append(u8"from ");
          Serialise(builder, context, angleCalc);
          wroteSomething = true;
        });
    }

    if (gradientBox.Position && !IsCenterPosition(*gradientBox.Position))
    {
      if (wroteSomething)
      {
        builder.push_back(u8' ');
      }
      builder.append(u8"at ");
      Serialise(builder, context, *gradientBox.Position);
    }
  }

  void Serialiser<ConicGradient>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                             const ConicGradient &gradient) noexcept
  {
    auto lengthBefore = builder.size();
    Serialise(builder, context, gradient.GradientBox);
    bool wroteSomething = builder.size() != lengthBefore;

    if (AppendColorInterpolationMethod(builder, gradient.ColorInterpolationMethod, wroteSomething))
    {
      wroteSomething = true;
    }

    if (wroteSomething)
    {
      builder.append(u8", ");
    }

    Serialise(builder, context, gradient.Stops);
  }
}
