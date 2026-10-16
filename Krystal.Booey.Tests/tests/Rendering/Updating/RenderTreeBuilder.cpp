#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;
  using style::tests::WithView;

  namespace
  {
    /// @brief The elements rendered by the children of @p parent, by id ("::before"/"::after" for
    /// pseudo-elements).
    KRYS_NODISCARD List<std::string> ChildIds(const RenderElement &parent)
    {
      List<std::string> ids;
      for (auto &child : ChildrenOfType<RenderElement>(parent))
      {
        auto *element = child.GetElement();
        REQUIRE(element);
        if (auto *pseudoElement = DynamicDowncast<dom::PseudoElement>(*element))
        {
          ids.push_back(pseudoElement->GetPseudoElementType() == PseudoElementType::Before ? "::before"
                                                                                           : "::after");
          continue;
        }
        ids.push_back(css::tests::ToNarrow(element->GetAttribute(u8"id").value_or(u8"")));
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
      CHECK(ChildIds(*host) == List<std::string> {"::before", "h1", "::after"});
      CHECK(fixture.ById(u8"host").BeforePseudoElement()->Renderer()->Parent() == host);
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
      // host, its pseudo-elements and h1.
      CHECK(view->RendererCount() == count - 5);
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
}
