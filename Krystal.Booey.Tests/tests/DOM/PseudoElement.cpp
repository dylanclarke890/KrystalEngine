#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::dom::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    const Color Blue {Color::blue};
    const Color Red {Color::red};
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "PseudoElement: ::before and ::after", "[DOM][PseudoElement]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      #a::before { content: "x"; color: blue }
      #a::after { content: "y"; display: block }
      #b::before { color: red }
      #b::after { content: none }
      #c::before { content: "x"; display: none }
      #d::before { content: "x"; display: contents }
      #hidden { display: none }
      #hidden::before { content: "x" }
    </style><div id=a>text</div><div id=b></div><div id=c></div><div id=d></div><div id=hidden></div>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();

    SECTION("Generated content gets a pseudo-element with a renderer")
    {
      auto &a = fixture.ById(u8"a");
      auto *before = a.BeforePseudoElement();
      REQUIRE(before);
      CHECK(Is<PseudoElement>(*before));
      CHECK_FALSE(Is<PseudoElement>(a));
      CHECK(before->GetPseudoElementType() == PseudoElementType::Before);
      CHECK(before->HostElement() == &a);
      CHECK(before->LocalName() == u8"<pseudo>");
      CHECK(&before->NodeDocument() == &document);
      CHECK(before->IsConnected());
      CHECK_FALSE(before->ParentNode());
      CHECK(before->ParentElementInComposedTree() == &a);
      CHECK(a.PseudoElementIfExists({PseudoElementType::Before}).get() == before);
      CHECK_FALSE(a.PseudoElementIfExists({PseudoElementType::FirstLine}));

      auto *renderer = before->Renderer();
      REQUIRE(renderer);
      CHECK(renderer->GetElement() == before);
      CHECK(renderer->GetType() == RenderObject::Type::Inline);
      CHECK(renderer->Style().GetColor() == Blue);
      CHECK(a.ComputedStyle(style::PseudoElementIdentifier {PseudoElementType::Before})
            == &renderer->Style());

      auto *after = a.AfterPseudoElement();
      REQUIRE(after);
      CHECK(after->GetPseudoElementType() == PseudoElementType::After);
      REQUIRE(after->Renderer());
      CHECK(after->Renderer()->GetType() == RenderObject::Type::BlockFlow);
    }

    SECTION("Pseudo-elements without content, or with display: none, aren't generated")
    {
      CHECK_FALSE(fixture.ById(u8"b").BeforePseudoElement());
      CHECK_FALSE(fixture.ById(u8"b").AfterPseudoElement());
      CHECK_FALSE(fixture.ById(u8"c").BeforePseudoElement());
      CHECK_FALSE(fixture.ById(u8"hidden").BeforePseudoElement());
    }

    SECTION("display: contents pseudo-elements get an inline wrapper")
    {
      auto *before = fixture.ById(u8"d").BeforePseudoElement();
      REQUIRE(before);
      CHECK(before->HasDisplayContents());
      REQUIRE(before->Renderer());
      CHECK(before->Renderer()->GetType() == RenderObject::Type::Inline);
      REQUIRE(before->RenderOrDisplayContentsStyle());
      CHECK(before->RenderOrDisplayContentsStyle()->Display() == DisplayType::Contents);
    }

    SECTION("A style change updates the pseudo-element's renderer")
    {
      auto &a = fixture.ById(u8"a");
      auto *before = a.BeforePseudoElement();
      auto *renderer = before->Renderer();
      REQUIRE_FALSE(a.SetAttribute(u8"style", u8"color: red").HasException());
      document.UpdateStyleIfNeeded();
      CHECK(a.BeforePseudoElement() == before);
      CHECK(before->Renderer() == renderer);
      // ::before sets its own color.
      CHECK(renderer->Style().GetColor() == Blue);
    }

    SECTION("Losing the content removes the pseudo-element, and getting it back creates one")
    {
      auto &b = fixture.ById(u8"b");
      REQUIRE_FALSE(b.SetAttribute(u8"id", u8"a").HasException());
      document.UpdateStyleIfNeeded();
      auto before = ShareRefPtr(b.BeforePseudoElement());
      REQUIRE(before);
      CHECK(before->HostElement() == &b);

      REQUIRE_FALSE(b.SetAttribute(u8"id", u8"b").HasException());
      document.UpdateStyleIfNeeded();
      CHECK_FALSE(b.BeforePseudoElement());
      CHECK_FALSE(before->HostElement());
      CHECK_FALSE(before->Renderer());
    }

    SECTION("Hiding the host removes its pseudo-elements")
    {
      auto &a = fixture.ById(u8"a");
      REQUIRE_FALSE(a.SetAttribute(u8"style", u8"display: none").HasException());
      document.UpdateStyleIfNeeded();
      CHECK_FALSE(a.Renderer());
      CHECK_FALSE(a.BeforePseudoElement());
      CHECK_FALSE(a.AfterPseudoElement());
    }

    SECTION("Removing the host from the document removes its pseudo-elements")
    {
      auto a = ShareRef(fixture.ById(u8"a"));
      REQUIRE_FALSE(a->ParentNode()->RemoveChild(*a).HasException());
      CHECK_FALSE(a->BeforePseudoElement());
      CHECK_FALSE(a->AfterPseudoElement());
    }
  }
}
