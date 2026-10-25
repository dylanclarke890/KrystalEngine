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
#include "Krystal.Booey/Page/PointerLockController.hpp"
#include "Krystal.Booey/Page/WindowFeatures.hpp"
#include "Krystal.Booey/Platform/Cursor.hpp"
#include "Krystal.Booey/Platform/Network/ResourceError.hpp"
#include "Krystal.Booey/Platform/PlatformMouseEvent.hpp"
#include "Krystal.Booey/Platform/PlatformTouchEvent.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
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
    /// @brief The pointing devices the system has, as the host last said.
    pal::PointingDevices _pointingDevices;
    /// @brief Locks the pointer (true) or unlocks it (false) for the page, returning whether it did.
    Func<bool(bool)> _pointerLockHandler;
    /// @brief Whether the host has locked the pointer for the page.
    bool _pointerLocked {false};

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

    /// @brief What the touches that the page leaves alone are doing.
    enum class GestureState : uint8
    {
      /// @brief Nothing: no touch, or the page has the touches.
      None,
      /// @brief One touch that hasn't moved far: a tap if it lifts soon, a long press if it stays.
      MayTap,
      /// @brief A touch scrolling what's under it.
      Pan,
      /// @brief A touch held still, which asks for a context menu as it lifts.
      LongPress,
      /// @brief Two touches scaling the page.
      Pinch,
    };

    GestureState _gestureState {GestureState::None};
    /// @brief The touch the gesture follows (its id in the page's touch events), and where it began.
    uint32 _gestureTouchId {0};
    boo::DoublePoint _gestureStart;
    /// @brief Where the pan last scrolled to.
    boo::DoublePoint _gestureLast;
    /// @brief The used `touch-action` where the gesture began.
    EnumSet<boo::TouchAction> _gestureTouchActions;
    /// @brief Whether the touch can still tap: not when it stopped a fling.
    bool _gestureCanTap {false};
    /// @brief The axes the pan scrolls.
    bool _panX {false};
    bool _panY {false};
    /// @brief The part of the pan not scrolled yet (scrolls are in whole pixels).
    boo::DoubleSize _panRemainder;
    /// @brief The pan's recent positions, at the event loop's time, which the fling's velocity comes from.
    List<std::pair<MonotonicTime::duration, boo::DoublePoint>> _panSamples;
    /// @brief The page's scale when the pinch began, the distance between its touches then, and the point
    /// between them in the unscaled document, which stays between them.
    float _pinchStartScale {1};
    double _pinchStartDistance {0};
    boo::DoublePoint _pinchDocumentPoint;

    /// @brief The scroll that goes on, slowing, after a pan lifts.
    struct Fling
    {
      /// @brief Where the pan began, which the fling scrolls from.
      boo::DoublePoint Position;
      /// @brief In pixels a second.
      boo::DoubleSize Velocity;
      boo::DoubleSize Remainder;
      /// @brief The host's time at the fling's last step.
      MonotonicTime::duration LastTime {};
    };

    Maybe<Fling> _fling;
    /// @brief Fires when a touch has been held still long enough to be a long press.
    boo::Timer _longPressTimer {*this, &View::LongPressed};

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

    // FIXME: The system's touch thresholds, once the host reports them.
    /// @brief How near a tap is to the last for the two to be a double tap.
    static constexpr int32 DoubleTapDistance {20};
    /// @brief How far a touch moves before it's a pan, not a tap.
    static constexpr double PanThreshold {10};
    /// @brief How long a still touch is held before it's a long press.
    static constexpr std::chrono::milliseconds LongPressDuration {500};
    /// @brief How far back the pan's positions give the fling its velocity.
    static constexpr std::chrono::milliseconds FlingSampleWindow {100};
    /// @brief The slowest pan (in pixels a second) that flings, and the speed a fling stops at.
    static constexpr double MinimumFlingVelocity {100};
    static constexpr double StopFlingVelocity {20};
    /// @brief How quickly a fling slows: its velocity falls by a factor of e in this many seconds.
    static constexpr double FlingTimeConstant {0.325};
    /// @brief How far a pinch scales the page.
    static constexpr float MinimumPageScale {1};
    static constexpr float MaximumPageScale {5};

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
    ///
    /// Touches the page doesn't cancel make gestures, as the used `touch-action` where they began allows:
    /// a touch that moves pans (the box under it scrolls as the wheel scrolls it, after `pointercancel`), and
    /// flings on as `Update` runs if it lifts moving; one that lifts soon taps (a click, with its
    /// compatibility mouse events; two quickly, a double click); one held still is a long press
    /// (`contextmenu` as it lifts); two pinch, scaling the page about the point between them.
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

    /// @brief The cursor the page wants over it, as of the last mouse move (none while the pointer is
    /// locked).
    KRYS_NODISCARD pal::CursorShape CurrentCursor() const noexcept;

    /// @brief Sets what locks the pointer for the page (`requestPointerLock()`) and unlocks it: called with
    /// true, it hides the cursor, keeps it in place and reports the mouse's movement as
    /// `LockedMouseMoved()`, and returns whether it did; called with false, it shows the cursor again.
    /// Without one, pages can't lock the pointer; a page can't lock it while the window doesn't have focus,
    /// and loses it when the window does.
    void SetPointerLockHandler(Func<bool(bool)> &&handler) noexcept;
    /// @brief Whether the pointer is locked for the page.
    KRYS_NODISCARD bool PointerLocked() const noexcept
    {
      return _pointerLocked;
    }
    /// @brief The mouse moved by @p movement (with the system's acceleration; @p unadjustedMovement without
    /// it) while the pointer is locked: `mousemove` fires at the locked element with the movement, where the
    /// mouse was when it was locked.
    void LockedMouseMoved(const boo::DoublePoint &movement, const boo::DoublePoint &unadjustedMovement,
                          pal::KeyModifiers modifiers = {}) noexcept;

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
    /// @param movement For a move, how far the mouse moved since the last (@p unadjustedMovement, before the
    /// system's acceleration, while the pointer is locked).
    KRYS_NODISCARD boo::PlatformMouseEvent
      MouseEvent(boo::PlatformEvent::Type type, const boo::IntPoint &point, boo::MouseButton button,
                 int32 clickCount, EnumSet<boo::PlatformEvent::Modifier> modifiers,
                 const boo::DoublePoint &movement = {},
                 const boo::DoublePoint &unadjustedMovement = {}) const noexcept;
    /// @brief The button a move carries: the one held, as Windows' do (the left, then the middle, then the
    /// right).
    KRYS_NODISCARD boo::MouseButton HeldButton() const noexcept;
    /// @brief Asks the host to lock the pointer for the page.
    KRYS_NODISCARD boo::PointerLockRequestResult LockPointer() noexcept;
    /// @brief Asks the host to unlock the pointer.
    bool UnlockPointer() noexcept;
    static void PerformMicrotaskCheckpoint() noexcept;
    /// @brief @p button's bit in the DOM's `buttons` bit set.
    KRYS_NODISCARD static uint16 ButtonsBit(boo::MouseButton button) noexcept;
    KRYS_NODISCARD boo::PlatformMouseEvent PenEvent(boo::PlatformEvent::Type type,
                                                    const boo::DoublePoint &point, boo::MouseButton button,
                                                    int32 clickCount,
                                                    const pal::PointerInput &input) const noexcept;
    /// @brief Counts a press of @p button at @p point on from the last, if it's soon after it and within
    /// @p distance of it.
    KRYS_NODISCARD int32 CountClick(const boo::IntPoint &point, boo::MouseButton button,
                                    int32 distance = DoubleClickDistance) noexcept;

    /// @brief Follows the gestures the touches make, after the touch event for @p action of the touch @p id
    /// at @p position, which the page cancelled if @p cancelled.
    void RecogniseGesture(uint32 id, pal::PointerAction action, const boo::DoublePoint &position,
                          bool cancelled) noexcept;
    /// @brief The used `touch-action` of the element at @p point.
    KRYS_NODISCARD EnumSet<boo::TouchAction> TouchActionsAt(const boo::DoublePoint &point) noexcept;
    /// @brief Ends the gesture, without its default action.
    void ResetGesture() noexcept;
    /// @brief The touch @p id's pointer events end, as the page loses the touch to a pan or a pinch
    /// (`pointercancel`).
    void CancelTouchPointer(uint32 id, const boo::DoublePoint &position) noexcept;
    /// @brief The pan moved to @p position: what's under its start scrolls by the movement, on the axes it
    /// scrolls.
    void PanTo(const boo::DoublePoint &position) noexcept;
    /// @brief Scrolls what's under @p position by @p delta (the content moving with the touch).
    /// @return Whether something scrolled.
    bool ScrollForTouch(const boo::DoublePoint &position, const boo::DoubleSize &delta) noexcept;
    /// @brief The pan lifted: it flings on if it was moving fast enough.
    void StartFling() noexcept;
    /// @brief Moves the fling on to the host's time @p now.
    void StepFling(MonotonicTime::duration now) noexcept;
    /// @brief The two touches moved: the page scales by how far apart they are, about the point between them.
    void UpdatePinch() noexcept;
    /// @brief A touch tapped at @p position: a click there, with its compatibility mouse events.
    void Tap(uint32 id, const boo::DoublePoint &position) noexcept;
    /// @brief The long press timer fired: the touch is a long press.
    void LongPressed() noexcept;
    /// @brief A long press of the touch @p id lifted at @p position: the page gets `contextmenu` there.
    void RequestContextMenu(uint32 id, const boo::DoublePoint &position) noexcept;
  };
}
