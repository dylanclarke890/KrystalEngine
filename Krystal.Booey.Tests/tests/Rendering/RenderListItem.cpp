#include "Krystal.Booey/Rendering/RenderListItem.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/HTML/HTMLLIElement.hpp"
#include "Krystal.Booey/HTML/HTMLOListElement.hpp"
#include "Krystal.Booey/Rendering/RenderListMarker.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::tests
{
  using css::tests::ToNarrow;
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD RenderListItem &Item(ResolverFixture &fixture, dom::DOMStringView id)
    {
      return Downcast<RenderListItem>(*fixture.ById(id).Renderer());
    }

    KRYS_NODISCARD std::string MarkerText(ResolverFixture &fixture, dom::DOMStringView id)
    {
      auto *marker = Item(fixture, id).MarkerRenderer();
      if (!marker)
      {
        return "(none)";
      }
      marker->UpdateInlineMarginsAndContent();
      return ToNarrow(Item(fixture, id).MarkerTextWithSuffix());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderListItem: values and markers",
                   "[Rendering][RenderListItem]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      #roman { list-style-type: lower-roman }
      #inside { list-style-position: inside }
      #nomarker { list-style-type: none }
      #string { list-style-type: "-> " }
    </style><ol id=ol><li id=l1>a<li id=l2>b<li id=l3 value=10>c<li id=l4>d</ol><ol id=rev reversed><li id=r1>a<li id=r2>b<li id=r3>c</ol><ol id=st start=5><li id=s1>a<li id=s2>b</ol><ul><li id=u1>x</ul><ol id=roman><li id=ro1>a<li id=ro2>b</ol><ul><li id=inside>i<li id=nomarker>n<li id=string>s</ul><div><li id=loose>l</div>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();

    SECTION("Ordinal values")
    {
      CHECK(Item(fixture, u8"l1").Value() == 1);
      CHECK(Item(fixture, u8"l2").Value() == 2);
      CHECK(Item(fixture, u8"l3").Value() == 10);
      CHECK(Item(fixture, u8"l4").Value() == 11);
      CHECK(Item(fixture, u8"r1").Value() == 3);
      CHECK(Item(fixture, u8"r2").Value() == 2);
      CHECK(Item(fixture, u8"r3").Value() == 1);
      CHECK(Item(fixture, u8"r1").IsInReversedOrderedList());
      CHECK(Item(fixture, u8"s1").Value() == 5);
      CHECK(Item(fixture, u8"s2").Value() == 6);
      CHECK(Item(fixture, u8"loose").Value() == 1);
    }

    SECTION("Markers")
    {
      auto &l1 = Item(fixture, u8"l1");
      auto *marker = l1.MarkerRenderer();
      REQUIRE(marker);
      CHECK(marker->RenderName() == u8"RenderListMarker");
      CHECK(marker->ListItem() == &l1);
      CHECK(marker->IsAnonymous());
      CHECK(marker->IsInline());
      CHECK_FALSE(marker->IsInside());
      // The marker is the first child of the list item's first line box.
      CHECK(l1.FirstChild() == marker);

      CHECK(MarkerText(fixture, u8"l1") == "1. ");
      CHECK(MarkerText(fixture, u8"l3") == "10. ");
      CHECK(ToNarrow(Item(fixture, u8"l3").MarkerTextWithoutSuffix()) == "10");
      CHECK(MarkerText(fixture, u8"r1") == "3. ");
      CHECK(MarkerText(fixture, u8"ro2") == "ii. ");
      CHECK(MarkerText(fixture, u8"string") == "-> ");
      CHECK(ToNarrow(Item(fixture, u8"string").MarkerTextWithoutSuffix()) == "-> ");
      REQUIRE(Item(fixture, u8"u1").MarkerRenderer());
      Item(fixture, u8"u1").MarkerRenderer()->UpdateInlineMarginsAndContent();
      CHECK(Item(fixture, u8"u1").MarkerTextWithSuffix() == u8"• ");
      CHECK(Item(fixture, u8"inside").MarkerRenderer()->IsInside());
      CHECK_FALSE(Item(fixture, u8"nomarker").MarkerRenderer());
    }

    SECTION("Attribute changes renumber the list")
    {
      auto &ol = fixture.ById(u8"ol");
      REQUIRE_FALSE(ol.SetAttribute(u8"start", u8"7").HasException());
      CHECK(Item(fixture, u8"l1").Value() == 7);
      CHECK(Item(fixture, u8"l2").Value() == 8);
      CHECK(Downcast<html::HTMLOListElement>(Downcast<html::HTMLElement>(ol)).StartValue() == 7);

      REQUIRE_FALSE(ol.SetAttribute(u8"reversed", u8"").HasException());
      ol.RemoveAttribute(u8"start");
      // Reversed without a start counts down from the number of items.
      CHECK(Item(fixture, u8"l1").Value() == 4);
      CHECK(Item(fixture, u8"l2").Value() == 3);
    }

    SECTION("Inserting and removing items renumbers the list")
    {
      auto created = document.CreateElement(u8"li");
      REQUIRE_FALSE(created.HasException());
      auto inserted = created.ReleaseValue();
      REQUIRE_FALSE(fixture.ById(u8"st").InsertBefore(*inserted, &fixture.ById(u8"s1")).HasException());
      document.UpdateStyleIfNeeded();
      CHECK(Downcast<RenderListItem>(*inserted->Renderer()).Value() == 5);
      CHECK(Item(fixture, u8"s1").Value() == 6);
      CHECK(Item(fixture, u8"s2").Value() == 7);

      REQUIRE_FALSE(inserted->Remove().HasException());
      document.UpdateStyleIfNeeded();
      CHECK(Item(fixture, u8"s1").Value() == 5);
    }

    SECTION("Style changes update or remove the marker")
    {
      auto &l1 = fixture.ById(u8"l1");
      auto *marker = Item(fixture, u8"l1").MarkerRenderer();
      REQUIRE_FALSE(l1.SetAttribute(u8"style", u8"list-style-type: upper-alpha").HasException());
      document.UpdateStyleIfNeeded();
      CHECK(Item(fixture, u8"l1").MarkerRenderer() == marker);
      CHECK(MarkerText(fixture, u8"l1") == "A. ");

      REQUIRE_FALSE(l1.SetAttribute(u8"style", u8"list-style-type: none").HasException());
      document.UpdateStyleIfNeeded();
      CHECK_FALSE(Item(fixture, u8"l1").MarkerRenderer());
    }

    SECTION("The li value attribute reflects as an integer")
    {
      auto &l3 = Downcast<html::HTMLElement>(fixture.ById(u8"l3"));
      CHECK(Downcast<html::HTMLLIElement>(l3).Value() == 10);
    }
  }
}
