#include "Krystal.Gfx/Render2D/Gradient2D.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Colour/ColourConversion.hpp"
#include "Krystal.Core/Colour/ColourInterpolation.hpp"
#include "Krystal.Core/Visitor.hpp"
#include <algorithm>
#include <cmath>

namespace krys::gfx::tests
{
  namespace
  {
    using Colour = Array<float, 4>;

    KRYS_NODISCARD float Distance(const Colour &a, const Colour &b) noexcept
    {
      return std::max(
        {std::abs(a[0] - b[0]), std::abs(a[1] - b[1]), std::abs(a[2] - b[2]), std::abs(a[3] - b[3])});
    }

    KRYS_NODISCARD Colour Premultiplied(const ExtendedSRGBA<float> &colour) noexcept
    {
      const auto [red, green, blue, alpha] = colour.Resolved();
      auto unit = [](float value)
      {
        return std::clamp(value, 0.f, 1.f);
      };
      const float a = unit(alpha);
      return {unit(red) * a, unit(green) * a, unit(blue) * a, a};
    }

    /// @brief The colour Core's interpolation gives between two stops, premultiplied in sRGB.
    KRYS_NODISCARD Colour Interpolated(const ColourInterpolationMethod &method,
                                       const ExtendedSRGBA<float> &from, const ExtendedSRGBA<float> &to,
                                       float u) noexcept
    {
      return SwitchOn(
        method.ColourSpace,
        [&]<typename TSpace>(const TSpace &space)
        {
          using ColourType = typename TSpace::ColourType;
          const auto a = ConvertColour<ColourType>(from);
          const auto b = ConvertColour<ColourType>(to);
          const auto mixed =
            method.AlphaPremultiplication == AlphaPremultiplication::Premultiplied
              ? InterpolateColourComponents<AlphaPremultiplication::Premultiplied>(space, a, 1.0 - u, b, u)
              : InterpolateColourComponents<AlphaPremultiplication::Unpremultiplied>(space, a, 1.0 - u, b, u);
          return Premultiplied(ConvertColour<ExtendedSRGBA<float>>(mixed));
        });
    }

    KRYS_NODISCARD Gradient2D TwoStops(const ColourInterpolationMethod &method,
                                       const ExtendedSRGBA<float> &from, const ExtendedSRGBA<float> &to)
    {
      return {.Stops = {{.Offset = 0.f, .Colour = from}, {.Offset = 1.f, .Colour = to}},
              .Interpolation = method};
    }

    /// @brief The largest gap between the flattened stops and Core's interpolation, over many offsets (not
    /// the ones flattening samples).
    KRYS_NODISCARD float LargestError(const Gradient2D &gradient, const List<detail::FlatGradientStop> &stops)
    {
      float error = 0.f;
      for (uint32 i = 0u; i <= 999u; ++i)
      {
        const float u = static_cast<float>(i) / 999.f;
        error = std::max(error, Distance(detail::EvaluateFlatStops(stops, u),
                                         Interpolated(gradient.Interpolation, gradient.Stops[0].Colour,
                                                      gradient.Stops[1].Colour, u)));
      }
      return error;
    }

    constexpr ColourInterpolationMethod SrgbPremultiplied {.ColourSpace = ColourInterpolationMethod::SRGB {},
                                                           .AlphaPremultiplication =
                                                             AlphaPremultiplication::Premultiplied};
  }

  TEST_CASE("Gradient2D: sRGB with premultiplied alpha flattens to its own stops", "[Gfx][Render2D]")
  {
    // Interpolating premultiplied sRGB is already linear there, translucent stops and all.
    const auto gradient = TwoStops(SrgbPremultiplied, {1.f, 0.f, 0.f, 1.f}, {0.f, 0.f, 1.f, 0.25f});
    List<detail::FlatGradientStop> stops;
    detail::FlattenGradientStops(gradient, stops);
    REQUIRE(stops.size() == 2u);
    CHECK(stops[0].Offset == 0.f);
    CHECK(stops[1].Offset == 1.f);
    CHECK(Distance(stops[1].Colour, {0.f, 0.f, 0.25f, 0.25f}) < 1e-6f);
    CHECK(LargestError(gradient, stops) < 1e-5f);
  }

