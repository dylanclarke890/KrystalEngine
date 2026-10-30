#pragma once

#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include "Krystal.Gfx/Render2D/FilterRenderer2D.hpp"
#include "Krystal.Gfx/Render2D/Renderer2D.hpp"

namespace krys::boo
{
  class ImageBuffer;
  class NativeImage;
}

namespace krys::ui
{
  class FontFaceCache;
  class ImageBufferRender2DBackend;

  /// @brief Renders Booey's image buffers into textures for a replay to draw, through renderers of its own
  /// that record into the replay's command buffer ahead of its passes: a buffer that records into a display
  /// list once per replay, and an accelerated buffer into the texture it keeps.
  /// @note Textures are pooled by size and reused by later replays: the device runs its commands in order, so
  /// a texture drawn in one replay is done with before the next draws into it again (OpenGL).
  /// @note Also uploads native images (decoded pixels) into textures, kept for as long as the images live.
  class ImageBufferTextures
  {
    KRYS_NON_COPY_MOVABLE(ImageBufferTextures);

  private:
    struct PooledTexture
    {
      gfx::TextureHandle Texture {};
      uint32 Width {0u};
      uint32 Height {0u};
      bool InUse {false};
    };

    gfx::Device &_device;
    FontFaceCache &_fonts;
    /// @brief One renderer per level of image buffers drawn inside image buffers.
    List<UniquePtr<gfx::Renderer2D>> _renderers;
    size_t _depth {0u};
    List<PooledTexture> _pool;
    /// @brief The buffers rendered in this replay, by rendering resource identifier.
    HashMap<uint64, gfx::TextureHandle> _rendered;

    struct UploadedImage
    {
      WeakPtr<boo::NativeImage> Image;
      gfx::TextureHandle Texture {};
    };
    /// @brief The native images uploaded, by rendering resource identifier.
    HashMap<uint64, UploadedImage> _uploaded;
    /// @brief Evaluates filter graphs, made on first use.
    UniquePtr<gfx::FilterRenderer2D> _filters;

  public:
    ImageBufferTextures(gfx::Device &device, FontFaceCache &fonts) noexcept;
    ~ImageBufferTextures() noexcept;

    /// @brief A texture holding what @p buffer recorded, at its backend size, rendered on first use in this
    /// replay into @p commands; invalid if it can't be.
    KRYS_NODISCARD gfx::TextureHandle Render(boo::ImageBuffer &buffer, gfx::CommandBuffer &commands) noexcept;
    /// @brief The texture an accelerated buffer keeps, with what's been flushed rendered into it in
    /// @p commands; invalid if it can't be.
    KRYS_NODISCARD gfx::TextureHandle Render(ImageBufferRender2DBackend &backend,
                                             gfx::CommandBuffer &commands) noexcept;

    /// @brief A texture of @p width by @p height holding @p source of @p texture, stretched to fill it; for
    /// pattern tiles that are part of an image.
    KRYS_NODISCARD gfx::TextureHandle Copy(gfx::TextureHandle texture, const gfx::Rect2D &source,
                                           uint32 width, uint32 height,
                                           gfx::CommandBuffer &commands) noexcept;

    /// @brief A texture holding @p image's pixels (premultiplied), uploaded on first use and kept until a
    /// replay ends after the image is gone; invalid if it can't be. An engine texture's image is its texture,
    /// premultiplied or not as its image says.
    KRYS_NODISCARD gfx::TextureHandle Upload(boo::NativeImage &image) noexcept;

    /// @brief The renderer that evaluates filter graphs into the replay's command buffer; null if it can't be
    /// made.
    KRYS_NODISCARD gfx::FilterRenderer2D *Filters() noexcept;

    /// @brief Ends a replay: its textures can be reused by the next, and the textures of native images that
    /// are gone are destroyed.
    void Release() noexcept;

  private:
    KRYS_NODISCARD gfx::TextureHandle Acquire(uint32 width, uint32 height) noexcept;
    /// @brief A renderer for the current level of nesting, made on first use.
    KRYS_NODISCARD gfx::Renderer2D *RendererForDepth() noexcept;
  };
}
