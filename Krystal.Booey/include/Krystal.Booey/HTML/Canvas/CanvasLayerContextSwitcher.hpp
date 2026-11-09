/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/BoxExtents.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class Filter;
  class GraphicsContext;
  class GraphicsContextSwitcher;
}

namespace krys::boo::html
{
  class CanvasRenderingContext2DBase;

  /// @brief Redirects a 2D context's drawing into a layer that's filtered as it's drawn back into the canvas,
  /// when the switcher goes.
  class CanvasLayerContextSwitcher : public RefCounted<CanvasLayerContextSwitcher>
  {
  private:
    CanvasRenderingContext2DBase &_context;
    GraphicsContext *_effectiveDrawingContext;
    FloatRect _bounds;
    UniquePtr<GraphicsContextSwitcher> _targetSwitcher;

  public:
    /// @brief A switcher for drawing within @p bounds through @p filter; null if the context has nowhere to
    /// draw.
    KRYS_NODISCARD static RefPtr<CanvasLayerContextSwitcher>
      Create(CanvasRenderingContext2DBase &, const FloatRect &bounds, RefPtr<Filter> &&) noexcept;
    ~CanvasLayerContextSwitcher() noexcept;

    /// @brief Where the context draws while the switcher is in place.
    KRYS_NODISCARD GraphicsContext *DrawingContext() const noexcept;
    /// @brief The bounds, with the filter's outsets.
    KRYS_NODISCARD FloatRect ExpandedBounds() const noexcept
    {
      return _bounds + Outsets();
    }

  private:
    CanvasLayerContextSwitcher(CanvasRenderingContext2DBase &, const FloatRect &bounds,
                               UniquePtr<GraphicsContextSwitcher> &&) noexcept;

    KRYS_NODISCARD FloatBoxExtent Outsets() const noexcept;
  };
}
