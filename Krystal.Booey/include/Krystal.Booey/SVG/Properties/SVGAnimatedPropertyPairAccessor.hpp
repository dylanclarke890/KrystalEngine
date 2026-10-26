#pragma once

#include "Krystal.Booey/SVG/Properties/SVGMemberAccessor.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo::svg
{
  /// @brief Reaches two animated properties an owner reflects one attribute through.
  template <typename TOwner, typename TAccessor1, typename TAccessor2>
  class SVGAnimatedPropertyPairAccessor : public SVGMemberAccessor<TOwner>
  {
    using AnimatedPropertyType1 = typename TAccessor1::AnimatedProperty;
    using AnimatedPropertyType2 = typename TAccessor2::AnimatedProperty;
    using Base = SVGMemberAccessor<TOwner>;

  protected:
    TAccessor1 _accessor1;
    TAccessor2 _accessor2;

  public:
    SVGAnimatedPropertyPairAccessor(Ref<AnimatedPropertyType1> TOwner::*property1,
                                    Ref<AnimatedPropertyType2> TOwner::*property2) noexcept
        : _accessor1(property1), _accessor2(property2)
    {
    }

  protected:
    template <typename TAccessor, Ref<AnimatedPropertyType1> TOwner::*property1,
              Ref<AnimatedPropertyType2> TOwner::*property2>
    KRYS_NODISCARD static const SVGMemberAccessor<TOwner> &Singleton() noexcept
    {
      static NeverDestroyed<TAccessor> propertyAccessor {property1, property2};
      return propertyAccessor;
    }

    KRYS_NODISCARD bool IsAnimatedProperty() const noexcept override
    {
      return true;
    }

    KRYS_NODISCARD Ref<AnimatedPropertyType1> &Property1(TOwner &owner) const noexcept
    {
      return _accessor1.Property(owner);
    }

    KRYS_NODISCARD const Ref<AnimatedPropertyType1> &Property1(const TOwner &owner) const noexcept
    {
      return _accessor1.Property(owner);
    }

    KRYS_NODISCARD Ref<AnimatedPropertyType2> &Property2(TOwner &owner) const noexcept
    {
      return _accessor2.Property(owner);
    }

    KRYS_NODISCARD const Ref<AnimatedPropertyType2> &Property2(const TOwner &owner) const noexcept
    {
      return _accessor2.Property(owner);
    }

    void Detach(const TOwner &owner) const noexcept override
    {
      Property1(owner)->Detach();
      Property2(owner)->Detach();
    }

    KRYS_NODISCARD bool Matches(const TOwner &owner,
                                const SVGAnimatedProperty &animatedProperty) const noexcept override
    {
      return _accessor1.Matches(owner, animatedProperty) || _accessor2.Matches(owner, animatedProperty);
    }
  };
}
