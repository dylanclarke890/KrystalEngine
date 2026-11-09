/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  /// @brief Which of a filter primitive's subregion attributes are given.
  enum class FilterEffectGeometryFlag : uint8
  {
    HasX,
    HasY,
    HasWidth,
    HasHeight
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::FilterEffectGeometryFlag, 4uz);

namespace krys::boo
{
  /// @brief A filter primitive's subregion (its `x`, `y`, `width` and `height`), the parts that are given.
  class FilterEffectGeometry
  {
  public:
    using Flags = FilterEffectGeometryFlag;

  private:
    FloatRect _boundaries;
    EnumSet<Flags> _flags;

  public:
    FilterEffectGeometry() noexcept = default;
    FilterEffectGeometry(const FloatRect &boundaries, EnumSet<Flags> flags) noexcept
        : _boundaries(boundaries), _flags(flags)
    {
    }

    KRYS_NODISCARD Maybe<float> X() const noexcept
    {
      return _flags.Contains(Flags::HasX) ? Maybe<float> {_boundaries.X()} : null;
    }
    KRYS_NODISCARD Maybe<float> Y() const noexcept
    {
      return _flags.Contains(Flags::HasY) ? Maybe<float> {_boundaries.Y()} : null;
    }
    KRYS_NODISCARD Maybe<float> Width() const noexcept
    {
      return _flags.Contains(Flags::HasWidth) ? Maybe<float> {_boundaries.Width()} : null;
    }
    KRYS_NODISCARD Maybe<float> Height() const noexcept
    {
      return _flags.Contains(Flags::HasHeight) ? Maybe<float> {_boundaries.Height()} : null;
    }
  };
}
