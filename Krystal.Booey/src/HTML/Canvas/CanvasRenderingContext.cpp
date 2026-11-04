#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext.hpp"
#include "Krystal.Booey/Graphics/Color/DestinationColorSpace.hpp"
#include "Krystal.Booey/Graphics/ImageBuffers/ImageBuffer.hpp"
#include "Krystal.Booey/Graphics/Images/NativeImage.hpp"
#include "Krystal.Booey/Graphics/Layers/GraphicsLayer.hpp"
#include "Krystal.Booey/Graphics/Layers/GraphicsLayerContentsDisplayDelegate.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext2DBase.hpp"

namespace krys::boo::html
{
  CanvasRenderingContext::CanvasRenderingContext(CanvasBase &canvas, Type type) noexcept
      : _canvas(canvas), _type(type)
  {
  }

  bool CanvasRenderingContext::DelegatesDisplay() const noexcept
  {
    if (auto *context2D = DynamicDowncast<CanvasRenderingContext2DBase>(*this))
    {
      return context2D->IsAccelerated();
    }
    return IsGPUBased();
  }

  RefPtr<ImageBuffer> CanvasRenderingContext::SurfaceBufferToImageBuffer(SurfaceBuffer) noexcept
  {
    // This will be removed once all contexts store their own buffers.
    return ShareRefPtr(_canvas.Buffer());
  }

  RefPtr<NativeImage> CanvasRenderingContext::SurfaceBufferToNativeImage(SurfaceBuffer) noexcept
  {
    return nullptr;
  }

  bool CanvasRenderingContext::IsSurfaceBufferTransparentBlack(SurfaceBuffer) const noexcept
  {
    return false;
  }

  RefPtr<GraphicsLayerContentsDisplayDelegate> CanvasRenderingContext::LayerContentsDisplayDelegate() noexcept
  {
    return nullptr;
  }

  void CanvasRenderingContext::SetContentsToLayer(GraphicsLayer &layer) noexcept
  {
    layer.SetContentsDisplayDelegate(LayerContentsDisplayDelegate(),
                                     GraphicsLayer::ContentsLayerPurpose::Canvas);
  }

  DestinationColorSpace CanvasRenderingContext::ColorSpace() const noexcept
  {
    return DestinationColorSpace::SRGB();
  }

  bool CanvasRenderingContext::WillReadFrequently() const noexcept
  {
    return false;
  }
}
