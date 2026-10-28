#pragma once

#include "Krystal.Booey/HTML/Canvas/EngineTexture.hpp"
#include "Krystal.Core/Colour/AlphaPremultiplication.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include "Krystal.Gfx/Handles.hpp"

namespace krys::boo
{
  class NativeImage;
}

namespace krys::ui
{
  class EngineTextureDisplayDelegate;

  /// @brief A texture the host draws, shown in a page by a `<canvas>`'s `"krystal-texture"` context.
  /// @note The texture belongs to the device the view paints with, and the host keeps it alive while it's
  /// shown. The host draws into it before the view paints, in the same frame, then calls the context's
  /// `TextureChanged`. Its colours are sRGB-encoded, as the renderer's images are.
  /// @code
  /// auto context = canvas.GetContext(boo::html::EngineTextureRenderingContext::ContextType);
  /// auto &textureContext = Downcast<boo::html::EngineTextureRenderingContext>(*context);
  /// textureContext.SetTexture(ui::EngineTexture::Create(texture, {width, height}));
  /// // Each frame, after drawing into the texture:
  /// textureContext.TextureChanged();
  /// @endcode
  class EngineTexture final : public boo::html::EngineTexture
  {
  private:
    gfx::TextureHandle _texture;
    boo::IntSize _size;
    AlphaPremultiplication _alpha;
    WeakPtr<EngineTextureDisplayDelegate> _delegate;
    WeakPtr<boo::NativeImage> _image;

  public:
    /// @param alpha Whether the texture's colours are already multiplied by their alpha.
    KRYS_NODISCARD static Ref<EngineTexture>
      Create(gfx::TextureHandle texture, const boo::IntSize &size,
             AlphaPremultiplication alpha = AlphaPremultiplication::Premultiplied) noexcept;

    KRYS_NODISCARD gfx::TextureHandle Handle() const noexcept
    {
      return _texture;
    }
    KRYS_NODISCARD AlphaPremultiplication Alpha() const noexcept
    {
      return _alpha;
    }

    KRYS_NODISCARD boo::IntSize Size() const noexcept override
    {
      return _size;
    }
    KRYS_NODISCARD RefPtr<boo::GraphicsLayerContentsDisplayDelegate>
      LayerContentsDisplayDelegate() noexcept override;
    KRYS_NODISCARD RefPtr<boo::NativeImage> GetNativeImage() noexcept override;

  private:
    EngineTexture(gfx::TextureHandle texture, const boo::IntSize &size,
                  AlphaPremultiplication alpha) noexcept;
  };
}