  TEST_CASE("Gradient2D: flattens other interpolations within the tolerance", "[Gfx][Render2D]")
  {
    using Method = ColourInterpolationMethod;
    struct Case
    {
      const char *Name;
      Method Interpolation;
    };
    const Case cases[] {
      {"srgb, straight alpha",
       {.ColourSpace = Method::SRGB {}, .AlphaPremultiplication = AlphaPremultiplication::Unpremultiplied}},
      {"srgb-linear",
       {.ColourSpace = Method::SRGBLinear {},
        .AlphaPremultiplication = AlphaPremultiplication::Premultiplied}},
      {"oklab",
       {.ColourSpace = Method::OKLab {}, .AlphaPremultiplication = AlphaPremultiplication::Premultiplied}},
      {"lab",
       {.ColourSpace = Method::Lab {}, .AlphaPremultiplication = AlphaPremultiplication::Premultiplied}},
      {"oklch",
       {.ColourSpace = Method::OKLCH {}, .AlphaPremultiplication = AlphaPremultiplication::Premultiplied}},
      {"oklch longer hue",
       {.ColourSpace = Method::OKLCH {HueInterpolationMethod::Longer},
        .AlphaPremultiplication = AlphaPremultiplication::Premultiplied}},
      {"hsl",
       {.ColourSpace = Method::HSL {}, .AlphaPremultiplication = AlphaPremultiplication::Premultiplied}},
      {"hsl longer hue",
       {.ColourSpace = Method::HSL {HueInterpolationMethod::Longer},
        .AlphaPremultiplication = AlphaPremultiplication::Premultiplied}},
      {"hsl increasing hue",
       {.ColourSpace = Method::HSL {HueInterpolationMethod::Increasing},
        .AlphaPremultiplication = AlphaPremultiplication::Premultiplied}},
      {"hsl decreasing hue",
       {.ColourSpace = Method::HSL {HueInterpolationMethod::Decreasing},
        .AlphaPremultiplication = AlphaPremultiplication::Unpremultiplied}},
      {"hwb",
       {.ColourSpace = Method::HWB {}, .AlphaPremultiplication = AlphaPremultiplication::Premultiplied}},
      {"display-p3",
       {.ColourSpace = Method::DisplayP3 {},
        .AlphaPremultiplication = AlphaPremultiplication::Premultiplied}},
    };

    for (const auto &item : cases)
    {
      INFO(item.Name);
      // Translucent orange to opaque blue: curved in every space but premultiplied sRGB.
      const auto gradient = TwoStops(item.Interpolation, {1.f, 0.5f, 0.f, 0.5f}, {0.f, 0.2f, 1.f, 1.f});
      List<detail::FlatGradientStop> stops;
      detail::FlattenGradientStops(gradient, stops);
      CHECK(stops.size() > 2u);
      CHECK(stops.size() <= detail::MaxGradientStops);
      // Between the samples flattening checks, colours stray a little further.
      CHECK(LargestError(gradient, stops) <= detail::GradientTolerance * 2.f);
      CHECK(std::is_sorted(stops.begin(), stops.end(),
                           [](const auto &a, const auto &b) { return a.Offset < b.Offset; }));
    }
  }

  TEST_CASE("Gradient2D: keeps hard stops", "[Gfx][Render2D]")
  {
    const ExtendedSRGBA<float> red {1.f, 0.f, 0.f, 1.f};
    const ExtendedSRGBA<float> green {0.f, 1.f, 0.f, 1.f};
    const ExtendedSRGBA<float> blue {0.f, 0.f, 1.f, 1.f};
    const Gradient2D gradient {.Stops = {{.Offset = 0.f, .Colour = red},
                                         {.Offset = 0.5f, .Colour = red},
                                         {.Offset = 0.5f, .Colour = green},
                                         {.Offset = 0.5f, .Colour = blue},
                                         {.Offset = 1.f, .Colour = blue}}};
    List<detail::FlatGradientStop> stops;
    detail::FlattenGradientStops(gradient, stops);

    // Of the three stops at one offset, the middle one never shows.
    REQUIRE(stops.size() == 4u);
    CHECK(Distance(detail::EvaluateFlatStops(stops, 0.4999f), {1.f, 0.f, 0.f, 1.f}) < 1e-6f);
    CHECK(Distance(detail::EvaluateFlatStops(stops, 0.5f), {0.f, 0.f, 1.f, 1.f}) < 1e-6f);
    CHECK(Distance(detail::EvaluateFlatStops(stops, 0.75f), {0.f, 0.f, 1.f, 1.f}) < 1e-6f);
  }

