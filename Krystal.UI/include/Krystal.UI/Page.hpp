#pragma once

#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayList.hpp"
#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Platform/Cursor.hpp"
#include "Krystal.Booey/Platform/PlatformMouseEvent.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/Expected.hpp"
#include "Krystal.Gfx/Render2D/Renderer2D.hpp"
#include "Krystal.UI/FontFaceCache.hpp"
#include "Krystal.UI/ImageBufferTextures.hpp"
#include <chrono>
#include <filesystem>

namespace krys::ui
{
  /// @brief Hosts a Booey document: parses it, shows it in a frame with a view of the viewport's size, keeps
  /// its style and layout up to date, and paints it into a `Renderer2D` through a display list.
  class Page
  {
    KRYS_NON_COPY_MOVABLE(Page);

  private:
    Ref<boo::LocalFrame> _frame;
    RefPtr<boo::dom::HTMLDocument> _document;
    FontFaceCache _fonts;
    /// @brief Made on the first replay, with the renderer's device.
    UniquePtr<ImageBufferTextures> _imageBuffers;
    /// @brief The buttons held, as the DOM's `buttons` bit set.
    uint16 _buttons {0};
    /// @brief The last press, which the next one counts on from if it's a double click.
    int32 _clickCount {0};
    boo::MouseButton _lastClickButton {boo::MouseButton::None};
    MonotonicTime::duration _lastClickTime {};
    boo::IntPoint _lastClickPosition;

    // FIXME: The system's double click time and distance, once the host reports them.
    static constexpr std::chrono::milliseconds DoubleClickTime {500};
    static constexpr int32 DoubleClickDistance {2};

  public:
    /// @brief A page with an empty document and a view of @p viewportSize.
    explicit Page(const boo::IntSize &viewportSize) noexcept;
    ~Page() noexcept;

    /// @brief Replaces the document with one parsed from @p markup.
    void LoadHTML(utf8_string &&markup) noexcept;
    /// @brief Replaces the document with one parsed from the UTF-8 file at @p path.
    KRYS_NODISCARD Expected<void> LoadFile(const std::filesystem::path &path) noexcept;

    /// @brief Registers the font files (`.ttf`, `.otf`, `.ttc`, `.otc`) in @p directory for every page, so
    /// `font-family` finds them by name; documents already loaded pick them up at their next update.
    /// @return How many faces were registered.
    static uint32 AddFontDirectory(const std::filesystem::path &directory);

    /// @brief Resizes the viewport; the document is laid out again at its next update.
    void Resize(const boo::IntSize &viewportSize) noexcept;
    KRYS_NODISCARD boo::IntSize ViewportSize() const noexcept;

    /// @brief Brings the document's style and layout up to date.
    void UpdateRendering() noexcept;

    /// @brief Paints the page, up to date, into a display list.
    KRYS_NODISCARD Ref<const boo::display_list::DisplayList> Record() noexcept;

    /// @brief Paints the page into @p renderer, between its `Begin` and `End`.
    void Paint(gfx::Renderer2D &renderer) noexcept;

    /// @brief Replays @p displayList into @p renderer, with this page's fonts, between its `Begin` and `End`.
    void Replay(const boo::display_list::DisplayList &displayList, gfx::Renderer2D &renderer) noexcept;

    /// @brief The topmost element at @p point (in viewport coordinates), or null if there's none.
    KRYS_NODISCARD boo::dom::Element *HitTest(const boo::IntPoint &point) noexcept;

    /// @brief Moves the mouse to @p point (in viewport coordinates): mouse events fire at the elements it
    /// leaves and enters, the elements under it become `:hover`, and the cursor follows.
    void MouseMoved(const boo::IntPoint &point,
                    EnumSet<boo::PlatformEvent::Modifier> modifiers = {}) noexcept;
    /// @brief Presses @p button at @p point: `mousedown` fires, and the element under the mouse and its
    /// ancestors become `:active` until the release. A press soon after the last and near it is a double
    /// click.
    void MousePressed(const boo::IntPoint &point, boo::MouseButton button,
                      EnumSet<boo::PlatformEvent::Modifier> modifiers = {}) noexcept;
    /// @brief Releases @p button at @p point: `mouseup` fires, then `click` (`auxclick` for other buttons)
    /// and `dblclick` for a double click, and `:active` is cleared.
    void MouseReleased(const boo::IntPoint &point, boo::MouseButton button,
                       EnumSet<boo::PlatformEvent::Modifier> modifiers = {}) noexcept;
    /// @brief The mouse left the page: it leaves the element it was over, and nothing is `:hover`.
    void MouseLeft() noexcept;

    /// @brief The cursor the page wants over it, as of the last mouse move.
    KRYS_NODISCARD boo::Cursor CurrentCursor() const noexcept;

    KRYS_NODISCARD boo::dom::HTMLDocument &Document() const noexcept
    {
      return *_document;
    }

    KRYS_NODISCARD boo::LocalFrame &Frame() const noexcept
    {
      return *_frame;
    }

    KRYS_NODISCARD FontFaceCache &Fonts() noexcept
    {
      return _fonts;
    }

  private:
    KRYS_NODISCARD boo::PlatformMouseEvent
      MouseEvent(boo::PlatformEvent::Type type, const boo::IntPoint &point, boo::MouseButton button,
                 int32 clickCount, EnumSet<boo::PlatformEvent::Modifier> modifiers) const noexcept;
    /// @brief @p button's bit in the DOM's `buttons` bit set.
    KRYS_NODISCARD static uint16 ButtonsBit(boo::MouseButton button) noexcept;
  };
}
