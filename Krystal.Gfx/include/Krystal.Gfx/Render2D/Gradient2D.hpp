#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Colour/ColourInterpolationMethod.hpp"
#include "Krystal.Core/Colour/ColourTypes.hpp"
#include "Krystal.Core/Colour/GradientSpreadMethod.hpp"
#include "Krystal.Core/Maths/Vector.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::gfx
{
  /// @brief A gradient along the line from `Point0` (offset 0) to `Point1` (offset 1).
  struct LinearGradient2D
  {
    Vec2f Point0 {};
    Vec2f Point1 {};
  };

  /// @brief A gradient between two circles: offset 0 is the circle at `Point0` with `StartRadius`, offset 1
  /// the one at `Point1` with `EndRadius`, and each offset between is the circle between them. Where circles
  /// overlap, the one with the largest offset shows; where none reaches, nothing does.
  struct RadialGradient2D
  {
    Vec2f Point0 {};
    Vec2f Point1 {};
    float StartRadius {0.f};
    float EndRadius {0.f};
    /// @brief For an elliptical gradient, its width over its height: the circles are squashed vertically
    /// about `Point0`.
    float AspectRatio {1.f};
  };

  /// @brief A gradient around `Point0`, clockwise from `AngleRadians` (0 points up) all the way round.
  struct ConicGradient2D
  {
    Vec2f Point0 {};
    float AngleRadians {0.f};
  };

  struct GradientStop2D
  {
    /// @brief Where the stop is, from 0 to 1.
    float Offset {0.f};
    /// @brief Straight alpha; components outside 0 to 1 hold colours outside sRGB until they're interpolated.
    ExtendedSRGBA<float> Colour {};
  };

  /// @brief A gradient for `Paint2D`: a shape, colour stops, how it carries on past its ends and how its
  /// colours are interpolated.
  ///
  /// Stops are sorted by offset, keeping the order of stops at the same offset (a hard stop), and offsets are
  /// clamped to 0 to 1. Before the first stop and after the last, their colours carry on.
  struct Gradient2D
  {
    Variant<LinearGradient2D, RadialGradient2D, ConicGradient2D> Shape {LinearGradient2D {}};
    List<GradientStop2D> Stops {};
    GradientSpreadMethod Spread {GradientSpreadMethod::Pad};
    ColourInterpolationMethod Interpolation {.ColourSpace = ColourInterpolationMethod::SRGB {},
                                             .AlphaPremultiplication = AlphaPremultiplication::Premultiplied};

    /// @brief Whether the shape has no extent: a line between equal points, or circles with equal centres and
    /// radii.
    KRYS_NODISCARD bool IsZeroSize() const noexcept;
  };
}

/// @brief Gradient stops flattened for the shader.
namespace krys::gfx::detail
{
  /// @brief The most flattened stops a gradient has.
  constexpr uint32 MaxGradientStops = 256u;
  /// @brief How far flattened colours may be from the interpolated ones, per channel: a quarter of a step.
  constexpr float GradientTolerance = 0.25f / 255.f;

  /// @brief A flattened stop: the colour, premultiplied sRGB-encoded values from 0 to 1, is mixed linearly
  /// with the next stop's.
  struct FlatGradientStop
  {
    float Offset {0.f};
    Array<float, 4> Colour {};

    constexpr bool operator==(const FlatGradientStop &) const noexcept = default;
  };

  /// @brief Flattens the gradient's stops into stops that are linear in premultiplied encoded sRGB, within
  /// `GradientTolerance` of the gradient's own interpolation, and at most `MaxGradientStops` of them (the
  /// tolerance grows until they fit). Empty when the gradient has no stops.
  void FlattenGradientStops(const Gradient2D &gradient, List<FlatGradientStop> &stops) noexcept;

  /// @brief The flattened stops' colour at `offset`, padded past their ends; at a hard stop, the later one's.
  KRYS_NODISCARD Array<float, 4> EvaluateFlatStops(const List<FlatGradientStop> &stops,
                                                   float offset) noexcept;

  /// @brief The flattened stops' average colour over 0 to 1.
  KRYS_NODISCARD Array<float, 4> AverageFlatStops(const List<FlatGradientStop> &stops) noexcept;
}
