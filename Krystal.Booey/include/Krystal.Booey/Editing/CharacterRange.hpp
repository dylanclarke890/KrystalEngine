#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  /// @brief A range of the characters a text iterator emits: bytes of its UTF-8 text.
  struct CharacterRange
  {
    uint64 Location {0};
    uint64 Length {0};

    bool operator==(const CharacterRange &) const = default;
  };
}
