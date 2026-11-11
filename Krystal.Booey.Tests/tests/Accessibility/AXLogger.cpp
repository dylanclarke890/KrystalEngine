#include "Krystal.Booey/Accessibility/AXLogger.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Accessibility/AccessibilityTestUtils.hpp"

namespace krys::boo::ax::tests
{
  TEST_CASE("AXLogger: the tree as text", "[Accessibility][AXLogger]")
  {
    AccessibilityFixture page(u8"<main><h1>Title</h1><p>Some <a href='#'>link</a>.</p>"
                              u8"<label>Name <input id=name value=Ada></label><button>Save</button></main>");

    SECTION("roles and names")
    {
      CHECK(page.Tree()
            == u8"WebArea\n"
               u8"  LandmarkMain\n"
               u8"    Heading name=\"Title\"\n"
               u8"      StaticText name=\"Title\"\n"
               u8"    Paragraph\n"
               u8"      StaticText name=\"Some\"\n"
               u8"      Link name=\"link\"\n"
               u8"        StaticText name=\"link\"\n"
               u8"      StaticText name=\".\"\n"
               u8"    Label\n"
               u8"      StaticText name=\"Name\"\n"
               u8"      TextField name=\"Name\"\n"
               u8"    Button name=\"Save\"\n");
    }

    SECTION("values and identifiers")
    {
      CHECK(AXLogger::ObjectAsText(
              page.Object(u8"name"),
              {AXStreamOption::Role, AXStreamOption::IdentifierAttribute, AXStreamOption::Value})
            == u8"TextField identifier=\"name\" value=\"Ada\"");
    }
  }
}
