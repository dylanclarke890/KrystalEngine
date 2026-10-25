#pragma once

#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayList.hpp"
#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Graphics/Region.hpp"
#include "Krystal.Booey/History/BackForwardList.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementInterface.hpp"
#include "Krystal.Booey/Page/ConsoleTypes.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/WindowFeatures.hpp"
#include "Krystal.Booey/Platform/Cursor.hpp"
#include "Krystal.Booey/Platform/Network/ResourceError.hpp"
#include "Krystal.Booey/Platform/PlatformMouseEvent.hpp"
#include "Krystal.Booey/Platform/PlatformTouchEvent.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/Expected.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Gfx/Render2D/Renderer2D.hpp"
#include "Krystal.PAL/CursorShapes.hpp"
#include "Krystal.PAL/KeyModifiers.hpp"
#include "Krystal.PAL/Keys.hpp"
#include "Krystal.PAL/MouseButtons.hpp"
#include "Krystal.PAL/Platform.hpp"
#include "Krystal.PAL/Pointer.hpp"
#include "Krystal.UI/Compositing/TextureMapper.hpp"
#include "Krystal.UI/FontFaceCache.hpp"
#include "Krystal.UI/ImageBufferTextures.hpp"
#include "Krystal.UI/Overlay.hpp"
#include <chrono>
#include <filesystem>

namespace krys::ui
{
  /// @brief What the host decides to do with a navigation.
  enum class NavigationPolicy : uint8
  {
    /// @brief Navigate: within the page (to a fragment), or to another document, which loads.
    Allow,
    /// @brief Leave the page as it is: the host may have handled the navigation itself.
    Ignore,
  };

  /// @brief A navigation the page asks the host about (a link followed or a form submitted), as the host's
  /// handler receives it.
  struct NavigationRequest
  {
    /// @brief The URL to navigate to: for a form, the action, with the form data as its query for a `GET`.
    utf8_string URL;
    /// @brief The browsing context named (the link's or the form's target, or the document's base target);
    /// empty for the view's own.
    utf8_string Target;
    /// @brief Whether the target names another browsing context (`_blank` or a name), which the host would
    /// open.
    bool OpensNewWindow {false};
    /// @brief Whether allowing it navigates within the page: to a fragment of the page's URL.
    bool IsSameDocument {false};
    /// @brief The element that started it: the link or the form.
    RefPtr<boo::dom::Element> SourceElement;
    /// @brief The mouse button that followed the link, if a click did.
    Maybe<boo::MouseButton> Button;
    /// @brief The modifier keys held as the link was followed or the form submitted.
    bool ShiftKey {false};
    bool CtrlKey {false};
    bool AltKey {false};
    bool MetaKey {false};
    /// @brief The link's `download` attribute, if it has one.
    Maybe<utf8_string> Download;
    /// @brief The features `window.open()` asked the new window to have, if it opened one.
    Maybe<boo::WindowFeatures> Features;

    /// @brief Whether a form was submitted.
    bool IsFormSubmission {false};
    /// @brief `GET` or `POST`.
    utf8_string Method {u8"GET"};
    /// @brief The `Content-Type` of a `POST`'s body, with its boundary for `multipart/form-data`.
    utf8_string ContentType;
    /// @brief The body of a `POST`; empty otherwise.
    utf8_string Body;
  };

  /// @brief What happened to a load in the view, as the host's load handler hears it.
  enum class LoadEventType : uint8
  {
    /// @brief A document started loading; the view still shows the old one.
    Started,
    /// @brief The loading document is the view's, made and about to be parsed.
    Committed,
    /// @brief The document finished loading.
    Finished,
    /// @brief The document couldn't be loaded (or the load was stopped); the view keeps the old one.
    Failed,
  };

  /// @brief A load event: its type, and why it failed.
  struct LoadEvent
  {
    LoadEventType Type {LoadEventType::Started};
    /// @brief The error, for a failure.
    boo::ResourceError Error;
    /// @brief Whether the committed document came back from the back/forward cache, already loaded.
    bool FromBackForwardCache {false};
  };

