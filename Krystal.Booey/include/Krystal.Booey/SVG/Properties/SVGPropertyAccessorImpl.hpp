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

#include "Krystal.Booey/SVG/Properties/SVGPropertyAccessor.hpp"
#include "Krystal.Booey/SVG/SVGStringList.hpp"
#include "Krystal.Booey/SVG/SVGTests.hpp"
#include "Krystal.Booey/SVG/SVGTransformList.hpp"

namespace krys::boo::svg
{
  /// @brief Reaches one of an element's conditional processing attributes, which it keeps in its rare data.
  template <typename TOwner>
  class SVGConditionalProcessingAttributeAccessor final : public SVGMemberAccessor<TOwner>
  {
    using Base = SVGMemberAccessor<TOwner>;

  private:
    Ref<SVGStringList> SVGConditionalProcessingAttributes::*_property;

  public:
    explicit SVGConditionalProcessingAttributeAccessor(
      Ref<SVGStringList> SVGConditionalProcessingAttributes::*property) noexcept
        : _property(property)
    {
    }

    KRYS_NODISCARD Ref<SVGStringList> &Property(TOwner &owner) const noexcept
    {
      return owner.ConditionalProcessingAttributes().*_property;
    }

    KRYS_NODISCARD const Ref<SVGStringList> &Property(const TOwner &owner) const noexcept
    {
      return const_cast<TOwner &>(owner).ConditionalProcessingAttributes().*_property;
    }

    void Detach(const TOwner &owner) const noexcept override
    {
      Property(owner)->Detach();
    }

    KRYS_NODISCARD Maybe<dom::DOMString> Synchronize(const TOwner &owner) const noexcept override
    {
      return Property(owner)->Synchronize();
    }

    KRYS_NODISCARD bool Matches(const TOwner &owner, const SVGProperty &property) const noexcept override
    {
      return Property(owner).get() == &property;
    }

    template <Ref<SVGStringList> SVGConditionalProcessingAttributes::*property>
    KRYS_NODISCARD static const SVGMemberAccessor<TOwner> &Singleton() noexcept
    {
      static NeverDestroyed<SVGConditionalProcessingAttributeAccessor<TOwner>> propertyAccessor {property};
      return propertyAccessor;
    }
  };

  template <typename TOwner>
  class SVGTransformListAccessor final : public SVGPropertyAccessor<TOwner, SVGTransformList>
  {
    using Base = SVGPropertyAccessor<TOwner, SVGTransformList>;

  public:
    using Base::Base;

    template <Ref<SVGTransformList> TOwner::*property>
    KRYS_NODISCARD static const SVGMemberAccessor<TOwner> &Singleton() noexcept
    {
      return Base::template Singleton<SVGTransformListAccessor, property>();
    }
  };
}
