#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI
{
  /// @brief Corresponds to CSS auto box sizes. Missing "min-content".
  /// https://www.w3.org/TR/css-sizing-3/#auto-box-sizes & https://www.w3.org/TR/css-flexbox-1/#min-size-auto
  enum class SizingMode : uint8
  {
    /// @brief The size a box would take if its outer size filled the available space in
    /// the given axis; in other words, the stretch fit into the available space,
    /// if that is definite. Undefined if the available space is indefinite.
    StretchFit,

    /// @brief A box's 'ideal' size in a given axis when given infinite available space.
    /// Usually this is the smallest size the box could take in that axis while
    /// still fitting around its contents, i.e. minimizing unfilled space while
    /// avoiding overflow.
    MaxContent,

    /// @brief If the available space in a given axis is definite, equal to
    /// clamp(min-content size, stretch-fit size, max-content size) (i.e.
    /// max(min-content size, min(max-content size, stretch-fit size))). When
    /// sizing under a min-content constraint, equal to the min-content size.
    /// Otherwise, equal to the max-content size in that axis.
    FitContent,
  };
}

namespace Krys
{
  template <>
  constexpr uint32 OrdinalCount<UI::SizingMode>()
  {
    return 3;
  }
}