  /// @brief A message the page sends to the developer console (an error it didn't handle, a warning).
  struct ConsoleMessage
  {
    boo::MessageSource Source {boo::MessageSource::Other};
    boo::MessageLevel Level {boo::MessageLevel::Log};
    utf8_string Message;
    /// @brief Where it came from: the document's URL.
    utf8_string SourceURL;
    uint32 Line {0};
    uint32 Column {0};
  };

  /// @brief Shows a Booey page: parses its document into the page's main frame, with a view of the viewport's
  /// size; runs Booey's event loop with the host's time and the page's rendering update once a frame;
  /// composites its layer tree into a `Renderer2D`; and turns the platform layer's mouse and keyboard input
  /// into DOM events.
  /// @note The first view makes Krystal.UI's `FileLoaderStrategy` Booey's loader, unless the host set one, so
  /// pages load `file:` images.
  /// @note A menu list's popup and the date and colour pickers show as overlays above the page: while one is
  /// open, it takes the keys and the mouse, and a press outside it closes it.
  class View
  {
    friend class Overlay;
    KRYS_NON_COPY_MOVABLE(View);

  public:
    class ChromeClient;
    class FrameLoaderClient;

  private:
    /// @brief The page's chrome client, which the page owns: it hands Booey the graphics layers and keeps the
    /// root of the page's layer tree.
    ChromeClient *_chromeClient {nullptr};
    /// @brief The main frame's loader client, which the frame owns: it asks the navigation handler.
    FrameLoaderClient *_frameLoaderClient {nullptr};
    /// @brief The page's session history, which the page shares.
    Ref<boo::BackForwardList> _backForwardList;
    Ref<boo::Page> _page;
    Ref<boo::LocalFrame> _frame;
    FontFaceCache _fonts;
    /// @brief Made on the first replay, with the renderer's device.
    UniquePtr<ImageBufferTextures> _imageBuffers;
    /// @brief Composites the page's layer tree: made on the first paint, with the renderer's device.
    UniquePtr<TextureMapper> _textureMapper;
    /// @brief Whether the page may look different since it was last painted.
    bool _needsPaint {true};
    /// @brief The buttons held, as the DOM's `buttons` bit set.
    uint16 _buttons {0};
    /// @brief Where the last mouse event was, so moves that don't move can be dropped.
    Maybe<boo::IntPoint> _lastMousePosition;
    /// @brief The last press, which the next one counts on from if it's a double click.
    int32 _clickCount {0};
    boo::MouseButton _lastClickButton {boo::MouseButton::None};
    MonotonicTime::duration _lastClickTime {};
    boo::IntPoint _lastClickPosition;
    /// @brief Whether the window showing the page has system focus.
    bool _focused {true};
    /// @brief The `key` of each key held, which its release reports too.
    HashMap<pal::Key, utf8_string> _heldKeyValues;
    /// @brief The overlay open over the page (a menu list's popup, a picker), if any.
    RefPtr<Overlay> _overlay;
    /// @brief Whether a button was held as the overlay opened: its release goes to the page.
    bool _pageOwnsPress {false};
    /// @brief The pen's buttons held (its tip, its barrel button or its eraser), as the DOM's `buttons` bit
    /// set.
    uint16 _penButtons {0};
    /// @brief The button the pen pressed with, which its release reports.
    boo::MouseButton _penButton {boo::MouseButton::None};

    /// @brief A touch on the screen.
    struct ActiveTouch
    {
      /// @brief The host's id for it.
      uint32 HostId {0};
      /// @brief Its id in the page's touch events: the lowest that no other touch has.
      uint32 Id {0};
      boo::PlatformTouchPoint Point;
    };

    /// @brief The touches on the screen, the first first.
    List<ActiveTouch> _touches;

    /// @brief A custom element definition the view gives each document it loads.
    struct CustomElementDefinition
    {
      boo::dom::DOMStringAtom Name;
      boo::html::CustomElementFactory Factory;
      boo::html::CustomElementOptions Options;
    };

    List<CustomElementDefinition> _customElements;

    /// @brief How many pages the back/forward cache keeps, unless the host sets it
    /// (`boo::BackForwardCache::Singleton().SetMaxSize`).
    static constexpr uint32 DefaultBackForwardCacheCapacity {3};

