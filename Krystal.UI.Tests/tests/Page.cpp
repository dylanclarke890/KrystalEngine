#include "Krystal.UI/Page.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include <filesystem>
#include <fstream>

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
      file << "<!DOCTYPE html><style>body { margin: 0 }</style><div id=box style='width: 75px'>x</div>";
    }

    Page page({200, 100});
    REQUIRE(page.LoadFile(path).has_value());
    page.UpdateRendering();
    CHECK(WidthOf(page, u8"box") == 75);
    std::filesystem::remove(path);

    CHECK_FALSE(page.LoadFile(path).has_value());
  }

  TEST_CASE("Page: records nothing until painting is ported", "[UI]")
  {
    Page page({200, 100});
    page.LoadHTML(u8"<!DOCTYPE html><div style='background: red'>x</div>");
    CHECK(page.Record()->Items().empty());
  }
}
