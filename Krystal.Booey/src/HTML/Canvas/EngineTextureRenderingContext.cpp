#include "Krystal.Booey/HTML/Canvas/EngineTextureRenderingContext.hpp"
#include "Krystal.Booey/Graphics/Geometry/GeometryUtilities.hpp"
#include "Krystal.Booey/Graphics/Images/NativeImage.hpp"
#include "Krystal.Booey/Graphics/Layers/GraphicsLayerContentsDisplayDelegate.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLCanvasElement.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"

namespace krys::boo::html
{
  EngineTextureRenderingContext::EngineTextureRenderingContext(CanvasBase &canvas) noexcept
      : GPUBasedCanvasRenderingContext(canvas, Type::EngineTexture)
  {
  }

  void EngineTextureRenderingContext::SetTexture(RefPtr<EngineTexture> texture) noexcept
  {
    if (_texture == texture)
    {
      return;
    }
    _texture = krys::move(texture);

    // The layer shows the new texture's delegate.
    if (auto *canvas = HtmlCanvas())
    {
      canvas->InvalidateStyleAndLayerComposition();
      if (auto *renderer = canvas->GetRenderBox())
      {
        renderer->ContentChanged(ContentChangeType::Canvas);
      }
    }
    MarkCanvasChanged();
  }

  void EngineTextureRenderingContext::TextureChanged(const Maybe<RectI> &rect) noexcept
  {
    if (!_texture)
    {
      return;
    }

    auto &canvas = GetCanvasBase();
    RectF canvasRect {{}, SizeF(canvas.Size())};
    if (!rect)
    {
      canvas.DidDraw(canvasRect);
      return;
    }

    // The texture is scaled to the canvas.
    RectF textureRect {{}, SizeF(_texture->Size())};
    if (textureRect.IsEmpty())
    {
      return;
    }
    canvas.DidDraw(MapRect(RectF(*rect), textureRect, canvasRect));
  }

  RefPtr<NativeImage> EngineTextureRenderingContext::SurfaceBufferToNativeImage(SurfaceBuffer) noexcept
  {
    return _texture ? _texture->GetNativeImage() : nullptr;
  }

  bool EngineTextureRenderingContext::IsSurfaceBufferTransparentBlack(SurfaceBuffer) const noexcept
  {
    return !_texture;
  }

  RefPtr<GraphicsLayerContentsDisplayDelegate>
    EngineTextureRenderingContext::LayerContentsDisplayDelegate() noexcept
  {
    return _texture ? _texture->LayerContentsDisplayDelegate() : nullptr;
  }
}
