#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include <string_view>

namespace krys::boo
{
  /// @brief A CSS named colour and its packed ARGB value.
  struct NamedColor
  {
    std::u8string_view Name;
    uint32 ARGBValue;
  };

  /// @brief Looks up a named colour by its lowercase name.
  /// @return The colour, or nullptr if there's no colour with that name.
  KRYS_NODISCARD const NamedColor *FindColor(std::u8string_view name) noexcept;
}
