#include "Krystal.Booey/SVG/Graphics/SVGResourceImage.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceMasker.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo::svg
{
  Ref<SVGResourceImage> SVGResourceImage::Create(LegacyRenderSVGResourceContainer &renderResource,
                                                 const style::URL &resourceURL) noexcept
  {
    return AdoptRef(*new SVGResourceImage(renderResource, resourceURL));
  }

  SVGResourceImage::SVGResourceImage(LegacyRenderSVGResourceContainer &renderResource,
                                     const style::URL &resourceURL) noexcept
      : _legacyRenderResource(CreateWeakPtr(&renderResource)), _resourceURL(resourceURL)
  {
  }

  SVGResourceImage::~SVGResourceImage() noexcept = default;

  ImageDrawResult SVGResourceImage::Draw(GraphicsContext &context, const FloatRect &destinationRect,
                                         const FloatRect &sourceRect, ImagePaintingOptions options) noexcept
  {
    if (auto *masker = DynamicDowncast<LegacyRenderSVGResourceMasker>(_legacyRenderResource.get()))
    {
      if (masker->DrawContentIntoContext(context, destinationRect, sourceRect, options))
      {
        return ImageDrawResult::DidDraw;
      }
    }

    return ImageDrawResult::DidNothing;
  }

  void SVGResourceImage::DrawPattern(GraphicsContext &context, const FloatRect &destinationRect,
                                     const FloatRect &sourceRect, const AffineTransform &patternTransform,
                                     const FloatPoint &phase, const FloatSize &spacing,
                                     ImagePaintingOptions options) noexcept
  {
    auto imageBuffer = context.CreateImageBuffer(Size());
    if (!imageBuffer)
    {
      return;
    }

    // Fill with the SVG resource.
    GraphicsContext &graphicsContext = imageBuffer->Context();
    (void)graphicsContext.DrawImage(*this, FloatPoint());

    // Tile the image buffer into the context.
    context.DrawPattern(*imageBuffer, destinationRect, sourceRect, patternTransform, phase, spacing, options);
  }

  void SVGResourceImage::Dump(TextStream &ts) const noexcept
  {
    GeneratedImage::Dump(ts);
    ts << _resourceURL.Resolved.String();
  }
}
