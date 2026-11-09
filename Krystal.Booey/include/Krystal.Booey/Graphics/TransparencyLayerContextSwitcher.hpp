/*
 * Copyright (C) 2022-2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Filters/FilterStyle.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextSwitcher.hpp"

namespace krys::boo
{
  /// @brief Applies a filter through the graphics context: the source is drawn in a transparency layer per
  /// filter style, each composited with its style.
  class TransparencyLayerContextSwitcher final : public GraphicsContextSwitcher
  {
  private:
    FilterStyleVector _filterStyles;

  public:
    TransparencyLayerContextSwitcher(GraphicsContext &destinationContext, const FloatRect &sourceImageRect,
                                     RefPtr<Filter> &&filter) noexcept;

    void BeginClipAndDrawSourceImage(GraphicsContext &destinationContext, const FloatRect &repaintRect,
                                     const FloatRect &clipRect) noexcept override;
    void EndClipAndDrawSourceImage(GraphicsContext &destinationContext) noexcept override
    {
      EndDrawSourceImage(destinationContext);
    }

    void BeginDrawSourceImage(GraphicsContext &destinationContext, float opacity = 1.f) noexcept override;
    void EndDrawSourceImage(GraphicsContext &destinationContext) noexcept override;
  };
}
