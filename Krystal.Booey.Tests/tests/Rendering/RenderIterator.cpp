#include "Krystal.Booey/Rendering/RenderIterator.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Rendering/RenderAncestorIterator.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderDescendantIterator.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    template <typename TRange>
    KRYS_NODISCARD List<std::string> Ids(TRange &&range)
    {
      List<std::string> ids;
      for (auto &renderer : range)
      {
        auto *element = DynamicDowncast<dom::Element>(renderer.GetNode());
        ids.push_back(element ? css::tests::ToNarrow(element->GetAttribute(u8"id").value_or(u8"")) : "view");
      }
      return ids;
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderIterator: traversing the render tree",
                   "[Rendering][RenderIterator]")
  {
    // r
    // ├ a
    // │ ├ a1
    // │ └ a2
    // │   └ a21
    // └ b
    ResolverFixture fixture {u8R"(<!DOCTYPE html><body id=r><div id=a><div id=a1></div><div id=a2><div id=a21>
      </div></div></div><div id=b></div></body>)"};
    fixture.Document->UpdateStyleIfNeeded();

    auto renderer = [&](utf8_stringview id) noexcept -> RenderElement &
    {
      auto *r = fixture.ById(id).Renderer();
      krys_assert(r);
      return *r;
    };
    auto &r = renderer(u8"r");
    auto &a = renderer(u8"a");
    auto &a2 = renderer(u8"a2");
    auto &a21 = renderer(u8"a21");
    auto &b = renderer(u8"b");

    SECTION("Children")
    {
      CHECK(Ids(ChildrenOfType<RenderElement>(r)) == List<std::string> {"a", "b"});
      CHECK(Ids(ChildrenOfType<RenderElement>(a)) == List<std::string> {"a1", "a2"});
      CHECK(ChildrenOfType<RenderElement>(a).First() == &renderer(u8"a1"));
      CHECK(ChildrenOfType<RenderElement>(a).Last() == &a2);
      CHECK(Ids(ChildrenOfType<RenderElement>(b)).empty());

      const RenderElement &constR = r;
      CHECK(Ids(ChildrenOfType<RenderElement>(constR)) == List<std::string> {"a", "b"});
    }

    SECTION("Descendants")
    {
      CHECK(Ids(DescendantsOfType<RenderElement>(r)) == List<std::string> {"a", "a1", "a2", "a21", "b"});
      CHECK(Ids(DescendantsOfType<RenderObject>(a)) == List<std::string> {"a1", "a2", "a21"});
      CHECK(Ids(DescendantsOfTypePostOrder<RenderElement>(r))
            == List<std::string> {"a1", "a21", "a2", "a", "b"});
      const RenderElement &constR = r;
      CHECK(Ids(DescendantsOfType<RenderElement>(constR)) == List<std::string> {"a", "a1", "a2", "a21", "b"});

      auto descendants = DescendantsOfType<RenderElement>(r);
      auto it = descendants.at(a2);
      ++it;
      CHECK(&*it == &a21);
    }

    SECTION("Ancestors")
    {
      auto ancestors = Ids(AncestorsOfType<RenderElement>(a21));
      REQUIRE(ancestors.size() == 5u);
      CHECK(List<std::string>(ancestors.begin(), ancestors.begin() + 3)
            == List<std::string> {"a2", "a", "r"});
      CHECK(ancestors.back() == "view");
      CHECK(AncestorsOfType<RenderView>(a21).First() == fixture.Document->GetRenderView());
      CHECK(LineageOfType<RenderElement>(a21).First() == &a21);
      const RenderObject &constA21 = a21;
      CHECK(AncestorsOfType<RenderElement>(constA21).First() == &a2);
    }

    SECTION("Pre-order traversal")
    {
      CHECK(a.NextInPreOrder() == &renderer(u8"a1"));
      CHECK(a21.NextInPreOrder() == &b);
      CHECK(a21.NextInPreOrder(&a2) == nullptr);
      CHECK(a.NextInPreOrderAfterChildren() == &b);
      CHECK(b.PreviousInPreOrder() == &a21);
      CHECK(a.PreviousInPreOrder() == &r);
      CHECK(r.FirstLeafChild() == &renderer(u8"a1"));
      CHECK(r.LastLeafChild() == &b);
      CHECK(a.LastLeafChild() == &a21);
      CHECK(r.ChildAt(1) == &b);
      CHECK_FALSE(r.ChildAt(2));
    }
  }
}
