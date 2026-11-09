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
