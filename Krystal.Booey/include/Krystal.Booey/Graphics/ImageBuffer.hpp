#pragma once

#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/ImageBufferBackend.hpp"
#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Graphics/RenderingResourceIdentifier.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <utility>

namespace krys::boo
{
  class DestinationColorSpace;
  class FloatRect;
  class GraphicsClient;
  class GraphicsContext;
  class GraphicsLayerContentsDisplayDelegate;
  class IntRect;
  class NativeImage;
  class PixelBuffer;
  class PixelBufferSourceView;
  struct PixelBufferFormat;

  /// @brief What an image buffer is made with: its logical size, how many backend pixels a unit has, and what
  /// it's for.
  struct ImageBufferParameters
  {
    FloatSize LogicalSize;
    float ResolutionScale;
    RenderingPurpose Purpose {RenderingPurpose::Unspecified};
  };

  /// @brief An offscreen surface drawn into through a graphics context, then drawn (or tiled) as an image.
  /// @note The host makes accelerated buffers with its own backend (`GraphicsClient`); without one, and for
  /// every other mode, the buffer records into a display list.
  /// FIXME: The bitmap backend, colour spaces and pixel formats, volatility, cloning and serialisation come
  /// with bitmap images, and data URLs with B14e.
  class ImageBuffer final : public RefCounted<ImageBuffer>, public CanMakeWeakPtr<ImageBuffer>
  {
  public:
    using Parameters = ImageBufferParameters;

  private:
    Parameters _parameters;
    ImageBufferBackend::Info _backendInfo;
    UniquePtr<ImageBufferBackend> _backend;
    RenderingResourceIdentifier _renderingResourceIdentifier;

  public:
    /// @brief A buffer of @p size logical units, @p resolutionScale backend pixels a unit, drawn as @p mode
    /// asks: by @p graphicsClient's backend when it has one, otherwise into a display list; null when the
    /// backend size is empty.
    KRYS_NODISCARD static RefPtr<ImageBuffer> Create(const FloatSize &size, RenderingMode mode,
                                                     RenderingPurpose purpose, float resolutionScale,
                                                     const DestinationColorSpace &colorSpace,
                                                     GraphicsClient *graphicsClient = nullptr) noexcept;
    /// @brief An unaccelerated buffer for no purpose in particular, in sRGB.
    KRYS_NODISCARD static RefPtr<ImageBuffer> Create(const FloatSize &size, float resolutionScale) noexcept;

    /// @brief A buffer whose backend is a @p TBackend, made from the backend parameters and @p arguments;
    /// null when the backend size is empty or the backend can't be made.
    template <typename TBackend, typename... TArguments>
    KRYS_NODISCARD static RefPtr<ImageBuffer> Create(const FloatSize &size, float resolutionScale,
                                                     RenderingPurpose purpose,
                                                     TArguments &&...arguments) noexcept
    {
      Parameters parameters {size, resolutionScale, purpose};
      auto backendParameters = BackendParameters(parameters);
      if (backendParameters.BackendSize.IsEmpty())
      {
        return nullptr;
      }
      UniquePtr<ImageBufferBackend> backend =
        TBackend::Create(backendParameters, std::forward<TArguments>(arguments)...);
      if (!backend)
      {
        return nullptr;
      }
      ImageBufferBackend::Info backendInfo {TBackend::BackendRenderingMode,
                                            ImageBufferBackend::CalculateBaseTransform(backendParameters)};
      return AdoptRef(*new ImageBuffer(parameters, backendInfo, krys::move(backend)));
    }

    ~ImageBuffer() noexcept;

    KRYS_NODISCARD static IntSize CalculateBackendSize(FloatSize logicalSize, float resolutionScale) noexcept;
    KRYS_NODISCARD static ImageBufferBackendParameters BackendParameters(const Parameters &) noexcept;

    KRYS_NODISCARD static bool SizeNeedsClamping(const FloatSize &) noexcept;
    KRYS_NODISCARD static bool SizeNeedsClamping(const FloatSize &, FloatSize &scale) noexcept;

    KRYS_NODISCARD GraphicsContext &Context() const noexcept;
    void FlushDrawingContext() noexcept;
    /// @brief Flushes what's been drawn without waiting for it to be done; returns whether it was flushed.
    bool FlushDrawingContextAsync() noexcept;

