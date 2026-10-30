#include "Krystal.Booey/Graphics/ImageBufferBackend.hpp"
#include "Krystal.Booey/Graphics/DestinationColorSpace.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerContentsDisplayDelegate.hpp"
#include "Krystal.Booey/Graphics/ImagePaintingOptions.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Graphics/NativeImage.hpp"
#include "Krystal.Booey/Graphics/PixelBuffer.hpp"
#include "Krystal.Booey/Graphics/PixelBufferConversion.hpp"

namespace krys::boo
{
  ImageBufferBackend::ImageBufferBackend(const Parameters &parameters) noexcept : _parameters(parameters)
  {
  }

  ImageBufferBackend::~ImageBufferBackend() noexcept = default;

  AffineTransform ImageBufferBackend::CalculateBaseTransform(const Parameters &parameters) noexcept
  {
    AffineTransform baseTransform;
    baseTransform.Scale(parameters.ResolutionScale);
    return baseTransform;
  }

  RefPtr<GraphicsLayerContentsDisplayDelegate>
    ImageBufferBackend::LayerContentsDisplayDelegate() const noexcept
  {
    return nullptr;
  }

  RefPtr<NativeImage> ImageBufferBackend::CopyNativeImage() noexcept
  {
    return nullptr;
  }

  RefPtr<NativeImage> ImageBufferBackend::CreateNativeImageReference() noexcept
  {
    return CopyNativeImage();
  }

  RefPtr<NativeImage> ImageBufferBackend::SinkIntoNativeImage() noexcept
  {
    return CreateNativeImageReference();
  }

  uint32 ImageBufferBackend::BytesPerRow() const noexcept
  {
    return static_cast<uint32>(Size().Width()) * PixelBuffer::BytesPerPixel(GetPixelFormat());
  }

  const DestinationColorSpace &ImageBufferBackend::ColorSpace() const noexcept
  {
    return DestinationColorSpace::SRGB();
  }

  void ImageBufferBackend::GetPixelBuffer(const IntRect &, PixelBuffer &destination) noexcept
  {
    destination.ZeroFill();
  }

  void ImageBufferBackend::PutPixelBuffer(const PixelBufferSourceView &sourcePixelBuffer,
                                          const IntRect &sourceRect, const IntPoint &destinationPoint,
                                          AlphaPremultiplication) noexcept
  {
    IntRect backendRect {{}, Size()};
    auto sourceRectClipped = Intersection({IntPoint::Zero(), sourcePixelBuffer.Size()}, sourceRect);
    auto destinationRect = sourceRectClipped;
    destinationRect.MoveBy(destinationPoint);
    if (sourceRect.X() < 0)
    {
      destinationRect.SetX(destinationRect.X() - sourceRect.X());
    }
    if (sourceRect.Y() < 0)
    {
      destinationRect.SetY(destinationRect.Y() - sourceRect.Y());
    }
    destinationRect.Intersect(backendRect);
    sourceRectClipped.SetSize(destinationRect.Size());
    if (destinationRect.IsEmpty())
    {
      return;
    }

    // The pixels are drawn as an image: premultiplied ARGB (BGRA in memory), the destination's size.
    auto pixels = AdoptRef(*new PixelData(static_cast<size_t>(destinationRect.Size().UnclampedArea())));
    auto sourceBytesPerPixel = PixelBuffer::BytesPerPixel(sourcePixelBuffer.Format().Format);
    auto sourceBytesPerRow = static_cast<uint32>(sourcePixelBuffer.Size().Width()) * sourceBytesPerPixel;
    ConstPixelBufferConversionView source {
      sourcePixelBuffer.Format(), sourceBytesPerRow,
      sourcePixelBuffer.Bytes().subspan(static_cast<size_t>(sourceRectClipped.Y()) * sourceBytesPerRow
                                        + static_cast<size_t>(sourceRectClipped.X()) * sourceBytesPerPixel)};
    PixelBufferConversionView destination {
      {AlphaPremultiplication::Premultiplied, PixelFormat::BGRA8, ColorSpace()},
      static_cast<uint32>(destinationRect.Width()) * 4u,
      {reinterpret_cast<uint8 *>(pixels->Pixels.data()), pixels->Pixels.size() * sizeof(uint32)}};
    ConvertImagePixels(source, destination, destinationRect.Size());

    auto image =
      NativeImage::Create(AdoptRef(*new PlatformImage(destinationRect.Size(), krys::move(pixels), true)));
    if (!image)
    {
      return;
    }

    auto &context = Context();
    GraphicsContextStateSaver stateSaver(context);
    context.SetCTM(CalculateBaseTransform(_parameters));
    context.ResetClip();
    context.SetAlpha(1);
    context.ClearDropShadow();
    context.SetImageInterpolationQuality(InterpolationQuality::DoNotInterpolate);
    context.Clip(FloatRect(destinationRect));
    context.DrawNativeImage(*image, FloatRect(destinationRect),
                            FloatRect({}, FloatSize(destinationRect.Size())),
                            ImagePaintingOptions(CompositeOperator::Copy));
  }