  TEST_CASE("Gradient2D: sorts and clamps stops, and pads past the ends", "[Gfx][Render2D]")
  {
    const ExtendedSRGBA<float> red {1.f, 0.f, 0.f, 1.f};
    const ExtendedSRGBA<float> blue {0.f, 0.f, 1.f, 1.f};
    const ExtendedSRGBA<float> white {1.f, 1.f, 1.f, 1.f};
    List<detail::FlatGradientStop> stops;

    // Sorted by offset; stops at the same offset keep their order.
    detail::FlattenGradientStops({.Stops = {{.Offset = 0.75f, .Colour = blue},
                                            {.Offset = 0.25f, .Colour = red},
                                            {.Offset = 0.75f, .Colour = white}}},
                                 stops);
    REQUIRE(stops.size() == 3u);
    CHECK(stops[0].Offset == 0.25f);
    CHECK(Distance(detail::EvaluateFlatStops(stops, 0.f), {1.f, 0.f, 0.f, 1.f}) < 1e-6f);
    CHECK(Distance(detail::EvaluateFlatStops(stops, 0.5f), {0.5f, 0.f, 0.5f, 1.f}) < 1e-6f);
    CHECK(Distance(detail::EvaluateFlatStops(stops, 0.8f), {1.f, 1.f, 1.f, 1.f}) < 1e-6f);
    CHECK(Distance(detail::EvaluateFlatStops(stops, 1.f), {1.f, 1.f, 1.f, 1.f}) < 1e-6f);

    // Offsets outside 0 to 1 are clamped into it.
    detail::FlattenGradientStops(
      {.Stops = {{.Offset = -1.f, .Colour = red}, {.Offset = 2.f, .Colour = blue}}}, stops);
    REQUIRE(stops.size() == 2u);
    CHECK(stops[0].Offset == 0.f);
    CHECK(stops[1].Offset == 1.f);

    // Colours outside sRGB are clamped once interpolated.
    detail::FlattenGradientStops({.Stops = {{.Offset = 0.f, .Colour = {1.5f, -0.5f, 0.f, 1.f}}}}, stops);
    REQUIRE(stops.size() == 1u);
    CHECK(Distance(stops[0].Colour, {1.f, 0.f, 0.f, 1.f}) < 1e-6f);

    detail::FlattenGradientStops({}, stops);
    CHECK(stops.empty());
  }

  TEST_CASE("Gradient2D: keeps to the stop limit", "[Gfx][Render2D]")
  {
    // More stops than fit are sampled evenly.
    Gradient2D many {};
    for (uint32 i = 0u; i < 300u; ++i)
    {
      const float level = static_cast<float>(i % 2u);
      many.Stops.push_back({.Offset = static_cast<float>(i) / 299.f, .Colour = {level, level, level, 1.f}});
    }
    List<detail::FlatGradientStop> stops;
    detail::FlattenGradientStops(many, stops);
    CHECK(stops.size() == detail::MaxGradientStops);

    // A gradient that would split into too many fits by splitting less.
    Gradient2D hues {
      .Interpolation = {.ColourSpace = ColourInterpolationMethod::HSL {HueInterpolationMethod::Longer},
                        .AlphaPremultiplication = AlphaPremultiplication::Premultiplied}};
    for (uint32 i = 0u; i < 40u; ++i)
    {
      hues.Stops.push_back({.Offset = static_cast<float>(i) / 39.f, .Colour = {1.f, 0.f, 0.f, 1.f}});
    }
    detail::FlattenGradientStops(hues, stops);
    CHECK(stops.size() > 40u);
    CHECK(stops.size() <= detail::MaxGradientStops);
  }

  TEST_CASE("Gradient2D: averages flattened stops", "[Gfx][Render2D]")
  {
    List<detail::FlatGradientStop> stops {{.Offset = 0.5f, .Colour = {1.f, 0.f, 0.f, 1.f}},
                                          {.Offset = 1.f, .Colour = {0.f, 0.f, 1.f, 1.f}}};
    // Half red, then a quarter each of red and blue as it fades.
    CHECK(Distance(detail::AverageFlatStops(stops), {0.75f, 0.f, 0.25f, 1.f}) < 1e-6f);
  }

  TEST_CASE("Gradient2D: knows when it has no size", "[Gfx][Render2D]")
  {
    CHECK(Gradient2D {.Shape = LinearGradient2D {{1.f, 2.f}, {1.f, 2.f}}}.IsZeroSize());
    CHECK_FALSE(Gradient2D {.Shape = LinearGradient2D {{1.f, 2.f}, {1.f, 3.f}}}.IsZeroSize());
    CHECK(Gradient2D {.Shape = RadialGradient2D {{1.f, 2.f}, {1.f, 2.f}, 3.f, 3.f}}.IsZeroSize());
    CHECK_FALSE(Gradient2D {.Shape = RadialGradient2D {{1.f, 2.f}, {1.f, 2.f}, 0.f, 3.f}}.IsZeroSize());
    CHECK_FALSE(Gradient2D {.Shape = ConicGradient2D {}}.IsZeroSize());
  }
}
