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

#include "Krystal.Booey/HTML/Canvas/CanvasLayerContextSwitcher.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextSwitcher.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext2DBase.hpp"

namespace krys::boo::html
{
  RefPtr<CanvasLayerContextSwitcher> CanvasLayerContextSwitcher::Create(CanvasRenderingContext2DBase &context,
                                                                        const FloatRect &bounds,
                                                                        RefPtr<Filter> &&filter) noexcept
  {
    krys_debug_assert(!bounds.IsEmpty());
    auto *effectiveDrawingContext = context.EffectiveDrawingContext();
    if (!effectiveDrawingContext)
    {
      return nullptr;
    }

    auto targetSwitcher =
      GraphicsContextSwitcher::Create(*effectiveDrawingContext, bounds, krys::move(filter));
    if (!targetSwitcher)
    {
      return nullptr;
    }

    return AdoptRef(*new CanvasLayerContextSwitcher(context, bounds, krys::move(targetSwitcher)));
  }

  CanvasLayerContextSwitcher::CanvasLayerContextSwitcher(
    CanvasRenderingContext2DBase &context, const FloatRect &bounds,
    UniquePtr<GraphicsContextSwitcher> &&targetSwitcher) noexcept
      : _context(context), _effectiveDrawingContext(context.EffectiveDrawingContext()), _bounds(bounds),
        _targetSwitcher(krys::move(targetSwitcher))
  {
    krys_debug_assert(_targetSwitcher);
    krys_debug_assert(_effectiveDrawingContext);
    _targetSwitcher->BeginDrawSourceImage(*_effectiveDrawingContext,
                                          static_cast<float>(context.GlobalAlpha()));
  }

  CanvasLayerContextSwitcher::~CanvasLayerContextSwitcher() noexcept
  {
    _targetSwitcher->EndDrawSourceImage(*_effectiveDrawingContext);
  }

  GraphicsContext *CanvasLayerContextSwitcher::DrawingContext() const noexcept
  {
    if (!_effectiveDrawingContext)
    {
      return nullptr;
    }
    return _targetSwitcher->DrawingContext(*_effectiveDrawingContext);
  }

  FloatBoxExtent CanvasLayerContextSwitcher::Outsets() const noexcept
  {
    return ToFloatBoxExtent(_context.CalculateFilterOutsets(_bounds));
  }
}
