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
