#include "Krystal.Booey/HTML/HTMLMapElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/HTMLCollection.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/HTMLAreaElement.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Rendering/HitTestRequest.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"

namespace krys::boo::html::tests
{
  namespace
  {
    // A 100x100 image whose map has a rectangle at the top left, a circle at the bottom right and a triangle
    // at the top right.
    constexpr utf8_stringview MapMarkup =
      u8"<!DOCTYPE html><style>body{margin:0;font:16px sans-serif} "
      u8"img{display:block;width:100px;height:100px}"
      u8"</style><img id=image alt=map usemap=#shapes>"
      u8"<map name=shapes id=map><area id=rect shape=rect coords=0,0,50,50 href=#rect>"
      u8"<area id=circle shape=circle coords=75,75,20 href=#circle>"
      u8"<area id=poly shape=poly coords=60,0,100,0,100,40 href=#poly></map>";

    /// @brief Counts the clicks at each element.
    struct ClickRecorder : public dom::EventListenerCallback
    {
      List<dom::Element *> &Targets;

      explicit ClickRecorder(List<dom::Element *> &targets) noexcept : Targets(targets)
      {
      }

      void HandleEvent(dom::Event &event) noexcept override
      {
        auto *node = DynamicDowncast<dom::Node>(event.Target().get());
        Targets.push_back(node ? DynamicDowncast<dom::Element>(*node) : nullptr);
      }
    };

    struct MapFixture : boo::tests::PaintFixture
    {
      explicit MapFixture(utf8_stringview markup = MapMarkup) noexcept : PaintFixture(utf8_string(markup))
      {
        (void)Document->UpdateLayout();
      }

      KRYS_NODISCARD HTMLAreaElement &Area(dom::DOMStringView id) noexcept
      {
        return Downcast<HTMLAreaElement>(ById(id));
      }

      KRYS_NODISCARD HTMLImageElement &Image() noexcept
      {
        return Downcast<HTMLImageElement>(ById(u8"image"));
      }

      KRYS_NODISCARD HitTestResult HitTest(double x, double y) noexcept
      {
        HitTestResult result(LayoutPoint(x, y));
        Document->HitTest(HitTestRequest(), result);
        return result;
      }

      KRYS_NODISCARD static PlatformMouseEvent Event(PlatformEvent::Type type, double x, double y) noexcept
      {
        return PlatformMouseEvent(DoublePoint(x, y), DoublePoint(x, y), MouseButton::Left, type, 1, {},
                                  MonotonicTime::Now());
      }

