/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Controls/ControlStyle.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  class ControlPart;
  class FloatRoundedRect;
  class GraphicsContext;

  /// @brief A theme's drawing of a control part: sizes it and draws it.
  class PlatformControl
  {
  protected:
    ControlPart &_owningPart;

  public:
    explicit PlatformControl(ControlPart &owningPart) noexcept : _owningPart(owningPart)
    {
    }

    virtual ~PlatformControl() noexcept = default;

    virtual void SetFocusRingClipRect(const FloatRect &) noexcept
    {
    }

    virtual void UpdateCellStates(const FloatRect &, const ControlStyle &) noexcept
    {
    }

    KRYS_NODISCARD virtual FloatSize SizeForBounds(const FloatRect &bounds,
                                                   const ControlStyle &) const noexcept
    {
      return bounds.Size();
    }

    KRYS_NODISCARD virtual FloatRect RectForBounds(const FloatRect &bounds,
                                                   const ControlStyle &) const noexcept
    {
      return bounds;
    }

    virtual void Draw(GraphicsContext &, const FloatRoundedRect &, float, const ControlStyle &) noexcept
    {
    }
  };
}
