#include "Krystal.Gfx/Render2D/Gradient2D.hpp"
#include "Krystal.Core/Colour/ColourConversion.hpp"
#include "Krystal.Core/Colour/ColourInterpolation.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.Core/Visitor.hpp"
#include <algorithm>
#include <cmath>

namespace krys::gfx
{
  bool Gradient2D::IsZeroSize() const noexcept
  {
    return SwitchOn(
      Shape, [](const LinearGradient2D &data)
      { return data.Point0.x == data.Point1.x && data.Point0.y == data.Point1.y; },
      [](const RadialGradient2D &data)
      {
        return data.Point0.x == data.Point1.x && data.Point0.y == data.Point1.y
               && data.StartRadius == data.EndRadius;
      },
      [](const ConicGradient2D &) { return false; });
  }
}

namespace krys::gfx::detail
{
  namespace
  {
    /// @brief How many samples of each span flattening is checked against.
    constexpr uint32 SpanSamples = 512u;
    /// @brief How many times a sample interval is halved at most, around a kink.
    constexpr uint32 MaxRefinement = 8u;

    /// @brief A colour clamped into sRGB and premultiplied.
    KRYS_NODISCARD Array<float, 4> ToFlatColour(const ExtendedSRGBA<float> &colour) noexcept
    {
      const auto [red, green, blue, alpha] = colour.Resolved();
      auto unit = [](float value)
      {
        return std::isnan(value) ? 0.f : std::clamp(value, 0.f, 1.f);
      };
      const float a = unit(alpha);
      return {unit(red) * a, unit(green) * a, unit(blue) * a, a};
    }

    KRYS_NODISCARD Array<float, 4> Mix(const Array<float, 4> &a, const Array<float, 4> &b, float f) noexcept
    {
      return {a[0] + (b[0] - a[0]) * f, a[1] + (b[1] - a[1]) * f, a[2] + (b[2] - a[2]) * f,
              a[3] + (b[3] - a[3]) * f};
    }

    KRYS_NODISCARD float Distance(const Array<float, 4> &a, const Array<float, 4> &b) noexcept
    {
      return std::max(
        {std::abs(a[0] - b[0]), std::abs(a[1] - b[1]), std::abs(a[2] - b[2]), std::abs(a[3] - b[3])});
    }

    /// @brief Whether a colour's components are all within sRGB.
    KRYS_NODISCARD bool InGamut(const ExtendedSRGBA<float> &colour) noexcept
    {
      const auto [red, green, blue, alpha] = colour.Resolved();
      auto unit = [](float value)
      {
        return value >= 0.f && value <= 1.f;
      };
      return unit(red) && unit(green) && unit(blue) && unit(alpha);
    }

    /// @brief Adds the stops after `u0` up to and including `u1`, within one sample interval of a span:
    /// halved while the colour halfway strays from the straight line, which only a kink (where a colour is
    /// clamped into sRGB) makes it do.
    template <typename TEvaluate>
    void Refine(const TEvaluate &evaluate, float offset0, float offset1, double u0, const Array<float, 4> &c0,
                double u1, const Array<float, 4> &c1, uint32 depth, float tolerance,
                List<FlatGradientStop> &stops) noexcept
    {
      const double um = (u0 + u1) * 0.5;
      const auto middle = evaluate(um);
      if (depth < MaxRefinement && Distance(middle, Mix(c0, c1, 0.5f)) > tolerance)
      {
        Refine(evaluate, offset0, offset1, u0, c0, um, middle, depth + 1u, tolerance, stops);
        Refine(evaluate, offset0, offset1, um, middle, u1, c1, depth + 1u, tolerance, stops);
        return;
      }
      stops.push_back({.Offset = offset0 + (offset1 - offset0) * static_cast<float>(u1), .Colour = c1});
    }

    /// @brief Adds the stops after sample `first` up to and including sample `last` of a span, halving it
    /// while any sample between strays from the straight line between them. Checking every sample, rather
    /// than a few, catches the kinks where colours are clamped into sRGB.
    template <typename TEvaluate>
    void Subdivide(const TEvaluate &evaluate, const List<Array<float, 4>> &samples, float offset0,
                   float offset1, uint32 first, uint32 last, float tolerance,
                   List<FlatGradientStop> &stops) noexcept
    {
      const double count = static_cast<double>(samples.size() - 1u);
      if (last - first == 1u)
      {
        Refine(evaluate, offset0, offset1, first / count, samples[first], last / count, samples[last], 0u,
               tolerance, stops);
        return;
      }

      const float length = static_cast<float>(last - first);
      float error = 0.f;
      for (uint32 i = first + 1u; i < last && error <= tolerance; ++i)
      {
        const float f = static_cast<float>(i - first) / length;
        error = std::max(error, Distance(samples[i], Mix(samples[first], samples[last], f)));
      }
      if (error > tolerance)
      {
        const uint32 middle = first + (last - first) / 2u;
        Subdivide(evaluate, samples, offset0, offset1, first, middle, tolerance, stops);
        Subdivide(evaluate, samples, offset0, offset1, middle, last, tolerance, stops);
        return;
      }
      stops.push_back({.Offset = offset0 + (offset1 - offset0) * static_cast<float>(last / count),
                       .Colour = samples[last]});
    }

