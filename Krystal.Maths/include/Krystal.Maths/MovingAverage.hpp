#pragma once

#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Core/Compiler.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::Maths
{
  template <Number T, size_t MaxSamples>
  class MovingAverage
  {
    using Type = T;

    Array<T, MaxSamples> _samples;
    size_t _currentSample, _sampleCount;
    T _sum;

  public:
    constexpr MovingAverage() noexcept
        : _sum(static_cast<T>(0)), _samples({}), _currentSample(0), _sampleCount(0)
    {
    }

    constexpr void Add(T sample) noexcept
    {
      if (_sampleCount == MaxSamples)
        _sum -= _samples[_currentSample];
      else
        _sampleCount++;

      _samples[_currentSample] = sample;
      _sum += sample;

      _currentSample = (_currentSample + 1) % MaxSamples;
    }

    KRYS_NODISCARD constexpr T GetAverage() const noexcept
    {
      if (_sampleCount == 0)
        return static_cast<T>(0);
      return _sum / static_cast<T>(_sampleCount);
    }
  };
}
