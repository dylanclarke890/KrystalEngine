#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  /// @brief Which of a filter primitive's subregion attributes are given.
  enum class FilterEffectGeometryFlag : uint8
  {
    HasX,
    HasY,
    HasWidth,
    HasHeight
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::FilterEffectGeometryFlag, 4uz);

namespace krys::boo
{
  /// @brief A filter primitive's subregion (its `x`, `y`, `width` and `height`), the parts that are given.
  class FilterEffectGeometry
  {
  public:
    using Flags = FilterEffectGeometryFlag;

  private:
    FloatRect _boundaries;
    EnumSet<Flags> _flags;

  public:
    FilterEffectGeometry() noexcept = default;
    FilterEffectGeometry(const FloatRect &boundaries, EnumSet<Flags> flags) noexcept
        : _boundaries(boundaries), _flags(flags)
    {
    }

    KRYS_NODISCARD Maybe<float> X() const noexcept
    {
      return _flags.Contains(Flags::HasX) ? Maybe<float> {_boundaries.X()} : null;
    }
    KRYS_NODISCARD Maybe<float> Y() const noexcept
    {
      return _flags.Contains(Flags::HasY) ? Maybe<float> {_boundaries.Y()} : null;
    }
    KRYS_NODISCARD Maybe<float> Width() const noexcept
    {
      return _flags.Contains(Flags::HasWidth) ? Maybe<float> {_boundaries.Width()} : null;
    }
    KRYS_NODISCARD Maybe<float> Height() const noexcept
    {
      return _flags.Contains(Flags::HasHeight) ? Maybe<float> {_boundaries.Height()} : null;
    }
  };
}
