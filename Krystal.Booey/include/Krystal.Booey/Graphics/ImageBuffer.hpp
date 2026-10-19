#pragma once

#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/ImageBufferBackend.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Graphics/RenderingResourceIdentifier.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class FloatRect;
  class GraphicsContext;

  /// @brief What an image buffer is made with: its logical size and how many backend pixels a unit has.
  struct ImageBufferParameters
  {
    FloatSize LogicalSize;
    float ResolutionScale;
  };

  /// @brief An offscreen surface drawn into through a graphics context, then drawn (or tiled) as an image.
  /// FIXME: Rendering modes and purposes (every buffer records into a display list; there are no bitmap or
  /// accelerated backends), colour spaces and pixel formats, native images, pixel buffers, filters,
  /// luminance masks, volatility, flushing, cloning, serialisation and data URLs come with bitmap images.
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
    /// @brief A buffer of @p size logical units, @p resolutionScale backend pixels a unit; null when the
    /// backend size is empty.
    KRYS_NODISCARD static RefPtr<ImageBuffer> Create(const FloatSize &size, float resolutionScale) noexcept;

    ~ImageBuffer() noexcept;

    KRYS_NODISCARD static IntSize CalculateBackendSize(FloatSize logicalSize, float resolutionScale) noexcept;
    KRYS_NODISCARD static ImageBufferBackendParameters BackendParameters(const Parameters &) noexcept;

    KRYS_NODISCARD static bool SizeNeedsClamping(const FloatSize &) noexcept;
    KRYS_NODISCARD static bool SizeNeedsClamping(const FloatSize &, FloatSize &scale) noexcept;

    KRYS_NODISCARD GraphicsContext &Context() const noexcept;
    void FlushDrawingContext() noexcept;

    KRYS_NODISCARD IntSize BackendSize() const noexcept;

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
