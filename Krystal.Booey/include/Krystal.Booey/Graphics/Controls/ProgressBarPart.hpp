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

#include "Krystal.Booey/Graphics/Controls/ControlPart.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"

namespace krys::boo
{
  /// @brief A progress bar: how far along it is (negative when indeterminate), and when its indeterminate
  /// animation started.
  class ProgressBarPart : public ControlPart
  {
  private:
    double _position;
    MonotonicTime::duration _animationStartTime;

  public:
    KRYS_NODISCARD static Ref<ProgressBarPart> Create() noexcept;
    KRYS_NODISCARD static Ref<ProgressBarPart> Create(double position,
                                                      MonotonicTime::duration animationStartTime) noexcept;

    KRYS_NODISCARD double Position() const noexcept
    {
      return _position;
    }

    void SetPosition(double position) noexcept
    {
      _position = position;
    }

    KRYS_NODISCARD MonotonicTime::duration AnimationStartTime() const noexcept
    {
      return _animationStartTime;
    }

    void SetAnimationStartTime(MonotonicTime::duration animationStartTime) noexcept
    {
      _animationStartTime = animationStartTime;
    }

  private:
    ProgressBarPart(double position, MonotonicTime::duration animationStartTime) noexcept;

    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformControl() noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_CONTROL_PART(ProgressBar)
