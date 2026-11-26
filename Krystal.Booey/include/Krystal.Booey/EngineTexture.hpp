#pragma once

#include "Krystal.Core/Colour/AlphaPremultiplication.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Gfx/Handles.hpp"
#include "Krystal.Web/Html/Canvas/EngineTexture.hpp"

namespace krys::web
{
  class NativeImage;
}

namespace krys::boo
{
  class EngineTextureDisplayDelegate;

  /// @brief A texture the host draws, shown in a page by a `<canvas>`'s `"krystal-texture"` context.
  /// @note The texture belongs to the device the view paints with, and the host keeps it alive while it's
  /// shown. The host draws into it before the view paints, in the same frame, then calls the context's
  /// `TextureChanged`. Its colours are sRGB-encoded, as the renderer's images are.
  /// @code
  /// auto context = canvas.GetContext(web::html::EngineTextureRenderingContext::ContextType);
  /// auto &textureContext = Downcast<web::html::EngineTextureRenderingContext>(*context);
  /// textureContext.SetTexture(boo::EngineTexture::Create(texture, {width, height}));
  /// // Each frame, after drawing into the texture:
  /// textureContext.TextureChanged();
  /// @endcode
  class EngineTexture final : public web::html::EngineTexture
  {
  private:
    gfx::TextureHandle _texture;
    SizeI _size;
    AlphaPremultiplication _alpha;
    WeakPtr<EngineTextureDisplayDelegate> _delegate;
    WeakPtr<web::NativeImage> _image;

  public:
    /// @param alpha Whether the texture's colours are already multiplied by their alpha.
    KRYS_NODISCARD static Ref<EngineTexture>
      Create(gfx::TextureHandle texture, const SizeI &size,
             AlphaPremultiplication alpha = AlphaPremultiplication::Premultiplied) noexcept;

    KRYS_NODISCARD gfx::TextureHandle Handle() const noexcept
    {
      return _texture;
    }

    KRYS_NODISCARD AlphaPremultiplication Alpha() const noexcept
    {
      return _alpha;
    }

    KRYS_NODISCARD SizeI Size() const noexcept override
    {
      return _size;
    }

    KRYS_NODISCARD RefPtr<web::GraphicsLayerContentsDisplayDelegate>
      LayerContentsDisplayDelegate() noexcept override;

    KRYS_NODISCARD RefPtr<web::NativeImage> GetNativeImage() noexcept override;

  private:
    EngineTexture(gfx::TextureHandle texture, const SizeI &size, AlphaPremultiplication alpha) noexcept;
  };
}
