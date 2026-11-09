/*
 * Copyright (C) 2014 Adobe Systems Incorporated. All rights reserved.
 * Copyright (C) 2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/DOM/Geometry/DOMPointReadOnly.hpp"

namespace krys::boo::dom
{
  /// @brief A point whose coordinates can be changed.
  /// @see https://drafts.fxtf.org/geometry/#dompoint
  class DOMPoint final : public DOMPointReadOnly
  {
  public:
    KRYS_NODISCARD static Ref<DOMPoint> Create(double x, double y, double z = 0, double w = 1) noexcept
    {
      return AdoptRef(*new DOMPoint(x, y, z, w));
    }
    KRYS_NODISCARD static Ref<DOMPoint> Create(const DOMPointInit &init) noexcept
    {
      return Create(init.X, init.Y, init.Z, init.W);
    }
    KRYS_NODISCARD static Ref<DOMPoint> FromPoint(const DOMPointInit &init) noexcept
    {
      return Create(init.X, init.Y, init.Z, init.W);
    }

    void SetX(double x) noexcept
    {
      _x = x;
    }
    void SetY(double y) noexcept
    {
      _y = y;
    }
    void SetZ(double z) noexcept
    {
      _z = z;
    }
    void SetW(double w) noexcept
    {
      _w = w;
    }

  private:
    DOMPoint(double x, double y, double z, double w) noexcept : DOMPointReadOnly(x, y, z, w)
    {
    }
  };

  static_assert(sizeof(DOMPoint) == sizeof(DOMPointReadOnly));
}
