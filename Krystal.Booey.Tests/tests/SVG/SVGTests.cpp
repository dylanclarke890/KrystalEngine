#include "Krystal.Booey/SVG/SVGTests.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey.Tests/SVG/TestSVGElement.hpp"
#include "Krystal.Booey/Core/PlatformLocale.hpp"

namespace krys::boo::svg::tests
{
  using style::tests::ResolverFixture;

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGTests: conditional processing", "[SVG][SVGTests]")
  {
    auto document = ResolverFixture::Parse(u8"<svg id=root></svg>");
    auto element = CreateTestSVGElement(*document);
    REQUIRE_FALSE(document->GetElementById(u8"root")->AppendChild(*element).HasException());

    // Without the attributes, the element doesn't have its conditional processing attributes.
    CHECK(element->SVGElement::ConditionalProcessingAttributesIfExists() == nullptr);
    CHECK(element->IsValid());

    CHECK(SVGTests::HasExtension(u8"http://www.w3.org/1999/xhtml"));
    CHECK(SVGTests::HasExtension(u8"http://www.w3.org/1998/Math/MathML"));
    CHECK_FALSE(SVGTests::HasExtension(u8"http://www.w3.org/2000/svg"));

    REQUIRE_FALSE(element
                    ->SetAttribute(u8"requiredExtensions",
                                   u8"http://www.w3.org/1999/xhtml http://www.w3.org/1998/Math/MathML")
                    .HasException());
    CHECK(element->SVGElement::ConditionalProcessingAttributesIfExists() != nullptr);
    CHECK(element->RequiredExtensions().NumberOfItems() == 2);
    CHECK(element->IsValid());
    REQUIRE_FALSE(element->SetAttribute(u8"requiredExtensions", u8"bogus").HasException());
    CHECK_FALSE(element->IsValid());
    // Present but empty, the attribute is false; absent, it's true.
    REQUIRE_FALSE(element->SetAttribute(u8"requiredExtensions", u8"").HasException());
    CHECK(element->RequiredExtensions().NumberOfItems() == 1);
    CHECK_FALSE(element->IsValid());
    element->RemoveAttribute(u8"requiredExtensions");
    CHECK(element->RequiredExtensions().NumberOfItems() == 0);
    CHECK(element->IsValid());

    // A language matches the first two letters of the user's language.
    auto language = DefaultLanguage().substr(0, 2);
    REQUIRE_FALSE(element->SetAttribute(u8"systemLanguage", dom::DOMString {language}).HasException());
    CHECK(element->IsValid());
    REQUIRE_FALSE(
      element->SetAttribute(u8"systemLanguage", dom::DOMString {language} + u8" x-none").HasException());
    CHECK_FALSE(element->IsValid());
  }
}
