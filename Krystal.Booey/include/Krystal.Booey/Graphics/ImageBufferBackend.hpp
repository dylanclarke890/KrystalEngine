#pragma once

#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"

namespace krys::boo
{
  class GraphicsContext;

  /// @brief What an image buffer's backend was made for.
  struct ImageBufferBackendParameters
  {
    IntSize BackendSize;
    float ResolutionScale; // Resolution scale is of the ImageBuffer logical size.
  };

  /// @brief Where an image buffer's drawing goes: its graphics context and whatever holds what's drawn.
  /// FIXME: Pixel access (bytes per row, getting and putting pixel buffers), native images, volatility,
  /// colour spaces and pixel formats (sRGB, 8 bits a channel) and the bitmap and accelerated backends come
  /// with bitmap images.
  class ImageBufferBackend
  {
    KRYS_NON_COPY_MOVABLE(ImageBufferBackend);

  public:
    using Parameters = ImageBufferBackendParameters;

    struct Info
    {
      AffineTransform BaseTransform;
    };

  protected:
    Parameters _parameters;

  public:
    virtual ~ImageBufferBackend() noexcept;

    KRYS_NODISCARD static AffineTransform CalculateBaseTransform(const Parameters &) noexcept;

    KRYS_NODISCARD virtual GraphicsContext &Context() noexcept = 0;
    virtual void FlushContext() noexcept
    {
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

  protected:
    explicit ImageBufferBackend(const Parameters &) noexcept;
  };
}