    /// @brief Makes what's been drawn ready for the compositor, before it composites.
    void PrepareForDisplay() noexcept;
    /// @brief What shows the buffer as a layer's contents; null when it's painted instead.
    KRYS_NODISCARD RefPtr<GraphicsLayerContentsDisplayDelegate> LayerContentsDisplayDelegate() const noexcept;

    KRYS_NODISCARD IntSize BackendSize() const noexcept;

    /// @brief An immutable copy of what's been drawn; null if the backend can't make one.
    KRYS_NODISCARD RefPtr<NativeImage> CopyNativeImage() const noexcept;
    /// @brief An image of what's been drawn that may follow later drawing; for uses that end before the next
    /// draw.
    KRYS_NODISCARD RefPtr<NativeImage> CreateNativeImageReference() const noexcept;
    /// @brief An image of what's been drawn, as the buffer goes.
    KRYS_NODISCARD RefPtr<NativeImage> SinkIntoNativeImage() noexcept;
    KRYS_NODISCARD static RefPtr<NativeImage> SinkIntoNativeImage(RefPtr<ImageBuffer>) noexcept;

    /// @brief Reads @p srcRect (in logical units) into a new pixel buffer of @p outputFormat; what's outside
    /// the buffer is transparent black. Null if the pixel buffer can't be made.
    KRYS_NODISCARD RefPtr<PixelBuffer> GetPixelBuffer(const PixelBufferFormat &outputFormat,
                                                      const IntRect &srcRect) const noexcept;
    /// @brief What's been drawn, encoded as @p mimeType (PNG, JPEG or WebP; a JPEG is drawn over black) as a
    /// `data:` URL; `data:,` if it can't be.
    KRYS_NODISCARD utf8_string ToDataURL(utf8_stringview mimeType, Maybe<double> quality = {}) const noexcept;
    /// @brief What's been drawn, encoded as @p mimeType (PNG, JPEG or WebP; a JPEG is drawn over black);
    /// empty if it can't be.
    KRYS_NODISCARD List<uint8> ToData(utf8_stringview mimeType, Maybe<double> quality = {}) const noexcept;
    KRYS_NODISCARD static utf8_string ToDataURL(Ref<ImageBuffer> source, utf8_stringview mimeType,
                                                Maybe<double> quality = {}) noexcept;
    KRYS_NODISCARD static List<uint8> ToData(Ref<ImageBuffer> source, utf8_stringview mimeType,
                                             Maybe<double> quality = {}) noexcept;

    /// @brief Writes @p srcRect of @p pixelBuffer at @p destPoint, replacing what's there, whatever the
    /// context's state.
    void PutPixelBuffer(const PixelBufferSourceView &pixelBuffer, const IntRect &srcRect,
                        const IntPoint &destPoint = {},
                        AlphaPremultiplication destFormat = AlphaPremultiplication::Premultiplied) noexcept;

    /// @brief Turns what's drawn into a luminance mask (for `mask-mode: luminance`).
    void ConvertToLuminanceMask() noexcept;

    KRYS_NODISCARD RenderingResourceIdentifier GetRenderingResourceIdentifier() const noexcept
    {
      return _renderingResourceIdentifier;
    }

    KRYS_NODISCARD FloatSize LogicalSize() const noexcept
    {
      return _parameters.LogicalSize;
    }
    KRYS_NODISCARD IntSize TruncatedLogicalSize() const noexcept // You probably should be calling
                                                                 // logicalSize() instead.
    {
      return IntSize(_parameters.LogicalSize);
    }
    KRYS_NODISCARD float ResolutionScale() const noexcept
    {
      return _parameters.ResolutionScale;
    }
    KRYS_NODISCARD const Parameters &GetParameters() const noexcept
    {
      return _parameters;
    }
    KRYS_NODISCARD RenderingPurpose Purpose() const noexcept
    {
      return _parameters.Purpose;
    }
    KRYS_NODISCARD RenderingMode GetRenderingMode() const noexcept
    {
      return _backendInfo.Mode;
    }
    KRYS_NODISCARD AffineTransform BaseTransform() const noexcept
    {
      return _backendInfo.BaseTransform;
    }

    /// @brief The backend holding what's drawn, for drawing backends that render it (Krystal.UI reads the
    /// display list a buffer recorded).
    KRYS_NODISCARD ImageBufferBackend *Backend() const noexcept
    {
      return _backend.get();
    }

  private:
    ImageBuffer(const Parameters &, const ImageBufferBackend::Info &, UniquePtr<ImageBufferBackend> &&,
                RenderingResourceIdentifier = RenderingResourceIdentifier::Generate()) noexcept;
  };
}
