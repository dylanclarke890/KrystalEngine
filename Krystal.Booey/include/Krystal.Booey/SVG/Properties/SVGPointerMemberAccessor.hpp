#pragma once

#include "Krystal.Booey/SVG/Properties/SVGMemberAccessor.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo::svg
{
  /// @brief Reaches a property an owner holds as a member.
  template <typename TOwner, typename TProperty>
  class SVGPointerMemberAccessor : public SVGMemberAccessor<TOwner>
  {
    using Base = SVGMemberAccessor<TOwner>;

  protected:
    Ref<TProperty> TOwner::*_property;

  public:
    explicit SVGPointerMemberAccessor(Ref<TProperty> TOwner::*property) noexcept : _property(property)
    {
    }

    KRYS_NODISCARD Ref<TProperty> &Property(TOwner &owner) const noexcept
    {
      return owner.*_property;
    }

    KRYS_NODISCARD const Ref<TProperty> &Property(const TOwner &owner) const noexcept
    {
      return owner.*_property;
    }

    void Detach(const TOwner &owner) const noexcept override
    {
      Property(owner)->Detach();
    }

    KRYS_NODISCARD Maybe<dom::DOMString> Synchronize(const TOwner &owner) const noexcept override
    {
      return Property(owner)->Synchronize();
    }

  protected:
    template <typename TAccessor, Ref<TProperty> TOwner::*property>
    KRYS_NODISCARD static const SVGMemberAccessor<TOwner> &Singleton() noexcept
    {
      static NeverDestroyed<TAccessor> propertyAccessor {property};
      return propertyAccessor;
    }
  };
}
