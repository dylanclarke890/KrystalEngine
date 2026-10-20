#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DOMRect.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
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
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LocalFrameView: the scroll position and its range",
                   "[Page][LocalFrameView][Scrolling]")
  {
    ScrollFixture fixture {TallPage};
    auto &view = fixture.View();

    // The contents are the document's size; the range is what doesn't fit in the view.
    CHECK(view.ContentsSize() == IntSize(400, 600));
    CHECK(view.MinimumScrollPosition() == ScrollPosition());
    CHECK(view.MaximumScrollPosition() == ScrollPosition(200, 500));
    CHECK(view.GetScrollPosition() == ScrollPosition());

    view.SetScrollPosition({50, 120});
    CHECK(view.GetScrollPosition() == ScrollPosition(50, 120));
    CHECK(view.VisibleContentRect() == IntRect(50, 120, 200, 100));
    CHECK(view.WindowToContents(IntPoint(10, 10)) == IntPoint(60, 130));
    CHECK(view.ContentsToWindow(IntPoint(60, 130)) == IntPoint(10, 10));

    // Clamped to the range.
    view.SetScrollPosition({1'000, -10});
    CHECK(view.GetScrollPosition() == ScrollPosition(200, 0));

    SECTION("a layout that shrinks the document moves the scroll position back")
    {
      view.SetScrollPosition({0, 400});
      REQUIRE_FALSE(fixture.ById(u8"target").SetAttribute(u8"style", u8"display:none").HasException());
      REQUIRE_FALSE(fixture.ById(u8"page").SetAttribute(u8"style", u8"height:150px").HasException());
      (void)fixture.Document->UpdateLayout();
      CHECK(view.ContentsSize() == IntSize(400, 150));
      CHECK(view.GetScrollPosition() == ScrollPosition(0, 50));
    }

    SECTION("the scrolling element's scroll APIs scroll the view")
    {
      auto &root = *fixture.Document->ScrollingElement();
      CHECK(&root == dom::TreeQueries::DocumentElement(*fixture.Document));
      CHECK(root.ScrollLeft() == 200);
      CHECK(root.ScrollTop() == 0);
      CHECK(root.ScrollWidth() == 400);
      CHECK(root.ScrollHeight() == 600);

      root.SetScrollTop(300);
      CHECK(view.GetScrollPosition() == ScrollPosition(200, 300));
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
  (rect at (30,40) size 200x100)))"));
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
      // Tab focuses the link, below the view, and scrolls it into view.
      REQUIRE(html::FocusAlgorithms::SequentialNavigation(*fixture.Document, dom::FocusDirection::Forward));
      auto &link = fixture.ById(u8"link");
      REQUIRE(fixture.Document->FocusedElement() == &link);
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
}
