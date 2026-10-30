#pragma once

#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Graphics/PixelBufferFormat.hpp"
#include "Krystal.Booey/Graphics/RenderingMode.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo
{
  class GraphicsContext;
  class GraphicsLayerContentsDisplayDelegate;
  class IntPoint;
  class IntRect;
  class NativeImage;
  class PixelBuffer;
  class PixelBufferSourceView;

  /// @brief What an image buffer's backend was made for.
  struct ImageBufferBackendParameters
  {
    IntSize BackendSize;
    float ResolutionScale; // Resolution scale is of the ImageBuffer logical size.
  };

  /// @brief Where an image buffer's drawing goes: its graphics context and whatever holds what's drawn.
  /// @note A backend's pixels are premultiplied sRGB RGBA, 8 bits a channel.
  /// FIXME: Volatility, colour spaces and pixel formats other than sRGB RGBA8, and the bitmap backend come
  /// with bitmap images.
  class ImageBufferBackend
  {
    KRYS_NON_COPY_MOVABLE(ImageBufferBackend);

  public:
    using Parameters = ImageBufferBackendParameters;

    struct Info
    {
      RenderingMode Mode;
      AffineTransform BaseTransform;
    };

  protected:
    Parameters _parameters;
    bool _isLuminanceMask {false};

  public:
    virtual ~ImageBufferBackend() noexcept;

    KRYS_NODISCARD static AffineTransform CalculateBaseTransform(const Parameters &) noexcept;

    KRYS_NODISCARD virtual GraphicsContext &Context() noexcept = 0;
    virtual void FlushContext() noexcept
    {
    }

    /// @brief Makes what's been drawn ready for the compositor to show, before it composites.
    virtual void PrepareForDisplay() noexcept
    {
    }
    /// @brief What shows the buffer as a layer's contents; null when it's painted instead.
    KRYS_NODISCARD virtual RefPtr<GraphicsLayerContentsDisplayDelegate>
      LayerContentsDisplayDelegate() const noexcept;

    /// @brief An immutable copy of what's been drawn; null if the backend can't make one.
    KRYS_NODISCARD virtual RefPtr<NativeImage> CopyNativeImage() noexcept;
    /// @brief An image of what's been drawn that may follow later drawing; for uses that end before the next
    /// draw.
    KRYS_NODISCARD virtual RefPtr<NativeImage> CreateNativeImageReference() noexcept;
    /// @brief An image of what's been drawn, as the buffer goes.
    KRYS_NODISCARD virtual RefPtr<NativeImage> SinkIntoNativeImage() noexcept;

    /// @brief Reads @p srcRect of the backend's pixels into @p destination, converted to its format; what's
    /// outside the backend is transparent black.
    /// @note What's drawn can't be read back from a display list, so it reads transparent black.
    virtual void GetPixelBuffer(const IntRect &srcRect, PixelBuffer &destination) noexcept;
    /// @brief Writes @p srcRect of @p source at @p destPoint, replacing what's there, ignoring the context's
    /// state (its transform, clip, alpha, shadow and compositing).
    /// @note It's drawn into the context as an image, in order with what's drawn before and after.
    virtual void PutPixelBuffer(const PixelBufferSourceView &source, const IntRect &srcRect,
                                const IntPoint &destPoint, AlphaPremultiplication destFormat) noexcept;

    KRYS_NODISCARD virtual uint32 BytesPerRow() const noexcept;
    KRYS_NODISCARD const DestinationColorSpace &ColorSpace() const noexcept;
    KRYS_NODISCARD PixelFormat GetPixelFormat() const noexcept
    {
      return PixelFormat::RGBA8;
    }

    KRYS_NODISCARD IntSize Size() const noexcept
    {
      return _parameters.BackendSize;
    }
    KRYS_NODISCARD float ResolutionScale() const noexcept
    {
      return _parameters.ResolutionScale;
    }
    KRYS_NODISCARD const Parameters &GetParameters() const noexcept
    {
      return _parameters;
    }

    /// @brief Turns what's drawn into a luminance mask: alpha is the luminance times the alpha, the colour
    /// black. The drawing backend applies it when it renders the buffer.
    void ConvertToLuminanceMask() noexcept
    {
      _isLuminanceMask = true;
    }
    KRYS_NODISCARD bool IsLuminanceMask() const noexcept
    {
      return _isLuminanceMask;
    }

  protected:
    explicit ImageBufferBackend(const Parameters &) noexcept;

    /// @brief Reads @p srcRect of @p sourceData, the backend's pixels, into @p destination.
    void GetPixelBuffer(const IntRect &srcRect, Span<const uint8> sourceData,
                        PixelBuffer &destination) noexcept;
    /// @brief Writes @p srcRect of @p source at @p destPoint into @p destinationData, the backend's pixels.
    void PutPixelBuffer(const PixelBufferSourceView &source, const IntRect &srcRect,
                        const IntPoint &destPoint, AlphaPremultiplication destFormat,
                        Span<uint8> destinationData) noexcept;
  };
}
