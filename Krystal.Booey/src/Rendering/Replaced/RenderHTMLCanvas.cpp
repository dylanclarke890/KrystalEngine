/*
 * Copyright (C) 2004, 2006, 2007 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/Replaced/RenderHTMLCanvas.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLCanvasElement.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayerBacking.hpp"
#include "Krystal.Booey/Rendering/Painting/ImageQualityController.hpp"
#include "Krystal.Booey/Rendering/Painting/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  RenderHTMLCanvas::RenderHTMLCanvas(html::HTMLCanvasElement &element, RenderStyle &&style) noexcept
      : RenderReplaced(Type::HTMLCanvas, element, krys::move(style), LayoutSize(element.Size()))
  {
    krys_debug_assert(IsRenderHTMLCanvas());
  }

  RenderHTMLCanvas::~RenderHTMLCanvas() noexcept = default;

  html::HTMLCanvasElement &RenderHTMLCanvas::CanvasElement() const noexcept
  {
    return Downcast<html::HTMLCanvasElement>(NodeForNonAnonymous());
  }

  bool RenderHTMLCanvas::RequiresLayer() const noexcept
  {
    if (RenderReplaced::RequiresLayer())
    {
      return true;
    }

    return GetCanvasCompositingStrategy(*this) != CanvasCompositingStrategy::CanvasPaintedToEnclosingLayer;
  }

  void RenderHTMLCanvas::PaintReplaced(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    GraphicsContext &context = paintInfo.Context();

    LayoutRect contentBoxRect = ContentBoxRect();

    if (context.DetectingContentfulPaint())
    {
      if (!context.ContentfulPaintDetected() && CanvasElement().RenderingContext())
      {
        context.SetContentfulPaintDetected();
      }
      return;
    }

    contentBoxRect.MoveBy(paintOffset);
    LayoutRect replacedContentRect = ReplacedContentRect();
    replacedContentRect.MoveBy(paintOffset);

    // Not allowed to overflow the content box.
    bool clip = !contentBoxRect.Contains(replacedContentRect);
    GraphicsContextStateSaver stateSaver(context, clip);
    if (clip)
    {
      context.Clip(FloatRect(SnappedIntRect(contentBoxRect)));
    }

    // FIXME: Recording repainted objects for paint milestones (addRelevantRepaintedObject), once there are
    // paint milestones.

    InterpolationQualityMaintainer interpolationMaintainer(
      context, ImageQualityController::InterpolationQualityFromStyle(Style()));

    CanvasElement().SetIsSnapshotting(paintInfo.PaintBehavior.Contains(PaintBehavior::Snapshotting));
    CanvasElement().Paint(context, replacedContentRect);
    CanvasElement().SetIsSnapshotting(false);
  }

  void RenderHTMLCanvas::CanvasSizeChanged() noexcept
  {
    IntSize canvasSize = CanvasElement().Size();
    LayoutSize zoomedSize(canvasSize.Width() * Style().UsedZoom(), canvasSize.Height() * Style().UsedZoom());

    if (zoomedSize == IntrinsicSize())
    {
      return;
    }

    SetIntrinsicSize(zoomedSize);

    if (!Parent())
    {
      return;
    }

    (void)SetNeedsLayoutIfNeededAfterIntrinsicSizeChange();
  }
}
