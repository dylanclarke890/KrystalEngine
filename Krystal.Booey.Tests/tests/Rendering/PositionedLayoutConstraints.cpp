#include "Krystal.Booey/Rendering/PositionedLayoutConstraints.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderTreeAsText.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    constexpr const char8 *Markup = u8R"html(<!DOCTYPE html><style>
      body { margin: 0 }
      #cb { position: relative; width: 400px; height: 300px; border: 5px solid; padding: 10px }
      #tl { position: absolute; top: 10px; left: 20px; width: 50px; height: 40px }
      #br { position: absolute; right: 10px; bottom: 20px; width: 30px; height: 30px; margin: 5px }
      #stretch { position: absolute; left: 0; right: 0; top: 50%; height: 10% }
      #center { position: absolute; inset: 0; margin: auto; width: 100px; height: 50px }
      #static { position: absolute }
      #shrink { position: absolute; top: 0; left: 0 }
      #align { position: absolute; inset: 0; width: 60px; height: 60px; justify-self: center; align-self: end }
      #fixed { position: fixed; bottom: 0; right: 0; width: 10px; height: 10px }
    </style><body><div id=cb><div id=tl></div><div id=br></div><div id=stretch></div><div id=center></div><div style="height:20px"></div><div id=static></div><div id=shrink><div style="width:70px;height:5px"></div></div><div id=align></div></div><div id=fixed></div></body>)html";

    KRYS_NODISCARD RenderBox &BoxOf(ResolverFixture &fixture, dom::DOMStringView id) noexcept
    {
      return Downcast<RenderBox>(*fixture.ById(id).Renderer());
    }

    KRYS_NODISCARD bool HasOutOfFlowBox(const RenderBlock &block, const RenderBox &box) noexcept
    {
      auto *boxes = block.OutOfFlowBoxes();
      return boxes && boxes->contains(const_cast<RenderBox *>(&box));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "PositionedLayoutConstraints: out-of-flow layout",
                   "[Rendering][PositionedLayoutConstraints]")
  {
    ResolverFixture fixture {Markup};

    // Insets, auto margins and alignment resolve against the containing block's padding box; boxes without
    // insets take their static position; boxes with auto widths shrink to fit; fixed boxes are contained by
    // the view.
    CHECK(ExternalRepresentation(fixture.Frame.get(),
                                 {RenderAsTextFlag::ShowAllLayers, RenderAsTextFlag::ShowIDAndClass})
          == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x330
  RenderBlock {HTML} at (0,0) size 800x330
    RenderBody {BODY} at (0,0) size 800x330
layer at (0,0) size 430x330
  RenderBlock (relative positioned) {DIV} at (0,0) size 430x330 [border: (5px solid #000000)] id="cb"
    RenderBlock {DIV} at (15,15) size 400x20
layer at (25,15) size 50x40
  RenderBlock (positioned) {DIV} at (25,15) size 50x40 id="tl"
layer at (380,270) size 30x30
  RenderBlock (positioned) {DIV} at (380,270) size 30x30 id="br"
layer at (5,165) size 420x32
  RenderBlock (positioned) {DIV} at (5,165) size 420x32 id="stretch"
layer at (165,140) size 100x50
  RenderBlock (positioned) {DIV} at (165,140) size 100x50 id="center"
layer at (15,35) size 0x0
  RenderBlock (positioned) {DIV} at (15,35) size 0x0 id="static"
layer at (5,5) size 70x5
  RenderBlock (positioned) {DIV} at (5,5) size 70x5 id="shrink"
    RenderBlock {DIV} at (0,0) size 70x5
layer at (185,265) size 60x60
  RenderBlock (positioned) {DIV} at (185,265) size 60x60 id="align"
layer at (790,590) size 10x10
  RenderBlock (positioned) {DIV} at (790,590) size 10x10 id="fixed"
)");

    auto &containingBlock = Downcast<RenderBlock>(BoxOf(fixture, u8"cb"));
    auto &view = *fixture.Document->GetRenderView();
    CHECK(HasOutOfFlowBox(containingBlock, BoxOf(fixture, u8"tl")));
    CHECK(HasOutOfFlowBox(view, BoxOf(fixture, u8"fixed")));
    CHECK_FALSE(HasOutOfFlowBox(view, BoxOf(fixture, u8"tl")));

    SECTION("The containing block stops being one")
    {
      REQUIRE_FALSE(fixture.ById(u8"cb").SetAttribute(u8"style", u8"position: static").HasException());
      (void)fixture.Document->UpdateLayout();
      CHECK_FALSE(containingBlock.HasOutOfFlowBoxes());
      CHECK(HasOutOfFlowBox(view, BoxOf(fixture, u8"tl")));
      CHECK(BoxOf(fixture, u8"tl").Location() == LayoutPoint(20, 10));
      CHECK(BoxOf(fixture, u8"static").Location() == LayoutPoint(15, 35));
    }

    SECTION("An out-of-flow box goes back into the flow")
    {
      REQUIRE_FALSE(
        fixture.ById(u8"static").SetAttribute(u8"style", u8"position: static; height: 7px").HasException());
      (void)fixture.Document->UpdateLayout();
      CHECK_FALSE(HasOutOfFlowBox(containingBlock, BoxOf(fixture, u8"static")));
      CHECK(BoxOf(fixture, u8"static").FrameRect() == LayoutRect(15, 35, 400, 7));
    }

    SECTION("Removed boxes leave their containing block")
    {
      auto *removed = &BoxOf(fixture, u8"tl");
      REQUIRE_FALSE(fixture.ById(u8"tl").Remove().HasException());
      (void)fixture.Document->UpdateLayout();
      CHECK_FALSE(HasOutOfFlowBox(containingBlock, *removed));
      CHECK(containingBlock.OutOfFlowBoxes()->size() == 6);

      REQUIRE_FALSE(fixture.ById(u8"cb").Remove().HasException());
      (void)fixture.Document->UpdateLayout();
      CHECK(view.OutOfFlowBoxes()->size() == 1);
    }

    SECTION("Relayout after a containing block's size change")
    {
      REQUIRE_FALSE(fixture.ById(u8"cb").SetAttribute(u8"style", u8"width: 200px").HasException());
      (void)fixture.Document->UpdateLayout();
      CHECK(BoxOf(fixture, u8"br").X() == 180_lu);
      CHECK(BoxOf(fixture, u8"stretch").Width() == 220_lu);
      CHECK(BoxOf(fixture, u8"center").X() == 65_lu);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "PositionedLayoutConstraints: inline containing blocks and content",
                   "[Rendering][PositionedLayoutConstraints]")
  {
    // FIXME: Inline content has no lines until inline layout, so the static position of out-of-flow boxes in
    // it is the start of the block's content box.
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      body { margin: 0 }
      #block { padding: 5px }
      #abs { position: absolute; width: 10px; height: 10px }
    </style><body><div id=block>text <span><div id=abs></div></span></div></body>)html"};

    (void)fixture.Document->UpdateLayout();
    auto &box = BoxOf(fixture, u8"abs");
    CHECK(box.Location() == LayoutPoint(5, 5));
    CHECK(box.Size() == LayoutSize(10, 10));
    CHECK(HasOutOfFlowBox(*fixture.Document->GetRenderView(), box));
  }
}
