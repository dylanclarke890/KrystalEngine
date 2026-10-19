#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  /// @brief Identifies a rendering resource (an image buffer) uniquely while the process runs, so backends
  /// can cache what they make from it.
  /// @note Upstream's identifiers are process qualified, for the GPU process; there's one process here.
  class RenderingResourceIdentifier
  {
  private:
    uint64 _value {0};

    explicit constexpr RenderingResourceIdentifier(uint64 value) noexcept : _value(value)
    {
    }

  public:
    constexpr RenderingResourceIdentifier() noexcept = default;

    KRYS_NODISCARD static RenderingResourceIdentifier Generate() noexcept;

    KRYS_NODISCARD constexpr uint64 ToUInt64() const noexcept
    {
      return _value;
    }

    KRYS_NODISCARD constexpr bool operator==(const RenderingResourceIdentifier &) const noexcept = default;
  };

  TextStream &operator<<(TextStream &, RenderingResourceIdentifier) noexcept;
}
