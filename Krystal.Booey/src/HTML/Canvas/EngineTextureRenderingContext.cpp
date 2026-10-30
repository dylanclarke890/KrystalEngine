#include "Krystal.Booey/HTML/Canvas/EngineTextureRenderingContext.hpp"
#include "Krystal.Booey/Graphics/GeometryUtilities.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerContentsDisplayDelegate.hpp"
#include "Krystal.Booey/Graphics/NativeImage.hpp"
#include "Krystal.Booey/HTML/HTMLCanvasElement.hpp"
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

  void EngineTextureRenderingContext::TextureChanged(const Maybe<IntRect> &rect) noexcept
  {
    if (!_texture)
    {
      return;
    }

    auto &canvas = GetCanvasBase();
    FloatRect canvasRect {{}, FloatSize(canvas.Size())};
    if (!rect)
    {
      canvas.DidDraw(canvasRect);
      return;
    }

    // The texture is scaled to the canvas.
    FloatRect textureRect {{}, FloatSize(_texture->Size())};
    if (textureRect.IsEmpty())
    {
      return;
    }
    canvas.DidDraw(MapRect(FloatRect(*rect), textureRect, canvasRect));
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
