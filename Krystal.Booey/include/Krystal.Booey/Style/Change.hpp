#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys::boo
{
  class RenderStyle;
}

namespace krys::boo::style
{
  /// @brief How an element's style changed, and so what has to be updated.
  enum class Change : uint8
  {
    NonInherited,
    Inherited,
    FastPathInherited,
    Container,
    Renderer
  };

  KRYS_NODISCARD constexpr EnumSet<Change> InheritedChanges() noexcept
  {
    return {Change::Inherited, Change::FastPathInherited};
  }

  /// @brief The changes from `s1` to `s2`.
  KRYS_NODISCARD EnumSet<Change> DetermineChanges(const RenderStyle &s1, const RenderStyle &s2) noexcept;
}
