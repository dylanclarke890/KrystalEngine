/*
 * Copyright (C) 2007-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2012, 2013 Adobe Systems Incorporated. All rights reserved.
 * Copyright (C) 2025 Sam Weinig. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"

namespace krys::boo
{
  class RenderStyle;
  enum class CompositeOperation : uint8;
}

namespace krys::boo::style::interpolation
{
  struct Context;

  /// @brief How one property is compared and interpolated between two styles.
  class WrapperBase
  {
    KRYS_NON_COPYABLE(WrapperBase);

  private:
    css::PropertyId _property;

  public:
    explicit WrapperBase(css::PropertyId property) noexcept : _property(property)
    {
    }
    virtual ~WrapperBase() noexcept = default;

    KRYS_NODISCARD css::PropertyId Property() const noexcept
    {
      return _property;
    }

    KRYS_NODISCARD virtual bool Equals(const RenderStyle &, const RenderStyle &) const noexcept = 0;
    KRYS_NODISCARD virtual bool CanInterpolate(const RenderStyle &, const RenderStyle &,
                                               CompositeOperation) const noexcept
    {
      return true;
    }
    KRYS_NODISCARD virtual bool
      RequiresInterpolationForAccumulativeIteration(const RenderStyle &, const RenderStyle &) const noexcept
    {
      return false;
    }
    virtual void Interpolate(RenderStyle &, const RenderStyle &, const RenderStyle &,
                             const Context &) const noexcept = 0;
  };
}
