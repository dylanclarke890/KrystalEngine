#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    const Color Blue {Color::blue};
    const Color Red {Color::red};
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderElement: elements get renderers",
                   "[Rendering][RenderElement]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      #flex { display: flex }
      #cell { display: table-cell }
      #hidden { display: none }
      #contents { display: contents }
      span { color: blue }
    </style><p id=p>text<span id=span>x</span></p><div id=flex></div><div id=cell></div>
    <div id=hidden></div><div id=contents></div>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();

    SECTION("A rendered element's renderer holds its style")
    {
      auto &p = fixture.ById(u8"p");
      auto *renderer = p.Renderer();
      REQUIRE(renderer);
      CHECK(static_cast<dom::Node &>(p).Renderer() == renderer);
      CHECK(renderer->GetElement() == &p);
      CHECK(renderer->GetNode() == &p);
      CHECK(&renderer->GetDocument() == &document);
      CHECK_FALSE(renderer->IsAnonymous());
      CHECK(renderer->IsRenderElement());
      CHECK_FALSE(renderer->IsRenderText());
      CHECK(p.GetRenderStyle() == &renderer->Style());
      CHECK(renderer->Style().Display() == DisplayType::Block);
      CHECK(renderer->GetType() == RenderObject::Type::BlockFlow);
      CHECK(renderer->RenderName() == u8"RenderBlockFlow");
      CHECK(Is<RenderElement>(static_cast<RenderObject &>(*renderer)));

      auto *span = fixture.ById(u8"span").Renderer();
      REQUIRE(span);
      CHECK(span->GetType() == RenderObject::Type::Inline);
      CHECK(span->Style().GetColor() == Blue);
    }

    SECTION("The display chooses the renderer's type")
    {
      REQUIRE(fixture.ById(u8"flex").Renderer());
      CHECK(fixture.ById(u8"flex").Renderer()->GetType() == RenderObject::Type::FlexibleBox);
      REQUIRE(fixture.ById(u8"cell").Renderer());
      CHECK(fixture.ById(u8"cell").Renderer()->GetType() == RenderObject::Type::TableCell);
      CHECK(fixture.ById(u8"cell").Renderer()->RenderName() == u8"RenderTableCell");
    }

    SECTION("display: none and display: contents don't generate renderers")
    {
      auto &hidden = fixture.ById(u8"hidden");
      CHECK_FALSE(hidden.Renderer());
      CHECK_FALSE(hidden.GetRenderStyle());

      auto &contents = fixture.ById(u8"contents");
      CHECK_FALSE(contents.Renderer());
      CHECK(contents.HasDisplayContents());
      REQUIRE(contents.ExistingComputedStyle());
      CHECK(contents.ExistingComputedStyle()->Display() == DisplayType::Contents);
    }

    SECTION("Text isn't rendered yet")
    {
      auto *text = fixture.ById(u8"p").FirstChild();
      REQUIRE(Is<dom::Text>(text));
      CHECK_FALSE(text->Renderer());
      CHECK_FALSE(text->GetRenderStyle());
    }

    SECTION("A style change updates the renderer's style")
    {
      auto &span = fixture.ById(u8"span");
      auto *renderer = span.Renderer();
      REQUIRE_FALSE(span.SetAttribute(u8"style", u8"color: red").HasException());
      document.UpdateStyleIfNeeded();
      CHECK(span.Renderer() == renderer);
      CHECK(renderer->Style().GetColor() == Red);
    }

    SECTION("Changing display to none tears the renderer down, and back creates one")
    {
      auto &span = fixture.ById(u8"span");
      REQUIRE_FALSE(span.SetAttribute(u8"style", u8"display: none").HasException());
      document.UpdateStyleIfNeeded();
      CHECK_FALSE(span.Renderer());

      REQUIRE_FALSE(span.SetAttribute(u8"style", u8"display: block").HasException());
      document.UpdateStyleIfNeeded();
      REQUIRE(span.Renderer());
      CHECK(span.Renderer()->GetType() == RenderObject::Type::BlockFlow);
    }

    SECTION("Removing an element from the document tears its renderers down")
    {
      auto p = ShareRef(fixture.ById(u8"p"));
      auto span = ShareRef(fixture.ById(u8"span"));
      REQUIRE_FALSE(p->ParentNode()->RemoveChild(*p).HasException());
      CHECK_FALSE(p->Renderer());
      CHECK_FALSE(span->Renderer());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderElement: elements choose their renderers",
                   "[Rendering][RenderElement]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      .inline { display: inline }
      .cell { display: table-cell }
      .row-group { display: table-row-group }
      .list-item { display: list-item }
      .flex { display: flex }
      .none { display: none }
    </style><fieldset id=f-inline class=inline></fieldset><fieldset id=f-cell class=cell></fieldset>
    <fieldset id=f-row-group class=row-group></fieldset><fieldset id=f-list-item class=list-item></fieldset>
    <fieldset id=f-flex class=flex></fieldset><fieldset id=f-none class=none></fieldset>
    <span id=s-inline class=inline></span><span id=s-cell class=cell></span>)"};
    fixture.Document->UpdateStyleIfNeeded();

    auto type = [&](utf8_stringview id) noexcept
    {
      auto *renderer = fixture.ById(id).Renderer();
      return renderer ? Maybe<RenderObject::Type> {renderer->GetType()} : Maybe<RenderObject::Type> {};
    };

    SECTION("Fieldsets make a block flow for inline and table displays")
    {
      CHECK(type(u8"f-inline") == RenderObject::Type::BlockFlow);
      CHECK(type(u8"f-cell") == RenderObject::Type::BlockFlow);
      CHECK(type(u8"f-row-group") == RenderObject::Type::BlockFlow);
    }

    SECTION("Fieldsets keep the usual renderer for other displays")
    {
      CHECK(type(u8"f-list-item") == RenderObject::Type::ListItem);
      CHECK(type(u8"f-flex") == RenderObject::Type::FlexibleBox);
      CHECK_FALSE(type(u8"f-none"));
    }

    SECTION("Other elements get the usual renderer")
    {
      CHECK(type(u8"s-inline") == RenderObject::Type::Inline);
      CHECK(type(u8"s-cell") == RenderObject::Type::TableCell);
    }
  }
}