    // FIXME: The system's double click time and distance, once the host reports them.
    static constexpr std::chrono::milliseconds DoubleClickTime {500};
    static constexpr int32 DoubleClickDistance {2};

  public:
    /// @brief A view of an empty document, with a viewport of @p viewportSize.
    explicit View(const boo::IntSize &viewportSize) noexcept;
    ~View() noexcept;

    /// @brief Starts loading a document parsed from @p markup, at @p url (which relative URLs resolve
    /// against); `about:blank` if it's null. The document replaces the current one on a later task: run
    /// `Update` until `IsLoading` is false (or the load handler hears it committed).
    /// @param cachePolicy How its style sheets, images and other resources use the memory cache: `Revalidate`
    /// (or `Reload`) loads them again, as reloading a page does.
    void LoadHTML(utf8_string &&markup, boo::url::URL &&url = {},
                  boo::CachePolicy cachePolicy = boo::CachePolicy::Verify) noexcept;
    /// @brief Starts loading the UTF-8 file at @p path, at the file's URL, through the loader strategy; as
    /// `LoadHTML`, it replaces the current document on a later task.
    /// @param cachePolicy As for `LoadHTML`.
    /// @return An error if the file doesn't exist.
    KRYS_NODISCARD Expected<void> LoadFile(const std::filesystem::path &path,
                                           boo::CachePolicy cachePolicy = boo::CachePolicy::Verify) noexcept;
    /// @brief Starts loading @p url through the loader strategy.
    /// @param cachePolicy As for `LoadHTML`.
    void Load(boo::url::URL &&url, boo::CachePolicy cachePolicy = boo::CachePolicy::Verify) noexcept;
    /// @brief Whether a document is loading.
    KRYS_NODISCARD bool IsLoading() const noexcept;
    /// @brief Stops the loading document, if any: the view keeps its current one.
    void Stop() noexcept;
    /// @brief Calls @p handler as loads start, commit, finish and fail.
    void SetLoadHandler(Func<void(const LoadEvent &)> &&handler) noexcept;

    /// @brief The view's session history: an item for each document it showed (and each fragment it went
    /// to), the current one and those before and after it.
    KRYS_NODISCARD boo::BackForwardList &GetBackForwardList() const noexcept
    {
      return *_backForwardList;
    }
    /// @brief Whether there's an item before the current one.
    KRYS_NODISCARD bool CanGoBack() const noexcept;
    /// @brief Whether there's an item after the current one.
    KRYS_NODISCARD bool CanGoForward() const noexcept;
    /// @brief Goes back to the item before the current one: its document loads again on a later task (with
    /// its scroll position and form state), unless the item is for the current document (a fragment).
    /// @return Whether there was an item to go back to.
    bool GoBack() noexcept;
    /// @brief Goes forward to the item after the current one, as `GoBack` goes back.
    /// @return Whether there was an item to go forward to.
    bool GoForward() noexcept;
    /// @brief Goes to @p item, one of the back/forward list's.
    void GoToItem(boo::HistoryItem &item) noexcept;
    /// @brief Loads the current document again, its resources revalidated; it keeps its history item and
    /// scroll position, but not its form state.
    void Reload() noexcept;

    /// @brief Defines @p name as a custom element, whose behaviour @p factory creates, in every document the
    /// view loads (before it's parsed, so its parser creates the elements) and in the current one (whose
    /// elements of that name are upgraded).
    /// @return An error if @p name isn't a valid custom element name or the view already defines it.
    KRYS_NODISCARD Expected<void>
      DefineCustomElement(boo::dom::DOMStringAtom name, boo::html::CustomElementFactory &&factory,
                          boo::html::CustomElementOptions &&options = {}) noexcept;

    /// @brief Registers the font files (`.ttf`, `.otf`, `.ttc`, `.otc`) in @p directory for every page, so
    /// `font-family` finds them by name; documents already loaded pick them up at their next update.
    /// @return How many faces were registered.
    static uint32 AddFontDirectory(const std::filesystem::path &directory);

