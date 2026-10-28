#include "Krystal.Booey/Graphics/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/ByteArrayPixelBuffer.hpp"
#include "Krystal.Booey/Graphics/DestinationColorSpace.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerContentsDisplayDelegate.hpp"
#include "Krystal.Booey/Graphics/ImageBufferDisplayListBackend.hpp"
#include "Krystal.Booey/Graphics/ImageBufferUtilities.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Graphics/NativeImage.hpp"
#include "Krystal.Booey/Platform/GraphicsClient.hpp"
#include "Krystal.Booey/Platform/MIMETypeRegistry.hpp"
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

  RefPtr<ImageBuffer> ImageBuffer::Create(const FloatSize &size, RenderingMode mode, RenderingPurpose purpose,
                                          float resolutionScale, const DestinationColorSpace &colorSpace,
                                          GraphicsClient *graphicsClient) noexcept
  {
    if (graphicsClient)
    {
      if (auto imageBuffer =
            graphicsClient->CreateImageBuffer(size, mode, purpose, resolutionScale, colorSpace))
      {
        return imageBuffer;
      }
    }

    // FIXME: The unaccelerated bitmap backend comes with bitmap images; without the host's backend, every
    // buffer records into a display list.
    return Create<ImageBufferDisplayListBackend>(size, resolutionScale, purpose);
  }

  RefPtr<ImageBuffer> ImageBuffer::Create(const FloatSize &size, float resolutionScale) noexcept
  {
    return Create(size, RenderingMode::Unaccelerated, RenderingPurpose::Unspecified, resolutionScale,
                  DestinationColorSpace::SRGB());
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

  bool ImageBuffer::FlushDrawingContextAsync() noexcept
  {
    FlushDrawingContext();
    return true;
  }

  void ImageBuffer::PrepareForDisplay() noexcept
  {
    std::ignore = FlushDrawingContextAsync();
    if (_backend)
    {
      _backend->PrepareForDisplay();
    }
  }

  RefPtr<GraphicsLayerContentsDisplayDelegate> ImageBuffer::LayerContentsDisplayDelegate() const noexcept
  {
    return _backend ? _backend->LayerContentsDisplayDelegate() : nullptr;
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

  RefPtr<NativeImage> ImageBuffer::CopyNativeImage() const noexcept
  {
    return _backend ? _backend->CopyNativeImage() : nullptr;
  }

  RefPtr<NativeImage> ImageBuffer::CreateNativeImageReference() const noexcept
  {
    return _backend ? _backend->CreateNativeImageReference() : nullptr;
  }

  RefPtr<NativeImage> ImageBuffer::SinkIntoNativeImage() noexcept
  {
    return _backend ? _backend->SinkIntoNativeImage() : nullptr;
  }

  RefPtr<NativeImage> ImageBuffer::SinkIntoNativeImage(RefPtr<ImageBuffer> source) noexcept
  {
    if (!source)
    {
      return nullptr;
    }
    return source->SinkIntoNativeImage();
  }

  utf8_string ImageBuffer::ToDataURL(utf8_stringview mimeType, Maybe<double> quality) const noexcept
  {
    return ToDataURL(ShareRef(const_cast<ImageBuffer &>(*this)), mimeType, quality);
  }

  List<uint8> ImageBuffer::ToData(utf8_stringview mimeType, Maybe<double> quality) const noexcept
  {
    return ToData(ShareRef(const_cast<ImageBuffer &>(*this)), mimeType, quality);
  }

  utf8_string ImageBuffer::ToDataURL(Ref<ImageBuffer> source, utf8_stringview mimeType,
                                     Maybe<double> quality) noexcept
  {
    auto encodedData = ToData(krys::move(source), mimeType, quality);
    return DataURL(encodedData, mimeType);
  }

  List<uint8> ImageBuffer::ToData(Ref<ImageBuffer> source, utf8_stringview mimeType,
                                  Maybe<double> quality) noexcept
  {
    // The pixels are read back to be encoded. A JPEG has no alpha, so its pixels are premultiplied: what's
    // drawn over opaque black.
    auto alphaFormat = MIMETypeRegistry::IsJPEGMIMEType(mimeType) ? AlphaPremultiplication::Premultiplied
                                                                  : AlphaPremultiplication::Unpremultiplied;
    auto size = source->TruncatedLogicalSize();
    auto pixelBuffer = source->GetPixelBuffer(
      {alphaFormat, PixelFormat::RGBA8, DestinationColorSpace::SRGB()}, IntRect({}, size));
    if (!pixelBuffer)
    {
      return {};
    }
    return EncodeData(*pixelBuffer, mimeType, quality);
  }

  RefPtr<PixelBuffer> ImageBuffer::GetPixelBuffer(const PixelBufferFormat &destinationFormat,
                                                  const IntRect &sourceRect) const noexcept
  {
    krys_debug_assert(PixelBuffer::SupportedPixelFormat(destinationFormat.Format));
    auto sourceRectScaled = sourceRect;
    sourceRectScaled.Scale(ResolutionScale());
    RefPtr<PixelBuffer> destination =
      ByteArrayPixelBuffer::TryCreate(destinationFormat, sourceRectScaled.Size());
    if (!destination)
    {
      return nullptr;
    }
    if (_backend)
    {
      _backend->GetPixelBuffer(sourceRectScaled, *destination);
    }
    else
    {
      destination->ZeroFill();
    }
    return destination;
  }

  void ImageBuffer::PutPixelBuffer(const PixelBufferSourceView &pixelBuffer, const IntRect &sourceRect,
                                   const IntPoint &destinationPoint,
                                   AlphaPremultiplication destinationFormat) noexcept
  {
    krys_debug_assert(ResolutionScale() == 1);
    if (!_backend)
    {
      return;
    }
    auto sourceRectScaled = sourceRect;
    sourceRectScaled.Scale(ResolutionScale());
    auto destinationPointScaled = destinationPoint;
    destinationPointScaled.Scale(ResolutionScale());
    _backend->PutPixelBuffer(pixelBuffer, sourceRectScaled, destinationPointScaled, destinationFormat);
  }
}
