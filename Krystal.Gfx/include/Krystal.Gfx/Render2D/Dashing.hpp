#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Gfx/Render2D/StrokeStyle.hpp"
#include <cmath>

/// @brief Dash patterns, shared by the stroker and straight lines.
namespace krys::gfx::detail
{
  /// @brief Beyond this many dashes, a contour is stroked solid instead.
  constexpr uint64 MaxDashes = 100'000u;

  /// @brief The style's pattern, repeated to an even length since dashes and gaps alternate; empty when the
  /// style strokes solid (no pattern, a negative or non-finite length, or nothing but zeros).
  inline void PrepareDashes(const StrokeStyle &style, List<float> &pattern) noexcept
  {
    pattern.clear();
    float length = 0.f;
    for (const auto dash : style.Dashes)
    {
      if (!(dash >= 0.f) || !std::isfinite(dash))
      {
        return;
      }
      length += dash;
    }
    if (!(length > 0.f))
    {
      return;
    }
    pattern.assign(style.Dashes.begin(), style.Dashes.end());
    if (pattern.size() % 2u == 1u)
    {
      pattern.insert(pattern.end(), style.Dashes.begin(), style.Dashes.end());
    }
  }

  KRYS_NODISCARD inline float PatternLength(Span<const float> pattern) noexcept
  {
    float length = 0.f;
    for (const auto dash : pattern)
    {
      length += dash;
    }
    return length;
  }

  /// @brief Where a contour starts in the pattern.
  struct DashPosition
  {
    size_t Index {0u};
    /// @brief How much of the current dash or gap is left.
    float Remaining {0.f};
    /// @brief Whether the current entry is a dash.
    bool On {true};

    /// @brief Moves on to the next dash or gap.
    void Next(Span<const float> pattern) noexcept
    {
      Index = (Index + 1u) % pattern.size();
      Remaining = pattern[Index];
      On = !On;
    }
  };

  /// @brief Where `offset` (either way, wrapping round the pattern) starts a contour in `pattern`.
  KRYS_NODISCARD inline DashPosition StartDashes(Span<const float> pattern, float offset) noexcept
  {
    const float period = PatternLength(pattern);
    float phase = std::fmod(offset, period);
    if (!std::isfinite(phase))
    {
      phase = 0.f;
    }
    if (phase < 0.f)
    {
      phase += period;
    }

    size_t index = 0u;
    while (phase > 0.f && phase >= pattern[index])
    {
      phase -= pattern[index];
      index = (index + 1u) % pattern.size();
    }
    return {.Index = index, .Remaining = pattern[index] - phase, .On = index % 2u == 0u};
  }
}
