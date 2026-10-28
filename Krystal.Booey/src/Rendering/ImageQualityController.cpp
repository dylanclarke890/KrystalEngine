#include "Krystal.Booey/Rendering/ImageQualityController.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/Image.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  Maybe<InterpolationQuality>
    ImageQualityController::InterpolationQualityFromStyle(const RenderStyle &style) noexcept
  {
    switch (style.GetImageRendering())
    {
      case ImageRendering::OptimizeSpeed:   return InterpolationQuality::Low;
      case ImageRendering::CrispEdges:
      case ImageRendering::Pixelated:       return InterpolationQuality::DoNotInterpolate;
      // FIXME: CSS 3 Images says that optimizeQuality should behave like 'auto', but that prevents authors
      // from overriding this low quality rendering behavior.
      case ImageRendering::OptimizeQuality: return InterpolationQuality::Default;
      case ImageRendering::Auto:            break;
    }
    return null;
  }

  InterpolationQuality ImageQualityController::ChooseInterpolationQualityForSVG(
    GraphicsContext &context, const RenderElement &renderElement, Image &image) noexcept
  {
    return ChooseInterpolationQuality(context, renderElement, image);
  }

  InterpolationQuality ImageQualityController::ChooseInterpolationQuality(GraphicsContext &context,
                                                                          const RenderElement &renderer,
                                                                          Image &image) noexcept
  {
    // If the image is not a bitmap image, then none of this is relevant and we just paint at high quality.
    if (!image.IsBitmapImage() || context.PaintingDisabled())
    {
      return InterpolationQuality::Default;
    }

    if (auto styleInterpolation = InterpolationQualityFromStyle(renderer.Style()))
    {
      return *styleInterpolation;
    }

    return InterpolationQuality::Default;
  }
}
