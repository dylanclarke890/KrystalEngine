#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DOMRect.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/WheelEvent.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Platform/Cursor.hpp"
#include "Krystal.Booey/Platform/PlatformKeyboardEvent.hpp"
#include "Krystal.Booey/Platform/PlatformWheelEvent.hpp"
#include "Krystal.Booey/Platform/Scrollbar.hpp"
#include "Krystal.Booey/Platform/ScrollbarTheme.hpp"
#include "Krystal.Booey/Platform/WindowsKeyboardCodes.hpp"
#include <string>

namespace krys::boo::tests
{
  namespace
  {
    /// @brief A 400x600 document in a 200x100 view: a 50 pixel box, then `#target`, 20 pixels tall, at 300.
    constexpr utf8_stringview TallPage =
      u8"<!DOCTYPE html><style>body{margin:0}#page{width:400px;height:600px}</style><div id=page>"
      u8"<div id=top style=height:50px></div><div id=target style=margin-top:250px;height:20px></div></div>";

    /// @brief A laid out document in a 200x100 view.
    struct ScrollFixture : PaintFixture
    {
      explicit ScrollFixture(utf8_stringview markup) noexcept : PaintFixture(utf8_string(markup))
      {
        (void)Document->UpdateLayout();
      }

      KRYS_NODISCARD LocalFrameView &View() noexcept
      {
        return *Frame->View();
      }

      /// @brief The display list of the view painted as the host paints it: scrolled.
      KRYS_NODISCARD utf8_string PaintedScrolled() noexcept
      {
        (void)Document->UpdateLayout();
        display_list::RecorderImpl recorder(
          FloatSize(static_cast<float>(ViewSize.Width()), static_cast<float>(ViewSize.Height())));
        View().Paint(recorder, IntRect(IntPoint(), ViewSize));
        return recorder.TakeDisplayList()->AsText({});
      }
    };

    /// @brief Records the events it handles as `type:target`, with `^` for those that bubble.
    struct EventRecorder : public dom::EventListenerCallback
    {
      List<std::string> &Records;

      explicit EventRecorder(List<std::string> &records) noexcept : Records(records)
      {
      }

      void HandleEvent(dom::Event &event) noexcept override
      {
        auto type = event.Type().View();
        std::string target = "document";
        if (auto *node = DynamicDowncast<dom::Node>(event.Target().get()))
        {
          if (auto *element = DynamicDowncast<dom::Element>(*node))
          {
            auto id = element->GetAttribute(u8"id").value_or(dom::DOMString());
            target = std::string(id.begin(), id.end());
          }
        }
        Records.push_back(std::string(type.begin(), type.end()) + ":" + target
                          + (event.Bubbles() ? "^" : ""));
      }
    };

    using Strings = List<std::string>;

    KRYS_NODISCARD PlatformMouseEvent MouseEvent(PlatformEvent::Type type, IntPoint position,
                                                 MouseButton button = MouseButton::Left) noexcept
    {
      DoublePoint point(position.X(), position.Y());
      int32 clickCount = type == PlatformEvent::Type::MousePressed ? 1 : 0;
      return PlatformMouseEvent(point, point,
                                type == PlatformEvent::Type::MouseMoved ? MouseButton::None : button, type,
                                clickCount, {}, MonotonicTime::Now());
    }

    void Move(EventHandler &handler, IntPoint position) noexcept
    {
      (void)handler.MouseMoved(MouseEvent(PlatformEvent::Type::MouseMoved, position));
    }

    void Press(EventHandler &handler, IntPoint position) noexcept
    {
      (void)handler.HandleMousePressEvent(MouseEvent(PlatformEvent::Type::MousePressed, position));
    }

    void Release(EventHandler &handler, IntPoint position) noexcept
    {
      (void)handler.HandleMouseReleaseEvent(MouseEvent(PlatformEvent::Type::MouseReleased, position));
    }

