#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys::boo
{
  /// @brief How a filter is applied: in software on image buffers, by an accelerated backend, or by the
  /// graphics context, as styles on transparency layers.
  enum class FilterRenderingMode : uint8
  {
    Software,
    Accelerated,
    GraphicsContext
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::FilterRenderingMode, 3uz);

namespace krys::boo
{
  inline constexpr EnumSet<FilterRenderingMode> AllFilterRenderingModes = {
    FilterRenderingMode::Software, FilterRenderingMode::Accelerated, FilterRenderingMode::GraphicsContext};
}
