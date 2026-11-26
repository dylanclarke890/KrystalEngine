#pragma once

#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Core/Strings/String.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Web/Html/Canvas/GPUBasedCanvasRenderingContext.hpp"

namespace krys::web::html
{
  class EngineTexture;

  /// @brief A canvas context that shows a texture the host draws (`getContext("krystal-texture")`).
  /// @note The canvas's `width` and `height` are its natural size, whatever the texture's; the texture is
  /// scaled to it. A host that wants them to match resizes its texture when the canvas resizes.
  class EngineTextureRenderingContext final : public GPUBasedCanvasRenderingContext
  {
  private:
    RefPtr<EngineTexture> _texture;

  public:
    constexpr static utf8_stringview ContextType = u8"krystal-texture";

    explicit EngineTextureRenderingContext(CanvasBase &canvas) noexcept;

    ~EngineTextureRenderingContext() noexcept override;

    /// @brief Shows @p texture, or nothing if it's null.
    void SetTexture(RefPtr<EngineTexture> texture) noexcept;

    KRYS_NODISCARD EngineTexture *Texture() const noexcept
    {
      return _texture.get();
    }

    /// @brief Tells the canvas the host drew into @p rect of the texture, or all of it.
    void TextureChanged(const Maybe<RectI> &rect = null) noexcept;

    void Reshape() noexcept override
    {
    }

    KRYS_NODISCARD RefPtr<NativeImage> SurfaceBufferToNativeImage(SurfaceBuffer) noexcept override;

    KRYS_NODISCARD bool IsSurfaceBufferTransparentBlack(SurfaceBuffer) const noexcept override;

    KRYS_NODISCARD RefPtr<GraphicsLayerContentsDisplayDelegate>
      LayerContentsDisplayDelegate() noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_CANVAS_RENDERING_CONTEXT(krys::web::html::EngineTextureRenderingContext,
                                                     IsEngineTexture())
