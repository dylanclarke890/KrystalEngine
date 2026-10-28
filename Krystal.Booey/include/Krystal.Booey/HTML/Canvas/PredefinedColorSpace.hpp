#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class DestinationColorSpace;
}

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#predefinedcolorspace
  enum class PredefinedColorSpace : uint8
  {
    SRGB,
    DisplayP3
  };

  KRYS_NODISCARD DestinationColorSpace ToDestinationColorSpace(PredefinedColorSpace) noexcept;
  KRYS_NODISCARD Maybe<PredefinedColorSpace> ToPredefinedColorSpace(const DestinationColorSpace &) noexcept;
}