    /// @brief Resizes the viewport; the document is laid out again at its next update.
    void Resize(const boo::IntSize &viewportSize) noexcept;
    KRYS_NODISCARD boo::IntSize ViewportSize() const noexcept;

    /// @brief Runs a frame of the page at the host's time @p now: Booey's event loop (the timers that are
    /// due, such as style and layout, then the queued tasks, each followed by a microtask checkpoint), then
    /// the page's rendering update if something scheduled one, then the page's idle callbacks for what's left
    /// of @p budget (at most 50ms). Call it once a frame, before painting, with a time that only moves
    /// forwards (the engine may pause or scale it).
    /// @param budget How long (in real time) tasks and idle callbacks may run this frame, if limited.
    /// @return Whether the rendering update ran: the page may look different, so a host that keeps its last
    /// frame paints again.
    bool Update(MonotonicTime::duration now, Maybe<MonotonicTime::duration> budget = null) noexcept;

    /// @brief Runs the page's rendering update now, whether or not one was scheduled.
    void UpdateRendering() noexcept;

    /// @brief How far the viewport is scrolled into the document, in pixels.
    KRYS_NODISCARD boo::IntPoint ScrollPosition() const noexcept;
    /// @brief Scrolls the viewport to @p position (in pixels), within the document; `scroll` fires at the
    /// next rendering update.
    void ScrollTo(const boo::IntPoint &position) noexcept;

    /// @brief Paints the page, up to date, into a display list, its composited layers flattened into it.
    KRYS_NODISCARD Ref<const boo::display_list::DisplayList> Record() noexcept;

    /// @brief Paints the page into @p renderer, between its `Begin` and `End`: the page's layer tree is
    /// flushed, the parts of its layers that changed are painted again, and the layers are composited.
    void Paint(gfx::Renderer2D &renderer) noexcept;
    /// @brief Whether the page has changed since it was last painted (or never was).
    KRYS_NODISCARD bool NeedsPaint() const noexcept;

    /// @brief Replays @p displayList into @p renderer, with this page's fonts, between its `Begin` and `End`.
    void Replay(const boo::display_list::DisplayList &displayList, gfx::Renderer2D &renderer) noexcept;

    /// @brief The topmost element at @p point (in viewport coordinates), or null if there's none.
    KRYS_NODISCARD boo::dom::Element *HitTest(const boo::IntPoint &point) noexcept;

    /// @brief Moves the mouse to @p point (in viewport coordinates): mouse events fire at the elements it
    /// leaves and enters, the elements under it become `:hover`, and the cursor follows.
    void MouseMoved(const boo::IntPoint &point, pal::KeyModifiers modifiers = {}) noexcept;
    /// @brief Presses @p button at @p point: `mousedown` fires, and the element under the mouse and its
    /// ancestors become `:active` until the release. A press soon after the last and near it is a double
    /// click.
    void MousePressed(const boo::IntPoint &point, pal::MouseButton button,
                      pal::KeyModifiers modifiers = {}) noexcept;
    /// @brief Releases @p button at @p point: `mouseup` fires, then `click` (`auxclick` for other buttons)
    /// and `dblclick` for a double click, and `:active` is cleared.
    void MouseReleased(const boo::IntPoint &point, pal::MouseButton button,
                       pal::KeyModifiers modifiers = {}) noexcept;
    /// @brief The mouse left the page: it leaves the element it was over, and nothing is `:hover`.
    void MouseLeft() noexcept;
    /// @brief Turns the wheel at @p point by @p notchesX and @p notchesY (positive is away from the user, or
    /// to the right): `wheel` fires at the element under the mouse, then, unless it's cancelled, the nearest
    /// box that can scroll that way scrolls, or else the viewport. A notch is @p settings' lines (or a page),
    /// at 100 pixels per three lines; with Shift, the vertical wheel scrolls sideways.
    /// @return Whether the page cancelled the wheel or scrolled.
    bool MouseWheel(const boo::IntPoint &point, float notchesX, float notchesY,
                    pal::KeyModifiers modifiers = {},
                    const pal::WheelScrollSettings &settings = pal::GetWheelScrollSettings()) noexcept;

