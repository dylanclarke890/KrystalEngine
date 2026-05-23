#pragma once

#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Text/CodePoint.hpp"
#include "Krystal.Text/CodeUnit.hpp"

namespace Krys::Text
{
  /// @brief Gets the maximum number of code units needed to represent a full DecodeOne
  /// operation without running out of output space.
  template <typename TFrom>
  constexpr inline static std::size_t MaxDecodeCodePoints =
    ::Krys::Text::MaxCodePoints<remove_cvref_t<TFrom>> * 2;

  /// @brief Gets the maximum number of code units needed to represent a full EncodeOne
  /// operation without running out of space.
  template <typename TFrom>
  constexpr inline static std::size_t MaxEncodeCodeUnits =
    ::Krys::Text::MaxCodeUnits<remove_cvref_t<TFrom>> * 2;

  /// @brief Gets the maximum number of code points needed to represent a full TranscodeOne
  /// operation without running out of output space.
  template <typename TFrom, typename TTo>
  constexpr inline static std::size_t MaxRecodeCodePoints =
    ::Krys::Text::MaxCodeUnits<remove_cvref_t<TFrom>> * ::Krys::Text::MaxCodePoints<remove_cvref_t<TTo>> * 2;

  /// @brief Gets the maximum number of code units needed to represent a full TranscodeOne
  /// operation without running out of output space.
  template <typename TFrom, typename TTo>
  constexpr inline static std::size_t MaxTranscodeCodeUnits =
    ::Krys::Text::MaxCodePoints<remove_cvref_t<TFrom>> * ::Krys::Text::MaxCodeUnits<remove_cvref_t<TTo>> * 2;
}
