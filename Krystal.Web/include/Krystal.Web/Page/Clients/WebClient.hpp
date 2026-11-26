#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Web/Dom/Types/DOMString.hpp"
#include "Krystal.Web/Input/PointerEvents/PointerCharacteristics.hpp"
#include "Krystal.Web/Input/PointerLock/PointerLockController.hpp"
#include "Krystal.Web/Platform/Choosers/ColorChooser.hpp"
#include "Krystal.Web/Platform/Choosers/DataListSuggestionPicker.hpp"
#include "Krystal.Web/Platform/Choosers/DateTimeChooser.hpp"
#include "Krystal.Web/Platform/Choosers/FileChooser.hpp"
#include "Krystal.Web/Platform/Choosers/PopupMenu.hpp"
#include "Krystal.Web/Platform/GraphicsClient.hpp"
#include "Krystal.Web/Platform/Scrolling/ScrollTypes.hpp"

namespace krys::web
{
  enum class AXDetachmentType : uint8;
  enum class AXNotification : uint8;
  class AXObject;
  enum class AXTextChange : uint8;
  class Color;
  class ColorChooserClient;
  class DataListSuggestionsClient;
  class DateTimeChooserClient;
  class Frame;
  class GraphicsLayer;
  class GraphicsLayerFactory;
  class HTMLSelectElement;
  class PopupMenuClient;
  class SVGImageElement;

  /// @brief What a page asks of its host: repainting and rendering updates, the pickers and popups its
  /// controls open, accessibility notifications, the screen and pointing devices media features describe,
  /// pointer lock, and its graphics layers and buffers. Every member has a default for a host without it.
  // FIXME: The rest of the Krystal.Web client: tooltips, cursors, scrolling the host's view, search popups and the
  // other compositing hooks (layer flushes, compositing triggers, display refresh monitors), once there are
  // any.
  class WebClient : public GraphicsClient
  {
    KRYS_NON_COPY_MOVABLE(WebClient);

  public:
    WebClient() noexcept = default;

    ~WebClient() noexcept override = default;

    /// @brief The page asks to be brought to the front (`window.focus()`); the host decides whether it is.
    virtual void Focus() noexcept
    {
    }

    /// @brief Called when @p rect (in the main frame's view) needs repainting.
    virtual void InvalidateContentsAndRootView(const RectI &rect) noexcept
    {
      (void)rect;
    }

    /// @brief Asks the client to schedule the page's next rendering update.
    /// @return Whether the client scheduled it; if not, the page waits for the host's next frame.
    virtual bool ScheduleRenderingUpdate() noexcept
    {
      return false;
    }

    /// @brief Whether this is the Krystal.Web client of an SVG image's page.
    KRYS_NODISCARD virtual bool IsSVGImageWebClient() const noexcept
    {
      return false;
    }

    /// @brief Called when the page's accessibility tree posts @p notification for @p object, for the host to
    /// pass on to assistive technology.
    virtual void PostAccessibilityNotification(AXObject &object, AXNotification notification) noexcept
    {
      (void)object;
      (void)notification;
    }

    /// @brief Called when @p object leaves the page's accessibility tree (its node or renderer went, or the
    /// tree is being destroyed), for the host to let go of what it made for it.
    virtual void AccessibilityObjectDetached(AXObject &object, AXDetachmentType detachmentType) noexcept
    {
      (void)object;
      (void)detachmentType;
    }

    /// @brief Called when @p text was inserted into or deleted from @p object's text (an editable region's,
    /// or the web area's) at @p offset, a byte index in its UTF-8 text.
    virtual void PostAccessibilityTextChange(AXObject &object, AXTextChange change, uint64 offset,
                                             utf8_stringview text) noexcept
    {
      (void)object;
      (void)change;
      (void)offset;
      (void)text;
    }

    /// @brief Called when an `<image>` in the page has loaded its image.
    virtual void DidFinishLoadingImageForSVGImage(SVGImageElement &) noexcept
    {
    }

    /// @brief Gives the host @p frame's root graphics layer to composite, or null to take it away.
    virtual void AttachRootGraphicsLayer(Frame &, GraphicsLayer *) noexcept = 0;

    /// @brief The factory the page's graphics layers come from; null when the host doesn't composite, so the
    /// page isn't composited.
    KRYS_NODISCARD virtual GraphicsLayerFactory *GetGraphicsLayerFactory() const noexcept
    {
      return nullptr;
    }

