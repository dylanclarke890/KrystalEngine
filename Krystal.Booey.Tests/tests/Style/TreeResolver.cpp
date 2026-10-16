#include "Krystal.Booey/Style/TreeResolver.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Iterators/ComposedTreeIterator.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/HTMLStyleElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/ContainerQueryEvaluator.hpp"
#include "Krystal.Booey/Style/Update.hpp"

namespace krys::boo::style::tests
{
  namespace
  {
    const boo::Color Blue {boo::Color::blue};
    const boo::Color Red {boo::Color::red};

    KRYS_NODISCARD Ref<dom::Element> Create(dom::Document &document, dom::DOMStringAtom localName)
    {
      auto element = document.CreateElement(localName);
      REQUIRE_FALSE(element.HasException());
      return element.ReleaseValue();
    }

    KRYS_NODISCARD const RenderStyle &Rendered(const dom::Element &element)
    {
      auto *style = element.GetRenderStyle();
      REQUIRE(style);
      return *style;
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "TreeResolver: resolves a document", "[Style][TreeResolver]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      p { color: rgb(0, 0, 255) }
      .none { display: none }
      span { display: contents; color: rgb(255, 0, 0) }
    </style><p id=p><b id=b>x</b></p><div class=none id=none><i id=i></i></div><span id=c><em id=em></em></span>)"};
    auto &document = *fixture.Document;

    // Showing the document in a view resolves its style and builds its render tree.
    CHECK_FALSE(document.NeedsStyleRecalc());
    CHECK(fixture.ById(u8"p").GetRenderStyle());

    CHECK_FALSE(document.UpdateStyleIfNeeded());
    CHECK_FALSE(document.NeedsStyleRecalc());
    CHECK_FALSE(document.ChildNeedsStyleRecalc());
    CHECK_FALSE(document.UpdateStyleIfNeeded());

    CHECK(Rendered(*document.DocumentElement()).Display() == DisplayType::Block);
    CHECK(Rendered(fixture.ById(u8"p")).GetColor() == Blue);
    CHECK(Rendered(fixture.ById(u8"b")).GetColor() == Blue);
    CHECK(fixture.ById(u8"b").StyleValidity() == Validity::Valid);

    SECTION("display: none")
    {
      auto &none = fixture.ById(u8"none");
      // An element that has never been rendered keeps no style with display: none; it's resolved on demand.
      CHECK_FALSE(none.GetRenderStyle());
      CHECK_FALSE(none.HasDisplayNone());
      CHECK_FALSE(none.ExistingComputedStyle());
      REQUIRE(none.ComputedStyle());
      CHECK(none.ComputedStyle()->Display() == DisplayType::None);

      // Nor are its descendants resolved, but their computed style can be.
      auto &i = fixture.ById(u8"i");
      CHECK_FALSE(i.GetRenderStyle());
      CHECK_FALSE(i.ExistingComputedStyle());
      CHECK_FALSE(i.ResolveComputedStyle(dom::Element::ResolveComputedStyleMode::RenderedOnly));
      auto *computed = i.ComputedStyle();
      REQUIRE(computed);
      CHECK(computed->Display() == DisplayType::Inline);
      CHECK(i.ExistingComputedStyle() == computed);
    }

