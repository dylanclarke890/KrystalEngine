#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderMultiColumn.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Rendering/Iterators/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/MultiColumn/RenderMultiColumnFlow.hpp"
#include "Krystal.Booey/Rendering/MultiColumn/RenderMultiColumnSet.hpp"
#include "Krystal.Booey/Rendering/MultiColumn/RenderMultiColumnSpannerPlaceholder.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    /// @brief The element children of @p parent: elements by id, multi-column flows as "flow", column sets as
    /// "set" and spanner placeholders as "placeholder", with their own element children in parentheses.
    KRYS_NODISCARD std::string MultiColumnTree(const RenderElement &parent)
    {
      std::string tree;
      for (auto &child : ChildrenOfType<RenderElement>(parent))
      {
        if (!tree.empty())
        {
          tree += ' ';
        }
        if (child.IsRenderMultiColumnFlow())
        {
          tree += "flow";
        }
        else if (child.IsRenderMultiColumnSet())
        {
          tree += "set";
        }
        else if (child.IsRenderMultiColumnSpannerPlaceholder())
        {
          tree += "placeholder";
        }
        else if (auto *element = child.GetElement())
        {
          tree += css::tests::ToNarrow(element->GetAttribute(u8"id").value_or(u8""));
        }
        else
        {
          tree += "anon";
        }
        if (auto children = MultiColumnTree(child); !children.empty())
        {
          tree += '(' + children + ')';
        }
      }
      return tree;
    }

    struct MultiColumnFixture : ResolverFixture
    {
      explicit MultiColumnFixture(utf8_string &&markup) noexcept : ResolverFixture(krys::move(markup))
      {
        Document->UpdateStyleIfNeeded();
      }

      KRYS_NODISCARD RenderBlockFlow &Block(dom::DOMStringView id) noexcept
      {
        return Downcast<RenderBlockFlow>(*ById(id).Renderer());
      }

      KRYS_NODISCARD std::string Tree(dom::DOMStringView id) noexcept
      {
        return MultiColumnTree(*ById(id).Renderer());
      }

      void SetStyle(dom::DOMStringView id, const char8_t *style) noexcept
      {
        krys_assert(!ById(id).SetAttribute(u8"style", style).HasException());
        Document->UpdateStyleIfNeeded();
      }
    };

    KRYS_NODISCARD bool InsideFlow(const RenderObject &renderer) noexcept
    {
      return renderer.GetFragmentedFlowState() == RenderObject::FragmentedFlowState::InsideFlow;
    }
  }

  TEST_CASE("RenderTreeBuilder::MultiColumn: a multi-column container's flow and column set",
            "[Rendering][RenderTreeBuilder][MultiColumn]")
  {
    MultiColumnFixture fixture {
      u8"<div id=m style=columns:2><p id=a>x</p><p id=b></p></div><div id=c style=column-width:10px></div>"};
    auto &container = fixture.Block(u8"m");

    // The content moves into the flow, followed by one column set.
    CHECK(fixture.Tree(u8"m") == "flow(a b) set");
    auto *flow = container.MultiColumnFlow();
    REQUIRE(flow);
    CHECK(flow->MultiColumnBlockFlow() == &container);
    CHECK(flow->FirstMultiColumnSet() == flow->NextSibling());
    CHECK(flow->FirstMultiColumnSet() == flow->LastMultiColumnSet());
    CHECK(flow->FirstMultiColumnSet()->MultiColumnFlow() == flow);
    CHECK_FALSE(flow->HasColumnSpanner());

    // The flow's content is inside it; the container isn't.
    auto &a = *fixture.ById(u8"a").Renderer();
    CHECK(InsideFlow(*flow));
    CHECK(InsideFlow(a));
    CHECK(InsideFlow(*a.FirstChildSlow()));
    CHECK(a.EnclosingFragmentedFlow() == flow);
    CHECK_FALSE(InsideFlow(container));
    CHECK_FALSE(container.EnclosingFragmentedFlow());

    // An empty container has no flow.
    CHECK_FALSE(fixture.Block(u8"c").MultiColumnFlow());

    SECTION("The flow and set take the container's style")
    {
      fixture.SetStyle(u8"m", u8"columns: 2; direction: rtl");
      CHECK(flow->Style().ComputedDirection() == TextDirection::RTL);
      CHECK(flow->FirstMultiColumnSet()->Style().ComputedDirection() == TextDirection::RTL);
    }
  }

  TEST_CASE("RenderTreeBuilder::MultiColumn: spanners", "[Rendering][RenderTreeBuilder][MultiColumn]")
  {
    MultiColumnFixture fixture {u8"<style>.s { column-span: all }</style>"
                                u8"<div id=m style=columns:2><p id=a></p><h1 id=s class=s></h1>"
                                u8"<div id=d><h2 id=t class=s></h2></div><p id=b></p></div>"};
    auto &container = fixture.Block(u8"m");
    auto *flow = container.MultiColumnFlow();
    REQUIRE(flow);

    // A spanner moves out of the flow to sit between column sets, leaving a placeholder; one inside a block
    // in the flow does too.
    CHECK(fixture.Tree(u8"m") == "flow(a placeholder d(placeholder) b) set s set t set");
    auto &s = Downcast<RenderBox>(*fixture.ById(u8"s").Renderer());
    auto *placeholder = flow->FindColumnSpannerPlaceholder(s);
    REQUIRE(placeholder);
    CHECK(placeholder->Spanner() == &s);
    CHECK(placeholder->FragmentedFlow() == flow);
    CHECK(placeholder->Parent() == flow);
    CHECK(flow->HasColumnSpanner());

    // The sets know the content they hold.
    auto *firstSet = flow->FirstMultiColumnSet();
    CHECK(firstSet->ContainsRendererInFragmentedFlow(*fixture.ById(u8"a").Renderer()));
    CHECK_FALSE(firstSet->ContainsRendererInFragmentedFlow(*fixture.ById(u8"b").Renderer()));
    CHECK(flow->LastMultiColumnSet()->ContainsRendererInFragmentedFlow(*fixture.ById(u8"b").Renderer()));

    SECTION("Removing a spanner merges the sets around it")
    {
      REQUIRE_FALSE(fixture.ById(u8"s").Remove().HasException());
      fixture.Document->UpdateStyleIfNeeded();
      CHECK(fixture.Tree(u8"m") == "flow(a d(placeholder) b) set t set");
    }

    SECTION("A spanner that stops spanning goes back into the flow")
    {
      fixture.SetStyle(u8"t", u8"column-span: none");
      CHECK(fixture.Tree(u8"m") == "flow(a placeholder d(t) b) set s set");
    }

    SECTION("Inserting a spanner splits the sets")
    {
      auto spanner = fixture.Document->CreateElement(u8"div").ReleaseValue();
      REQUIRE_FALSE(spanner->SetAttribute(u8"id", u8"u").HasException());
      REQUIRE_FALSE(spanner->SetAttribute(u8"class", u8"s").HasException());
      REQUIRE_FALSE(fixture.ById(u8"m").AppendChild(*spanner).HasException());
      fixture.Document->UpdateStyleIfNeeded();
      CHECK(fixture.Tree(u8"m") == "flow(a placeholder d(placeholder) b placeholder) set s set t set u set");
    }

    SECTION("Making a block in the flow multi-column moves its spanners into its own columns")
    {
      fixture.SetStyle(u8"d", u8"columns: 2");
      CHECK(fixture.Tree(u8"m") == "flow(a placeholder d(flow(placeholder) t set) b) set s set");
    }

    SECTION("Removing the columns puts the spanners back")
    {
      fixture.SetStyle(u8"m", u8"columns: auto");
      CHECK_FALSE(container.MultiColumnFlow());
      CHECK(fixture.Tree(u8"m") == "a s d(t) b");
      CHECK_FALSE(InsideFlow(*fixture.ById(u8"a").Renderer()));
      CHECK_FALSE(InsideFlow(*fixture.ById(u8"t").Renderer()));
    }
  }

  TEST_CASE("RenderTreeBuilder::MultiColumn: spanners go back to their places",
            "[Rendering][RenderTreeBuilder][MultiColumn]")
  {
    MultiColumnFixture fixture {u8"<div id=m style=columns:2><div id=d><p id=x></p>"
                                u8"<h1 id=s style=column-span:all></h1><p id=y></p></div></div>"};
    CHECK(fixture.Tree(u8"m") == "flow(d(x placeholder y)) set s set");

    SECTION("When the container stops being multi-column")
    {
      fixture.SetStyle(u8"m", u8"columns: auto");
      CHECK(fixture.Tree(u8"m") == "d(x s y)");
    }

    SECTION("When a block around them becomes multi-column")
    {
      fixture.SetStyle(u8"d", u8"columns: 2");
      CHECK(fixture.Tree(u8"m") == "flow(d(flow(x placeholder y) set s set)) set");
    }
  }

  TEST_CASE("RenderTreeBuilder::MultiColumn: boxes that don't span",
            "[Rendering][RenderTreeBuilder][MultiColumn]")
  {
    MultiColumnFixture fixture {
      u8"<style>.s { column-span: all }</style><div id=m style=columns:2>"
      u8"<div id=f class=s style=float:left></div><span id=i class=s></span>"
      u8"<div id=o style=\"overflow:scroll;height:10px\"><div id=x class=s></div></div>"
      u8"<div id=n style=columns:2><div id=y class=s></div></div></div>"};

    // Floats, inline boxes and boxes in something unbreakable (a scroller) aren't spanners; one in a nested
    // multi-column container spans that container's columns (with no column set before it, since there's no
    // content before it).
    CHECK(fixture.Tree(u8"m") == "flow(anon(f i) o(x) n(flow(placeholder) y set)) set");
  }

  TEST_CASE("RenderTreeBuilder::MultiColumn: positioned boxes", "[Rendering][RenderTreeBuilder][MultiColumn]")
  {
    MultiColumnFixture fixture {u8"<div id=m style=columns:2><div id=r style=position:relative>"
                                u8"<div id=abs style=position:absolute></div>"
                                u8"<div id=fixed style=position:fixed></div></div></div>"};
    auto *flow = fixture.Block(u8"m").MultiColumnFlow();
    REQUIRE(flow);

    // A positioned box is in the flow only if its containing block is.
    CHECK(InsideFlow(*fixture.ById(u8"abs").Renderer()));
    CHECK(fixture.ById(u8"abs").Renderer()->EnclosingFragmentedFlow() == flow);
    CHECK_FALSE(InsideFlow(*fixture.ById(u8"fixed").Renderer()));
    CHECK_FALSE(fixture.ById(u8"fixed").Renderer()->EnclosingFragmentedFlow());

    SECTION("A box whose containing block moves outside the flow leaves it")
    {
      fixture.SetStyle(u8"abs", u8"position:fixed");
      CHECK_FALSE(InsideFlow(*fixture.ById(u8"abs").Renderer()));
    }
  }

  TEST_CASE("RenderTreeBuilder::MultiColumn: fieldset legends stay out of the flow",
            "[Rendering][RenderTreeBuilder][MultiColumn]")
  {
    MultiColumnFixture fixture {
      u8"<fieldset id=m style=columns:2><legend id=l></legend><p id=a></p></fieldset>"};

    // The rendered legend is found wherever it is among the fieldset's children.
    CHECK(fixture.Tree(u8"m") == "flow(a) set l");
    CHECK_FALSE(InsideFlow(*fixture.ById(u8"l").Renderer()));

    SECTION("Removing the columns keeps the legend first")
    {
      fixture.SetStyle(u8"m", u8"columns: auto");
      CHECK(fixture.Tree(u8"m") == "l a");
    }
  }

  TEST_CASE("RenderTreeBuilder::MultiColumn: list items", "[Rendering][RenderTreeBuilder][MultiColumn]")
  {
    MultiColumnFixture fixture {u8"<ul><li id=m style=columns:2><p id=a>x</p></li></ul>"};

    // The marker goes in the flow, in the first line it marks.
    CHECK(fixture.Tree(u8"m") == "flow(a(anon)) set");
    auto &marker = *fixture.ById(u8"a").Renderer()->FirstChild();
    CHECK(marker.IsRenderListMarker());
    CHECK(InsideFlow(marker));
  }

  TEST_CASE("RenderTreeBuilder::MultiColumn: content moved out of a removed flow forgets its percent heights",
            "[Rendering][RenderTreeBuilder][MultiColumn]")
  {
    MultiColumnFixture fixture {
      u8"<div id=m style='columns:2;height:100px'><div id=a style='height:50%'></div></div>"};
    (void)fixture.Document->UpdateLayout();
    auto &container = fixture.Block(u8"m");
    auto &a = Downcast<RenderBox>(*fixture.ById(u8"a").Renderer());
    auto isPercentHeightDescendant = [&]
    {
      auto *descendants = container.PercentHeightDescendants();
      return descendants && descendants->contains(&a);
    };
    REQUIRE(isPercentHeightDescendant());

    // Moving the content back into the container drops it from the percent height maps, to be added again
    // by its next layout.
    fixture.SetStyle(u8"m", u8"height:100px");
    REQUIRE(fixture.Tree(u8"m") == "a");
    CHECK_FALSE(isPercentHeightDescendant());
    (void)fixture.Document->UpdateLayout();
    CHECK(isPercentHeightDescendant());
    CHECK(a.Height() == 50_lu);
  }
}