    /// @brief A pen's input at @p point (in viewport coordinates): the pen moves (hovering or touching),
    /// presses and releases as the mouse does, with its pressure, tilt and twist, as pointer events of type
    /// `pen` (and their compatibility mouse events). Its tip presses the left button, with the barrel button
    /// held the right, and its eraser the eraser button (5); leaving the page leaves the element it was over.
    void PenInput(const boo::DoublePoint &point, const pal::PointerInput &input) noexcept;
    /// @brief A touch's input at @p point (in viewport coordinates): `touchstart`, `touchmove`, `touchend` or
    /// `touchcancel` at the element it began on, with every touch on the screen, each preceded by the touch's
    /// pointer events of type `touch`. While an overlay is open, touches are ignored.
    /// @return Whether the page cancelled or handled the touch event.
    bool TouchInput(const boo::DoublePoint &point, const pal::PointerInput &input) noexcept;
    /// @brief The touches on the screen.
    KRYS_NODISCARD uint32 TouchCount() const noexcept
    {
      return static_cast<uint32>(_touches.size());
    }

    /// @brief Tells the page what pointing devices the system has (@p devices): the `pointer`, `hover`,
    /// `any-pointer` and `any-hover` media features and `navigator.maxTouchPoints` follow them. Until the
    /// host does, it's a mouse alone.
    void SetPointingDevices(const pal::PointingDevices &devices) noexcept;

    /// @brief The cursor the page wants over it, as of the last mouse move.
    KRYS_NODISCARD pal::CursorShape CurrentCursor() const noexcept;

    /// @brief Presses @p key (again, if @p isAutoRepeat): `keydown` fires at the focused element (or the
    /// body), then `keypress` if it isn't cancelled and the key types text; the focused element becomes focus
    /// visible.
    /// @return Whether the page cancelled the key or handled it (Tab and Shift+Tab move the focus).
    bool KeyPressed(pal::Key key, pal::KeyModifiers modifiers = {}, const pal::KeyText &text = {},
                    bool isAutoRepeat = false) noexcept;
    /// @brief Releases @p key: `keyup` fires at the focused element (or the body).
    /// @return Whether the page cancelled the key.
    bool KeyReleased(pal::Key key, pal::KeyModifiers modifiers = {}) noexcept;

    /// @brief The window showing the page gained (true) or lost (false) system focus: `:focus` matches only
    /// while it's focused, and the focused element gets `focus` or `blur`. A page starts focused.
    void SetFocused(bool focused) noexcept;

    /// @brief Asks @p handler about each navigation the page starts (links followed and forms submitted). A
    /// view without a handler allows every navigation, so only navigations within the page happen.
    void SetNavigationHandler(Func<NavigationPolicy(const NavigationRequest &)> &&handler) noexcept;

    /// @brief Calls @p handler when the page asks to be brought to the front (`window.focus()`); the host
    /// decides whether to raise its window. A view without a handler ignores it.
    void SetFocusRequestHandler(Func<void()> &&handler) noexcept;

    /// @brief Calls @p handler when the page asks to be closed (`window.close()`), on a task after the
    /// request; the host closes the view (it may destroy it then) or keeps it. A view without a handler stays
    /// open.
    void SetCloseRequestHandler(Func<void()> &&handler) noexcept;

    /// @brief Tells the page where the host's window is on the screen and its size (`screenX`, `outerWidth`
    /// and the others); until the host does, it's the viewport at the screen's origin.
    void SetWindowRect(const boo::FloatRect &rect) noexcept;

    /// @brief Calls @p handler when the page asks to move or resize the host's window (`moveTo()`,
    /// `resizeTo()` and the others), with the rectangle it asks for, already kept on the screen; the host
    /// moves its window (and reports it with `SetWindowRect`) or ignores it.
    void SetWindowRectRequestHandler(Func<void(const boo::FloatRect &)> &&handler) noexcept;

    /// @brief Calls @p handler to show the page's `alert()` message. Without one, alerts show nothing.
    void SetAlertHandler(Func<void(const utf8_string &)> &&handler) noexcept;

