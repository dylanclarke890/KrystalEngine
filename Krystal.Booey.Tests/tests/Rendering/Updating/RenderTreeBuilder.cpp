#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderTable.hpp"
#include "Krystal.Booey/Rendering/RenderTableCell.hpp"
#include "Krystal.Booey/Rendering/RenderTableRow.hpp"
#include "Krystal.Booey/Rendering/RenderTableSection.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;
  using style::tests::WithView;

  namespace
  {
    /// @brief The element @p renderer renders, by id ("::before"/"::after" for pseudo-elements, "anon" for
    /// anonymous boxes).
    KRYS_NODISCARD std::string Describe(const RenderElement &renderer)
    {
      auto *element = renderer.GetElement();
      if (!element)
      {
        return "anon";
      }
      if (auto *pseudoElement = DynamicDowncast<dom::PseudoElement>(*element))
      {
        return pseudoElement->GetPseudoElementType() == PseudoElementType::Before ? "::before" : "::after";
      }
      return css::tests::ToNarrow(element->GetAttribute(u8"id").value_or(u8""));
    }

    /// @brief The children of @p parent, described, with their own children in parentheses.
    KRYS_NODISCARD std::string Tree(const RenderElement &parent)
    {
      std::string tree;
      for (auto &child : ChildrenOfType<RenderElement>(parent))
      {
        if (!tree.empty())
        {
          tree += ' ';
        }
        tree += Describe(child);
        if (child.FirstChild())
        {
          tree += '(' + Tree(child) + ')';
        }
      }
      return tree;
    }

    /// @brief The children of @p parent, described.
    KRYS_NODISCARD List<std::string> ChildIds(const RenderElement &parent)
    {
      List<std::string> ids;
      for (auto &child : ChildrenOfType<RenderElement>(parent))
      {
        ids.push_back(Describe(child));
      }
      return ids;
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderTreeBuilder: the render tree",
                   "[Rendering][RenderTreeBuilder]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      #host::before { content: "b" }
      #host::after { content: "a" }
      .none { display: none }
    </style><body id=body><div id=a></div><div id=b class=none></div><div id=c></div>
    <div id=p><span id=contents style="display: contents"><div id=x></div></span><div id=y></div></div>
    <div id=host><div id=h1></div></div></body>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();

    auto *view = document.GetRenderView();
    REQUIRE(view);
    auto *body = fixture.ById(u8"body").Renderer();
    REQUIRE(body);

    SECTION("The render view is the document's renderer and the tree's root")
    {
      CHECK(static_cast<const dom::Node &>(document).Renderer() == view);
      CHECK(view->IsRenderView());
      CHECK(view->IsAnonymous());
      CHECK_FALSE(view->Parent());
      CHECK(view->RenderName() == u8"RenderView");
      CHECK(&view->View() == view);
      CHECK(view->IsRooted());
      CHECK(&document.RenderViewStyle() == &view->Style());

      auto *html = document.DocumentElement()->Renderer();
      REQUIRE(html);
      CHECK(html->Parent() == view);
      CHECK(view->FirstChild() == html);
      CHECK(view->LastChild() == html);
      CHECK(body->Parent() == html);
      CHECK(body->IsRooted());
      CHECK(body->IsDescendantOf(view));
      CHECK_FALSE(view->IsDescendantOf(body));
    }

    SECTION("Renderers are in tree order, skipping elements without renderers")
    {
      CHECK(ChildIds(*body) == List<std::string> {"a", "c", "p", "host"});
      auto *a = fixture.ById(u8"a").Renderer();
      auto *c = fixture.ById(u8"c").Renderer();
      CHECK(a->NextSibling() == c);
      CHECK(c->PreviousSibling() == a);
      CHECK_FALSE(a->PreviousSibling());
      CHECK(body->FirstChild() == a);
      CHECK(body->ChildAt(1) == c);
    }

    SECTION("Children of display: contents elements are their rendering parent's children")
    {
      auto *p = fixture.ById(u8"p").Renderer();
      REQUIRE(p);
      CHECK(ChildIds(*p) == List<std::string> {"x", "y"});
    }

    SECTION("::before and ::after are the first and last children")
    {
      auto *host = fixture.ById(u8"host").Renderer();
      REQUIRE(host);
      // The inline ::before and ::after are wrapped in anonymous blocks, since h1 is a block.
      CHECK(Tree(*host) == "anon(::before) h1 anon(::after)");
      CHECK(fixture.ById(u8"host").BeforePseudoElement()->Renderer()->Parent()->Parent() == host);
    }

    SECTION("New renderers are inserted at their place")
    {
      auto &b = fixture.ById(u8"b");
      REQUIRE_FALSE(b.SetAttribute(u8"class", u8"").HasException());
      document.UpdateStyleIfNeeded();
      CHECK(ChildIds(*body) == List<std::string> {"a", "b", "c", "p", "host"});

      auto created = document.CreateElement(u8"div");
      REQUIRE_FALSE(created.HasException());
      auto inserted = created.ReleaseValue();
      REQUIRE_FALSE(inserted->SetAttribute(u8"id", u8"z").HasException());
      REQUIRE_FALSE(body->GetElement()->InsertBefore(*inserted, &fixture.ById(u8"a")).HasException());
      document.UpdateStyleIfNeeded();
      CHECK(ChildIds(*body) == List<std::string> {"z", "a", "b", "c", "p", "host"});
    }

    SECTION("Removed elements and hidden elements lose their renderers")
    {
      auto count = view->RendererCount();
      auto c = ShareRef(fixture.ById(u8"c"));
      REQUIRE_FALSE(c->Remove().HasException());
      CHECK_FALSE(c->Renderer());
      CHECK(ChildIds(*body) == List<std::string> {"a", "p", "host"});
      CHECK(view->RendererCount() == count - 1);

      auto &host = fixture.ById(u8"host");
      REQUIRE_FALSE(host.SetAttribute(u8"class", u8"none").HasException());
      document.UpdateStyleIfNeeded();
      CHECK_FALSE(host.Renderer());
      CHECK_FALSE(fixture.ById(u8"h1").Renderer());
      CHECK_FALSE(host.BeforePseudoElement());
      CHECK(ChildIds(*body) == List<std::string> {"a", "p"});
      // host, its pseudo-elements, their anonymous wrappers and h1.
      CHECK(view->RendererCount() == count - 7);
    }

    SECTION("Destroying the render tree")
    {
      auto a = ShareRef(fixture.ById(u8"a"));
      fixture.Frame->SetView(nullptr);
      CHECK_FALSE(document.GetRenderView());
      CHECK_FALSE(document.HasLivingRenderTree());
      CHECK_FALSE(static_cast<const dom::Node &>(document).Renderer());
      CHECK_FALSE(a->Renderer());
      CHECK_FALSE(fixture.ById(u8"host").BeforePseudoElement());

      fixture.Frame->CreateView({800, 600});
      REQUIRE(document.GetRenderView());
      REQUIRE(a->Renderer());
      CHECK(a->Renderer()->IsRooted());
    }

    SECTION("Documents without a view have no renderers")
    {
      ResolverFixture viewless {u8"<!DOCTYPE html><div id=a></div>", WithView::No};
      viewless.Document->UpdateStyleIfNeeded();
      CHECK_FALSE(viewless.Document->GetRenderView());
      CHECK_FALSE(viewless.ById(u8"a").Renderer());
      REQUIRE(viewless.ById(u8"a").ComputedStyle());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderTreeBuilder: anonymous boxes",
                   "[Rendering][RenderTreeBuilder]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      .cell { display: table-cell }
      .row { display: table-row }
      .table { display: table }
      .float { float: left }
    </style><body>
      <div id=mix><span id=s1></span><div id=d1></div><span id=s2></span><span id=s3></span></div>
      <div id=inl><span id=i1></span><span id=i2></span></div>
      <div id=blocks><div id=b1></div></div>
      <div id=outerparent><span id=outer><div id=inner></div></span></div>
      <div id=floats><div id=blk></div><div id=fl class=float></div><span id=sp></span></div>
      <div id=inlinefloat><div id=fl2 class=float></div><span id=sp2></span></div>
      <div id=cells><span id=c1 class=cell></span><span id=c2 class=cell></span></div>
      <div id=t class=table><div id=r class=row></div></div>
      <div id=inlinecell><span id=ic><span id=ic1 class=cell></span></span></div>
      <div id=norm><div id=nb></div><span id=ns></span></div>
    </body>)html"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();

    auto renderer = [&](utf8_stringview id) noexcept -> RenderElement &
    {
      auto *r = fixture.ById(id).Renderer();
      krys_assert(r);
      return *r;
    };
    auto create = [&](const char8_t *tag, const char8_t *id) -> Ref<dom::Element>
    {
      auto created = document.CreateElement(tag);
      REQUIRE_FALSE(created.HasException());
      auto element = created.ReleaseValue();
      REQUIRE_FALSE(element->SetAttribute(u8"id", id).HasException());
      return element;
    };

    SECTION("Inline children of a block with block children are wrapped in anonymous blocks")
    {
      CHECK(Tree(renderer(u8"mix")) == "anon(s1) d1 anon(s2 s3)");
      CHECK_FALSE(renderer(u8"mix").ChildrenInline());
      auto &wrapper = *renderer(u8"s1").Parent();
      CHECK(wrapper.IsAnonymousBlock());
      CHECK(wrapper.ChildrenInline());
      CHECK(wrapper.RenderName() == u8"RenderBlock (anonymous)");
      CHECK(wrapper.Style().Display() == DisplayType::Block);
      CHECK(Tree(renderer(u8"inl")) == "i1 i2");
      CHECK(renderer(u8"inl").ChildrenInline());
    }

    SECTION("Inserting a block among inlines wraps them; removing it merges and drops the wrappers")
    {
      auto block = create(u8"div", u8"new");
      REQUIRE_FALSE(fixture.ById(u8"inl").InsertBefore(*block, &fixture.ById(u8"i2")).HasException());
      document.UpdateStyleIfNeeded();
      CHECK(Tree(renderer(u8"inl")) == "anon(i1) new anon(i2)");

      REQUIRE_FALSE(block->Remove().HasException());
      CHECK(Tree(renderer(u8"inl")) == "i1 i2");
      CHECK(renderer(u8"inl").ChildrenInline());

      REQUIRE_FALSE(fixture.ById(u8"d1").Remove().HasException());
      CHECK(Tree(renderer(u8"mix")) == "s1 s2 s3");
    }

    SECTION("Inlines appended after blocks go in an anonymous block, reused for the next ones")
    {
      auto &blocks = fixture.ById(u8"blocks");
      REQUIRE_FALSE(blocks.AppendChild(*create(u8"span", u8"x1")).HasException());
      document.UpdateStyleIfNeeded();
      REQUIRE_FALSE(blocks.AppendChild(*create(u8"span", u8"x2")).HasException());
      document.UpdateStyleIfNeeded();
      CHECK(Tree(renderer(u8"blocks")) == "b1 anon(x1 x2)");
    }

    SECTION("Removing the only child of an anonymous block destroys the block")
    {
      REQUIRE_FALSE(fixture.ById(u8"sp").Remove().HasException());
      CHECK(Tree(renderer(u8"floats")) == "blk fl");
    }

    SECTION("Blocks inside inlines stay their children")
    {
      CHECK(Tree(renderer(u8"outer")) == "inner");
      CHECK(Tree(renderer(u8"outerparent")) == "outer(inner)");
      CHECK(renderer(u8"outerparent").ChildrenInline());
    }

    SECTION("Floats don't make their parent wrap its inlines")
    {
      CHECK(Tree(renderer(u8"floats")) == "blk fl anon(sp)");
      CHECK(Tree(renderer(u8"inlinefloat")) == "fl2 sp2");
      CHECK(renderer(u8"inlinefloat").ChildrenInline());
    }

    SECTION("Table parts outside a table get anonymous table wrappers")
    {
      CHECK(Tree(renderer(u8"cells")) == "anon(anon(anon(c1 c2)))");
      auto &row = *renderer(u8"c1").Parent();
      CHECK(Is<RenderTableRow>(row));
      CHECK(row.IsAnonymous());
      CHECK(Is<RenderTableSection>(*row.Parent()));
      auto &table = *row.Parent()->Parent();
      CHECK(Is<RenderTable>(table));
      CHECK(table.Style().Display() == DisplayType::Table);
      CHECK(Tree(renderer(u8"t")) == "anon(r)");
      CHECK(Is<RenderTableSection>(*renderer(u8"r").Parent()));

      // A table part inside an inline gets an inline table.
      auto &inlineTable = *renderer(u8"ic1").Parent()->Parent()->Parent();
      CHECK(Is<RenderTable>(inlineTable));
      CHECK(inlineTable.Style().Display() == DisplayType::InlineTable);
      CHECK(inlineTable.Parent() == &renderer(u8"ic"));
    }

    SECTION("Removing table parts collapses their anonymous wrappers")
    {
      REQUIRE_FALSE(fixture.ById(u8"c1").Remove().HasException());
      CHECK(Tree(renderer(u8"cells")) == "anon(anon(anon(c2)))");
      REQUIRE_FALSE(fixture.ById(u8"c2").Remove().HasException());
      CHECK(Tree(renderer(u8"cells")).empty());
    }

    SECTION("A block that floats stops making its inline siblings wrap")
    {
      CHECK(Tree(renderer(u8"norm")) == "nb anon(ns)");
      auto &nb = fixture.ById(u8"nb");
      REQUIRE_FALSE(nb.SetAttribute(u8"class", u8"float").HasException());
      document.UpdateStyleIfNeeded();
      CHECK(Tree(renderer(u8"norm")) == "nb ns");
      CHECK(renderer(u8"norm").ChildrenInline());

      REQUIRE_FALSE(nb.SetAttribute(u8"class", u8"").HasException());
      document.UpdateStyleIfNeeded();
      CHECK(Tree(renderer(u8"norm")) == "nb anon(ns)");
    }
  }
}
