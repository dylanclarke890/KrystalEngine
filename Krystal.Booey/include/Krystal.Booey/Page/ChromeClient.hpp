#pragma once

#include "Krystal.Booey/Platform/PopupMenu.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class FormSubmission;
  class GraphicsLayer;
  class GraphicsLayerFactory;
  class LocalFrame;
  class PopupMenuClient;

  /// @brief What a page asks of its host: the window's side of the page.
  // FIXME: The rest of the chrome client: windows and dialogs, focus, the status bar, tooltips, cursors,
  // scrolling and repainting the host's view, search popups and the other compositing hooks (layer flushes,
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

    /// @brief Navigates to a form submission: its method, action, target and data. The page never navigates
    /// itself; a host that doesn't override this ignores submissions.
    virtual void SubmitForm(LocalFrame &frame, FormSubmission &submission) noexcept
    {
      (void)frame;
      (void)submission;
    }

    /// @brief Whether a menu list's items are shown left to right regardless of their direction.
    KRYS_NODISCARD virtual bool SelectItemWritingDirectionIsNatural() const noexcept
    {
      return false;
    }

    /// @brief Whether a menu list's text is aligned in its selected option's direction.
    KRYS_NODISCARD virtual bool SelectItemAlignmentFollowsMenuWritingDirection() const noexcept
    {
      return true;
    }

    /// @brief Makes the popup menu a menu list shows its options in; a host that doesn't override this has no
    /// popups, so menu lists don't open.
    KRYS_NODISCARD virtual RefPtr<PopupMenu> CreatePopupMenu(PopupMenuClient &client) const noexcept
    {
      (void)client;
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
