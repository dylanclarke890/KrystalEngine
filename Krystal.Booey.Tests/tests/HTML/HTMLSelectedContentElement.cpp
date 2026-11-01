#include "Krystal.Booey/HTML/HTMLSelectedContentElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/HTML/FormTestUtils.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"

namespace krys::boo::html::tests
{
  using boo::tests::FormFixture;

  namespace
  {
    KRYS_NODISCARD std::string Text(const dom::Node &node) noexcept
    {
      auto text = node.TextContent().value_or(dom::DOMString());
      return std::string(reinterpret_cast<const char *>(text.data()), text.size());
    }

    void RunMicrotasks() noexcept
    {
      EventLoop::Current().RunUntilIdle();
    }
  }

  TEST_CASE("HTMLSelectedContentElement: mirrors the select's first selected option",
            "[HTML][HTMLSelectedContentElement][html:update-a-select's-descendant-selectedcontent-elements]"
            "[html:update-a-selectedcontent][html:clone-an-option-into-a-selectedcontent]"
            "[html:clear-a-selectedcontent][html:update-descendant-selectedcontent-elements-for-an-option]")
  {
    FormFixture fixture(
      u8"<form id=f><select id=s><button><selectedcontent id=sc></selectedcontent></button>"
      u8"<option id=one>One</option><option id=two selected>Two <b>bold</b></option></select></form>");
    auto &select = fixture.Get<HTMLSelectElement>(u8"s");
    auto &selectedcontent = fixture.Get<HTMLSelectedContentElement>(u8"sc");
    CHECK(Text(selectedcontent) == "Two bold");
    CHECK(selectedcontent.FirstElementChild());

    SECTION("selectedIndex, value and selected")
    {
      select.SelectedIndex(0);
      CHECK(Text(selectedcontent) == "One");
      select.Value(u8"nothing");
      CHECK(Text(selectedcontent).empty());
      fixture.Get<HTMLOptionElement>(u8"two").Selected(true);
      CHECK(Text(selectedcontent) == "Two bold");
    }

    SECTION("Resetting, and a clean option's selected attribute")
    {
      select.SelectedIndex(0);
      fixture.Get<HTMLFormElement>(u8"f").Reset();
      CHECK(Text(selectedcontent) == "Two bold");
      fixture.Get<HTMLOptionElement>(u8"one").DefaultSelected(true);
      CHECK(Text(selectedcontent) == "One");
    }

    SECTION("Removing the selected option updates it in a microtask")
    {
      (void)fixture.Get<HTMLOptionElement>(u8"two").Remove();
      CHECK(Text(selectedcontent) == "Two bold");
      RunMicrotasks();
      CHECK(Text(selectedcontent) == "One");
    }

    SECTION("It isn't kept up to date while the select has multiple")
    {
      select.Multiple(true);
      fixture.Get<HTMLOptionElement>(u8"two").Selected(false);
      fixture.Get<HTMLOptionElement>(u8"one").Selected(true);
      CHECK(Text(selectedcontent) == "Two bold");
      select.Multiple(false);
      CHECK(Text(selectedcontent) == "One");
    }

    SECTION("An inserted selectedcontent mirrors the selected option straight away")
    {
      auto inserted = fixture.Document->CreateElement(u8"selectedcontent").ReleaseValue();
      REQUIRE_FALSE(select.AppendChild(*inserted).HasException());
      CHECK(Text(*inserted) == "Two bold");
    }
  }

  TEST_CASE(
    "HTMLSelectedContentElement: one in an option, another selectedcontent or a nested select is disabled",
    "[HTML][HTMLSelectedContentElement][html:recalculate-a-selectedcontent-element's-disabledness]"
    "[html:selectedcontent-disabled][html:the-selectedcontent-element]")
  {
    FormFixture fixture(u8"<select id=s><option id=o>A</option></select>");
    auto &select = fixture.Get<HTMLSelectElement>(u8"s");
    auto &document = *fixture.Document;

    auto inOption = document.CreateElement(u8"selectedcontent").ReleaseValue();
    REQUIRE_FALSE(fixture.Get<HTMLOptionElement>(u8"o").AppendChild(*inOption).HasException());
    CHECK(Downcast<HTMLSelectedContentElement>(*inOption).Disabled());

    auto outer = document.CreateElement(u8"selectedcontent").ReleaseValue();
    auto inner = document.CreateElement(u8"selectedcontent").ReleaseValue();
    REQUIRE_FALSE(outer->AppendChild(*inner).HasException());
    REQUIRE_FALSE(select.AppendChild(*outer).HasException());
    CHECK_FALSE(Downcast<HTMLSelectedContentElement>(*outer).Disabled());
    CHECK(Downcast<HTMLSelectedContentElement>(*inner).Disabled());

    auto nestedSelect = document.CreateElement(u8"select").ReleaseValue();
    auto inNested = document.CreateElement(u8"selectedcontent").ReleaseValue();
    REQUIRE_FALSE(nestedSelect->AppendChild(*inNested).HasException());
    REQUIRE_FALSE(select.AppendChild(*nestedSelect).HasException());
    CHECK(Downcast<HTMLSelectedContentElement>(*inNested).Disabled());
  }
}