    /// @brief Adds the stops after the first, span by span, interpolating in `space`.
    template <typename TSpace>
    void FlattenSpans(const TSpace &space, AlphaPremultiplication alpha, const List<GradientStop2D> &sorted,
                      float tolerance, List<Array<float, 4>> &samples, List<FlatGradientStop> &stops) noexcept
    {
      using ColourType = typename TSpace::ColourType;
      for (size_t i = 0u; i + 1u < sorted.size(); ++i)
      {
        const auto &from = sorted[i];
        const auto &to = sorted[i + 1u];
        const auto end = ToFlatColour(to.Colour);
        if (!(to.Offset > from.Offset))
        {
          // A hard stop.
          stops.push_back({.Offset = to.Offset, .Colour = end});
          continue;
        }

        // In sRGB, premultiplied interpolation (or straight with one alpha) is already linear, unless
        // clamping bends it.
        if constexpr (SameType<TSpace, ColourInterpolationMethod::SRGB>)
        {
          const bool linear = alpha == AlphaPremultiplication::Premultiplied
                              || ToFlatColour(from.Colour)[3] == ToFlatColour(to.Colour)[3];
          if (linear && InGamut(from.Colour) && InGamut(to.Colour))
          {
            stops.push_back({.Offset = to.Offset, .Colour = end});
            continue;
          }
        }

        const auto a = ConvertColour<ColourType>(from.Colour);
        const auto b = ConvertColour<ColourType>(to.Colour);
        auto evaluate = [&](double u)
        {
          const auto mixed =
            alpha == AlphaPremultiplication::Premultiplied
              ? InterpolateColourComponents<AlphaPremultiplication::Premultiplied>(space, a, 1.0 - u, b, u)
              : InterpolateColourComponents<AlphaPremultiplication::Unpremultiplied>(space, a, 1.0 - u, b, u);
          return ToFlatColour(ConvertColour<ExtendedSRGBA<float>>(mixed));
        };
        samples.resize(SpanSamples + 1u);
        for (uint32 s = 0u; s <= SpanSamples; ++s)
        {
          samples[s] = evaluate(static_cast<double>(s) / SpanSamples);
        }
        // The ends are the stops' own colours.
        samples.front() = ToFlatColour(from.Colour);
        samples.back() = end;
        Subdivide(evaluate, samples, from.Offset, to.Offset, 0u, SpanSamples, tolerance, stops);
      }
    }
  }

  void FlattenGradientStops(const Gradient2D &gradient, List<FlatGradientStop> &stops) noexcept
  {
    stops.clear();
    if (gradient.Stops.empty())
    {
      return;
    }

    List<GradientStop2D> sorted = gradient.Stops;
    for (auto &stop : sorted)
    {
      stop.Offset = std::isnan(stop.Offset) ? 0.f : std::clamp(stop.Offset, 0.f, 1.f);
    }
    std::stable_sort(sorted.begin(), sorted.end(),
                     [](const GradientStop2D &a, const GradientStop2D &b) { return a.Offset < b.Offset; });

    // A tolerance of 1, a channel's whole range, splits nothing, leaving only the gradient's own stops.
    float tolerance = GradientTolerance;
    List<Array<float, 4>> samples;
    for (;;)
    {
      stops.clear();
      stops.push_back({.Offset = sorted.front().Offset, .Colour = ToFlatColour(sorted.front().Colour)});
      SwitchOn(gradient.Interpolation.ColourSpace,
               [&](const auto &space)
               {
                 FlattenSpans(space, gradient.Interpolation.AlphaPremultiplication, sorted, tolerance,
                              samples, stops);
               });

      // Of three or more stops at one offset, only the first and last show.
      size_t kept = 0u;
      for (size_t i = 0u; i < stops.size(); ++i)
      {
        const bool middle = kept > 0u && i + 1u < stops.size() && stops[kept - 1u].Offset == stops[i].Offset
                            && stops[i + 1u].Offset == stops[i].Offset;
        if (!middle)
        {
          stops[kept++] = stops[i];
        }
      }
      stops.resize(kept);

      if (stops.size() <= MaxGradientStops || tolerance >= 1.f)
      {
        break;
      }
      tolerance *= 2.f;
    }

    // More stops than fit even unsplit: sampled evenly instead.
    if (stops.size() > MaxGradientStops)
    {
      List<FlatGradientStop> sampled;
      sampled.reserve(MaxGradientStops);
      for (uint32 i = 0u; i < MaxGradientStops; ++i)
      {
        const float offset = static_cast<float>(i) / static_cast<float>(MaxGradientStops - 1u);
        sampled.push_back({.Offset = offset, .Colour = EvaluateFlatStops(stops, offset)});
      }
      stops = krys::move(sampled);
    }
  }

  Array<float, 4> EvaluateFlatStops(const List<FlatGradientStop> &stops, float offset) noexcept
  {
    if (stops.empty())
    {
      return {};
    }
    // The last stop at or before the offset.
    const auto after =
      std::upper_bound(stops.begin(), stops.end(), offset,
                       [](float value, const FlatGradientStop &stop) { return value < stop.Offset; });
    if (after == stops.begin())
    {
      return stops.front().Colour;
    }
    if (after == stops.end())
    {
      return stops.back().Colour;
    }
    const auto &from = *(after - 1);
    return Mix(from.Colour, after->Colour, (offset - from.Offset) / (after->Offset - from.Offset));
  }

  Array<float, 4> AverageFlatStops(const List<FlatGradientStop> &stops) noexcept
  {
    if (stops.empty())
    {
      return {};
    }
    Array<float, 4> sum {};
    auto add = [&](const Array<float, 4> &colour, float weight)
    {
      for (size_t c = 0u; c < 4u; ++c)
      {
        sum[c] += colour[c] * weight;
      }
    };
    add(stops.front().Colour, stops.front().Offset);
    for (size_t i = 0u; i + 1u < stops.size(); ++i)
    {
      const float width = stops[i + 1u].Offset - stops[i].Offset;
      add(stops[i].Colour, width * 0.5f);
      add(stops[i + 1u].Colour, width * 0.5f);
    }
    add(stops.back().Colour, 1.f - stops.back().Offset);
    return sum;
  }
}
