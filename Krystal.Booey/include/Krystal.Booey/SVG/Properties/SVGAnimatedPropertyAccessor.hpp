#pragma once

#include "Krystal.Booey/SVG/Properties/SVGPointerMemberAccessor.hpp"

namespace krys::boo::svg
{
  /// @brief Reaches an animated property an owner holds as a member.
  template <typename TOwner, typename TAnimatedProperty>
  class SVGAnimatedPropertyAccessor : public SVGPointerMemberAccessor<TOwner, TAnimatedProperty>
  {
    using Base = SVGPointerMemberAccessor<TOwner, TAnimatedProperty>;

  public:
    using Base::Base;
    using Base::Property;
    using Base::Singleton;
    using AnimatedProperty = TAnimatedProperty;

    KRYS_NODISCARD bool Matches(const TOwner &owner,
                                const SVGAnimatedProperty &animatedProperty) const noexcept override
    {
      return Property(owner).get() == &animatedProperty;
    }

  private:
    KRYS_NODISCARD bool IsAnimatedProperty() const noexcept override
    {
      return true;
    }
  };
}
