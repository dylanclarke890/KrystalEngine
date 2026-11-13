/*
 * Copyright (C) 2019 Igalia S.L. All rights reserved.
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
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  namespace dom
  {
    class Element;
  }

  /// @brief How a programmatic scroll moves.
  /// @see https://drafts.csswg.org/cssom-view/#enumdef-scrollbehavior
  enum class ScrollBehavior : uint8
  {
    Auto,
    Instant,
    Smooth
  };

  /// @brief Whether revealing something may only scroll the view down, never back up.
  enum class OnlyAllowForwardScrolling : bool
  {
    No,
    Yes
  };

  /// @brief Whether a scroll with @p behavior, of the scrolling box of @p associatedElement, is smooth.
  /// @see https://drafts.csswg.org/cssom-view/#scrolling
  KRYS_NODISCARD bool UseSmoothScrolling(ScrollBehavior behavior,
                                         const dom::Element *associatedElement) noexcept;
}
