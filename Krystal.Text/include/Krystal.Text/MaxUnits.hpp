#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"

namespace Krys
{
  /// @brief Gets the maximum number of code units needed to represent a full DecodeOne
  /// operation without running out of output space.
  template <typename TFrom>
  inline static constexpr std::size_t MaxDecodeCodePoints = MaxCodePoints<TFrom> * 2;

  /// @brief Gets the maximum number of code units needed to represent a full EncodeOne
  /// operation without running out of space.
  template <typename TFrom>
  inline static constexpr std::size_t MaxEncodeCodeUnits = MaxCodeUnits<TFrom> * 2;

  /// @brief Gets the maximum number of code units needed to represent a full TranscodeOne
  /// operation without running out of output space.
  template <typename TFrom, typename TTo>
  inline static constexpr std::size_t MaxRecodeCodePoints = MaxCodeUnits<TFrom> * MaxCodePoints<TTo> * 2;

  /// @brief Gets the maximum number of code units needed to represent a full TranscodeOne
  /// operation without running out of output space.
  template <typename TFrom, typename TTo>
  inline static constexpr std::size_t MaxTranscodeCodeUnits = MaxCodePoints<TFrom> * MaxCodeUnits<TTo> * 2;
}
