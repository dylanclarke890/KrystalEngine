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

#include "Krystal.Booey/Graphics/TransparencyLayerContextSwitcher.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"

namespace krys::boo
{
  TransparencyLayerContextSwitcher::TransparencyLayerContextSwitcher(GraphicsContext &destinationContext,
                                                                     const FloatRect &sourceImageRect,
                                                                     RefPtr<Filter> &&filter) noexcept
      : GraphicsContextSwitcher(krys::move(filter))
  {
    if (_filter)
    {
      _filterStyles = _filter->CreateFilterStyles(destinationContext, sourceImageRect);
    }
  }

  void TransparencyLayerContextSwitcher::BeginClipAndDrawSourceImage(GraphicsContext &destinationContext,
                                                                     const FloatRect &,
                                                                     const FloatRect &clipRect) noexcept
  {
    destinationContext.Save();
    destinationContext.BeginTransparencyLayer(1);

    for (const auto &filterStyle : _filterStyles)
    {
      destinationContext.Save();
      destinationContext.Clip(Intersection(filterStyle.ImageRect, clipRect));
      destinationContext.SetStyle(filterStyle.Style);
      destinationContext.BeginTransparencyLayer(1);
    }
  }

  void TransparencyLayerContextSwitcher::BeginDrawSourceImage(GraphicsContext &destinationContext,
                                                              float opacity) noexcept
  {
    destinationContext.Save();
    destinationContext.BeginTransparencyLayer(opacity);

    for (const auto &filterStyle : _filterStyles)
    {
      destinationContext.Save();
      destinationContext.Clip(filterStyle.ImageRect);
      destinationContext.SetStyle(filterStyle.Style);
      destinationContext.BeginTransparencyLayer(1);
    }
  }

  void TransparencyLayerContextSwitcher::EndDrawSourceImage(GraphicsContext &destinationContext) noexcept
  {
    for (size_t i = 0; i < _filterStyles.size(); ++i)
    {
      destinationContext.EndTransparencyLayer();
      destinationContext.Restore();
    }

    destinationContext.EndTransparencyLayer();
    destinationContext.Restore();
  }
}
