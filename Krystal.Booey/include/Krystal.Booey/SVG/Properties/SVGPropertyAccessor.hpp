#pragma once

#include "Krystal.Booey/SVG/Properties/SVGPointerMemberAccessor.hpp"

namespace krys::boo::svg
{
  /// @brief Reaches a property that isn't animated (an `SVGProperty`) an owner holds as a member.
  template <typename TOwner, typename TProperty>
  class SVGPropertyAccessor : public SVGPointerMemberAccessor<TOwner, TProperty>
  {
    using Base = SVGPointerMemberAccessor<TOwner, TProperty>;

  public:
    using Base::Base;

  protected:
    using Base::Property;
    using Base::Singleton;

    KRYS_NODISCARD bool Matches(const TOwner &owner, const SVGProperty &property) const noexcept override
    {
      return Property(owner).get() == &property;
    }
  };
}