    SECTION("display: contents")
    {
      auto &c = fixture.ById(u8"c");
      CHECK_FALSE(c.GetRenderStyle());
      CHECK(c.HasDisplayContents());
      REQUIRE(c.RenderOrDisplayContentsStyle());
      CHECK(c.RenderOrDisplayContentsStyle()->Display() == DisplayType::Contents);
      CHECK(Rendered(fixture.ById(u8"em")).GetColor() == Red);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "TreeResolver: restyles after changes", "[Style][TreeResolver]")
  {
    ResolverFixture fixture {
      u8R"(<!DOCTYPE html><style>.x { color: rgb(0, 0, 255) }</style><div id=d class=x></div>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();
    auto &d = fixture.ById(u8"d");

    SECTION("Insertion and removal")
    {
      auto span = Create(document, u8"span");
      REQUIRE_FALSE(d.AppendChild(*span).HasException());
      CHECK(span->StyleValidity() == Validity::SubtreeInvalid);
      CHECK(d.ChildNeedsStyleRecalc());
      CHECK(d.DirectChildNeedsStyleRecalc());
      CHECK(document.NeedsStyleRecalc());
      CHECK_FALSE(span->GetRenderStyle());

      CHECK(document.UpdateStyleIfNeeded());
      CHECK(Rendered(*span).GetColor() == Blue);
      CHECK_FALSE(d.ChildNeedsStyleRecalc());

      REQUIRE_FALSE(d.RemoveChild(*span).HasException());
      CHECK_FALSE(span->GetRenderStyle());
      CHECK_FALSE(span->ComputedStyle());
    }

    SECTION("Inline style")
    {
      auto span = Create(document, u8"span");
      REQUIRE_FALSE(d.AppendChild(*span).HasException());
      document.UpdateStyleIfNeeded();

      REQUIRE_FALSE(d.SetAttribute(u8"style", u8"color: rgb(255, 0, 0)").HasException());
      CHECK(d.StyleValidity() == Validity::InlineStyleInvalid);
      CHECK(document.UpdateStyleIfNeeded());
      CHECK(Rendered(d).GetColor() == Red);
      // The inherited change reaches the children.
      CHECK(Rendered(*span).GetColor() == Red);
    }

    SECTION("Presentational hints")
    {
      REQUIRE_FALSE(d.SetAttribute(u8"hidden", u8"").HasException());
      CHECK(d.StyleValidity() == Validity::ElementInvalid);
      document.UpdateStyleIfNeeded();
      CHECK_FALSE(d.GetRenderStyle());
      CHECK(d.HasDisplayNone());

      d.RemoveAttribute(u8"hidden");
      document.UpdateStyleIfNeeded();
      CHECK(Rendered(d).Display() == DisplayType::Block);
      CHECK_FALSE(d.HasDisplayNone());
    }

    SECTION("Explicit invalidation")
    {
      d.InvalidateStyleForSubtree();
      CHECK(d.StyleValidity() == Validity::SubtreeInvalid);
      CHECK(document.DocumentElement()->ChildNeedsStyleRecalc());
      CHECK(document.UpdateStyleIfNeeded());
      CHECK(d.StyleValidity() == Validity::Valid);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "TreeResolver: the document element and body",
                   "[Style][TreeResolver]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><html style="font-size: 20px">
      <body text=red style="writing-mode: vertical-rl"><p id=p style="width: 2rem"></p></body></html>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();

    CHECK(ResolverFixture::Serialised(Rendered(fixture.ById(u8"p")), Rendered(fixture.ById(u8"p")).Width())
          == "40px");

    // <body>'s colour becomes the document's text colour.
    CHECK(document.TextColor() == Red);

    // <body>'s writing mode propagates to the initial containing block and the document element's used style.
    CHECK(document.RenderViewStyle().GetWritingMode().ComputedWritingMode() == StyleWritingMode::VerticalRl);
    CHECK(Rendered(*document.DocumentElement()).GetWritingMode().ComputedWritingMode()
          == StyleWritingMode::VerticalRl);
    CHECK(document.InitialContainingBlockStyle()->GetWritingMode().ComputedWritingMode()
          == StyleWritingMode::HorizontalTb);

    // A font size change of the document element changes root-relative units everywhere.
    REQUIRE_FALSE(document.DocumentElement()->SetAttribute(u8"style", u8"font-size: 10px").HasException());
    document.UpdateStyleIfNeeded();
    CHECK(ResolverFixture::Serialised(Rendered(fixture.ById(u8"p")), Rendered(fixture.ById(u8"p")).Width())
          == "20px");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "TreeResolver: style relations and pseudo-elements",
                   "[Style][TreeResolver]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      li:first-child { color: rgb(0, 0, 255) }
      li:nth-child(2) { color: rgb(255, 0, 0) }
      p:empty { width: 1px }
      q::before { content: "x"; color: rgb(255, 0, 0) }
      q::first-line { color: rgb(0, 0, 255) }
      q { display: block }
    </style><ul id=u><li id=a></li><li id=b></li></ul><p id=e></p><q id=q>text</q>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();

    auto &u = fixture.ById(u8"u");
    CHECK(u.ChildrenAffectedByFirstChildRules());
    CHECK(u.ChildrenAffectedByForwardPositionalRules());
    CHECK(Rendered(fixture.ById(u8"a")).FirstChildState());
    CHECK(Rendered(fixture.ById(u8"a")).GetColor() == Blue);
    CHECK(fixture.ById(u8"b").ChildIndex() == 2);
    CHECK(Rendered(fixture.ById(u8"b")).GetColor() == Red);
    CHECK(fixture.ById(u8"e").StyleAffectedByEmpty());
    CHECK(Rendered(fixture.ById(u8"e")).EmptyState());

    auto &q = Rendered(fixture.ById(u8"q"));
    auto *before = q.GetCachedPseudoStyle({PseudoElementType::Before});
    REQUIRE(before);
    CHECK(before->GetColor() == Red);
    auto *firstLine = q.GetCachedPseudoStyle({PseudoElementType::FirstLine});
    REQUIRE(firstLine);
    CHECK(firstLine->GetColor() == Blue);
    // ::before has a pseudo-element, whose own style is its computed style.
    auto *beforeElement = fixture.ById(u8"q").BeforePseudoElement();
    REQUIRE(beforeElement);
    CHECK(fixture.ById(u8"q").ComputedStyle(PseudoElementIdentifier {PseudoElementType::Before})
          == beforeElement->GetRenderStyle());
    CHECK(fixture.ById(u8"q").ComputedStyle(PseudoElementIdentifier {PseudoElementType::Before})->GetColor()
          == Red);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "TreeResolver: shadow trees", "[Style][TreeResolver]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><div id=host style="color: rgb(255, 0, 0)"><span id=light></span></div>)html"};
    auto &document = *fixture.Document;
    auto &host = fixture.ById(u8"host");
    document.UpdateStyleIfNeeded();
    REQUIRE(fixture.ById(u8"light").GetRenderStyle());

    auto shadow = host.AttachShadow({}).ReleaseValue();
    // Attaching a shadow root removes the light children from the composed tree until they're slotted.
    CHECK_FALSE(fixture.ById(u8"light").GetRenderStyle());

    auto style = CreateRef<html::HTMLStyleElement>(document);
    REQUIRE_FALSE(style->TextContent(u8"slot { color: rgb(0, 0, 255) } div { width: 5px }").HasException());
    auto inner = Create(document, u8"div");
    auto slot = Create(document, u8"slot");
    REQUIRE_FALSE(shadow->AppendChild(*style).HasException());
    REQUIRE_FALSE(shadow->AppendChild(*inner).HasException());
    REQUIRE_FALSE(shadow->AppendChild(*slot).HasException());

    document.UpdateStyleIfNeeded();
    CHECK(Rendered(*inner).GetColor() == Red);
    CHECK(ResolverFixture::Serialised(Rendered(*inner), Rendered(*inner).Width()) == "5px");
    // The slotted element inherits from its slot, and the shadow tree's rules don't apply to it.
    CHECK(Rendered(fixture.ById(u8"light")).GetColor() == Blue);
    CHECK(ResolverFixture::Serialised(Rendered(fixture.ById(u8"light")),
                                      Rendered(fixture.ById(u8"light")).Width())
          == "auto");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "TreeResolver: container queries", "[Style][TreeResolver]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      #c { container-type: inline-size; --x: 1 }
      #n { container-name: foo; --y: a }
      @container style(--x: 1) { #p { color: rgb(0, 0, 255) } }
      @container style(--x: 2) { #p { width: 1px } }
      @container (width > 0px) { #p { height: 5px } }
      @container foo style(--y: a) { #q { color: rgb(255, 0, 0) } }
      @container bar style(--y: a) { #q { width: 5px } }
    </style><div id=c><p id=p></p></div><div id=n><p id=q></p></div>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();

    // Descendants of size containers are resolved after the container (and its layout, once there is any).
    auto &p = Rendered(fixture.ById(u8"p"));
    CHECK(p.GetColor() == Blue);
    CHECK(ResolverFixture::Serialised(p, p.Width()) == "auto");
    // Size queries are unknown without a principal box.
    CHECK(ResolverFixture::Serialised(p, p.Height()) == "auto");

    auto &q = Rendered(fixture.ById(u8"q"));
    CHECK(q.GetColor() == Red);
    CHECK(ResolverFixture::Serialised(q, q.Width()) == "auto");

    SECTION("Selecting containers")
    {
      using css::cq::Axis;
      auto &c = fixture.ById(u8"c");
      auto &n = fixture.ById(u8"n");
      CHECK(ContainerQueryEvaluator::SelectContainer(Axis::Width, {}, fixture.ById(u8"p")).get() == &c);
      CHECK_FALSE(ContainerQueryEvaluator::SelectContainer(Axis::Width, {}, fixture.ById(u8"q")));
      CHECK(ContainerQueryEvaluator::SelectContainer(Axis::None, u8"foo", fixture.ById(u8"q")).get() == &n);
      CHECK_FALSE(ContainerQueryEvaluator::SelectContainer(Axis::None, u8"bar", fixture.ById(u8"q")));
      // Any ancestor is a style container.
      CHECK(ContainerQueryEvaluator::SelectContainer(Axis::None, {}, fixture.ById(u8"q")).get() == &n);
    }

    SECTION("Changes")
    {
      REQUIRE_FALSE(fixture.ById(u8"c").SetAttribute(u8"style", u8"--x: 2").HasException());
      document.UpdateStyleIfNeeded();
      auto &changed = Rendered(fixture.ById(u8"p"));
      CHECK(changed.GetColor() != Blue);
      CHECK(ResolverFixture::Serialised(changed, changed.Width()) == "1px");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Update", "[Style][TreeResolver]")
  {
    ResolverFixture fixture {u8"<!DOCTYPE html><div id=d><span id=s></span></div><p id=p></p>"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();
    auto &d = fixture.ById(u8"d");
    auto &s = fixture.ById(u8"s");
    auto &body = *document.Body();

    auto update = CreateUnique<Update>(document);
    CHECK(update->IsEmpty());
    update->AddElement(s, &d, {RenderStyle::ClonePtr(Rendered(s)), {Change::NonInherited}});
    CHECK(update->Size() == 1);
    CHECK(update->Roots().contains(ShareRefPtr<dom::ContainerNode>(&d)));

    // Adding the parent makes its own parent the root.
    update->AddElement(d, &body, {RenderStyle::ClonePtr(Rendered(d)), {Change::NonInherited}});
    CHECK_FALSE(update->Roots().contains(ShareRefPtr<dom::ContainerNode>(&d)));
    CHECK(update->Roots().contains(ShareRefPtr<dom::ContainerNode>(&body)));

    CHECK(update->ElementStyle(s) == update->GetElementUpdate(s)->Style.get());
    CHECK(update->ElementStyle(fixture.ById(u8"p")) == fixture.ById(u8"p").GetRenderStyle());
  }
}
