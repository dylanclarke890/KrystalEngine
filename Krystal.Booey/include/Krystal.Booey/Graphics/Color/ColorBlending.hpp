/*
 * Copyright (C) 2020 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  struct BlendingContext;
  class Color;

  // NOTE: These functions do a lossy conversion to 8-bit sRGBA before blending.

  /// @brief An implementation of Porter-Duff's "source-over" equation.
  KRYS_NODISCARD Color BlendSourceOver(const Color &backdrop, const Color &source) noexcept;

  /// @brief Bespoke "whitening" algorithm used by RenderTheme::transformSelectionBackgroundColor.
  /// @note This is a no-op if the color to blend with isn't opaque, which is likely not what you were
  /// expecting.
  KRYS_NODISCARD Color BlendWithWhite(const Color &color) noexcept;

  KRYS_NODISCARD Color Blend(const Color &from, const Color &to, const BlendingContext &context) noexcept;

  KRYS_NODISCARD Color BlendWithoutPremultiply(const Color &from, const Color &to,
                                               const BlendingContext &context) noexcept;
}
