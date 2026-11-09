/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceSolidColor.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderSupport.hpp"

namespace krys::boo
{
  auto LegacyRenderSVGResourceSolidColor::ApplyResource(RenderElement &renderer, const RenderStyle &style,
                                                        GraphicsContext *&context,
                                                        EnumSet<RenderSVGResourceMode> resourceMode) noexcept
    -> EnumSet<ApplyResult>
  {
    krys_debug_assert(context);
    krys_debug_assert(!resourceMode.IsEmpty());

    bool isRenderingMask =
      renderer.View().FrameView().GetPaintBehavior().Contains(PaintBehavior::RenderingSVGClipOrMask);

    if (resourceMode.Contains(RenderSVGResourceMode::ApplyToFill))
    {
      if (!isRenderingMask)
      {
        context->SetAlpha(style.FillOpacity().Value.Value);
      }
      else
      {
        context->SetAlpha(1);
      }
      context->SetFillColor(_color);
      if (isRenderingMask)
      {
        context->SetFillRule(style.ClipRule());
      }
      else
      {
        context->SetFillRule(style.GetFillRule());
      }

      if (resourceMode.Contains(RenderSVGResourceMode::ApplyToText))
      {
        context->SetTextDrawingMode(TextDrawingMode::Fill);
      }
    }
    else if (resourceMode.Contains(RenderSVGResourceMode::ApplyToStroke))
    {
      // When rendering the mask for a LegacyRenderSVGResourceClipper, the stroke code path is never hit.
      krys_debug_assert(!isRenderingMask);
      context->SetAlpha(style.StrokeOpacity().Value.Value);
      context->SetStrokeColor(_color);

      SVGRenderSupport::ApplyStrokeStyleToContext(*context, style, renderer);

      if (resourceMode.Contains(RenderSVGResourceMode::ApplyToText))
      {
        context->SetTextDrawingMode(TextDrawingMode::Stroke);
      }
    }

    return {ApplyResult::ResourceApplied};
  }

  void LegacyRenderSVGResourceSolidColor::PostApplyResource(RenderElement &, GraphicsContext *&context,
                                                            EnumSet<RenderSVGResourceMode> resourceMode,
                                                            const Path *path,
                                                            const RenderElement *shape) noexcept
  {
    krys_debug_assert(context);
    krys_debug_assert(!resourceMode.IsEmpty());
    FillAndStrokePathOrShape(*context, resourceMode, path, shape);
  }
}
