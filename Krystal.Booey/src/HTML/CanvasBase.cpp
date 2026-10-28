#include "Krystal.Booey/HTML/CanvasBase.hpp"
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

  void CanvasBase::DidDraw(const Maybe<FloatRect> &) noexcept
  {
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

    // FIXME: The host's accelerated buffer (B14b), in the context's colour space.
    return ImageBuffer::Create(FloatSize(_size), 1);
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