    /// @brief Calls @p handler to ask the user to confirm the page's `confirm()` message. Without one,
    /// nothing is confirmed.
    void SetConfirmHandler(Func<bool(const utf8_string &)> &&handler) noexcept;

    /// @brief Calls @p handler to ask the user for the page's `prompt()` text, with its message and default
    /// value; null if they cancel. Without one, prompts are cancelled.
    void
      SetPromptHandler(Func<Maybe<utf8_string>(const utf8_string &, const utf8_string &)> &&handler) noexcept;

    /// @brief Calls @p handler when the page asks to be printed (`print()`).
    void SetPrintHandler(Func<void()> &&handler) noexcept;

    /// @brief Calls @p handler when the page asks the user to confirm leaving it (its `beforeunload` was
    /// cancelled, after the user interacted with it), with its message; the handler returns whether to leave.
    /// Without one, the page is left.
    void SetBeforeUnloadHandler(Func<bool(const utf8_string &)> &&handler) noexcept;

    /// @brief Calls @p handler with each message the page sends to the developer console.
    void SetConsoleMessageHandler(Func<void(const ConsoleMessage &)> &&handler) noexcept;

    /// @brief Calls @p handler when the page asks to handle a scheme's URLs (`registerProtocolHandler()`,
    /// with true) or stops (`unregisterProtocolHandler()`, with false), with the scheme and the handler's
    /// URL.
    void SetProtocolHandlerRequestHandler(
      Func<void(const utf8_string &scheme, const utf8_string &url, bool registered)> &&handler) noexcept;

    /// @brief The user agent string the page reports (`navigator.userAgent`); Booey's unless set.
    void SetUserAgent(utf8_string &&userAgent) noexcept;

    /// @brief The view's document: the main frame's.
    KRYS_NODISCARD boo::dom::HTMLDocument &Document() const noexcept;

    KRYS_NODISCARD boo::LocalFrame &Frame() const noexcept
    {
      return *_frame;
    }

    KRYS_NODISCARD boo::Page &GetPage() const noexcept
    {
      return *_page;
    }

    KRYS_NODISCARD FontFaceCache &Fonts() noexcept
    {
      return _fonts;
    }

    /// @brief The overlay open over the page (a menu list's popup, a picker), if any.
    KRYS_NODISCARD Overlay *ShownOverlay() const noexcept
    {
      return _overlay.get();
    }

  private:
    /// @brief A new document is the view's, before it's parsed: it takes the view's focus state and custom
    /// elements.
    void DidClearWindowObject() noexcept;

    /// @brief A load committed (a new document, or one the back/forward cache kept): it's painted with the
    /// view's focus state.
    void DidCommitLoad() noexcept;

    /// @brief Replaces the document with one parsed from @p markup at once, without a load: the overlays'
    /// content.
    void ReplaceDocument(utf8_string &&markup) noexcept;

    void ShowOverlay(Overlay &overlay) noexcept;
    void OverlayHidden(Overlay &overlay) noexcept;
    /// @brief Paints the open overlay over the page, at its place in the viewport.
    void PaintOverlay(gfx::Renderer2D &renderer) noexcept;
    KRYS_NODISCARD boo::PlatformMouseEvent
      MouseEvent(boo::PlatformEvent::Type type, const boo::IntPoint &point, boo::MouseButton button,
                 int32 clickCount, EnumSet<boo::PlatformEvent::Modifier> modifiers) const noexcept;
    static void PerformMicrotaskCheckpoint() noexcept;
    /// @brief @p button's bit in the DOM's `buttons` bit set.
    KRYS_NODISCARD static uint16 ButtonsBit(boo::MouseButton button) noexcept;
    KRYS_NODISCARD boo::PlatformMouseEvent PenEvent(boo::PlatformEvent::Type type,
                                                    const boo::DoublePoint &point, boo::MouseButton button,
                                                    int32 clickCount,
                                                    const pal::PointerInput &input) const noexcept;
    /// @brief Counts a press of @p button at @p point on from the last, if it's soon after it and near it.
    KRYS_NODISCARD int32 CountClick(const boo::IntPoint &point, boo::MouseButton button) noexcept;
  };
}
