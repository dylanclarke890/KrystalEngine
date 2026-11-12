#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Nodes/PseudoElement.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Booey/Rendering/GeneratedContent/RenderListItem.hpp"
#include "Krystal.Booey/Rendering/Grid/RenderGrid.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Tables/RenderTable.hpp"
#include "Krystal.Booey/Rendering/Tables/RenderTableCaption.hpp"
#include "Krystal.Booey/Rendering/Tables/RenderTableCell.hpp"
#include "Krystal.Booey/Rendering/Tables/RenderTableCol.hpp"
#include "Krystal.Booey/Rendering/Tables/RenderTableRow.hpp"
#include "Krystal.Booey/Rendering/Tables/RenderTableSection.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderBox: the renderer classes", "[Rendering][RenderBox]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      #flex { display: flex }
      #grid { display: grid }
      #li { display: list-item }
      #table { display: table }
      #caption { display: table-caption }
      #group { display: table-row-group }
      #row { display: table-row }
      #cell { display: table-cell }
      #colgroup { display: table-column-group }
      #col { display: table-column }
    </style><body id=body><div id=block></div><span id=span></span><div id=flex><div id=item></div></div>
    <div id=grid><div id=gitem></div></div><div id=li></div><div id=table></div>
    <div id=caption></div><div id=group></div><div id=row></div><div id=cell></div>
    <div id=colgroup></div><div id=col></div><fieldset id=fieldset></fieldset></body>)html"};
    fixture.Document->UpdateStyleIfNeeded();

    auto renderer = [&](utf8_stringview id) noexcept -> RenderElement &
    {
      auto *r = fixture.ById(id).Renderer();
      krys_assert(r);
      return *r;
    };

    SECTION("The display chooses the class")
    {
      CHECK(Is<RenderBlockFlow>(renderer(u8"block")));
      CHECK(Is<RenderInline>(renderer(u8"span")));
      CHECK_FALSE(Is<RenderBox>(renderer(u8"span")));
      CHECK(Is<RenderBoxModelObject>(renderer(u8"span")));
      CHECK(Is<RenderFlexibleBox>(renderer(u8"flex")));
      CHECK(Is<RenderGrid>(renderer(u8"grid")));
      CHECK(Is<RenderListItem>(renderer(u8"li")));
      CHECK(Is<RenderTable>(renderer(u8"table")));
      CHECK(Is<RenderTableCaption>(renderer(u8"caption")));
      CHECK(Is<RenderTableSection>(renderer(u8"group")));
      CHECK(Is<RenderTableRow>(renderer(u8"row")));
      CHECK(Is<RenderTableCell>(renderer(u8"cell")));
      CHECK(Is<RenderTableCol>(renderer(u8"col")));
      CHECK(Is<RenderBlockFlow>(static_cast<RenderObject &>(*fixture.Document->GetRenderView())));
      CHECK(renderer(u8"cell").IsTablePart());
      CHECK_FALSE(Is<RenderBlock>(renderer(u8"row")));
    }

    SECTION("Render names")
    {
      CHECK(renderer(u8"block").RenderName() == u8"RenderBlock");
      CHECK(renderer(u8"body").RenderName() == u8"RenderBody");
      CHECK(renderer(u8"fieldset").RenderName() == u8"RenderFieldSet");
      CHECK(renderer(u8"span").RenderName() == u8"RenderInline");
      CHECK(renderer(u8"flex").RenderName() == u8"RenderFlexibleBox");
      CHECK(renderer(u8"grid").RenderName() == u8"RenderGrid");
      CHECK(renderer(u8"li").RenderName() == u8"RenderListItem");
      CHECK(renderer(u8"cell").RenderName() == u8"RenderTableCell");
      CHECK(renderer(u8"group").RenderName() == u8"RenderTableSection");
      CHECK(renderer(u8"row").RenderName() == u8"RenderTableRow");
      CHECK(renderer(u8"col").RenderName() == u8"RenderTableCol");
      CHECK(fixture.Document->GetRenderView()->RenderName() == u8"RenderView");
    }

    SECTION("Children inline")
    {
      CHECK(renderer(u8"block").ChildrenInline());
      CHECK(renderer(u8"span").ChildrenInline());
      CHECK_FALSE(renderer(u8"flex").ChildrenInline());
    }

    SECTION("Flex and grid items")
    {
      auto &item = Downcast<RenderBox>(renderer(u8"item"));
      CHECK(item.IsFlexItem());
      CHECK(item.IsInFlowFlexChild());
      CHECK_FALSE(item.IsGridItem());
      CHECK_FALSE(item.IsBlockLevelBox());
      CHECK(Downcast<RenderBox>(renderer(u8"gitem")).IsGridItem());
      CHECK(renderer(u8"block").IsBlockLevelBox());
      CHECK(renderer(u8"block").IsBlockContainer());
      CHECK(renderer(u8"block").IsBlockBox());
      CHECK_FALSE(renderer(u8"span").IsBlockLevelBox());
      CHECK_FALSE(renderer(u8"flex").IsBlockContainer());
    }

    SECTION("Table columns")
    {
      auto &colgroup = Downcast<RenderTableCol>(renderer(u8"colgroup"));
      auto &col = Downcast<RenderTableCol>(renderer(u8"col"));
      CHECK(colgroup.IsTableColumnGroup());
      CHECK(colgroup.CanHaveChildren());
      CHECK(col.IsTableColumn());
      CHECK_FALSE(col.CanHaveChildren());
      CHECK(colgroup.IsChildAllowed(col, col.Style()));
      CHECK_FALSE(colgroup.IsChildAllowed(renderer(u8"block"), renderer(u8"block").Style()));
      CHECK_FALSE(renderer(u8"row").IsInline());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderBox: state from the style", "[Rendering][RenderBox]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      #float { float: left }
      #abs { position: absolute }
      #fixed { position: fixed }
      #rel { position: relative }
      #sticky { position: sticky }
      #vertical { writing-mode: vertical-rl }
      #hidden { overflow: hidden }
      #transform { transform: rotate(1deg) }
      #paint { contain: paint }
      #background { background-color: red }
      #size { container-type: size; contain: size }
      #inline-size { container-type: inline-size }
      #host::before { content: "x" }
    </style><body><div id=plain></div><span id=span></span><div id=float></div><div id=abs></div>
    <div id=fixed></div><div id=rel></div><div id=sticky></div><div id=vertical></div><div id=hidden></div>
    <div id=transform></div><div id=paint></div><div id=background></div><div id=size></div>
    <div id=inline-size></div><div id=host></div></body>)html"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();

    auto renderer = [&](utf8_stringview id) noexcept -> RenderElement &
    {
      auto *r = fixture.ById(id).Renderer();
      krys_assert(r);
      return *r;
    };

    SECTION("Inline, floating and positioned")
    {
      CHECK_FALSE(renderer(u8"plain").IsInline());
      CHECK(renderer(u8"span").IsInline());
      CHECK(renderer(u8"float").IsFloating());
      CHECK(renderer(u8"float").IsFloatingOrOutOfFlowPositioned());
      CHECK_FALSE(renderer(u8"float").IsInFlow());
      CHECK(renderer(u8"abs").IsOutOfFlowPositioned());
      CHECK(renderer(u8"abs").IsAbsolutelyPositioned());
      CHECK(renderer(u8"fixed").IsOutOfFlowPositioned());
      CHECK(renderer(u8"fixed").IsFixedPositioned());
      CHECK(renderer(u8"rel").IsRelativelyPositioned());
      CHECK(renderer(u8"rel").IsInFlowPositioned());
      CHECK(renderer(u8"sticky").IsStickilyPositioned());
      CHECK_FALSE(renderer(u8"plain").IsPositioned());
      CHECK(renderer(u8"plain").IsInFlow());
    }

    SECTION("Writing mode, overflow, transforms, containment and decorations")
    {
      CHECK(renderer(u8"plain").IsHorizontalWritingMode());
      CHECK_FALSE(renderer(u8"vertical").IsHorizontalWritingMode());
      CHECK(renderer(u8"hidden").HasNonVisibleOverflow());
      CHECK(renderer(u8"hidden").HasPotentiallyScrollableOverflow());
      CHECK_FALSE(renderer(u8"plain").HasNonVisibleOverflow());
      CHECK(renderer(u8"transform").HasTransformRelatedProperty());
      CHECK(renderer(u8"paint").PaintContainmentApplies());
      CHECK(renderer(u8"paint").EffectiveOverflowX() == Overflow::Clip);
      CHECK(renderer(u8"paint").HasNonVisibleOverflow());
      CHECK(renderer(u8"background").HasVisibleBoxDecorations());
      CHECK_FALSE(renderer(u8"plain").HasVisibleBoxDecorations());
      CHECK(document.DocumentElement()->Renderer()->HasVisibleBoxDecorations());
      CHECK(renderer(u8"size").HasEligibleContainmentForSizeQuery());
      CHECK(renderer(u8"inline-size").HasEligibleContainmentForSizeQuery());
      CHECK(renderer(u8"plain").HasEligibleContainmentForSizeQuery());
      CHECK_FALSE(document.GetRenderView()->IsInline());
    }

    SECTION("Style changes update the state")
    {
      auto &element = fixture.ById(u8"float");
      auto *before = element.Renderer();
      REQUIRE_FALSE(element.SetAttribute(u8"style", u8"float: none; position: relative").HasException());
      document.UpdateStyleIfNeeded();
      REQUIRE(element.Renderer() == before);
      CHECK_FALSE(before->IsFloating());
      CHECK(before->IsRelativelyPositioned());

      REQUIRE_FALSE(element.SetAttribute(u8"style", u8"overflow: hidden").HasException());
      document.UpdateStyleIfNeeded();
      CHECK(element.Renderer()->HasNonVisibleOverflow());
      CHECK_FALSE(element.Renderer()->IsPositioned());
    }

    SECTION("Generated content")
    {
      auto *before = fixture.ById(u8"host").BeforePseudoElement()->Renderer();
      REQUIRE(before);
      CHECK(before->IsBeforeContent());
      CHECK(before->IsBeforeOrAfterContent());
      CHECK_FALSE(before->IsAfterContent());
      CHECK(before->IsPseudoElement());
      CHECK(before->GeneratingElement() == &fixture.ById(u8"host"));
      CHECK(before->RenderName() == u8"RenderInline (generated)");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderBox: size containment of a menu list",
                   "[Rendering][RenderBox]")
  {
    // A size-contained menu list keeps the height its theme gives it, as if it had no options; a block is as
    // tall as its border and padding.
    ResolverFixture fixture {
      u8"<style>select,div{margin:0;contain:size}</style><select id=contained><option>a</option></select>"
      u8"<select id=empty></select><div id=block style='padding:2px'>a</div>"};
    (void)fixture.Document->UpdateLayout();
    auto &contained = Downcast<RenderBox>(*fixture.ById(u8"contained").Renderer());
    auto &empty = Downcast<RenderBox>(*fixture.ById(u8"empty").Renderer());
    auto &block = Downcast<RenderBox>(*fixture.ById(u8"block").Renderer());
    CHECK(contained.Height() > 4_lu);
    CHECK(contained.Height() == empty.Height());
    CHECK(block.Height() == 4_lu);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderBox: containing blocks", "[Rendering][RenderBox]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      .abs { position: absolute }
      .fixed { position: fixed }
      .rel { position: relative }
      .transform { transform: scale(1) }
    </style><body id=body>
      <div id=static><div id=abs1 class=abs></div><div id=fixed1 class=fixed></div><div id=child></div>
        <span id=span><b id=b></b></span></div>
      <div id=rel class=rel><div id=abs2 class=abs></div><div id=fixed2 class=fixed></div></div>
      <div id=transform class=transform><div id=fixed3 class=fixed></div></div>
      <div><span id=relspan class=rel><div id=abs3 class=abs></div></span></div>
    </body>)html"};
    fixture.Document->UpdateStyleIfNeeded();

    auto renderer = [&](utf8_stringview id) noexcept -> RenderElement &
    {
      auto *r = fixture.ById(id).Renderer();
      krys_assert(r);
      return *r;
    };
    RenderElement *view = fixture.Document->GetRenderView();

    SECTION("In-flow boxes are contained by their nearest block ancestor")
    {
      CHECK(renderer(u8"child").ContainingBlock() == &renderer(u8"static"));
      CHECK(renderer(u8"child").Container() == &renderer(u8"static"));
      // An inline's containing block skips inline ancestors; its container is its parent. The span is wrapped
      // in an anonymous block, since its siblings are blocks.
      auto *wrapper = renderer(u8"span").Parent();
      REQUIRE(wrapper);
      CHECK(wrapper->IsAnonymousBlock());
      CHECK(wrapper->Parent() == &renderer(u8"static"));
      CHECK(renderer(u8"b").ContainingBlock() == wrapper);
      CHECK(renderer(u8"b").Container() == &renderer(u8"span"));
      CHECK(renderer(u8"body").ContainingBlock() == fixture.Document->DocumentElement()->Renderer());
    }

    SECTION("Absolutely positioned boxes are contained by their nearest positioned ancestor")
    {
      CHECK(renderer(u8"abs1").ContainingBlock() == view);
      CHECK(renderer(u8"abs1").Container() == view);
      CHECK(renderer(u8"abs2").ContainingBlock() == &renderer(u8"rel"));
      CHECK(renderer(u8"abs2").Container() == &renderer(u8"rel"));
      // A relatively positioned inline forwards them to its containing block, but is their container.
      CHECK(renderer(u8"abs3").Container() == &renderer(u8"relspan"));
      CHECK(renderer(u8"abs3").ContainingBlock() == renderer(u8"relspan").ContainingBlock());
    }

    SECTION("Fixed positioned boxes are contained by the view or a transformed ancestor")
    {
      CHECK(renderer(u8"fixed1").ContainingBlock() == view);
      CHECK(renderer(u8"fixed2").ContainingBlock() == view);
      CHECK(renderer(u8"fixed3").ContainingBlock() == &renderer(u8"transform"));
      CHECK(renderer(u8"fixed3").Container() == &renderer(u8"transform"));
    }

    SECTION("Which ancestors can contain positioned boxes")
    {
      CHECK(renderer(u8"rel").CanContainAbsolutelyPositionedObjects());
      CHECK_FALSE(renderer(u8"rel").CanContainFixedPositionObjects());
      CHECK(renderer(u8"transform").CanContainFixedPositionObjects());
      CHECK(renderer(u8"transform").CanContainAbsolutelyPositionedObjects());
      CHECK_FALSE(renderer(u8"static").CanContainAbsolutelyPositionedObjects());
      CHECK(view->CanContainFixedPositionObjects());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "RenderBox: transforms add to the container's scrollable overflow",
                   "[Rendering][RenderBox][!shouldfail]")
  {
    // FIXME: Transforms aren't in the overflow they propagate yet (see KNOWN-ISSUES.md: they'd stretch
    // balanced columns).
    ResolverFixture fixture {
      u8"<!DOCTYPE html><style>body{margin:0}#s{width:100px;height:100px;overflow:hidden}"
      u8"#s>div{width:50px;height:50px}</style><div id=s><div id=child></div></div>"};
    auto &scroller = fixture.ById(u8"s");
    auto &child = fixture.ById(u8"child");
    (void)fixture.Document->UpdateLayout();
    CHECK(scroller.ScrollWidth() == 100);
    CHECK(scroller.ScrollHeight() == 100);

    REQUIRE_FALSE(child.SetAttribute(u8"style", u8"transform: translate(200px, 300px)").HasException());
    (void)fixture.Document->UpdateLayout();
    CHECK(scroller.ScrollWidth() == 250);
    CHECK(scroller.ScrollHeight() == 350);

    // Relative positioning moves the box before its transform.
    REQUIRE_FALSE(
      child.SetAttribute(u8"style", u8"position: relative; left: 10px; transform: translateX(200px)")
        .HasException());
    (void)fixture.Document->UpdateLayout();
    CHECK(scroller.ScrollWidth() == 260);
  }

  // A box's transform includes its motion path, placed relative to its containing block.
  TEST_CASE("RenderBox: transforms include the motion path data of the box", "[Rendering][RenderBox]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      body { margin: 0 }
      #container { position: relative; width: 200px; height: 200px }
      #moving { width: 10px; height: 10px; offset-path: ray(90deg); offset-distance: 50px }
    </style><div id=container><div id=moving></div></div>)html"};
    (void)fixture.Document->UpdateLayout();
    auto *box = DynamicDowncast<RenderBox>(fixture.ById(u8"moving").Renderer());
    REQUIRE(box);

    // The ray starts at the containing block's centre and points right, and the box's centre follows it.
    TransformationMatrix matrix;
    box->ApplyTransform(matrix, box->Style(), box->TransformReferenceBoxRect());
    CHECK(matrix.M41() == 145);
    CHECK(matrix.M42() == 95);
  }

  // The layer's transform waits for the containing block's layout when the motion path depends on its size.
  TEST_CASE("RenderBox: a motion path transform updates after its containing block's layout",
            "[Rendering][RenderBox]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      body { margin: 0 }
      #container { position: relative; width: 200px }
      #moving { width: 10px; height: 10px; offset-path: ray(90deg); offset-distance: 50px }
    </style><div id=container><div id=moving></div><div style=height:190px></div></div>)html"};
    (void)fixture.Document->UpdateLayout();
    auto *box = DynamicDowncast<RenderBox>(fixture.ById(u8"moving").Renderer());
    REQUIRE(box);
    REQUIRE(box->HasLayer());
    auto *transform = box->Layer()->Transform();
    REQUIRE(transform);

    // The ray starts at the centre of the 200px tall container, which is known only after its layout.
    CHECK(transform->M41() == 145);
    CHECK(transform->M42() == 95);
  }
}
