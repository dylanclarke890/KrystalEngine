#include "Krystal.Booey/Style/Values/Color/SystemColor.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::style::tests
{
  using css::ValueId;

  TEST_CASE("SystemColor", "[Style][Color][SystemColor]")
  {
    SECTION("CSS Color 4 system colours")
    {
      CHECK(SystemColor(ValueId::Canvas, ColorOptions::None) == boo::Color::white);
      CHECK(SystemColor(ValueId::Canvastext, ColorOptions::None) == boo::Color::black);
      CHECK(SystemColor(ValueId::Buttonface, ColorOptions::None) == boo::Color::lightGray);
      CHECK(SystemColor(ValueId::Highlight, ColorOptions::None) == boo::Color {SRGBA<uint8> {181, 213, 255}});
      CHECK(SystemColor(ValueId::Mark, ColorOptions::None) == boo::Color::yellow);
      CHECK(SystemColor(ValueId::Graytext, ColorOptions::None) == boo::Color::darkGray);
      CHECK(SystemColor(ValueId::Accentcolor, ColorOptions::None) == boo::Color {SRGBA<uint8> {0, 122, 255}});
      CHECK(SystemColor(ValueId::Text, ColorOptions::None) == boo::Color::black);
    }

    SECTION("Links depend on the appearance and visited state")
    {
      CHECK(SystemColor(ValueId::Linktext, ColorOptions::None) == boo::Color {SRGBA<uint8> {0, 0, 238}});
      CHECK(SystemColor(ValueId::Linktext, ColorOptions::UseDarkAppearance)
            == boo::Color {SRGBA<uint8> {158, 158, 255}});
      CHECK(SystemColor(ValueId::Visitedtext, ColorOptions::None) == boo::Color {SRGBA<uint8> {85, 26, 139}});
      CHECK(SystemColor(ValueId::Activetext, ColorOptions::None) == boo::Color::red);
      CHECK(SystemColor(ValueId::WebkitActivelink, ColorOptions::UseDarkAppearance)
            == boo::Color {SRGBA<uint8> {255, 158, 158}});

      CHECK(SystemColor(ValueId::WebkitLink, ColorOptions::None)
            == SystemColor(ValueId::Linktext, ColorOptions::None));
      CHECK(SystemColor(ValueId::WebkitLink, ColorOptions::ForVisitedLink)
            == SystemColor(ValueId::Visitedtext, ColorOptions::None));
      CHECK(SystemColor(ValueId::WebkitLink, ColorOptions::ForVisitedLink | ColorOptions::UseDarkAppearance)
            == SystemColor(ValueId::Visitedtext, ColorOptions::UseDarkAppearance));
    }

    SECTION("Deprecated system colours map to their replacements")
    {
      CHECK(SystemColor(ValueId::Activeborder, ColorOptions::None)
            == SystemColor(ValueId::Buttonborder, ColorOptions::None));
      CHECK(SystemColor(ValueId::Buttonshadow, ColorOptions::None)
            == SystemColor(ValueId::Buttonface, ColorOptions::None));
      CHECK(SystemColor(ValueId::Inactivecaptiontext, ColorOptions::None)
            == SystemColor(ValueId::Graytext, ColorOptions::None));
      CHECK(SystemColor(ValueId::Menu, ColorOptions::None)
            == SystemColor(ValueId::Canvas, ColorOptions::None));
      CHECK(SystemColor(ValueId::Windowtext, ColorOptions::None)
            == SystemColor(ValueId::Canvastext, ColorOptions::None));
    }

    SECTION("Anything else is the invalid colour")
    {
      CHECK_FALSE(SystemColor(ValueId::Red, ColorOptions::None).IsValid());
      CHECK_FALSE(SystemColor(ValueId::Currentcolor, ColorOptions::None).IsValid());
      CHECK_FALSE(SystemColor(ValueId::InternalDocumentTextColor, ColorOptions::None).IsValid());
    }
  }
}
