#pragma once

#include "Krystal.Booey/HTML/CanvasBase.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class DestinationColorSpace;
  class GraphicsLayer;
  class GraphicsLayerContentsDisplayDelegate;
  class ImageBuffer;
  class NativeImage;
}

namespace krys::boo::html
{
  /// @brief A canvas's rendering context, which draws its pixels. Owned by its canvas; references to it keep
  /// the canvas alive.
  class CanvasRenderingContext : public CanMakeWeakPtr<CanvasRenderingContext>
  {
    KRYS_NON_COPY_MOVABLE(CanvasRenderingContext);

  protected:
    enum class Type : uint8
    {
      CanvasElement2D,
      EngineTexture,
    };

  private:
    CanvasBase &_canvas;
    const Type _type;
    bool _isInPreparationForDisplayOrFlush {false};

  public:
    virtual ~CanvasRenderingContext() noexcept = default;

    void AddRef() const noexcept
    {
      _canvas.AddRef();
    }
    void SubRef() const noexcept
    {
      _canvas.SubRef();
    }
    KRYS_NODISCARD uint32 GetRefCount() const noexcept
    {
      return _canvas.GetRefCount();
    }

    KRYS_NODISCARD CanvasBase &GetCanvasBase() const noexcept
    {
      return _canvas;
    }

    KRYS_NODISCARD bool Is2d() const noexcept
    {
      return _type == Type::CanvasElement2D;
    }
    KRYS_NODISCARD bool IsEngineTexture() const noexcept
    {
      return _type == Type::EngineTexture;
    }
    /// @brief Whether the context draws on the GPU, into a surface the compositor shows as is.
    KRYS_NODISCARD bool IsGPUBased() const noexcept
    {
      return IsEngineTexture();
    }

    /// @brief Whether the context's pixels are shown as its canvas's layer contents rather than painted.
    KRYS_NODISCARD bool DelegatesDisplay() const noexcept;

    // A GPU-based context draws to its drawing buffer, which becomes the display buffer the compositor shows.
    // Readbacks read the drawing buffer; painting reads the display buffer.
    enum class SurfaceBuffer : uint8
    {
      DrawingBuffer,
      DisplayBuffer
    };

    /// @brief Forgets the area drawn since the canvas was last painted.
    virtual void ClearAccumulatedDirtyRect() noexcept
    {
    }

    /// @brief The image buffer holding @p buffer's pixels: the canvas's, unless the context keeps its own.
    KRYS_NODISCARD virtual RefPtr<ImageBuffer> SurfaceBufferToImageBuffer(SurfaceBuffer) noexcept;
    /// @brief The pixels of @p buffer, as an image, or null if there are none.
    KRYS_NODISCARD virtual RefPtr<NativeImage> SurfaceBufferToNativeImage(SurfaceBuffer) noexcept;
    KRYS_NODISCARD virtual bool IsSurfaceBufferTransparentBlack(SurfaceBuffer) const noexcept;

    /// @brief What shows the context's pixels as a layer's contents.
    KRYS_NODISCARD virtual RefPtr<GraphicsLayerContentsDisplayDelegate>
      LayerContentsDisplayDelegate() noexcept;
    virtual void SetContentsToLayer(GraphicsLayer &layer) noexcept;

    /// @brief Whether drawing waits to be flushed in the rendering update.
    KRYS_NODISCARD virtual bool HasDeferredOperations() const noexcept
    {
      return false;
    }
    virtual void FlushDeferredOperations() noexcept
    {
    }

    KRYS_NODISCARD virtual bool CompositingResultsNeedUpdating() const noexcept
    {
      return false;
    }
    /// @brief Whether the context has work to do after the rendering update and before its pixels are
    /// composited.
    KRYS_NODISCARD virtual bool NeedsPreparationForDisplay() const noexcept
    {
      return false;
    }
    virtual void PrepareForDisplay() noexcept
    {
    }

    void SetIsInPreparationForDisplayOrFlush(bool flag) noexcept
    {
      _isInPreparationForDisplayOrFlush = flag;
    }
    KRYS_NODISCARD bool IsInPreparationForDisplayOrFlush() const noexcept
    {
      return _isInPreparationForDisplayOrFlush;
    }

    // FIXME: The pixel format (sRGB, 8 bits a channel).
    KRYS_NODISCARD virtual DestinationColorSpace ColorSpace() const noexcept;
    KRYS_NODISCARD virtual bool WillReadFrequently() const noexcept;

  protected:
    CanvasRenderingContext(CanvasBase &canvas, Type type) noexcept;
  };
}

#define KRYS_SPECIALIZE_TYPE_TRAITS_CANVAS_RENDERING_CONTEXT(ToValueTypeName, predicate)                     \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(ToValueTypeName)                                                    \
    KRYS_NODISCARD static bool IsType(const krys::boo::html::CanvasRenderingContext &context) noexcept       \
    {                                                                                                        \
      return context.predicate;                                                                              \
    }                                                                                                        \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
