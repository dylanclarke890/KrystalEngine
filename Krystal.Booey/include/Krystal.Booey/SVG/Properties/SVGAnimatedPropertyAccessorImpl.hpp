/*
 * Copyright (C) 2018-2019 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyAccessor.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyAnimatorImpl.hpp"
#include "Krystal.Booey/SVG/Properties/SVGAnimatedPropertyImpl.hpp"

#define KRYS_SVG_ANIMATED_PROPERTY_ACCESSOR(AccessorName, AnimatedPropertyType, AnimatorType)                \
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
                                                                                                             \
  private:                                                                                                   \
    KRYS_NODISCARD RefPtr<SVGAttributeAnimator>                                                              \
      CreateAnimator(TOwner &owner, const dom::QualifiedName &attributeName, AnimationMode animationMode,    \
                     CalcMode calcMode, bool isAccumulated, bool isAdditive) const noexcept final            \
    {                                                                                                        \
      return AnimatorType::Create(attributeName, Base::Property(owner), animationMode, calcMode,             \
                                  isAccumulated, isAdditive);                                                \
    }                                                                                                        \
                                                                                                             \
    void AppendAnimatedInstance(TOwner &owner, SVGAttributeAnimator &animator) const noexcept final          \
    {                                                                                                        \
      static_cast<AnimatorType &>(animator).AppendAnimatedInstance(Base::Property(owner));                   \
    }                                                                                                        \
  };

namespace krys::boo::svg
{
  KRYS_SVG_ANIMATED_PROPERTY_ACCESSOR(SVGAnimatedAngleAccessor, SVGAnimatedAngle, SVGAnimatedAngleAnimator)
  KRYS_SVG_ANIMATED_PROPERTY_ACCESSOR(SVGAnimatedBooleanAccessor, SVGAnimatedBoolean,
                                      SVGAnimatedBooleanAnimator)
  KRYS_SVG_ANIMATED_PROPERTY_ACCESSOR(SVGAnimatedIntegerAccessor, SVGAnimatedInteger,
                                      SVGAnimatedIntegerAnimator)
  KRYS_SVG_ANIMATED_PROPERTY_ACCESSOR(SVGAnimatedNumberAccessor, SVGAnimatedNumber, SVGAnimatedNumberAnimator)
  KRYS_SVG_ANIMATED_PROPERTY_ACCESSOR(SVGAnimatedNumberListAccessor, SVGAnimatedNumberList,
                                      SVGAnimatedNumberListAnimator)
  KRYS_SVG_ANIMATED_PROPERTY_ACCESSOR(SVGAnimatedPointListAccessor, SVGAnimatedPointList,
                                      SVGAnimatedPointListAnimator)
  KRYS_SVG_ANIMATED_PROPERTY_ACCESSOR(SVGAnimatedOrientTypeAccessor, SVGAnimatedOrientType,
                                      SVGAnimatedOrientTypeAnimator)
  KRYS_SVG_ANIMATED_PROPERTY_ACCESSOR(SVGAnimatedPreserveAspectRatioAccessor, SVGAnimatedPreserveAspectRatio,
                                      SVGAnimatedPreserveAspectRatioAnimator)
  KRYS_SVG_ANIMATED_PROPERTY_ACCESSOR(SVGAnimatedRectAccessor, SVGAnimatedRect, SVGAnimatedRectAnimator)
  KRYS_SVG_ANIMATED_PROPERTY_ACCESSOR(SVGAnimatedStringAccessor, SVGAnimatedString, SVGAnimatedStringAnimator)
  KRYS_SVG_ANIMATED_PROPERTY_ACCESSOR(SVGAnimatedTransformListAccessor, SVGAnimatedTransformList,
                                      SVGAnimatedTransformListAnimator)

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

    KRYS_NODISCARD RefPtr<SVGAttributeAnimator>
      CreateAnimator(TOwner &owner, const dom::QualifiedName &attributeName, AnimationMode animationMode,
                     CalcMode calcMode, bool isAccumulated, bool isAdditive) const noexcept final
    {
      SVGLengthMode lengthMode = Base::Property(owner)->BaseVal()->Value().LengthMode();
      return SVGAnimatedLengthAnimator::Create(attributeName, Base::Property(owner), animationMode, calcMode,
                                               isAccumulated, isAdditive, lengthMode);
    }

    void AppendAnimatedInstance(TOwner &owner, SVGAttributeAnimator &animator) const noexcept final
    {
      static_cast<SVGAnimatedLengthAnimator &>(animator).AppendAnimatedInstance(Base::Property(owner));
    }
  };

  template <typename TOwner>
  class SVGAnimatedLengthListAccessor final
      : public SVGAnimatedPropertyAccessor<TOwner, SVGAnimatedLengthList>
  {
    using Base = SVGAnimatedPropertyAccessor<TOwner, SVGAnimatedLengthList>;

  public:
    using Base::Base;

    template <Ref<SVGAnimatedLengthList> TOwner::*property>
    KRYS_NODISCARD static const SVGMemberAccessor<TOwner> &Singleton() noexcept
    {
      return Base::template Singleton<SVGAnimatedLengthListAccessor, property>();
    }

  private:
    KRYS_NODISCARD RefPtr<SVGAttributeAnimator>
      CreateAnimator(TOwner &owner, const dom::QualifiedName &attributeName, AnimationMode animationMode,
                     CalcMode calcMode, bool isAccumulated, bool isAdditive) const noexcept final
    {
      return SVGAnimatedLengthListAnimator::Create(attributeName, Base::Property(owner), animationMode,
                                                   calcMode, isAccumulated, isAdditive, SVGLengthMode::Width);
    }

    void AppendAnimatedInstance(TOwner &owner, SVGAttributeAnimator &animator) const noexcept final
    {
      static_cast<SVGAnimatedLengthListAnimator &>(animator).AppendAnimatedInstance(Base::Property(owner));
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

  private:
    KRYS_NODISCARD RefPtr<SVGAttributeAnimator>
      CreateAnimator(TOwner &owner, const dom::QualifiedName &attributeName, AnimationMode animationMode,
                     CalcMode calcMode, bool isAccumulated, bool isAdditive) const noexcept final
    {
      return SVGAnimatedEnumerationAnimator<TEnum>::Create(
        attributeName, Base::Property(owner), animationMode, calcMode, isAccumulated, isAdditive);
    }

    void AppendAnimatedInstance(TOwner &owner, SVGAttributeAnimator &animator) const noexcept final
    {
      static_cast<SVGAnimatedEnumerationAnimator<TEnum> &>(animator).AppendAnimatedInstance(
        Base::Property(owner));
    }
  };

  KRYS_SVG_ANIMATED_PROPERTY_ACCESSOR(SVGAnimatedPathSegListAccessor, SVGAnimatedPathSegList,
                                      SVGAnimatedPathSegListAnimator)
}

#undef KRYS_SVG_ANIMATED_PROPERTY_ACCESSOR
