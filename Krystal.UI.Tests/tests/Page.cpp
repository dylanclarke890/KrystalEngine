#include "Krystal.UI/Page.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include <filesystem>
#include <fstream>
#include <string>

namespace krys::ui::tests
{
  namespace
  {
    KRYS_NODISCARD int32 WidthOf(Page &page, boo::dom::DOMStringView id)
    {
      auto element = page.Document().GetElementById(id);
      REQUIRE(element);
      return element->OffsetWidth();
    }

    /// @brief Records the presses, releases and clicks it handles as `type:id/detail`.
    struct ClickRecorder : public boo::dom::EventListenerCallback
    {
      List<std::string> &Records;

      explicit ClickRecorder(List<std::string> &records) noexcept : Records(records)
      {
      }

      void HandleEvent(boo::dom::Event &event) noexcept override
      {
        auto &mouse = static_cast<boo::dom::MouseEvent &>(event);
        auto type = mouse.Type().View();
        auto *node = DynamicDowncast<boo::dom::Node>(event.Target().get());
        auto *element = node ? DynamicDowncast<boo::dom::Element>(*node) : nullptr;
        auto id =
          element ? element->GetAttribute(u8"id").value_or(boo::dom::DOMString()) : boo::dom::DOMString();
        Records.push_back(std::string(type.begin(), type.end()) + ":" + std::string(id.begin(), id.end())
                          + "/" + std::to_string(mouse.Detail()) + "/" + std::to_string(mouse.Buttons()));
      }
    };
  }

