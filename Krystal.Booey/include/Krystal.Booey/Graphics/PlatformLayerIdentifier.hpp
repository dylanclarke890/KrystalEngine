#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  /// @brief Identifies a platform layer.
  struct PlatformLayerIdentifier
  {
    uint64 Value {0};

    KRYS_NODISCARD friend constexpr bool operator==(PlatformLayerIdentifier,
                                                    PlatformLayerIdentifier) noexcept = default;
  };

  TextStream &operator<<(TextStream &ts, PlatformLayerIdentifier identifier) noexcept;
}
