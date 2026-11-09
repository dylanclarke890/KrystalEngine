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

#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class Filter;
  class FilterResults;
  class GraphicsContext;

  /// @brief Where a filtered source is drawn, and how its result reaches the destination context.
  class GraphicsContextSwitcher
  {
    KRYS_NON_COPY_MOVABLE(GraphicsContextSwitcher);

  protected:
    RefPtr<Filter> _filter;

  public:
    /// @brief A switcher that applies @p filter as it's drawn: through the graphics context when the filter
    /// supports it; otherwise to an image buffer the source is drawn into.
    KRYS_NODISCARD static UniquePtr<GraphicsContextSwitcher>
      Create(GraphicsContext &destinationContext, const FloatRect &sourceImageRect,
             RefPtr<Filter> &&filter = nullptr, FilterResults *results = nullptr) noexcept;

    virtual ~GraphicsContextSwitcher() noexcept = default;

    KRYS_NODISCARD virtual GraphicsContext *DrawingContext(GraphicsContext &destinationContext) const noexcept
    {
      return &destinationContext;
    }

    KRYS_NODISCARD virtual bool HasSourceImage() const noexcept
    {
      return false;
    }

    virtual void BeginClipAndDrawSourceImage(GraphicsContext &destinationContext,
                                             const FloatRect &repaintRect,
                                             const FloatRect &clipRect) noexcept = 0;
    virtual void EndClipAndDrawSourceImage(GraphicsContext &destinationContext) noexcept = 0;

    virtual void BeginDrawSourceImage(GraphicsContext &destinationContext, float opacity = 1.f) noexcept = 0;
    virtual void EndDrawSourceImage(GraphicsContext &destinationContext) noexcept = 0;

  protected:
    explicit GraphicsContextSwitcher(RefPtr<Filter> &&filter) noexcept;
  };
}