  TEST_CASE("Page: lays out at the viewport's size", "[UI]")
  {
    Page page({400, 300});
    CHECK(page.ViewportSize() == boo::IntSize(400, 300));
    page.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0 } #half { width: 50% }</style>"
                  u8"<div id=full>a</div><div id=half>b</div>");
    page.UpdateRendering();
    CHECK(WidthOf(page, u8"full") == 400);
    CHECK(WidthOf(page, u8"half") == 200);

    SECTION("A resize lays it out again")
    {
      page.Resize({600, 300});
      page.UpdateRendering();
      CHECK(page.ViewportSize() == boo::IntSize(600, 300));
      CHECK(WidthOf(page, u8"full") == 600);
      CHECK(WidthOf(page, u8"half") == 300);
    }

    SECTION("Loading another document replaces it")
    {
      page.LoadHTML(
        u8"<!DOCTYPE html><style>body { margin: 0 }</style><p id=other style='width: 120px'>c</p>");
      page.UpdateRendering();
      CHECK_FALSE(page.Document().GetElementById(u8"full"));
      CHECK(WidthOf(page, u8"other") == 120);
    }
  }

  TEST_CASE("Page: loads files", "[UI]")
  {
    const auto path = std::filesystem::temp_directory_path() / "krystal-ui-page-test.html";
    {
      std::ofstream file(path, std::ios::binary);
      // A byte order mark is decoded away rather than becoming text above the box.
      file << "\xEF\xBB\xBF<!DOCTYPE html><style>body { margin: 0 }</style><div id=box style='width: "
              "75px'>x</div>";
    }

    Page page({200, 100});
    REQUIRE(page.LoadFile(path).has_value());
    page.UpdateRendering();
    CHECK(WidthOf(page, u8"box") == 75);
    auto box = page.Document().GetElementById(u8"box");
    REQUIRE(box);
    CHECK(box->OffsetTop() == 0);
    std::filesystem::remove(path);

    CHECK_FALSE(page.LoadFile(path).has_value());
  }

  TEST_CASE("Page: records its painting", "[UI]")
  {
    Page page({200, 100});
    page.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0 }</style>"
                  u8"<div style='width: 40px; height: 20px; background: red'></div>");
    const auto displayList = page.Record();
    // The view's base colour, the root's background over the canvas and the box's background.
    REQUIRE(displayList->Items().size() == 5u);
    CHECK(displayList->AsText({}).ends_with(u8R"((fill-composited-rect
  (rect at (0,0) size 40x20)
  (color #FF0000)
  (composite-operation source-over)
  (blend-mode normal)))"));
  }

  TEST_CASE("Page: hit tests, hovers and activates", "[UI]")
  {
    Page page({400, 300});
    page.LoadHTML(
      u8"<!DOCTYPE html><style>body { margin: 0 } div { height: 50px }"
      u8"#a:hover { width: 100px } #a:active { width: 50px }</style><div id=a></div><div id=b></div>");
    auto a = page.Document().GetElementById(u8"a");
    auto b = page.Document().GetElementById(u8"b");
    CHECK(page.HitTest({10, 10}) == a.get());
    CHECK(page.HitTest({10, 60}) == b.get());

    page.MouseMoved({10, 10});
    page.UpdateRendering();
    CHECK(a->Hovered());
    CHECK(WidthOf(page, u8"a") == 100);

    page.MousePressed({10, 10}, boo::MouseButton::Left);
    page.UpdateRendering();
    CHECK(a->Active());
    CHECK(WidthOf(page, u8"a") == 50);

    // While the button is down, only the active chain stays hovered.
    page.MouseMoved({10, 60});
    CHECK_FALSE(b->Hovered());
    page.MouseReleased({10, 60}, boo::MouseButton::Left);
    CHECK_FALSE(a->Active());
    page.MouseMoved({10, 10});
    CHECK(a->Hovered());
    page.MouseMoved({10, 60});
    CHECK(b->Hovered());
    CHECK_FALSE(a->Hovered());

    page.MouseLeft();
    CHECK_FALSE(b->Hovered());
  }

  TEST_CASE("Page: dispatches mouse events and chooses the cursor", "[UI]")
  {
    Page page({400, 300});
    page.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0; font: 16px sans-serif } div { height: 50px }"
                  u8"#b { cursor: pointer }</style><div id=a>text</div><div id=b></div>");
    page.UpdateRendering();
    List<std::string> records;
    Ref<boo::dom::EventListenerCallback> recorder = CreateRef<ClickRecorder>(records);
    for (auto type : {u8"mousedown", u8"mouseup", u8"click", u8"auxclick", u8"dblclick"})
    {
      page.Document().AddEventListener(boo::dom::DOMStringAtom(type), Ref {recorder}, false);
    }

    // The cursor follows the mouse: the arrow over a box, the I-beam over text and the style's cursor.
    page.MouseMoved({300, 10});
    CHECK(page.CurrentCursor().GetType() == boo::Cursor::Type::Pointer);
    page.MouseMoved({3, 10});
    CHECK(page.CurrentCursor().GetType() == boo::Cursor::Type::IBeam);
    page.MouseMoved({10, 60});
    CHECK(page.CurrentCursor().GetType() == boo::Cursor::Type::Hand);

    // Two quick presses in the same place are a double click; the buttons held are reported.
    page.MousePressed({10, 60}, boo::MouseButton::Left);
    page.MouseReleased({10, 60}, boo::MouseButton::Left);
    page.MousePressed({11, 61}, boo::MouseButton::Left);
    page.MouseReleased({11, 61}, boo::MouseButton::Left);
    CHECK(records
          == List<std::string> {"mousedown:b/1/1", "mouseup:b/1/0", "click:b/1/0", "mousedown:b/2/1",
                                "mouseup:b/2/0", "click:b/2/0", "dblclick:b/2/0"});
    records.clear();

    // A press far from the last starts counting again, as does another button.
    page.MousePressed({10, 10}, boo::MouseButton::Left);
    page.MouseReleased({10, 10}, boo::MouseButton::Left);
    page.MousePressed({10, 10}, boo::MouseButton::Right);
    page.MouseReleased({10, 10}, boo::MouseButton::Right);
    CHECK(records
          == List<std::string> {"mousedown:a/1/1", "mouseup:a/1/0", "click:a/1/0", "mousedown:a/1/2",
                                "mouseup:a/1/0", "auxclick:a/1/0"});
  }
}