    /// @brief A wheel event at @p position scrolling by @p delta pixels (the DOM's sense: positive is down
    /// and right).
    KRYS_NODISCARD bool Wheel(EventHandler &handler, IntPoint position, FloatSize delta,
                              PlatformWheelEventGranularity granularity =
                                PlatformWheelEventGranularity::ScrollByPixelWheelEvent) noexcept
    {
      return handler.HandleWheelEvent(PlatformWheelEvent(position, position, -delta.Width(), -delta.Height(),
                                                         0, delta.Height() > 0 ? -1.0f : 1.0f, granularity,
                                                         {}, MonotonicTime::Now()));
    }

    /// @brief A press of the key named @p key (with the legacy identifier @p keyIdentifier) that types @p
    /// text.
    KRYS_NODISCARD bool Key(EventHandler &handler, utf8_string key, utf8_string keyIdentifier, int32 keyCode,
                            utf8_string text = {}, EnumSet<PlatformEvent::Modifier> modifiers = {}) noexcept
    {
      return handler.KeyEvent(PlatformKeyboardEvent(PlatformEvent::Type::KeyDown, text, text, krys::move(key),
                                                    u8"", krys::move(keyIdentifier), keyCode, false, false,
                                                    false, modifiers, MonotonicTime::Now()));
    }

    /// @brief Cancels the events it handles, keeping the last one's vertical delta.
    struct WheelCanceller : public dom::EventListenerCallback
    {
      double DeltaY {0};

      void HandleEvent(dom::Event &event) noexcept override
      {
        if (event.IsWheelEvent())
        {
          DeltaY = static_cast<dom::WheelEvent &>(event).DeltaY();
        }
        event.PreventDefault();
      }
    };

    /// @brief The middle of @p scrollbar's thumb, in its coordinates along its length.
    KRYS_NODISCARD int32 ThumbCenter(Scrollbar &scrollbar) noexcept
    {
      auto &theme = ScrollbarTheme::Theme();
      return theme.TrackPosition(scrollbar) + theme.ThumbPosition(scrollbar)
             + theme.ThumbLength(scrollbar) / 2;
    }

