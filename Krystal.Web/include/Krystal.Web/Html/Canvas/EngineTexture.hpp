#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Geometry/Size.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"

namespace krys::web
{
  class GraphicsLayerContentsDisplayDelegate;
  class NativeImage;
}

namespace krys::web::html
{
  /// @brief A texture the host draws, shown by a canvas's engine texture context.
  /// @note The host implements it with its renderer's texture: the compositor shows it as a layer's contents
  /// through its display delegate, and painting draws it as an image. The host draws into it before the page
  /// is shown, then tells the context what it changed.
  class EngineTexture : public RefCounted<EngineTexture>
  {
    KRYS_NON_COPY_MOVABLE(EngineTexture);

  public:
    virtual ~EngineTexture() noexcept = default;

    /// @brief The texture's size in pixels.
    KRYS_NODISCARD virtual SizeI Size() const noexcept = 0;

    /// @brief What shows the texture as a layer's contents in the host's compositor.
    KRYS_NODISCARD virtual RefPtr<GraphicsLayerContentsDisplayDelegate>
      LayerContentsDisplayDelegate() noexcept = 0;

    /// @brief The texture as an image, drawn when its canvas is painted rather than composited.
    KRYS_NODISCARD virtual RefPtr<NativeImage> GetNativeImage() noexcept = 0;

  protected:
    EngineTexture() noexcept = default;
  };
}
