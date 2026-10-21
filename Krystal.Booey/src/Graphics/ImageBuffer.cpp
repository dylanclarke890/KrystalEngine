#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/ImageBufferDisplayListBackend.hpp"
#include "Krystal.Core/Text/TextStream.hpp"
#include <atomic>
#include <cmath>

namespace krys::boo
{
  namespace
  {
    constexpr float MaxClampedLength = 4'096;
    constexpr float MaxClampedArea = MaxClampedLength * MaxClampedLength;
  }

  RenderingResourceIdentifier RenderingResourceIdentifier::Generate() noexcept
  {
    static std::atomic<uint64> next {1};
    return RenderingResourceIdentifier(next.fetch_add(1, std::memory_order_relaxed));
  }

  TextStream &operator<<(TextStream &ts, RenderingResourceIdentifier identifier) noexcept
  {
    return ts << identifier.ToUInt64();
  }

  RefPtr<ImageBuffer> ImageBuffer::Create(const FloatSize &size, float resolutionScale) noexcept
  {
    // FIXME: The rendering mode (accelerated and unaccelerated bitmap backends, PDF documents) comes with
    // bitmap images; every buffer records into a display list.
    Parameters parameters {size, resolutionScale};
    auto backendParameters = BackendParameters(parameters);
    if (backendParameters.BackendSize.IsEmpty())
    {
      return nullptr;
    }
    auto backend = ImageBufferDisplayListBackend::Create(backendParameters);
    ImageBufferBackend::Info backendInfo {ImageBufferBackend::CalculateBaseTransform(backendParameters)};
    return AdoptRef(*new ImageBuffer(parameters, backendInfo, krys::move(backend)));
  }

  ImageBuffer::ImageBuffer(const Parameters &parameters, const ImageBufferBackend::Info &backendInfo,
                           UniquePtr<ImageBufferBackend> &&backend,
                           RenderingResourceIdentifier renderingResourceIdentifier) noexcept
      : _parameters(parameters), _backendInfo(backendInfo), _backend(krys::move(backend)),
        _renderingResourceIdentifier(renderingResourceIdentifier)
  {
  }

  ImageBuffer::~ImageBuffer() noexcept = default;

  IntSize ImageBuffer::CalculateBackendSize(FloatSize logicalSize, float resolutionScale) noexcept
  {
    FloatSize scaledSize = {std::ceil(resolutionScale * logicalSize.Width()),
                            std::ceil(resolutionScale * logicalSize.Height())};
    if (scaledSize.IsEmpty() || !scaledSize.IsExpressibleAsIntSize())
    {
      return {};
    }
    return IntSize {scaledSize};
  }

  ImageBufferBackendParameters
    ImageBuffer::BackendParameters(const ImageBufferParameters &parameters) noexcept
  {
    return {CalculateBackendSize(parameters.LogicalSize, parameters.ResolutionScale),
            parameters.ResolutionScale};
  }

  bool ImageBuffer::SizeNeedsClamping(const FloatSize &size) noexcept
  {
    if (size.IsEmpty())
    {
      return false;
    }

    return std::floor(size.Height()) * std::floor(size.Width()) > MaxClampedArea;
  }

  bool ImageBuffer::SizeNeedsClamping(const FloatSize &size, FloatSize &scale) noexcept
  {
    FloatSize scaledSize(size);
    scaledSize.Scale(scale.Width(), scale.Height());

    if (!SizeNeedsClamping(scaledSize))
    {
      return false;
    }

    // The area of scaled size is bigger than the upper limit, adjust the scale to fit.
    scale.Scale(std::sqrt(MaxClampedArea / (scaledSize.Width() * scaledSize.Height())));
    krys_debug_assert(!SizeNeedsClamping(size, scale));
    return true;
  }

  GraphicsContext &ImageBuffer::Context() const noexcept
  {
    krys_debug_assert(_backend);
    return _backend->Context();
  }

  void ImageBuffer::FlushDrawingContext() noexcept
  {
    if (_backend)
    {
      _backend->FlushContext();
    }
  }

  void ImageBuffer::ConvertToLuminanceMask() noexcept
  {
    if (_backend)
    {
      _backend->ConvertToLuminanceMask();
    }
  }

  IntSize ImageBuffer::BackendSize() const noexcept
  {
    return CalculateBackendSize(_parameters.LogicalSize, _parameters.ResolutionScale);
  }
}
