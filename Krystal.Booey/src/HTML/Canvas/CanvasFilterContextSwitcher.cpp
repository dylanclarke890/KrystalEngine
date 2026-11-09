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

#include "Krystal.Booey/HTML/Canvas/CanvasFilterContextSwitcher.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasLayerContextSwitcher.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext2DBase.hpp"

namespace krys::boo::html
{
  UniquePtr<CanvasFilterContextSwitcher>
    CanvasFilterContextSwitcher::Create(CanvasRenderingContext2DBase &context,
                                        const FloatRect &bounds) noexcept
  {
    if (context.GetState().Filter.IsNone())
    {
      return nullptr;
    }

    auto filter = context.CreateFilter(bounds);
    if (!filter)
    {
      return nullptr;
    }

    auto filterSwitcher = CreateUnique<CanvasFilterContextSwitcher>(context);

    auto targetSwitcher = CanvasLayerContextSwitcher::Create(context, bounds, krys::move(filter));
    if (!targetSwitcher)
    {
      return nullptr;
    }

    context.ModifiableState().TargetSwitcher = krys::move(targetSwitcher);
    return filterSwitcher;
  }

  CanvasFilterContextSwitcher::CanvasFilterContextSwitcher(CanvasRenderingContext2DBase &context) noexcept
      : _context(context)
  {
    context.Save();
    context.RealizeSaves();
  }

  CanvasFilterContextSwitcher::~CanvasFilterContextSwitcher() noexcept
  {
    _context.Restore();
  }

  FloatRect CanvasFilterContextSwitcher::ExpandedBounds() const noexcept
  {
    return _context.GetState().TargetSwitcher->ExpandedBounds();
  }
}
