#pragma once

#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyAccessorImpl.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyPairAccessor.hpp"
#include "Krystal.Booey/SVG/SVGMarkerTypes.hpp"

// FIXME: Each accessor makes the animator of its pair (createAnimator) and adds the instances of a <use>
// element to it (appendAnimatedInstance), with SMIL (B11i).

namespace krys::boo::svg
{
  /// @brief Reaches a marker's `orient`, reflected by an angle and an orient type.
  template <typename TOwner>
  class SVGAnimatedAngleOrientAccessor final
      : public SVGAnimatedPropertyPairAccessor<TOwner, SVGAnimatedAngleAccessor<TOwner>,
                                               SVGAnimatedOrientTypeAccessor<TOwner>>
  {
    using Base = SVGAnimatedPropertyPairAccessor<TOwner, SVGAnimatedAngleAccessor<TOwner>,
                                                 SVGAnimatedOrientTypeAccessor<TOwner>>;
    using Base::_accessor1;
    using Base::_accessor2;
    using Base::Property1;
    using Base::Property2;

  public:
    using Base::Base;

    template <Ref<SVGAnimatedAngle> TOwner::*property1, Ref<SVGAnimatedOrientType> TOwner::*property2>
    KRYS_NODISCARD static const SVGMemberAccessor<TOwner> &Singleton() noexcept
    {
      return Base::template Singleton<SVGAnimatedAngleOrientAccessor, property1, property2>();
    }

  private:
    void SetDirty(const TOwner &owner, SVGAnimatedProperty &animatedProperty) const noexcept final
    {
      auto type = Property2(owner)->BaseVal();
      if (_accessor1.Matches(owner, animatedProperty) && type != SVGMarkerOrientAngle)
      {
        Property2(owner)->SetBaseValInternal(SVGMarkerOrientAngle);
      }
      else if (_accessor2.Matches(owner, animatedProperty) && type != SVGMarkerOrientAngle)
      {
        Property1(owner)->SetBaseValInternal({});
      }
      animatedProperty.SetDirty();
    }

    KRYS_NODISCARD Maybe<dom::DOMString> Synchronize(const TOwner &owner) const noexcept final
    {
      bool dirty1 = Property1(owner)->IsDirty();
      bool dirty2 = Property2(owner)->IsDirty();
      if (!(dirty1 || dirty2))
      {
        return {};
      }

      auto type = Property2(owner)->BaseVal();

      dom::DOMString string1 =
        dirty1 ? *Property1(owner)->Synchronize() : Property1(owner)->BaseValAsString();
      dom::DOMString string2 =
        dirty2 ? *Property2(owner)->Synchronize() : Property2(owner)->BaseValAsString();
      return type == SVGMarkerOrientAuto || type == SVGMarkerOrientAutoStartReverse ? string2 : string1;
    }
  };

  /// @brief Reaches a pair of animated integers reflecting one attribute (`order`).
  template <typename TOwner>
  class SVGAnimatedIntegerPairAccessor final
      : public SVGAnimatedPropertyPairAccessor<TOwner, SVGAnimatedIntegerAccessor<TOwner>,
                                               SVGAnimatedIntegerAccessor<TOwner>>
  {
    using Base = SVGAnimatedPropertyPairAccessor<TOwner, SVGAnimatedIntegerAccessor<TOwner>,
                                                 SVGAnimatedIntegerAccessor<TOwner>>;
    using Base::Property1;
    using Base::Property2;

  public:
    using Base::Base;

    template <Ref<SVGAnimatedInteger> TOwner::*property1, Ref<SVGAnimatedInteger> TOwner::*property2>
    KRYS_NODISCARD static const SVGMemberAccessor<TOwner> &Singleton() noexcept
    {
      return Base::template Singleton<SVGAnimatedIntegerPairAccessor, property1, property2>();
    }

  private:
    KRYS_NODISCARD Maybe<dom::DOMString> Synchronize(const TOwner &owner) const noexcept final
    {
      bool dirty1 = Property1(owner)->IsDirty();
      bool dirty2 = Property2(owner)->IsDirty();
      if (!(dirty1 || dirty2))
      {
        return {};
      }

      dom::DOMString string1 =
        dirty1 ? *Property1(owner)->Synchronize() : Property1(owner)->BaseValAsString();
      dom::DOMString string2 =
        dirty2 ? *Property2(owner)->Synchronize() : Property2(owner)->BaseValAsString();
      return string1 == string2 ? string1 : string1 + u8", " + string2;
    }
  };

  /// @brief Reaches a pair of animated numbers reflecting one attribute (`stdDeviation`, `radius`, ...).
  template <typename TOwner>
  class SVGAnimatedNumberPairAccessor final
      : public SVGAnimatedPropertyPairAccessor<TOwner, SVGAnimatedNumberAccessor<TOwner>,
                                               SVGAnimatedNumberAccessor<TOwner>>
  {
    using Base = SVGAnimatedPropertyPairAccessor<TOwner, SVGAnimatedNumberAccessor<TOwner>,
                                                 SVGAnimatedNumberAccessor<TOwner>>;
    using Base::Property1;
    using Base::Property2;

  public:
    using Base::Base;

    template <Ref<SVGAnimatedNumber> TOwner::*property1, Ref<SVGAnimatedNumber> TOwner::*property2>
    KRYS_NODISCARD static const SVGMemberAccessor<TOwner> &Singleton() noexcept
    {
      return Base::template Singleton<SVGAnimatedNumberPairAccessor, property1, property2>();
    }

  private:
    KRYS_NODISCARD Maybe<dom::DOMString> Synchronize(const TOwner &owner) const noexcept final
    {
      bool dirty1 = Property1(owner)->IsDirty();
      bool dirty2 = Property2(owner)->IsDirty();
      if (!(dirty1 || dirty2))
      {
        return {};
      }

      dom::DOMString string1 =
        dirty1 ? *Property1(owner)->Synchronize() : Property1(owner)->BaseValAsString();
      dom::DOMString string2 =
        dirty2 ? *Property2(owner)->Synchronize() : Property2(owner)->BaseValAsString();
      return string1 == string2 ? string1 : string1 + u8", " + string2;
    }
  };
}
