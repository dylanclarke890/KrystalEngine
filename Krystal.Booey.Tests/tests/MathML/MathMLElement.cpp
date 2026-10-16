#include "Krystal.Booey/MathML/MathMLElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/CSS/Properties/ImmutableStyleProperties.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::mathml::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD std::string Hints(utf8_string markup) noexcept
    {
      auto document = ResolverFixture::Parse(krys::move(markup));
      auto element = document->GetElementById(u8"e");
      krys_assert(element);
      const auto *style = Downcast<dom::StyledElement>(*element).PresentationalHintStyle();
      return style ? css::tests::ToNarrow(style->AsText(css::DefaultSerialisationContext())) : std::string {};
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "MathMLElement: presentational hints",
                   "[MathML][PresentationalHints]")
  {
    CHECK(Hints(u8"<math><mi id=e mathcolor=red mathbackground=blue dir=rtl></mi></math>")
          == "color: red; background-color: blue; direction: rtl;");
    CHECK(Hints(u8"<math><mi id=e mathsize=big></mi></math>") == "font-size: 1.5em;");
    CHECK(Hints(u8"<math><mi id=e mathsize=2></mi></math>") == "font-size: 200%;");
    CHECK(Hints(u8"<math><mi id=e mathsize=12px></mi></math>") == "font-size: 12px;");
    CHECK(Hints(u8"<math><mstyle id=e displaystyle=TRUE></mstyle></math>") == "math-style: normal;");
    CHECK(Hints(u8"<math><mstyle id=e displaystyle=false></mstyle></math>") == "math-style: compact;");
    CHECK(Hints(u8"<math><mstyle id=e displaystyle=maybe></mstyle></math>").empty());
    CHECK(Hints(u8"<math><mstyle id=e scriptlevel=2></mstyle></math>") == "math-depth: 2;");
    CHECK(Hints(u8"<math><mstyle id=e scriptlevel=+1></mstyle></math>") == "math-depth: add(1);");
    CHECK(Hints(u8"<math><mstyle id=e scriptlevel=x></mstyle></math>").empty());
    // Deprecated attributes.
    CHECK(
      Hints(u8"<math><mi id=e fontsize=20px fontweight=bold fontstyle=italic fontfamily=serif></mi></math>")
      == "font-size: 20px; font-weight: bold; font-style: italic; font-family: serif;");
  }
}
