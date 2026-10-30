#include "Krystal.Booey/HTML/CanvasBase.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/DestinationColorSpace.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext.hpp"

namespace krys::boo::html
{
  namespace
  {
    // Matches the 2D context's default image smoothing quality (`low`).
    constexpr InterpolationQuality DefaultInterpolationQuality = InterpolationQuality::Low;

    KRYS_NODISCARD constexpr uint64 MaxCanvasArea() noexcept
    {
      // Firefox limits width/height to 32767 pixels, but slows down dramatically before it
      // reaches that limit. We limit by area instead, giving us larger maximum dimensions,
      // in exchange for a smaller maximum canvas size. The maximum canvas size is in device pixels.
      return 16'384 * 16'384;
    }
  }

  CanvasBase::CanvasBase(IntSize size) noexcept : _size(size)
  {
  }

  CanvasBase::~CanvasBase() noexcept = default;

  ImageBuffer *CanvasBase::Buffer() const noexcept
  {
    if (!HasCreatedImageBuffer())
    {
      CreateImageBuffer();
    }
    return _imageBuffer.get();
  }

  RefPtr<ImageBuffer> CanvasBase::MakeRenderingResultsAvailable() noexcept
  {
    if (auto *context = RenderingContext())
    {
      return context->SurfaceBufferToImageBuffer(CanvasRenderingContext::SurfaceBuffer::DrawingBuffer);
    }
    return ShareRefPtr(Buffer());
  }

  void CanvasBase::DidDraw(const Maybe<FloatRect> &) noexcept
  {
    AddCanvasNeedingPreparationForDisplayOrFlush();
  }

  void CanvasBase::AddCanvasNeedingPreparationForDisplayOrFlush() noexcept
  {
    auto *context = RenderingContext();
    if (!context || context->IsInPreparationForDisplayOrFlush())
    {
      return;
    }
    if (auto *document = CanvasDocument())
    {
      document->AddCanvasNeedingPreparationForDisplayOrFlush(*context);
    }
  }

  void CanvasBase::RemoveCanvasNeedingPreparationForDisplayOrFlush() noexcept
  {
    auto *context = RenderingContext();
    if (!context || !context->IsInPreparationForDisplayOrFlush())
    {
      return;
    }
    if (auto *document = CanvasDocument())
    {
      document->RemoveCanvasNeedingPreparationForDisplayOrFlush(*context);
    }
  }

  bool CanvasBase::ShouldAccelerate(uint64 area) const noexcept
  {
    // Accelerated drawing is on, and there's no minimum area: the host's backend draws every canvas, there
    // being no unaccelerated one.
    return area > 0;
  }

  void CanvasBase::SetSize(const IntSize &size) noexcept
  {
    _size = size;
  }

  RefPtr<ImageBuffer> CanvasBase::SetImageBuffer(RefPtr<ImageBuffer> &&buffer) const noexcept
  {
    _contextStateSaver = nullptr;
    auto returnBuffer = std::exchange(_imageBuffer, krys::move(buffer));

    if (_imageBuffer)
    {
      _size = _imageBuffer->TruncatedLogicalSize();
      auto &context = _imageBuffer->Context();
      context.SetShadowsIgnoreTransforms(true);
      context.SetImageInterpolationQuality(DefaultInterpolationQuality);
      context.SetStrokeThickness(1);
      _contextStateSaver = CreateUnique<GraphicsContextStateSaver>(context);
    }
    return returnBuffer;
  }

  RefPtr<ImageBuffer> CanvasBase::AllocateImageBuffer() const noexcept
  {
    const uint64 area = static_cast<uint64>(_size.Width()) * static_cast<uint64>(_size.Height());
    if (!area)
    {
      return nullptr;
    }
    if (area > MaxCanvasArea())
    {
      return nullptr;
    }

    auto *context = RenderingContext();
    // FIXME: A context that will read frequently draws unaccelerated, once there's an unaccelerated backend
    // to read from.
    const auto renderingMode =
      ShouldAccelerate(area) ? RenderingMode::Accelerated : RenderingMode::Unaccelerated;
    // FIXME: The context's pixel format (sRGB, 8 bits a channel).
    auto colorSpace = context ? context->ColorSpace() : DestinationColorSpace::SRGB();

    auto *document = CanvasDocument();
    return ImageBuffer::Create(FloatSize(_size), renderingMode, RenderingPurpose::Canvas, 1, colorSpace,
                               document ? document->GetGraphicsClient() : nullptr);
  }

  void CanvasBase::ResetGraphicsContextState() const noexcept
  {
    if (_contextStateSaver)
    {
      // Reset to the initial graphics context state.
      _contextStateSaver->Restore();
      _contextStateSaver->Save();
    }
  }
}
