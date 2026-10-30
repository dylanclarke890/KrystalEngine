#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"

namespace krys::boo
{
  class RenderStyle;
  enum class CompositeOperation : uint8;
}

namespace krys::boo::style::interpolation
{
  struct Context;

  /// @brief How one property is compared and interpolated between two styles.
  class WrapperBase
  {
    KRYS_NON_COPYABLE(WrapperBase);

  private:
    css::PropertyId _property;

  public:
    explicit WrapperBase(css::PropertyId property) noexcept : _property(property)
    {
    }
    virtual ~WrapperBase() noexcept = default;

    KRYS_NODISCARD css::PropertyId Property() const noexcept
    {
      return _property;
    }

    KRYS_NODISCARD virtual bool Equals(const RenderStyle &, const RenderStyle &) const noexcept = 0;
    KRYS_NODISCARD virtual bool CanInterpolate(const RenderStyle &, const RenderStyle &,
                                               CompositeOperation) const noexcept
    {
      return true;
    }
    KRYS_NODISCARD virtual bool
      RequiresInterpolationForAccumulativeIteration(const RenderStyle &, const RenderStyle &) const noexcept
    {
      return false;
    }
    virtual void Interpolate(RenderStyle &, const RenderStyle &, const RenderStyle &,
                             const Context &) const noexcept = 0;
  };
}
