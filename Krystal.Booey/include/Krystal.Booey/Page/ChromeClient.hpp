#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Page/ConsoleTypes.hpp"
#include "Krystal.Booey/Page/PointerLockController.hpp"
#include "Krystal.Booey/Platform/ColorChooser.hpp"
#include "Krystal.Booey/Platform/DateTimeChooser.hpp"
#include "Krystal.Booey/Platform/PopupMenu.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::svg
{
  class SVGImageElement;
}

namespace krys::boo
{
  class GraphicsLayer;
  class GraphicsLayerFactory;
  class IntRect;
  class LocalFrame;
  class Color;
  class ColorChooserClient;
  class DateTimeChooserClient;
  class PopupMenuClient;

  /// @brief What a page asks of its host: the window's side of the page.
  // FIXME: The rest of the chrome client: windows and dialogs, the status bar, tooltips, cursors,
  // scrolling the host's view, search popups and the other compositing hooks (layer flushes, compositing
  // triggers, display refresh monitors), once there are any.
  class ChromeClient
  {
    KRYS_NON_COPY_MOVABLE(ChromeClient);

  public:
    ChromeClient() noexcept = default;
    virtual ~ChromeClient() noexcept = default;

    /// @brief The page asks to be brought to the front (`window.focus()`); the host decides whether it is.
    virtual void Focus() noexcept
    {
    }

    /// @brief The page asks to be closed (`window.close()`), as a task after the request; the host closes its
    /// view (the page may be destroyed), or ignores it.
    virtual void CloseWindow() noexcept
    {
    }

    /// @brief The host's window's position on the screen and size, in CSS pixels; empty if there's no window.
    KRYS_NODISCARD virtual FloatRect WindowRect() const noexcept
    {
      return {};
    }

    /// @brief The page asks to move or resize the host's window (`moveTo()`, `resizeTo()` and the others),
    /// to @p rect, already kept on the screen; the host decides.
    virtual void SetWindowRect(const FloatRect &rect) noexcept
    {
      (void)rect;
    }

    /// @brief The smallest size the page may resize the host's window to.
    KRYS_NODISCARD virtual FloatSize MinimumWindowSize() const noexcept
    {
      return {100, 100};
    }

    /// @brief Shows @p message to the user (`alert()`), waiting until they dismiss it if the host can.
    virtual void RunJavaScriptAlert(LocalFrame &frame, const dom::DOMString &message) noexcept
    {
      (void)frame;
      (void)message;
    }

    /// @brief Asks the user to confirm @p message (`confirm()`).
    /// @return Whether they did: false without a host.
    KRYS_NODISCARD virtual bool RunJavaScriptConfirm(LocalFrame &frame,
                                                     const dom::DOMString &message) noexcept
    {
      (void)frame;
      (void)message;
      return false;
    }

    /// @brief Asks the user for text (`prompt()`), with @p message and @p defaultValue.
    /// @return What they entered; null if they cancelled, or without a host.
    KRYS_NODISCARD virtual Maybe<dom::DOMString>
      RunJavaScriptPrompt(LocalFrame &frame, const dom::DOMString &message,
                          const dom::DOMString &defaultValue) noexcept
    {
      (void)frame;
      (void)message;
      (void)defaultValue;
      return null;
    }

    /// @brief The page asks the user to confirm leaving it (its `beforeunload` was cancelled), with
    /// @p message (its `returnValue`).
    /// @return Whether to leave: true without a host.
    KRYS_NODISCARD virtual bool RunBeforeUnloadConfirmPanel(const dom::DOMString &message,
                                                            LocalFrame &frame) noexcept
    {
      (void)message;
      (void)frame;
      return true;
    }

    /// @brief The page asks to be printed (`print()`); the host decides.
    virtual void Print(LocalFrame &frame) noexcept
    {
      (void)frame;
    }

    /// @brief The page asks to handle @p scheme's URLs with @p url (`%s` stands for the URL handled); the
    /// host decides.
    virtual void RegisterProtocolHandler(const dom::DOMString &scheme, const dom::DOMString &url) noexcept
    {
      (void)scheme;
      (void)url;
    }

    /// @brief The page no longer handles @p scheme's URLs with @p url.
    virtual void UnregisterProtocolHandler(const dom::DOMString &scheme, const dom::DOMString &url) noexcept
    {
      (void)scheme;
      (void)url;
    }

    /// @brief A message for the developer console: an error the page didn't handle, a warning.
    virtual void AddMessageToConsole(MessageSource source, MessageLevel level, const dom::DOMString &message,
                                     uint32 lineNumber, uint32 columnNumber,
                                     const dom::DOMString &sourceID) noexcept
    {
      (void)source;
      (void)level;
      (void)message;
      (void)lineNumber;
      (void)columnNumber;
      (void)sourceID;
    }

    /// @brief Called when @p rect (in the main frame's view) needs repainting.
    virtual void InvalidateContentsAndRootView(const IntRect &rect) noexcept
    {
      (void)rect;
    }

    /// @brief Asks the client to schedule the page's next rendering update.
    /// @return Whether the client scheduled it; if not, the page waits for the host's next frame.
    virtual bool ScheduleRenderingUpdate() noexcept
    {
      return false;
    }

    /// @brief Whether this is the chrome client of an SVG image's page.
    KRYS_NODISCARD virtual bool IsSVGImageChromeClient() const noexcept
    {
      return false;
    }

    /// @brief Called when an `<image>` in the page has loaded its image.
    virtual void DidFinishLoadingImageForSVGImage(svg::SVGImageElement &) noexcept
    {
    }

    /// @brief Pass nullptr as the GraphicsLayer to detach the root layer.
    virtual void AttachRootGraphicsLayer(LocalFrame &, GraphicsLayer *) noexcept = 0;

    /// @brief The factory the page's graphics layers come from; null when the host doesn't composite, so the
    /// page isn't composited.
    KRYS_NODISCARD virtual GraphicsLayerFactory *GetGraphicsLayerFactory() const noexcept
    {
      return nullptr;
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

    /// @brief Makes and shows the picker a colour input chooses its colour in, with @p initialColor chosen; a
    /// host that doesn't override this has no colour pickers.
    KRYS_NODISCARD virtual RefPtr<ColorChooser> CreateColorChooser(ColorChooserClient &client,
                                                                   const Color &initialColor) noexcept
    {
      (void)client;
      (void)initialColor;
      return nullptr;
    }

    /// @brief Makes the picker a date input chooses its date in; a host that doesn't override this has no
    /// date pickers.
    KRYS_NODISCARD virtual RefPtr<DateTimeChooser>
      CreateDateTimeChooser(DateTimeChooserClient &client) noexcept
    {
      (void)client;
      return nullptr;
    }

    /// @brief Whether the system has a mouse, whose movement the host can report before the system's
    /// acceleration (pointer lock's unadjusted movement).
    KRYS_NODISCARD virtual bool HasAccessoryMousePointingDevice() const noexcept
    {
      return false;
    }

    /// @brief The page asks to lock the pointer: the host hides the cursor, keeps it where it is and reports
    /// the mouse's movement, then calls @p completionHandler with whether it did; a host that doesn't
    /// override this has no pointer lock.
    virtual void RequestPointerLock(Func<void(PointerLockRequestResult)> &&completionHandler) noexcept
    {
      completionHandler(PointerLockRequestResult::Unsupported);
    }

    /// @brief The page asks to unlock the pointer: the host shows the cursor again, then calls
    /// @p completionHandler with whether it did.
    virtual void RequestPointerUnlock(Func<void(bool)> &&completionHandler) noexcept
    {
      completionHandler(false);
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
