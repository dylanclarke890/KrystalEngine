/*
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"

namespace krys::boo::html
{
  class HTMLElement;
  class HTMLProgressElement;
}

namespace krys::boo
{
  /// @brief The renderer of a `<progress>` with a native appearance (or its inner element without one): a
  /// block the theme draws as a bar, animated while it's indeterminate.
  class RenderProgress final : public RenderBlockFlow
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderProgress);

  private:
    double _position;
    MonotonicTime::duration _animationStartTime {};
    bool _animating {false};
    Timer _animationTimer;

  public:
    RenderProgress(html::HTMLElement &, RenderStyle &&) noexcept;
    ~RenderProgress() noexcept override;

    KRYS_NODISCARD double Position() const noexcept
    {
      return _position;
    }

    KRYS_NODISCARD double AnimationProgress() const noexcept;

    KRYS_NODISCARD MonotonicTime::duration AnimationStartTime() const noexcept
    {
      return _animationStartTime;
    }

    KRYS_NODISCARD bool IsDeterminate() const noexcept;
    void UpdateFromElement() noexcept override;

    KRYS_NODISCARD html::HTMLProgressElement *ProgressElement() const noexcept;

  private:
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderProgress";
    }

    KRYS_NODISCARD LogicalExtentComputedValues
      ComputeLogicalHeight(LayoutUnit logicalHeight, LayoutUnit logicalTop) const noexcept override;
    void WillBeDestroyed() noexcept final;

    void AnimationTimerFired() noexcept;
    void UpdateAnimationState() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderProgress)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderProgress();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
