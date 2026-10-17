#include "Krystal.Booey/Layout/Integration/LayoutIntegrationBoxTreeUpdater.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/FormattingContext.hpp"
#include "Krystal.Booey/Layout/LayoutTree/FormattingContextBoxIterator.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxInlines.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutContainingBlockChainIterator.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutDescendantIterator.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInitialContainingBlock.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInlineTextBox.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderListItem.hpp"
#include "Krystal.Booey/Rendering/RenderListMarker.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD RenderBlockFlow &BlockOf(ResolverFixture &fixture, dom::DOMStringView id) noexcept
    {
      return Downcast<RenderBlockFlow>(*fixture.ById(id).Renderer());
    }

    KRYS_NODISCARD RenderElement &RendererOf(ResolverFixture &fixture, dom::DOMStringView id) noexcept
    {
      return *fixture.ById(id).Renderer();
    }

    KRYS_NODISCARD const layout::ElementBox &BoxOf(ResolverFixture &fixture, dom::DOMStringView id) noexcept
    {
      auto box = RendererOf(fixture, id).LayoutBox();
      REQUIRE(box);
      return *box;
    }

    KRYS_NODISCARD List<utf8_string> TextContents(const layout::ElementBox &root) noexcept
    {
      List<utf8_string> contents;
      for (auto &textBox : layout::DescendantsOfType<layout::InlineTextBox>(root))
      {
        contents.push_back(textBox.Content());
      }
      return contents;
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "BoxTreeUpdater: inline content",
                   "[Layout][Integration][BoxTreeUpdater]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      #ib { display: inline-block }
      #br { clear: left }
      #wbr { clear: both }
    </style><body><div id=root>a<span id=s>b<b id=bold>c</b></span><br id=br><wbr id=wbr><span id=ib>x</span>	tab</div>
    <div id=rtl>שלום</div></body>)html"};
    (void)fixture.Document->UpdateLayout();

    auto &view = *fixture.Document->GetRenderView();
    auto &initialContainingBlock = view.GetInitialContainingBlock();
    auto &rootRenderer = BlockOf(fixture, u8"root");
    CHECK_FALSE(rootRenderer.LayoutBox());

    auto &rootBox = *layout_integration::BoxTreeUpdater(rootRenderer).Build();
    REQUIRE(rootRenderer.LayoutBox() == &rootBox);
    CHECK(rootBox.RendererForIntegration() == &rootRenderer);
    CHECK(&rootBox.Parent() == &initialContainingBlock);
    CHECK(initialContainingBlock.FirstChild() == &rootBox);

    SECTION("The formatting context root")
    {
      CHECK(rootBox.IsInlineIntegrationRoot());
      CHECK(rootBox.EstablishesInlineFormattingContext());
      CHECK(rootBox.EstablishesBlockFormattingContext());
      CHECK(rootBox.EstablishesFormattingContext());
      CHECK(rootBox.IsBlockLevelBox());
      CHECK(rootBox.IsBlockContainer());
      CHECK(rootBox.IsBlockBox());
      CHECK_FALSE(rootBox.IsAnonymous());
      CHECK_FALSE(rootBox.IsAnonymousTextIndentCandidateForIntegration());
      CHECK(rootBox.IsPaddingApplicable());
      CHECK(rootBox.IsOverflowVisible());

      CHECK(initialContainingBlock.IsInitialContainingBlock());
      CHECK(initialContainingBlock.EstablishesBlockFormattingContext());
      CHECK_FALSE(initialContainingBlock.EstablishesInlineFormattingContext());
    }

    SECTION("Boxes for the inline-level renderers, in order")
    {
      // The inline-block's content is its own formatting context's, so it's not walked.
      CHECK(TextContents(rootBox) == List<utf8_string> {u8"a", u8"b", u8"c", u8"\ttab"});

      List<const layout::Box *> children;
      for (auto &child : layout::ChildrenOfType<layout::Box>(rootBox))
      {
        children.push_back(&child);
      }
      REQUIRE(children.size() == 6);
      CHECK(children[1] == &BoxOf(fixture, u8"s"));
      CHECK(children[2] == &BoxOf(fixture, u8"br"));
      CHECK(children[3] == &BoxOf(fixture, u8"wbr"));
      CHECK(children[4] == &BoxOf(fixture, u8"ib"));
      CHECK(layout::ChildrenOfType<layout::ElementBox>(rootBox).First() == &BoxOf(fixture, u8"s"));

      // The formatting context's boxes are all of them: the inline-block has no children.
      auto count = 0;
      for (auto &box : layout::FormattingContextBoxes(rootBox))
      {
        (void)box;
        ++count;
      }
      CHECK(count == 9);
    }

    SECTION("Text boxes")
    {
      auto &textA = Downcast<layout::InlineTextBox>(*rootBox.FirstChild());
      CHECK(textA.Content() == u8"a");
      CHECK(textA.IsAnonymous());
      CHECK(textA.IsInlineLevelBox());
      CHECK(textA.IsInlineBox());
      CHECK_FALSE(textA.IsAtomicInlineBox());
      CHECK(textA.Style().Display() == DisplayType::Inline);
      CHECK(textA.CanUseSimpleFontCodePath());
      CHECK_FALSE(textA.HasPositionDependentContentWidth());
      CHECK_FALSE(textA.HasStrongDirectionalityContent());
      auto &textRenderer = Downcast<RenderText>(*rootRenderer.FirstChild());
      CHECK(textA.RendererForIntegration() == &textRenderer);
      CHECK(textRenderer.LayoutBox() == &textA);

      auto &tab = Downcast<layout::InlineTextBox>(*rootBox.LastChild());
      CHECK(tab.HasPositionDependentContentWidth());

      auto &rtlRenderer = BlockOf(fixture, u8"rtl");
      auto &rtlBox = *layout_integration::BoxTreeUpdater(rtlRenderer).Build();
      auto &hebrew = Downcast<layout::InlineTextBox>(*rtlBox.FirstChild());
      CHECK(hebrew.HasStrongDirectionalityContent());
      layout_integration::BoxTreeUpdater(rtlRenderer).TearDown();
    }

    SECTION("Inline boxes, line breaks and atomic inline boxes")
    {
      auto &span = BoxOf(fixture, u8"s");
      CHECK(span.IsInlineBox());
      CHECK_FALSE(span.IsAtomicInlineBox());
      CHECK_FALSE(span.IsContainingBlockForInFlow());
      CHECK(TextContents(span) == List<utf8_string> {u8"b", u8"c"});

      // A line break clears like a block; a word break opportunity doesn't clear at all.
      auto &br = BoxOf(fixture, u8"br");
      CHECK(br.IsLineBreakBox());
      CHECK_FALSE(br.IsWordBreakOpportunity());
      CHECK(br.HasFloatClear());
      CHECK(br.Style().Display() == DisplayType::Inline);

      auto &wbr = BoxOf(fixture, u8"wbr");
      CHECK(wbr.IsLineBreakBox());
      CHECK(wbr.IsWordBreakOpportunity());
      CHECK(wbr.Style().GetClear() == Clear::None);
      CHECK_FALSE(wbr.HasFloatClear());

      auto &inlineBlock = BoxOf(fixture, u8"ib");
      CHECK(inlineBlock.IsInlineBlockBox());
      CHECK(inlineBlock.IsAtomicInlineBox());
      CHECK(inlineBlock.IsBlockContainer());
      CHECK_FALSE(inlineBlock.IsBlockBox());
      CHECK(inlineBlock.EstablishesBlockFormattingContext());
      CHECK(inlineBlock.IsFloatAvoider());
      CHECK_FALSE(inlineBlock.HasChild());
    }

    SECTION("Containing blocks")
    {
      auto &bold = BoxOf(fixture, u8"bold");
      auto &textC = *bold.FirstChild();
      CHECK(&layout::FormattingContext::ContainingBlock(textC) == &rootBox);
      CHECK(&layout::FormattingContext::ContainingBlock(bold) == &rootBox);
      CHECK(&layout::FormattingContext::GetInitialContainingBlock(textC) == &initialContainingBlock);
      CHECK(&layout::FormattingContext::ContainingBlock(rootBox) == &initialContainingBlock);

      List<const layout::ElementBox *> chain;
      for (auto &containingBlock : layout::ContainingBlockChain(textC))
      {
        chain.push_back(&containingBlock);
      }
      CHECK(chain == List<const layout::ElementBox *> {&rootBox, &initialContainingBlock});
      CHECK(layout::ContainingBlockChain(textC, rootBox).begin()
            == layout::ContainingBlockChain(textC, rootBox).end());

      CHECK(textC.IsDescendantOf(rootBox));
      CHECK(textC.IsDescendantOfWithinFormattingContext(BoxOf(fixture, u8"s")));
      CHECK(textC.IsInFormattingContextEstablishedBy(rootBox));
#if KRYS_DEBUG
      CHECK(&layout::FormattingContext::FormattingContextRoot(textC) == &rootBox);
#endif
    }

    SECTION("Removing and inserting boxes")
    {
      auto &textRenderer = *rootRenderer.FirstChild();
      auto removed = layout_integration::BoxTreeUpdater(rootRenderer).Remove(rootRenderer, textRenderer);
      CHECK_FALSE(textRenderer.LayoutBox());
      CHECK_FALSE(removed->RendererForIntegration());
      CHECK(rootBox.FirstChild() == &BoxOf(fixture, u8"s"));
      CHECK_FALSE(BoxOf(fixture, u8"s").PreviousSibling());

      // The box is inserted after the given renderer's box, or first.
      auto &inserted =
        layout_integration::BoxTreeUpdater(rootRenderer).Insert(rootRenderer, textRenderer, nullptr);
      CHECK(rootBox.FirstChild() == &inserted);
      CHECK(textRenderer.LayoutBox() == &inserted);
      CHECK(inserted.NextSibling() == &BoxOf(fixture, u8"s"));
      CHECK(TextContents(rootBox) == List<utf8_string> {u8"a", u8"b", u8"c", u8"\ttab"});
    }

    SECTION("Updating styles and text")
    {
      auto oldSpanStyle = RenderStyle::Clone(BoxOf(fixture, u8"s").Style());
      fixture.ById(u8"s").SetAttribute(u8"style", u8"color: red");
      fixture.ById(u8"bold").SetAttribute(u8"style", u8"color: green");
      (void)fixture.Document->UpdateLayout();

      auto &spanRenderer = RendererOf(fixture, u8"s");
      CHECK_FALSE(BoxOf(fixture, u8"s").Style() == spanRenderer.Style());
      layout_integration::BoxTreeUpdater::UpdateStyle(spanRenderer);
      CHECK(BoxOf(fixture, u8"s").Style() == spanRenderer.Style());
      CHECK_FALSE(BoxOf(fixture, u8"s").Style() == oldSpanStyle);

      auto &boldRenderer = RendererOf(fixture, u8"bold");
      layout_integration::BoxTreeUpdater::UpdateStyle(boldRenderer);
      CHECK(BoxOf(fixture, u8"bold").Style() == boldRenderer.Style());

      auto &textRenderer = Downcast<RenderText>(*rootRenderer.FirstChild());
      layout_integration::BoxTreeUpdater::UpdateStyle(textRenderer);
      CHECK(textRenderer.LayoutBox()->Style().Display() == DisplayType::Inline);

      textRenderer.SetText(u8"x\ty");
      CHECK_FALSE(textRenderer.HasPositionDependentContentWidth());
      layout_integration::BoxTreeUpdater::UpdateContent(textRenderer);
      CHECK(textRenderer.LayoutBox()->Content() == u8"x\ty");
      CHECK(textRenderer.LayoutBox()->HasPositionDependentContentWidth());
      CHECK(textRenderer.HasPositionDependentContentWidth() == true);
    }

    SECTION("Tearing down")
    {
      auto &textRenderer = *rootRenderer.FirstChild();
      layout_integration::BoxTreeUpdater(rootRenderer).TearDown();
      CHECK_FALSE(rootRenderer.LayoutBox());
      CHECK_FALSE(textRenderer.LayoutBox());
      CHECK_FALSE(initialContainingBlock.FirstChild());
      return;
    }

    layout_integration::BoxTreeUpdater(rootRenderer).TearDown();
    CHECK_FALSE(initialContainingBlock.FirstChild());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "BoxTreeUpdater: anonymous blocks and list markers",
                   "[Layout][Integration][BoxTreeUpdater]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><body><div id=c style="overflow: hidden">text<div>block</div></div>
    <ul><li id=li>item</li></ul></body>)html"};
    (void)fixture.Document->UpdateLayout();

    SECTION("An anonymous block gets its parent's overflow")
    {
      auto &anonymousBlock = Downcast<RenderBlockFlow>(*RendererOf(fixture, u8"c").FirstChild());
      REQUIRE(anonymousBlock.IsAnonymousBlock());
      CHECK(anonymousBlock.Style().OverflowX() == Overflow::Visible);

      auto &box = *layout_integration::BoxTreeUpdater(anonymousBlock).Build();
      CHECK(box.IsAnonymous());
      CHECK_FALSE(box.IsPaddingApplicable());
      CHECK(box.IsAnonymousTextIndentCandidateForIntegration());
      CHECK(box.Style().OverflowX() == Overflow::Hidden);
      CHECK(box.Style().OverflowY() == Overflow::Hidden);
      CHECK(TextContents(box) == List<utf8_string> {u8"text"});
      layout_integration::BoxTreeUpdater(anonymousBlock).TearDown();
    }

    SECTION("A list marker is a replaced box")
    {
      auto &listItem = Downcast<RenderListItem>(RendererOf(fixture, u8"li"));
      auto &box = *layout_integration::BoxTreeUpdater(listItem).Build();
      auto &marker = Downcast<layout::ElementBox>(*box.FirstChild());
      CHECK(marker.RendererForIntegration() == listItem.MarkerRenderer());
      CHECK(marker.IsListMarkerBox());
      CHECK(marker.IsReplacedBox());
      CHECK(marker.IsAtomicInlineBox());
      CHECK(marker.IsListMarkerOutside());
      CHECK_FALSE(marker.IsListMarkerImage());
      CHECK(marker.LayoutBoundsForListMarker() == Pair<int32, int32> {});
      CHECK(TextContents(box) == List<utf8_string> {u8"item"});
      layout_integration::BoxTreeUpdater(listItem).TearDown();
    }
  }
}
