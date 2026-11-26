/*
 * Copyright (C) 2018-2024 Apple Inc. All rights reserved.
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

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Web/Dom/Types/DOMString.hpp"
#include "Krystal.Web/Svg/Properties/SVGAnimatedProperty.hpp"
#include "Krystal.Web/Svg/Properties/SVGAttributeAnimator.hpp"

namespace krys::web
{
  class SVGProperty;

  /// @brief Reaches one of an owner's properties, for the owner's property registry.
  template <typename TOwner>
  class SVGMemberAccessor
  {
  public:
    virtual ~SVGMemberAccessor() noexcept = default;

    virtual void Detach(const TOwner &) const noexcept
    {
    }

    KRYS_NODISCARD virtual bool IsAnimatedProperty() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsAnimatedLength() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool Matches(const TOwner &, const SVGProperty &) const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool Matches(const TOwner &, const SVGAnimatedProperty &) const noexcept
    {
      return false;
    }

    virtual void SetDirty(const TOwner &, SVGAnimatedProperty &animatedProperty) const noexcept
    {
      animatedProperty.SetDirty();
    }

    KRYS_NODISCARD virtual Maybe<DOMString> Synchronize(const TOwner &) const noexcept
    {
      return {};
    }

    KRYS_NODISCARD virtual RefPtr<SVGAttributeAnimator> CreateAnimator(TOwner &, const QualifiedName &,
                                                                       svg::AnimationMode, svg::CalcMode,
                                                                       bool, bool) const noexcept
    {
      return nullptr;
    }

    virtual void AppendAnimatedInstance(TOwner &, SVGAttributeAnimator &) const noexcept
    {
    }

  protected:
    SVGMemberAccessor() noexcept = default;
  };
}
