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

#include "Krystal.Web/Rendering/SVG/Legacy/LegacyRenderSVGResourceSolidColor.hpp"
#include "Krystal.Web/Graphics/GraphicsContext.hpp"
#include "Krystal.Web/Page/FrameView.hpp"
#include "Krystal.Web/Rendering/RenderElement.hpp"
#include "Krystal.Web/Rendering/RenderView.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Web/Rendering/SVG/SVGRenderSupport.hpp"

namespace krys::web
{
  auto LegacyRenderSVGResourceSolidColor::ApplyResource(RenderElement &renderer, const RenderStyle &style,
                                                        GraphicsContext *&context,
                                                        EnumSet<RenderSVGResourceMode> resourceMode) noexcept
    -> EnumSet<ApplyResult>
  {
    krys_debug_assert(context);
    krys_debug_assert(!resourceMode.empty());

    bool isRenderingMask =
      renderer.View().FrameView().GetPaintBehavior().contains(PaintBehavior::RenderingSVGClipOrMask);

    if (resourceMode.contains(RenderSVGResourceMode::ApplyToFill))
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
        context->SetFillRule(style.FillRule());
      }

      if (resourceMode.contains(RenderSVGResourceMode::ApplyToText))
      {
        context->SetTextDrawingMode(TextDrawingMode::Fill);
      }
    }
    else if (resourceMode.contains(RenderSVGResourceMode::ApplyToStroke))
    {
      // When rendering the mask for a LegacyRenderSVGResourceClipper, the stroke code path is never hit.
      krys_debug_assert(!isRenderingMask);
      context->SetAlpha(style.StrokeOpacity().Value.Value);
      context->SetStrokeColor(_color);

      SVGRenderSupport::ApplyStrokeStyleToContext(*context, style, renderer);

      if (resourceMode.contains(RenderSVGResourceMode::ApplyToText))
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
    krys_debug_assert(!resourceMode.empty());
    FillAndStrokePathOrShape(*context, resourceMode, path, shape);
  }
}
