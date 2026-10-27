#pragma once

#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyAccessor.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyImpl.hpp"

// FIXME: Each accessor makes the animator of its property type (createAnimator) and adds the instances of a
// <use> element to it (appendAnimatedInstance), with SMIL (B11i).

#define KRYS_SVG_ANIMATED_PROPERTY_ACCESSOR(AccessorName, AnimatedPropertyType)                              \
  template <typename TOwner>                                                                                 \
  class AccessorName final : public SVGAnimatedPropertyAccessor<TOwner, AnimatedPropertyType>                \
  {                                                                                                          \
    using Base = SVGAnimatedPropertyAccessor<TOwner, AnimatedPropertyType>;                                  \
                                                                                                             \
  public:                                                                                                    \
    using Base::Base;                                                                                        \
                                                                                                             \
    template <Ref<AnimatedPropertyType> TOwner::*property>                                                   \
    KRYS_NODISCARD static const SVGMemberAccessor<TOwner> &Singleton() noexcept                              \
    {                                                                                                        \
      return Base::template Singleton<AccessorName, property>();                                             \
    }                                                                                                        \
  };

namespace krys::boo::svg
{
  KRYS_SVG_ANIMATED_PROPERTY_ACCESSOR(SVGAnimatedAngleAccessor, SVGAnimatedAngle)
  KRYS_SVG_ANIMATED_PROPERTY_ACCESSOR(SVGAnimatedBooleanAccessor, SVGAnimatedBoolean)
  KRYS_SVG_ANIMATED_PROPERTY_ACCESSOR(SVGAnimatedIntegerAccessor, SVGAnimatedInteger)
  KRYS_SVG_ANIMATED_PROPERTY_ACCESSOR(SVGAnimatedLengthListAccessor, SVGAnimatedLengthList)
  KRYS_SVG_ANIMATED_PROPERTY_ACCESSOR(SVGAnimatedNumberAccessor, SVGAnimatedNumber)
  KRYS_SVG_ANIMATED_PROPERTY_ACCESSOR(SVGAnimatedNumberListAccessor, SVGAnimatedNumberList)
  KRYS_SVG_ANIMATED_PROPERTY_ACCESSOR(SVGAnimatedPointListAccessor, SVGAnimatedPointList)
  KRYS_SVG_ANIMATED_PROPERTY_ACCESSOR(SVGAnimatedOrientTypeAccessor, SVGAnimatedOrientType)
  KRYS_SVG_ANIMATED_PROPERTY_ACCESSOR(SVGAnimatedPreserveAspectRatioAccessor, SVGAnimatedPreserveAspectRatio)
  KRYS_SVG_ANIMATED_PROPERTY_ACCESSOR(SVGAnimatedRectAccessor, SVGAnimatedRect)
  KRYS_SVG_ANIMATED_PROPERTY_ACCESSOR(SVGAnimatedStringAccessor, SVGAnimatedString)
  KRYS_SVG_ANIMATED_PROPERTY_ACCESSOR(SVGAnimatedTransformListAccessor, SVGAnimatedTransformList)

  template <typename TOwner>
  class SVGAnimatedLengthAccessor final : public SVGAnimatedPropertyAccessor<TOwner, SVGAnimatedLength>
  {
    using Base = SVGAnimatedPropertyAccessor<TOwner, SVGAnimatedLength>;

  public:
    using Base::Base;

    template <Ref<SVGAnimatedLength> TOwner::*property>
    KRYS_NODISCARD static const SVGMemberAccessor<TOwner> &Singleton() noexcept
    {
      return Base::template Singleton<SVGAnimatedLengthAccessor, property>();
    }

  private:
    KRYS_NODISCARD bool IsAnimatedLength() const noexcept override
    {
      return true;
    }
  };

  /// @brief Reaches an animated enumeration; @p TEnum is the enumeration's type, which its animator needs.
  template <typename TOwner, typename TEnum>
  class SVGAnimatedEnumerationAccessor final
      : public SVGAnimatedPropertyAccessor<TOwner, SVGAnimatedEnumeration>
  {
    using Base = SVGAnimatedPropertyAccessor<TOwner, SVGAnimatedEnumeration>;

  public:
    using Base::Base;

    template <Ref<SVGAnimatedEnumeration> TOwner::*property>
    KRYS_NODISCARD static const SVGMemberAccessor<TOwner> &Singleton() noexcept
    {
      return Base::template Singleton<SVGAnimatedEnumerationAccessor, property>();
    }
  };

  // FIXME: An accessor for <path>'s `d` as an animated property (SVGAnimatedPathSegListAccessor, without the
  // segment list SVG 2 removed), with SMIL (B11i).
}

#undef KRYS_SVG_ANIMATED_PROPERTY_ACCESSOR