      void Click(double x, double y) noexcept
      {
        (void)Frame->GetEventHandler().HandleMousePressEvent(Event(PlatformEvent::Type::MousePressed, x, y));
        (void)Frame->GetEventHandler().HandleMouseReleaseEvent(
          Event(PlatformEvent::Type::MouseReleased, x, y));
      }
    };
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLMapElement: an image's map and a map's image",
                   "[HTML][ImageMaps]")
  {
    MapFixture fixture {u8"<!DOCTYPE html><img id=image usemap=#second><img id=other usemap=second>"
                        u8"<map id=first name=first><area id=area></map><map id=second></map>"
                        u8"<map name=second id=later></map>"};
    auto &image = fixture.Image();
    auto &first = Downcast<HTMLMapElement>(fixture.ById(u8"first"));
    auto &second = Downcast<HTMLMapElement>(fixture.ById(u8"second"));

    // The first map whose id or name is the name after the #.
    CHECK(image.AssociatedMapElement() == &second);
    CHECK(second.ImageElement().get() == &image);
    // Without a #, there's no map.
    CHECK(Downcast<HTMLImageElement>(fixture.ById(u8"other")).AssociatedMapElement() == nullptr);
    CHECK(first.ImageElement() == nullptr);
    CHECK(fixture.Area(u8"area").ImageElement() == nullptr);

    // Names match exactly.
    REQUIRE_FALSE(image.SetAttribute(u8"usemap", u8"#FIRST").HasException());
    CHECK(image.AssociatedMapElement() == nullptr);
    REQUIRE_FALSE(image.SetAttribute(u8"usemap", u8"#first").HasException());
    CHECK(image.AssociatedMapElement() == &first);
    CHECK(fixture.Area(u8"area").ImageElement().get() == &image);

    // areas is live.
    auto areas = first.Areas();
    CHECK(areas->Length() == 1);
    auto added = fixture.Document->CreateElement(u8"area").ReleaseValue();
    REQUIRE_FALSE(first.AppendChild(*added).HasException());
    CHECK(areas->Length() == 2);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLMapElement: hit testing the areas' shapes",
                   "[HTML][ImageMaps]")
  {
    MapFixture fixture;
    auto &image = fixture.Image();

    auto hit = [&](double x, double y)
    {
      return fixture.Document->ElementFromPoint(x, y).get();
    };
    CHECK(hit(10, 10) == &fixture.ById(u8"rect"));
    CHECK(hit(75, 75) == &fixture.ById(u8"circle"));
    CHECK(hit(95, 5) == &fixture.ById(u8"poly"));
    // Outside every shape: the circle's bounding box corner, and below the triangle's diagonal.
    CHECK(hit(58, 58) == &image);
    CHECK(hit(65, 30) == &image);
    CHECK(hit(30, 80) == &image);

    // The area is the link, and the image the inner non-shared node.
    auto result = fixture.HitTest(10, 10);
    CHECK(result.URLElement() == &fixture.ById(u8"rect"));
    CHECK(result.InnerNonSharedNode() == &image);

    SECTION("a default area takes what the others don't")
    {
      auto area = fixture.Document->CreateElement(u8"area").ReleaseValue();
      REQUIRE_FALSE(area->SetAttribute(u8"shape", u8"DEFAULT").HasException());
      REQUIRE_FALSE(
        fixture.ById(u8"map").InsertBefore(*area, fixture.ById(u8"map").FirstChild()).HasException());
      (void)fixture.Document->UpdateLayout();
      CHECK(hit(30, 80) == area.get());
      CHECK(hit(10, 10) == &fixture.ById(u8"rect"));
    }

    SECTION("changing the shape and coordinates")
    {
      auto &rect = fixture.Area(u8"rect");
      REQUIRE_FALSE(rect.SetAttribute(u8"coords", u8"20,60,40,90").HasException());
      CHECK(hit(10, 10) == &image);
      CHECK(hit(30, 80) == &rect);
      REQUIRE_FALSE(rect.SetAttribute(u8"shape", u8"circ").HasException());
      // A circle at (20, 60) with a radius of 40.
      CHECK(hit(20, 30) == &rect);
      // Too few coordinates for a polygon.
      REQUIRE_FALSE(rect.SetAttribute(u8"shape", u8"polygon").HasException());
      CHECK(hit(20, 60) == &image);
    }

    SECTION("an image without its map")
    {
      image.RemoveAttribute(u8"usemap");
      CHECK(hit(10, 10) == &image);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLAreaElement: clicks and the cursor", "[HTML][ImageMaps]")
  {
    MapFixture fixture;
    List<dom::Element *> clicked;
    fixture.Document->AddEventListener(dom::DOMStringAtom(u8"click"), CreateRef<ClickRecorder>(clicked),
                                       true);

    fixture.Click(10, 10);
    fixture.Click(30, 80);
    CHECK(clicked == List<dom::Element *> {&fixture.ById(u8"rect"), &fixture.ById(u8"image")});

    (void)fixture.Frame->GetEventHandler().MouseMoved(fixture.Event(PlatformEvent::Type::MouseMoved, 75, 75));
    CHECK(fixture.Frame->GetEventHandler().CurrentMouseCursor().GetType() == Cursor::Type::Hand);
    (void)fixture.Frame->GetEventHandler().MouseMoved(fixture.Event(PlatformEvent::Type::MouseMoved, 30, 80));
    CHECK(fixture.Frame->GetEventHandler().CurrentMouseCursor().GetType() != Cursor::Type::Hand);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLAreaElement: focus", "[HTML][ImageMaps][Focus]")
  {
    MapFixture fixture {utf8_string(MapMarkup) + u8"<area id=loose href=#loose><button id=after></button>"};
    auto &document = *fixture.Document;
    auto &rect = fixture.Area(u8"rect");

    // Links in the map of a rendered image are focusable areas.
    CHECK(FocusAlgorithms::IsFocusableArea(rect));
    CHECK_FALSE(FocusAlgorithms::IsFocusableArea(fixture.ById(u8"loose")));
    rect.RemoveAttribute(u8"href");
    CHECK_FALSE(FocusAlgorithms::IsFocusableArea(rect));
    REQUIRE_FALSE(rect.SetAttribute(u8"tabindex", u8"0").HasException());
    CHECK(FocusAlgorithms::IsFocusableArea(rect));

    SECTION("not while the image isn't rendered")
    {
      REQUIRE_FALSE(fixture.Image().SetAttribute(u8"style", u8"display:none").HasException());
      document.UpdateStyleIfNeeded();
      CHECK_FALSE(FocusAlgorithms::IsFocusableArea(rect));
    }

    SECTION("sequential navigation goes through the areas")
    {
      List<dom::Element *> order;
      for (int32 i = 0; i < 5; ++i)
      {
        REQUIRE(FocusAlgorithms::SequentialNavigation(document, dom::FocusDirection::Forward));
        order.push_back(document.FocusedElement());
      }
      CHECK(order
            == List<dom::Element *> {&rect, &fixture.ById(u8"circle"), &fixture.ById(u8"poly"),
                                     &fixture.ById(u8"after"), &rect});
    }

    SECTION("clicking an area focuses it")
    {
      fixture.Click(75, 75);
      CHECK(document.FocusedElement() == &fixture.ById(u8"circle"));
      fixture.Click(30, 80);
      CHECK(document.FocusedElement() == nullptr);
    }

    SECTION("the image shows the focus")
    {
      CHECK(rect.FocusAppearanceUpdateTarget().get() == &fixture.Image());
      CHECK(fixture.ById(u8"loose").FocusAppearanceUpdateTarget() == nullptr);
      CHECK(fixture.ById(u8"after").FocusAppearanceUpdateTarget().get() == &fixture.ById(u8"after"));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLAreaElement: the focus ring",
                   "[HTML][ImageMaps][Rendering]")
  {
    MapFixture fixture {utf8_string(MapMarkup)
                        + u8"<style>area:focus-visible{outline:auto 3px #ff0000}</style>"};
    auto unfocused = fixture.Painted();
    CHECK(unfocused.find(u8"focus-ring") == utf8_string::npos);

    // A focus ring in the area's shape.
    fixture.Area(u8"circle").Focus({.FocusVisible = true});
    auto painted = fixture.Painted();
    CHECK(painted.find(u8"focus-ring") != utf8_string::npos);
    CHECK(painted.find(u8"#FF0000") != utf8_string::npos);

    // Not without the focus visible, as the outline is none.
    fixture.Area(u8"rect").Focus({.FocusVisible = false});
    CHECK(fixture.Painted() == unfocused);

    // Nor when the window isn't active.
    fixture.Area(u8"circle").Focus({.FocusVisible = true});
    fixture.Document->SetFocusedAndActive(false);
    CHECK(fixture.Painted() == unfocused);
  }
}