    /// @brief How far the content scrolls for each pixel @p scrollbar's thumb moves.
    KRYS_NODISCARD float ScrollPerThumbPixel(Scrollbar &scrollbar) noexcept
    {
      auto &theme = ScrollbarTheme::Theme();
      return static_cast<float>(scrollbar.Maximum())
             / static_cast<float>(theme.TrackLength(scrollbar) - theme.ThumbLength(scrollbar));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LocalFrameView: the scroll position and its range",
                   "[Page][LocalFrameView][Scrolling]")
  {
    ScrollFixture fixture {TallPage};
    auto &view = fixture.View();

    // The contents are the document's size; the range is what doesn't fit in the view, less its scrollbars
    // (21 pixels each, as the document overflows both ways).
    CHECK(view.ContentsSize() == IntSize(400, 600));
    REQUIRE(view.HorizontalScrollbar());
    REQUIRE(view.VerticalScrollbar());
    CHECK(view.MinimumScrollPosition() == ScrollPosition());
    CHECK(view.MaximumScrollPosition() == ScrollPosition(221, 521));
    CHECK(view.GetScrollPosition() == ScrollPosition());

    view.SetScrollPosition({50, 120});
    CHECK(view.GetScrollPosition() == ScrollPosition(50, 120));
    CHECK(view.VisibleContentRect() == IntRect(50, 120, 179, 79));
    CHECK(view.WindowToContents(IntPoint(10, 10)) == IntPoint(60, 130));
    CHECK(view.ContentsToWindow(IntPoint(60, 130)) == IntPoint(10, 10));

    // Clamped to the range.
    view.SetScrollPosition({1'000, -10});
    CHECK(view.GetScrollPosition() == ScrollPosition(221, 0));

    SECTION("a layout that shrinks the document moves the scroll position back")
    {
      view.SetScrollPosition({0, 400});
      REQUIRE_FALSE(fixture.ById(u8"target").SetAttribute(u8"style", u8"display:none").HasException());
      REQUIRE_FALSE(fixture.ById(u8"page").SetAttribute(u8"style", u8"height:150px").HasException());
      (void)fixture.Document->UpdateLayout();
      CHECK(view.ContentsSize() == IntSize(400, 150));
      // The view still scrolls horizontally, so its horizontal scrollbar stays: 150 less 79.
      CHECK(view.GetScrollPosition() == ScrollPosition(0, 71));
    }

    SECTION("the scrolling element's scroll APIs scroll the view")
    {
      auto &root = *fixture.Document->ScrollingElement();
      CHECK(&root == dom::TreeQueries::DocumentElement(*fixture.Document));
      CHECK(root.ScrollLeft() == 221);
      CHECK(root.ScrollTop() == 0);
      CHECK(root.ScrollWidth() == 400);
      CHECK(root.ScrollHeight() == 600);

      root.SetScrollTop(300);
      CHECK(view.GetScrollPosition() == ScrollPosition(221, 300));
      root.SetScrollLeft(20);
      CHECK(view.GetScrollPosition() == ScrollPosition(20, 300));
      root.ScrollTo(10, 20);
      CHECK(view.GetScrollPosition() == ScrollPosition(10, 20));
      root.ScrollBy(5, 5);
      CHECK(view.GetScrollPosition() == ScrollPosition(15, 25));
      CHECK(root.ScrollTop() == 25);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LocalFrameView: hit testing and coordinates follow the scroll",
                   "[Page][LocalFrameView][Scrolling]")
  {
    ScrollFixture fixture {TallPage};
    auto &target = fixture.ById(u8"target");
    fixture.View().SetScrollPosition({0, 300});

    // Client coordinates are the view's.
    CHECK(fixture.Document->ElementFromPoint(10, 5).get() == &target);
    CHECK(target.GetBoundingClientRect()->Y() == 0);
    CHECK(target.OffsetTop() == 300);

    // The mouse hits what's under it in the scrolled view.
    (void)fixture.Frame->GetEventHandler().MouseMoved(
      PlatformMouseEvent(DoublePoint(10, 5), DoublePoint(10, 5), MouseButton::None,
                         PlatformEvent::Type::MouseMoved, 0, {}, MonotonicTime::Now()));
    CHECK(fixture.Frame->GetEventHandler().ElementUnderMouse() == &target);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LocalFrameView: fixed boxes stay in view",
                   "[Page][LocalFrameView][Scrolling]")
  {
    ScrollFixture fixture {
      utf8_string(TallPage)
      + u8"<div id=fixed style=position:fixed;left:0;top:10px;width:30px;height:30px;background:red></div>"};
    auto &fixed = fixture.ById(u8"fixed");
    fixture.View().SetScrollPosition({0, 200});

    CHECK(fixed.GetBoundingClientRect()->Y() == 10);
    CHECK(fixture.Document->ElementFromPoint(5, 20).get() == &fixed);
    // Painted at the scroll position, in the contents' coordinates.
    CHECK(fixture.PaintedScrolled().contains(u8"(rect at (0,210) size 30x30)"));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LocalFrameView: sticky boxes stick to the view",
                   "[Page][LocalFrameView][Scrolling][Sticky]")
  {
    // A sticky box 50 pixels into a 200 pixel section, which holds it in, above 400 more pixels.
    ScrollFixture fixture {
      u8"<!DOCTYPE html><style>body{margin:0}</style><section style=height:200px>"
      u8"<div style=height:50px></div>"
      u8"<div id=sticky style=position:sticky;top:10px;height:20px;background:red></div></section>"
      u8"<div style=height:400px></div>"};
    auto &sticky = fixture.ById(u8"sticky");
    CHECK(sticky.GetBoundingClientRect()->Y() == 50);

    // Scrolled past it, it sticks 10 pixels from the top, where it's painted and hit.
    fixture.View().SetScrollPosition({0, 100});
    CHECK(sticky.GetBoundingClientRect()->Y() == 10);
    CHECK(sticky.OffsetTop() == 110);
    CHECK(fixture.Document->ElementFromPoint(5, 15).get() == &sticky);
    // (The view's vertical scrollbar takes 21 pixels of its width.)
    CHECK(fixture.PaintedScrolled().contains(u8"(rect at (0,110) size 179x20)"));

    // Its section holds it in: it scrolls away with the section's end.
    fixture.View().SetScrollPosition({0, 300});
    CHECK(sticky.GetBoundingClientRect()->Y() == -120);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LocalFrameView: sticky boxes stick inside their scroller",
                   "[Page][LocalFrameView][Scrolling][Sticky]")
  {
    ScrollFixture fixture {
      u8"<!DOCTYPE html><style>body{margin:0} .s{width:150px;height:60px;overflow:auto;"
      u8"margin-left:10px}</style>"
      u8"<div id=scroller class=s style=margin-top:20px>"
      u8"<div id=header style=position:sticky;top:0;height:10px></div><div style=height:200px></div>"
      u8"<div id=footer style=position:sticky;bottom:5px;height:10px></div></div>"
      u8"<div id=table-scroller class=s><table style=border-spacing:0><thead><tr>"
      u8"<th id=cell style=position:sticky;top:0;height:10px;padding:0>H</th></tr></thead><tbody>"
      u8"<tr><td style=height:200px>Body</td></tr></tbody></table></div>"
      u8"<div id=wide class=s style=white-space:nowrap;height:30px>"
      u8"<span style=display:inline-block;width:100px></span>"
      u8"<span id=inline style=position:sticky;left:5px>x</span>"
      u8"<span style=display:inline-block;width:300px></span></div>"};
    auto &scroller = fixture.ById(u8"scroller");
    auto &header = fixture.ById(u8"header");
    auto &footer = fixture.ById(u8"footer");

    // The footer, laid out below, sticks 5 pixels above the scroller's bottom until it's reached.
    CHECK(header.GetBoundingClientRect()->Y() == 20);
    CHECK(footer.GetBoundingClientRect()->Y() == 20 + 60 - 5 - 10);

    scroller.SetScrollTop(80);
    (void)fixture.Document->UpdateLayout();
    CHECK(header.GetBoundingClientRect()->Y() == 20);
    CHECK(footer.GetBoundingClientRect()->Y() == 20 + 60 - 5 - 10);

    // A table header cell sticks to the top of its scroller.
    auto &tableScroller = fixture.ById(u8"table-scroller");
    auto &cell = fixture.ById(u8"cell");
    const double tableTop = tableScroller.GetBoundingClientRect()->Y();
    tableScroller.SetScrollTop(50);
    (void)fixture.Document->UpdateLayout();
    CHECK(cell.GetBoundingClientRect()->Y() == tableTop);

    // An inline box sticks 5 pixels from the left of a box scrolled sideways.
    auto &wide = fixture.ById(u8"wide");
    auto &inl = fixture.ById(u8"inline");
    wide.SetScrollLeft(150);
    (void)fixture.Document->UpdateLayout();
    CHECK(inl.GetBoundingClientRect()->X() == 10 + 5);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LocalFrameView: painting is scrolled",
                   "[Page][LocalFrameView][Scrolling]")
  {
    ScrollFixture fixture {TallPage};
    fixture.View().SetScrollPosition({30, 40});
    auto painted = fixture.PaintedScrolled();
    // The contents move by the scroll position, clipped to the visible part.
    CHECK(painted.starts_with(u8R"(
(save)
(translate
  (x -30.00)
  (y -40.00))
(clip
  (rect at (30,40) size 179x79)))"));
    CHECK(painted.ends_with(u8"(restore)"));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LocalFrameView: scrolling into view",
                   "[Page][LocalFrameView][Scrolling]")
  {
    ScrollFixture fixture {utf8_string(TallPage)
                           + u8"<a id=link href=x style=display:block;height:20px>link</a>"};
    auto &view = fixture.View();

    SECTION("scrollIntoView()")
    {
      fixture.ById(u8"target").ScrollIntoView();
      CHECK(view.GetScrollPosition() == ScrollPosition(0, 300));
    }

    SECTION("focusing")
    {
      // Tab focuses the link, below the view, and scrolls it into view on a zero-delay timer.
      REQUIRE(html::FocusAlgorithms::SequentialNavigation(*fixture.Document, dom::FocusDirection::Forward));
      auto &link = fixture.ById(u8"link");
      REQUIRE(fixture.Document->FocusedElement() == &link);
      CHECK(view.GetScrollPosition() == ScrollPosition());
      html::EventLoop::Current().RunUntilIdle();
      auto rect = link.GetBoundingClientRect();
      CHECK(rect->Y() >= 0);
      CHECK(rect->Y() + rect->Height() <= 100);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LocalFrameView: scroll events",
                   "[Page][LocalFrameView][Scrolling]")
  {
    ScrollFixture fixture {
      utf8_string(TallPage)
      + u8"<div id=box style=overflow:auto;height:50px><div style=height:200px></div></div>"};
    List<std::string> records;
    Ref<dom::EventListenerCallback> recorder = CreateRef<EventRecorder>(records);
    for (auto type : {u8"scroll", u8"scrollend"})
    {
      fixture.Document->AddEventListener(dom::DOMStringAtom(type), Ref {recorder}, true);
    }

    // The view's scroll fires at the document, bubbling, at the next scroll steps, once however often it
    // scrolled.
    fixture.View().SetScrollPosition({0, 100});
    fixture.View().SetScrollPosition({0, 150});
    CHECK(records.empty());
    fixture.Document->RunScrollSteps();
    CHECK(records == Strings {"scroll:document^", "scrollend:document^"});
    records.clear();
    fixture.Document->RunScrollSteps();
    CHECK(records.empty());

    // Scrolling to where it is fires nothing.
    fixture.View().SetScrollPosition({0, 150});
    fixture.Document->RunScrollSteps();
    CHECK(records.empty());

    // A box's scroll fires at the box, without bubbling.
    fixture.ById(u8"box").SetScrollTop(30);
    fixture.Document->RunScrollSteps();
    CHECK(records == Strings {"scroll:box", "scrollend:box"});
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LocalFrameView: the view's scrollbars follow the mouse",
                   "[Page][LocalFrameView][Scrolling]")
  {
    ScrollFixture fixture {utf8_string(TallPage) + u8"<input id=field>"};
    auto &view = fixture.View();
    auto &handler = fixture.Frame->GetEventHandler();
    REQUIRE(view.VerticalScrollbar());
    auto &vBar = *view.VerticalScrollbar();
    // The vertical scrollbar runs down the right of the view, above the horizontal one.
    CHECK(vBar.FrameRect() == IntRect(179, 0, 21, 79));
    CHECK(view.ScrollbarAtPoint({190, 10}) == &vBar);
    CHECK(view.ScrollbarAtPoint({170, 10}) == nullptr);

    SECTION("moving over a scrollbar hovers its parts and shows the pointer")
    {
      Move(handler, {190, ThumbCenter(vBar)});
      CHECK(vBar.HoveredPart() == ThumbPart);
      CHECK(handler.CurrentMouseCursor().GetType() == Cursor::Type::Pointer);
      Move(handler, {190, 75});
      CHECK(vBar.HoveredPart() == ForwardTrackPart);
      Move(handler, {100, 10});
      CHECK(vBar.HoveredPart() == NoPart);
    }

    SECTION("dragging the thumb scrolls in proportion")
    {
      int32 thumb = ThumbCenter(vBar);
      Press(handler, {190, thumb});
      CHECK(vBar.PressedPart() == ThumbPart);
      // The drag carries on outside the scrollbar.
      Move(handler, {100, thumb + 10});
      CHECK(view.GetScrollPosition().Y() == static_cast<int32>(10 * ScrollPerThumbPixel(vBar)));
      Release(handler, {100, thumb + 10});
      CHECK(vBar.PressedPart() == NoPart);

      // Released, the mouse moves the thumb no more.
      auto scrolled = view.GetScrollPosition();
      Move(handler, {100, thumb + 20});
      CHECK(view.GetScrollPosition() == scrolled);
    }

    SECTION("pressing the track scrolls by a page")
    {
      Press(handler, {190, 75});
      CHECK(view.GetScrollPosition() == ScrollPosition(0, vBar.PageStep()));
      // Four fifths of the 79 pixels in view.
      CHECK(vBar.PageStep() == 63);

      // Held, it pages again after a quarter of a second, then every 50 milliseconds, until the thumb reaches
      // the mouse; the release stops it.
      auto &loop = html::EventLoop::Current();
      loop.Run(loop.Now() + std::chrono::milliseconds(249));
      CHECK(view.GetScrollPosition() == ScrollPosition(0, 63));
      loop.Run(loop.Now() + std::chrono::milliseconds(1));
      CHECK(view.GetScrollPosition() == ScrollPosition(0, 126));
      loop.Run(loop.Now() + std::chrono::milliseconds(50));
      CHECK(view.GetScrollPosition() == ScrollPosition(0, 189));
      Release(handler, {190, 75});
      loop.Run(loop.Now() + std::chrono::seconds(1));
      CHECK(view.GetScrollPosition() == ScrollPosition(0, 189));
    }

    SECTION("pressing a scrollbar leaves the focus alone")
    {
      auto &field = fixture.ById(u8"field");
      Downcast<html::HTMLElement>(field).Focus();
      REQUIRE(fixture.Document->FocusedElement() == &field);
      Press(handler, {190, 75});
      Release(handler, {190, 75});
      CHECK(fixture.Document->FocusedElement() == &field);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LocalFrameView: a box's scrollbars follow the mouse",
                   "[Page][LocalFrameView][Scrolling]")
  {
    ScrollFixture fixture {
      u8"<!DOCTYPE html><style>body{margin:0}</style>"
      u8"<div id=box style=position:absolute;top:0;width:100px;height:90px;overflow:scroll>"
      u8"<div style=height:300px></div></div>"
      u8"<input id=field style=position:absolute;left:120px;top:0;width:60px>"};
    auto &handler = fixture.Frame->GetEventHandler();
    auto &box = fixture.ById(u8"box");
    CHECK_FALSE(fixture.View().VerticalScrollbar());

    // The box's vertical scrollbar is on its right, 21 pixels wide; over it is the pointer.
    Move(handler, {90, 60});
    CHECK(handler.CurrentMouseCursor().GetType() == Cursor::Type::Pointer);

    SECTION("pressing the track scrolls by a page")
    {
      Press(handler, {90, 60});
      Release(handler, {90, 60});
      // Four fifths of the 69 pixels in view.
      CHECK(box.ScrollTop() == 55);
    }

    SECTION("pressing a scrollbar doesn't move the focus to the viewport")
    {
      auto &field = fixture.ById(u8"field");
      Downcast<html::HTMLElement>(field).Focus();
      Press(handler, {90, 60});
      Release(handler, {90, 60});
      CHECK(fixture.Document->FocusedElement() == &field);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LocalFrameView: the wheel scrolls",
                   "[Page][LocalFrameView][Scrolling]")
  {
    // The box is fixed at the top of the view: 50 pixels tall, with 150 more to scroll.
    ScrollFixture fixture {utf8_string(TallPage)
                           + u8"<div id=box style=position:fixed;top:0;width:100px;height:50px;overflow:auto>"
                             u8"<div style=height:200px></div></div>"};
    auto &view = fixture.View();
    auto &handler = fixture.Frame->GetEventHandler();
    auto &box = fixture.ById(u8"box");

    SECTION("the box under the mouse scrolls, then the view once it reaches its end")
    {
      CHECK(Wheel(handler, {20, 20}, {0, 100}));
      CHECK(box.ScrollTop() == 100);
      CHECK(view.GetScrollPosition() == ScrollPosition());
      // Only 50 pixels are left.
      CHECK(Wheel(handler, {20, 20}, {0, 100}));
      CHECK(box.ScrollTop() == 150);
      CHECK(Wheel(handler, {20, 20}, {0, 100}));
      CHECK(box.ScrollTop() == 150);
      CHECK(view.GetScrollPosition() == ScrollPosition(0, 100));

      // Back up: the box scrolls first again.
      CHECK(Wheel(handler, {20, 20}, {0, -100}));
      CHECK(box.ScrollTop() == 50);
      CHECK(view.GetScrollPosition() == ScrollPosition(0, 100));
    }

    SECTION("elsewhere the view scrolls, either way, to its ends")
    {
      CHECK(Wheel(handler, {150, 70}, {0, 100}));
      CHECK(view.GetScrollPosition() == ScrollPosition(0, 100));
      CHECK(Wheel(handler, {150, 70}, {50, 0}));
      CHECK(view.GetScrollPosition() == ScrollPosition(50, 100));
      CHECK(Wheel(handler, {150, 70}, {0, -1'000}));
      CHECK(view.GetScrollPosition() == ScrollPosition(50, 0));
      // At the top, nothing scrolls.
      CHECK_FALSE(Wheel(handler, {150, 70}, {0, -100}));
    }

    SECTION("a page at a time")
    {
      CHECK(Wheel(handler, {150, 70}, {0, 1}, PlatformWheelEventGranularity::ScrollByPageWheelEvent));
      // Four fifths of the 79 pixels in view.
      CHECK(view.GetScrollPosition() == ScrollPosition(0, 63));
    }

    SECTION("overscroll-behavior stops the scroll at the box's end")
    {
      REQUIRE_FALSE(box
                      .SetAttribute(u8"style", u8"position:fixed;top:0;width:100px;height:50px;overflow:auto;"
                                               u8"overscroll-behavior:contain")
                      .HasException());
      (void)fixture.Document->UpdateLayout();
      box.SetScrollTop(150);
      CHECK(Wheel(handler, {20, 20}, {0, 100}));
      CHECK(view.GetScrollPosition() == ScrollPosition());

      // On the view, it stops nothing: there's nothing beyond it.
      CHECK(Wheel(handler, {150, 70}, {0, 100}));
      CHECK(view.GetScrollPosition() == ScrollPosition(0, 100));
    }

    SECTION("a cancelled wheel event scrolls nothing")
    {
      // The document's wheel listeners are passive unless they say otherwise.
      auto canceller = CreateRef<WheelCanceller>();
      dom::AddEventListenerOptions options;
      options.Passive = false;
      fixture.Document->AddEventListener(dom::DOMStringAtom(u8"wheel"),
                                         Ref<dom::EventListenerCallback> {canceller}, options);
      CHECK(Wheel(handler, {20, 20}, {0, 100}));
      CHECK(canceller->DeltaY == 100);
      CHECK(box.ScrollTop() == 0);
      CHECK(view.GetScrollPosition() == ScrollPosition());
    }

    SECTION("a passive listener on the document can't cancel it")
    {
      auto canceller = CreateRef<WheelCanceller>();
      fixture.Document->AddEventListener(dom::DOMStringAtom(u8"wheel"),
                                         Ref<dom::EventListenerCallback> {canceller}, false);
      CHECK(Wheel(handler, {20, 20}, {0, 100}));
      CHECK(canceller->DeltaY == 100);
      CHECK(box.ScrollTop() == 100);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LocalFrameView: the keyboard scrolls",
                   "[Page][LocalFrameView][Scrolling]")
  {
    ScrollFixture fixture {
      utf8_string(TallPage)
      + u8"<div id=box tabindex=0 style=position:fixed;top:0;width:100px;height:50px;overflow:auto>"
        u8"<div style=height:200px></div></div>"};
    auto &view = fixture.View();
    auto &handler = fixture.Frame->GetEventHandler();
    auto down = [&]
    {
      return Key(handler, u8"ArrowDown", u8"Down", vk::Down);
    };

    SECTION("the view, by lines, pages and to its ends")
    {
      // A line is 40 pixels.
      CHECK(down());
      CHECK(view.GetScrollPosition() == ScrollPosition(0, 40));
      CHECK(Key(handler, u8"ArrowRight", u8"Right", vk::Right));
      CHECK(view.GetScrollPosition() == ScrollPosition(40, 40));
      CHECK(Key(handler, u8"ArrowLeft", u8"Left", vk::Left));
      CHECK(Key(handler, u8"ArrowUp", u8"Up", vk::Up));
      CHECK(view.GetScrollPosition() == ScrollPosition());

      // A page is four fifths of the 79 pixels in view.
      CHECK(Key(handler, u8"PageDown", u8"PageDown", vk::Next));
      CHECK(view.GetScrollPosition() == ScrollPosition(0, 63));
      CHECK(Key(handler, u8"PageUp", u8"PageUp", vk::Prior));
      CHECK(view.GetScrollPosition() == ScrollPosition());

      CHECK(Key(handler, u8"End", u8"End", vk::End));
      CHECK(view.GetScrollPosition() == ScrollPosition(0, 521));
      CHECK(Key(handler, u8"Home", u8"Home", vk::Home));
      CHECK(view.GetScrollPosition() == ScrollPosition());

      // At the top, Up does nothing.
      CHECK_FALSE(Key(handler, u8"ArrowUp", u8"Up", vk::Up));

      // With modifiers, the page keys do nothing.
      CHECK_FALSE(
        Key(handler, u8"PageDown", u8"PageDown", vk::Next, {}, {PlatformEvent::Modifier::ShiftKey}));
      CHECK(view.GetScrollPosition() == ScrollPosition());
    }

    SECTION("Space pages down, and Shift+Space up")
    {
      CHECK(Key(handler, u8" ", u8"U+0020", vk::Space, u8" "));
      CHECK(view.GetScrollPosition() == ScrollPosition(0, 63));
      CHECK(Key(handler, u8" ", u8"U+0020", vk::Space, u8" ", {PlatformEvent::Modifier::ShiftKey}));
      CHECK(view.GetScrollPosition() == ScrollPosition());
    }

    SECTION("the focused box scrolls first, then the view")
    {
      auto &box = fixture.ById(u8"box");
      Downcast<html::HTMLElement>(box).Focus();
      CHECK(down());
      CHECK(box.ScrollTop() == 40);
      CHECK(Key(handler, u8"End", u8"End", vk::End));
      CHECK(box.ScrollTop() == 150);
      CHECK(view.GetScrollPosition() == ScrollPosition());
      CHECK(down());
      CHECK(view.GetScrollPosition() == ScrollPosition(0, 40));
    }

    SECTION("a cancelled keydown scrolls nothing")
    {
      fixture.Document->AddEventListener(dom::DOMStringAtom(u8"keydown"),
                                         Ref<dom::EventListenerCallback> {CreateRef<WheelCanceller>()},
                                         false);
      CHECK(down());
      CHECK(view.GetScrollPosition() == ScrollPosition());
    }

    SECTION("a view whose root hides overflow doesn't scroll")
    {
      REQUIRE_FALSE(dom::TreeQueries::DocumentElement(*fixture.Document)
                      ->SetAttribute(u8"style", u8"overflow:hidden")
                      .HasException());
      (void)fixture.Document->UpdateLayout();
      CHECK_FALSE(down());
      CHECK(view.GetScrollPosition() == ScrollPosition());
    }
  }
}
