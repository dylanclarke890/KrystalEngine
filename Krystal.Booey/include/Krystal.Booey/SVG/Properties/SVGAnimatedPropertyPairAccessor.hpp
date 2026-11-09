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

#include "Krystal.Booey/SVG/Properties/SVGMemberAccessor.hpp"
#include "Krystal.Core/Threading/NeverDestroyed.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

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
