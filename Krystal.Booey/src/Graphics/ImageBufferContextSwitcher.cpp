#include "Krystal.Booey/Graphics/ImageBufferContextSwitcher.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterResults.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"

namespace krys::boo
{
  ImageBufferContextSwitcher::ImageBufferContextSwitcher(GraphicsContext &destinationContext,
                                                         const FloatRect &sourceImageRect,
                                                         RefPtr<Filter> &&filter,
                                                         FilterResults *results) noexcept
      : GraphicsContextSwitcher(krys::move(filter)), _sourceImageRect(sourceImageRect), _results(results)
  {
    if (sourceImageRect.IsEmpty())
    {
      return;
    }

    if (_filter)
    {
      _sourceImage = destinationContext.CreateScaledImageBuffer(_sourceImageRect, _filter->FilterScale());
    }
    else
    {
      _sourceImage =
        destinationContext.CreateScaledImageBuffer(_sourceImageRect, destinationContext.ScaleFactor());
    }

    if (!_sourceImage)
    {
      _filter = nullptr;
      return;
    }

    auto state = destinationContext.State();
    _sourceImage->Context().MergeAllChanges(state);
  }

  ImageBufferContextSwitcher::~ImageBufferContextSwitcher() noexcept = default;

  GraphicsContext *ImageBufferContextSwitcher::DrawingContext(GraphicsContext &context) const noexcept
  {
    return _sourceImage ? &_sourceImage->Context() : &context;
  }

  void ImageBufferContextSwitcher::BeginClipAndDrawSourceImage(GraphicsContext &destinationContext,
                                                               const FloatRect &repaintRect,
                                                               const FloatRect &) noexcept
  {
    if (auto *context = DrawingContext(destinationContext))
    {
      context->Save();
      context->ClearRect(repaintRect);
      context->Clip(repaintRect);
    }
  }

  void ImageBufferContextSwitcher::EndClipAndDrawSourceImage(GraphicsContext &destinationContext) noexcept
  {
    if (auto *context = DrawingContext(destinationContext))
    {
      context->Restore();
    }

    EndDrawSourceImage(destinationContext);
  }

  void ImageBufferContextSwitcher::EndDrawSourceImage(GraphicsContext &destinationContext) noexcept
  {
    if (!_filter)
    {
      if (_sourceImage)
      {
        destinationContext.DrawImageBuffer(
          *_sourceImage, _sourceImageRect,
          {destinationContext.CompositeOperation(), destinationContext.GetBlendMode()});
      }
      return;
    }

    FilterResults results;
    destinationContext.DrawFilteredImageBuffer(_sourceImage.get(), _sourceImageRect, *_filter,
                                               _results ? *_results : results);
  }
}