  void ImageBufferBackend::GetPixelBuffer(const IntRect &sourceRect, Span<const uint8> sourceData,
                                          PixelBuffer &destinationPixelBuffer) noexcept
  {
    IntRect backendRect {{}, Size()};
    auto sourceRectClipped = Intersection(backendRect, sourceRect);
    IntRect destinationRect {IntPoint::Zero(), sourceRectClipped.Size()};

    if (sourceRect.X() < 0)
    {
      destinationRect.SetX(-sourceRect.X());
    }

    if (sourceRect.Y() < 0)
    {
      destinationRect.SetY(-sourceRect.Y());
    }

    if (destinationRect.Size() != sourceRect.Size())
    {
      destinationPixelBuffer.ZeroFill();
    }
    if (destinationRect.IsEmpty())
    {
      return;
    }

    auto sourcePixelFormat = GetPixelFormat();
    uint32 sourceBytesPerRow = BytesPerRow();
    ConstPixelBufferConversionView source {
      {AlphaPremultiplication::Premultiplied, sourcePixelFormat, ColorSpace()},
      sourceBytesPerRow,
      sourceData.subspan(static_cast<size_t>(sourceRectClipped.Y()) * sourceBytesPerRow
                         + static_cast<size_t>(sourceRectClipped.X())
                             * PixelBuffer::BytesPerPixel(sourcePixelFormat))};

    auto destinationBytesPerPixel = PixelBuffer::BytesPerPixel(destinationPixelBuffer.Format().Format);
    auto destinationBytesPerRow = static_cast<uint32>(sourceRect.Width()) * destinationBytesPerPixel;
    size_t offset = static_cast<size_t>(destinationRect.Y()) * destinationBytesPerRow
                    + static_cast<size_t>(destinationRect.X()) * destinationBytesPerPixel;
    if (offset > destinationPixelBuffer.Bytes().size())
    {
      return;
    }

    PixelBufferConversionView destination {destinationPixelBuffer.Format(), destinationBytesPerRow,
                                           destinationPixelBuffer.Bytes().subspan(offset)};

    ConvertImagePixels(source, destination, destinationRect.Size());
  }

  void ImageBufferBackend::PutPixelBuffer(const PixelBufferSourceView &sourcePixelBuffer,
                                          const IntRect &sourceRect, const IntPoint &destinationPoint,
                                          AlphaPremultiplication destinationAlphaFormat,
                                          Span<uint8> destinationData) noexcept
  {
    IntRect backendRect {{}, Size()};
    auto sourceRectClipped = Intersection({IntPoint::Zero(), sourcePixelBuffer.Size()}, sourceRect);
    auto destinationRect = sourceRectClipped;
    destinationRect.MoveBy(destinationPoint);

    if (sourceRect.X() < 0)
    {
      destinationRect.SetX(destinationRect.X() - sourceRect.X());
    }

    if (sourceRect.Y() < 0)
    {
      destinationRect.SetY(destinationRect.Y() - sourceRect.Y());
    }

    destinationRect.Intersect(backendRect);
    sourceRectClipped.SetSize(destinationRect.Size());
    if (destinationRect.IsEmpty())
    {
      return;
    }

    auto sourceBytesPerPixel = PixelBuffer::BytesPerPixel(sourcePixelBuffer.Format().Format);
    auto sourceBytesPerRow = static_cast<uint32>(sourcePixelBuffer.Size().Width()) * sourceBytesPerPixel;

    ConstPixelBufferConversionView source {
      sourcePixelBuffer.Format(), sourceBytesPerRow,
      sourcePixelBuffer.Bytes().subspan(static_cast<size_t>(sourceRectClipped.Y()) * sourceBytesPerRow
                                        + static_cast<size_t>(sourceRectClipped.X()) * sourceBytesPerPixel)};

    auto destinationPixelFormat = GetPixelFormat();
    uint32 destinationBytesPerRow = BytesPerRow();

    PixelBufferConversionView destination {
      {destinationAlphaFormat, destinationPixelFormat, ColorSpace()},
      destinationBytesPerRow,
      destinationData.subspan(static_cast<size_t>(destinationRect.Y()) * destinationBytesPerRow
                              + static_cast<size_t>(destinationRect.X())
                                  * PixelBuffer::BytesPerPixel(destinationPixelFormat))};

    ConvertImagePixels(source, destination, destinationRect.Size());
  }
}
