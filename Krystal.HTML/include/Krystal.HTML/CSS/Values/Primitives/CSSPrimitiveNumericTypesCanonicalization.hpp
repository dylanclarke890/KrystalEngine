#pragma once

#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace Krys::HTML
{
#pragma region Angle

  KRYS_NODISCARD double CanonicalizeAngle(double value, AngleUnit unit) noexcept
  {
    return ConvertAngle<AngleUnit::Deg>(value, unit);
  }

  template <auto R, typename V>
  KRYS_NODISCARD double Canonicalize(AngleRaw<R, V> raw)
  {
    return CanonicalizeAngle(raw.Value, raw.Unit);
  }

#pragma endregion

#pragma region Time

  KRYS_NODISCARD double CanonicalizeTime(double value, TimeUnit unit) noexcept
  {
    return ConvertTime<TimeUnit::S>(value, unit);
  }

  template <auto R, typename V>
  KRYS_NODISCARD double Canonicalize(TimeRaw<R, V> raw)
  {
    return CanonicalizeTime(raw.Value, raw.Unit);
  }

#pragma endregion

#pragma region Frequency

  KRYS_NODISCARD double CanonicalizeFrequency(double value, FrequencyUnit unit) noexcept
  {
    return ConvertFrequency<FrequencyUnit::Hz>(value, unit);
  }

  template <auto R, typename V>
  KRYS_NODISCARD double Canonicalize(FrequencyRaw<R, V> raw)
  {
    return CanonicalizeFrequency(raw.Value, raw.Unit);
  }

#pragma endregion

#pragma region Resolution

  KRYS_NODISCARD double CanonicalizeResolution(double value, ResolutionUnit unit) noexcept
  {
    return ConvertResolution<ResolutionUnit::Dpi>(value, unit);
  }

  template <auto R, typename V>
  KRYS_NODISCARD double Canonicalize(ResolutionRaw<R, V> raw)
  {
    return CanonicalizeResolution(raw.Value, raw.Unit);
  }

#pragma endregion
}