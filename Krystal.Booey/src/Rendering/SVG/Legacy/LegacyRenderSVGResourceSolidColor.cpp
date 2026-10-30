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
      context->SetFillColor(style.ColorByApplyingColorFilter(_color));
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
      context->SetStrokeColor(style.ColorByApplyingColorFilter(_color));

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
