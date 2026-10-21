#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"

namespace krys::boo
{
  class GraphicsLayer;
  class GraphicsLayerFactory;
  class LocalFrame;

  /// @brief What a page asks of its host: the window's side of the page.
  // FIXME: The rest of the chrome client: windows and dialogs, focus, the status bar, tooltips, cursors,
  // scrolling and repainting the host's view, popups and the other compositing hooks (layer flushes,
  // compositing triggers, display refresh monitors), once there are any.
  class ChromeClient
  {
    KRYS_NON_COPY_MOVABLE(ChromeClient);

  public:
    ChromeClient() noexcept = default;
    virtual ~ChromeClient() noexcept = default;

    /// @brief Pass nullptr as the GraphicsLayer to detach the root layer.
    virtual void AttachRootGraphicsLayer(LocalFrame &, GraphicsLayer *) noexcept = 0;

    /// @brief The factory the page's graphics layers come from; null when the host doesn't composite, so the
    /// page isn't composited.
    KRYS_NODISCARD virtual GraphicsLayerFactory *GetGraphicsLayerFactory() const noexcept
    {
      return nullptr;
    }
  };

  /// @brief The chrome client of a page that has no host.
  class EmptyChromeClient final : public ChromeClient
  {
  public:
    void AttachRootGraphicsLayer(LocalFrame &, GraphicsLayer *) noexcept final
    {
    }
  };
}