    /// @brief Makes an image buffer with the host's own backend; null when the host has none for @p mode, so
    /// the buffer gets one of the page's (a display list).
    KRYS_NODISCARD RefPtr<ImageBuffer>
      CreateImageBuffer(const SizeF &size, RenderingMode mode, RenderingPurpose purpose,
                        float resolutionScale,
                        const DestinationColorSpace &colorSpace) const noexcept override
    {
      (void)size;
      (void)mode;
      (void)purpose;
      (void)resolutionScale;
      (void)colorSpace;
      return nullptr;
    }

    /// @brief Whether the host's popup shows the items without their CSS direction and alignment, so a menu
    /// list shows its text left aligned, in the text's own direction, to match.
    KRYS_NODISCARD virtual bool SelectItemWritingDirectionIsNatural() const noexcept
    {
      return false;
    }

    /// @brief Whether a menu list's text takes its selected option's direction (and `unicode-bidi`), aligned
    /// to the start of the menu list's own direction.
    KRYS_NODISCARD virtual bool SelectItemAlignmentFollowsMenuWritingDirection() const noexcept
    {
      return true;
    }

    /// @brief Called when the focused select's options change, for a host showing them in its own picker.
    virtual void FocusedSelectElementDidChangeOptions(const HTMLSelectElement &) noexcept
    {
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

    /// @brief Makes the list a text field shows its datalist suggestions in; a host that doesn't override
    /// this shows no suggestions.
    KRYS_NODISCARD virtual RefPtr<DataListSuggestionPicker>
      CreateDataListSuggestionPicker(DataListSuggestionsClient &client) noexcept
    {
      (void)client;
      return nullptr;
    }

    /// @brief Whether the host's suggestion list shows the suggestions' labels, so typing matches them too.
    KRYS_NODISCARD virtual bool CanShowDataListSuggestionLabels() const noexcept
    {
      return false;
    }

    /// @brief Makes the picker a date input chooses its date in; a host that doesn't override this has no
    /// date pickers.
    KRYS_NODISCARD virtual RefPtr<DateTimeChooser>
      CreateDateTimeChooser(DateTimeChooserClient &client) noexcept
    {
      (void)client;
      return nullptr;
    }

    /// @brief Shows @p message, a constraint the control @p anchor doesn't satisfy, near it (a validation
    /// bubble), in place of any shown; the host may hide it after a while. A host that doesn't override this
    /// shows no messages.
    virtual void ShowValidationMessage(Element &anchor, const DOMString &message) noexcept
    {
      (void)anchor;
      (void)message;
    }

    /// @brief Hides the validation message shown for @p anchor, if it's shown.
    virtual void HideValidationMessage(const Element &anchor) noexcept
    {
      (void)anchor;
    }

    /// @brief Hides any validation message shown.
    virtual void HideAnyValidationMessage() noexcept
    {
    }

    /// @brief Whether the validation message for @p anchor is shown.
    KRYS_NODISCARD virtual bool IsValidationMessageVisible(const Element &anchor) const noexcept
    {
      (void)anchor;
      return false;
    }

    /// @brief Shows the host's file picker for a file input in @p frame, answering @p chooser with the files
    /// the user chooses, or cancelling it, now or later; a host that doesn't override this has no file
    /// picker, so the choice is cancelled at once.
    virtual void RunOpenPanel(Frame &frame, FileChooser &chooser) noexcept
    {
      (void)frame;
      chooser.Cancel();
    }

    /// @brief Whether the system has a mouse, whose movement the host can report before the system's
    /// acceleration (pointer lock's unadjusted movement).
    KRYS_NODISCARD virtual bool HasAccessoryMousePointingDevice() const noexcept
    {
      return false;
    }

#pragma region The screen and pointing devices

    // Most of these are what media features evaluate against, besides the page and its view. When an answer
    // changes, the host calls `Page::UpdateStyleAfterChangeInEnvironment`.

    /// @brief What the primary pointing device can do (`pointer`).
    KRYS_NODISCARD virtual PointerCharacteristics PrimaryPointingDevice() const noexcept
    {
      return PointerCharacteristic::Fine;
    }

    /// @brief Whether the primary pointing device can hover (`hover`).
    KRYS_NODISCARD virtual bool HoverSupportedByPrimaryPointingDevice() const noexcept
    {
      return true;
    }

    /// @brief What the available pointing devices can do, together (`any-pointer`).
    KRYS_NODISCARD virtual PointerCharacteristics AllAvailablePointingDevices() const noexcept
    {
      return PointerCharacteristic::Fine;
    }

    /// @brief Whether any available pointing device can hover (`any-hover`).
    KRYS_NODISCARD virtual bool HoverSupportedByAnyAvailablePointingDevice() const noexcept
    {
      return true;
    }

    /// @brief Where @p point, in the root view's coordinates, is on the screen. The base puts the root view
    /// at the screen's origin.
    KRYS_NODISCARD virtual PointI RootViewToScreen(const PointI &point) const noexcept
    {
      return point;
    }

    /// @brief Where @p rect, in the root view's coordinates, is on the screen.
    KRYS_NODISCARD virtual RectI RootViewToScreen(const RectI &rect) const noexcept
    {
      return RectI(RootViewToScreen(rect.Location()), rect.Size());
    }

    /// @brief Where @p point, on the screen, is in the root view's coordinates.
    KRYS_NODISCARD virtual PointI ScreenToRootView(const PointI &point) const noexcept
    {
      return point;
    }

    /// @brief The overlay scrollbars' style the host wants, whatever the page's background; null to follow
    /// the background (light scrollbars over a dark one).
    KRYS_NODISCARD virtual Maybe<ScrollbarOverlayStyle> PreferredScrollbarOverlayStyle() const noexcept
    {
      return null;
    }

    /// @brief The screen's size, in CSS pixels (`device-width`, `device-height`, `device-aspect-ratio`).
    KRYS_NODISCARD virtual SizeF ScreenSize() const noexcept
    {
      return {};
    }

    /// @brief The screen's bits per colour component (`color`).
    KRYS_NODISCARD virtual int32 ScreenDepthPerComponent() const noexcept
    {
      return 8;
    }

    /// @brief Whether the screen shows only shades of one colour (`monochrome`).
    KRYS_NODISCARD virtual bool ScreenIsMonochrome() const noexcept
    {
      return false;
    }

    /// @brief Whether the screen shows colours beyond sRGB, as Display P3 does (`color-gamut`).
    KRYS_NODISCARD virtual bool ScreenSupportsExtendedColor() const noexcept
    {
      return false;
    }

    /// @brief Whether the screen shows (about) Rec. 2020's gamut (`color-gamut: rec2020`).
    KRYS_NODISCARD virtual bool ScreenSupportsRec2020Color() const noexcept
    {
      return false;
    }

    /// @brief Whether the screen supports high dynamic range (`dynamic-range`).
    KRYS_NODISCARD virtual bool ScreenSupportsHighDynamicRange() const noexcept
    {
      return false;
    }

    /// @brief Whether the system inverts colours (`inverted-colors`).
    KRYS_NODISCARD virtual bool ColorsAreInverted() const noexcept
    {
      return false;
    }

    /// @brief Whether the user asked for more contrast (`prefers-contrast`).
    KRYS_NODISCARD virtual bool UserPrefersContrast() const noexcept
    {
      return false;
    }

    /// @brief Whether the user asked for less motion (`prefers-reduced-motion`).
    KRYS_NODISCARD virtual bool UserPrefersReducedMotion() const noexcept
    {
      return false;
    }

#pragma endregion

    /// @brief The page asks to lock the pointer: the host hides the cursor, keeps it where it is and reports
    /// the mouse's movement, then calls @p completionHandler with whether it did; a host that doesn't
    /// override this has no pointer lock.
    // NOLINTNEXTLINE(performance-unnecessary-value-param): callables are taken by value.
    virtual void RequestPointerLock(Func<void(PointerLockRequestResult)> completionHandler) noexcept
    {
      completionHandler(PointerLockRequestResult::Unsupported);
    }

    /// @brief The page asks to unlock the pointer: the host shows the cursor again, then calls
    /// @p completionHandler with whether it did.
    // NOLINTNEXTLINE(performance-unnecessary-value-param): callables are taken by value.
    virtual void RequestPointerUnlock(Func<void(bool)> completionHandler) noexcept
    {
      completionHandler(false);
    }
  };

  /// @brief The Krystal.Web client of a page that has no host.
  class EmptyWebClient final : public WebClient
  {
  public:
    void AttachRootGraphicsLayer(Frame &, GraphicsLayer *) noexcept final
    {
    }
  };
}
