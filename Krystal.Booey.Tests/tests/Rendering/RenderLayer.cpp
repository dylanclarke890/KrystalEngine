#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD RenderLayer *LayerOf(ResolverFixture &fixture, dom::DOMStringView id)
    {
      auto *renderer = DynamicDowncast<RenderLayerModelObject>(fixture.ById(id).Renderer());
      return renderer ? renderer->Layer() : nullptr;
    }

    KRYS_NODISCARD RenderLayer &ViewLayer(ResolverFixture &fixture)
    {
      return *fixture.Document->GetRenderView()->Layer();
    }

    KRYS_NODISCARD List<RenderLayer *> Layers(RenderLayer::LayerList list)
    {
      return {list.begin(), list.end()};
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderLayer: the layer tree", "[Rendering][RenderLayer]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      .rel { position: relative }
      #abs { position: absolute }
      #z1 { position: relative; z-index: 1 }
      #z2 { position: relative; z-index: 2 }
      #zneg { position: relative; z-index: -1 }
      #clip { overflow: hidden }
      #opaque { opacity: 0.5 }
      #hidden { position: relative; visibility: hidden }
    </style><body id=body><div id=static><div id=rel class=rel><div id=inner class=rel></div></div></div>
    <div id=abs></div><div id=z2></div><div id=z1></div><div id=zneg></div><div id=clip></div>
    <div id=opaque></div><div id=hidden></div></body>)html"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();
    (void)document.UpdateLayout();

    auto &viewLayer = ViewLayer(fixture);
    auto *rootLayer =
      fixture.Document->DocumentElement()->Renderer()
        ? Downcast<RenderLayerModelObject>(*fixture.Document->DocumentElement()->Renderer()).Layer()
        : nullptr;

    SECTION("Which renderers have layers")
    {
      CHECK(viewLayer.IsRenderViewLayer());
      CHECK_FALSE(viewLayer.Parent());
      REQUIRE(rootLayer);
      CHECK(rootLayer->Parent() == &viewLayer);
      CHECK_FALSE(LayerOf(fixture, u8"body"));
      CHECK_FALSE(LayerOf(fixture, u8"static"));
      for (auto id :
           {u8"rel", u8"inner", u8"abs", u8"z1", u8"z2", u8"zneg", u8"clip", u8"opaque", u8"hidden"})
      {
        INFO(css::tests::ToNarrow(id));
        CHECK(LayerOf(fixture, id));
      }
    }

    SECTION("Layers attach to the nearest enclosing layer, in render tree order")
    {
      REQUIRE(rootLayer);
      CHECK(LayerOf(fixture, u8"rel")->Parent() == rootLayer);
      CHECK(LayerOf(fixture, u8"inner")->Parent() == LayerOf(fixture, u8"rel"));
      List<RenderLayer *> children;
      for (auto *child = rootLayer->FirstChild(); child; child = child->NextSibling())
      {
        children.push_back(child);
      }
      CHECK(children
            == List<RenderLayer *> {LayerOf(fixture, u8"rel"), LayerOf(fixture, u8"abs"),
                                    LayerOf(fixture, u8"z2"), LayerOf(fixture, u8"z1"),
                                    LayerOf(fixture, u8"zneg"), LayerOf(fixture, u8"clip"),
                                    LayerOf(fixture, u8"opaque"), LayerOf(fixture, u8"hidden")});
      CHECK(LayerOf(fixture, u8"inner")->CommonAncestorWithLayer(*LayerOf(fixture, u8"z1")) == rootLayer);
      CHECK(LayerOf(fixture, u8"inner")->IsDescendantOf(viewLayer));
    }

    SECTION("Stacking contexts")
    {
      REQUIRE(rootLayer);
      CHECK(viewLayer.IsStackingContext());
      // The root's `view-transition-name: root` (from the UA style sheet) makes it a stacking context.
      CHECK(rootLayer->IsStackingContext());
      CHECK(LayerOf(fixture, u8"z1")->IsStackingContext());
      CHECK(LayerOf(fixture, u8"zneg")->IsStackingContext());
      CHECK_FALSE(LayerOf(fixture, u8"rel")->IsStackingContext());
      CHECK(LayerOf(fixture, u8"inner")->StackingContext() == rootLayer);
      CHECK(rootLayer->StackingContext() == &viewLayer);
      CHECK(LayerOf(fixture, u8"clip")->IsNormalFlowOnly());
      CHECK_FALSE(LayerOf(fixture, u8"opaque")->IsNormalFlowOnly());
      CHECK(LayerOf(fixture, u8"z1")->ZIndex() == 1);
      CHECK(LayerOf(fixture, u8"zneg")->ZIndex() == -1);
      CHECK(LayerOf(fixture, u8"rel")->ZIndex() == 0);
    }

    SECTION("Z-order and normal flow lists")
    {
      REQUIRE(rootLayer);
      viewLayer.UpdateLayerListsIfNeeded();
      rootLayer->UpdateLayerListsIfNeeded();

      CHECK(Layers(viewLayer.PositiveZOrderLayers()) == List<RenderLayer *> {rootLayer});
      CHECK_FALSE(viewLayer.HasNegativeZOrderLayers());

      // Positive (and auto) z-indices in z-order, then tree order; the hidden layer isn't listed.
      CHECK(Layers(rootLayer->PositiveZOrderLayers())
            == List<RenderLayer *> {LayerOf(fixture, u8"rel"), LayerOf(fixture, u8"inner"),
                                    LayerOf(fixture, u8"abs"), LayerOf(fixture, u8"opaque"),
                                    LayerOf(fixture, u8"z1"), LayerOf(fixture, u8"z2")});
      CHECK(Layers(rootLayer->NegativeZOrderLayers()) == List<RenderLayer *> {LayerOf(fixture, u8"zneg")});
      CHECK(rootLayer->HasNegativeZOrderLayers());
      CHECK(Layers(rootLayer->NormalFlowLayers()) == List<RenderLayer *> {LayerOf(fixture, u8"clip")});
      CHECK_FALSE(LayerOf(fixture, u8"hidden")->HasVisibleContent());
      CHECK(LayerOf(fixture, u8"rel")->HasVisibleDescendant());
    }

    SECTION("Self-painting layers")
    {
      CHECK(LayerOf(fixture, u8"rel")->IsSelfPaintingLayer());
      CHECK_FALSE(LayerOf(fixture, u8"clip")->IsSelfPaintingLayer());
      viewLayer.UpdateDescendantDependentFlags();
      CHECK(viewLayer.HasSelfPaintingLayerDescendant());
    }

    SECTION("Gaining and losing layers")
    {
      auto &staticDiv = fixture.ById(u8"static");
      REQUIRE_FALSE(staticDiv.SetAttribute(u8"style", u8"position: relative; z-index: 3").HasException());
      document.UpdateStyleIfNeeded();
      auto *staticLayer = LayerOf(fixture, u8"static");
      REQUIRE(staticLayer);
      CHECK(staticLayer->Parent() == rootLayer);
      CHECK(LayerOf(fixture, u8"rel")->Parent() == staticLayer);
      CHECK(staticLayer->IsStackingContext());

      REQUIRE_FALSE(staticDiv.SetAttribute(u8"style", u8"").HasException());
      document.UpdateStyleIfNeeded();
      CHECK_FALSE(LayerOf(fixture, u8"static"));
      CHECK(LayerOf(fixture, u8"rel")->Parent() == rootLayer);

      REQUIRE_FALSE(fixture.ById(u8"rel").Remove().HasException());
      document.UpdateStyleIfNeeded();
      CHECK(rootLayer->FirstChild() == LayerOf(fixture, u8"abs"));
    }

    SECTION("Clip rects")
    {
      auto *clipLayer = LayerOf(fixture, u8"clip");
      RenderLayer::ClipRectsContext context(&viewLayer, PaintingClipRects,
                                            RenderLayer::ClipRectTemporaryOptions);
      auto rects = clipLayer->CalculateRects(context, clipLayer->OffsetFromAncestor(&viewLayer));
      // Without an overflow clip of its own, the root's clip is infinite.
      CHECK(rects.GetBackgroundRect().Rect() == clipLayer->Rect());
      CHECK_FALSE(rects.DirtyForegroundRect().IsInfinite());
      auto viewRects = viewLayer.CalculateRects(context, {});
      CHECK(viewRects.GetBackgroundRect().IsInfinite());
    }
  }

  TEST_CASE("RenderLayer: paints opacity in a transparency layer", "[Rendering][RenderLayer][Painting]")
  {
    CHECK(
      Painted(
        u8"<style>body{margin:0}div{width:40px;height:20px;background:yellow;opacity:.5}</style><div></div>")
      == u8R"(
(set-state
  (change-flags [composite-mode])
  (composite-mode 
    (composite-operation copy)
    (blend-mode normal)))
(fill-rect-with-color
  (rect at (0,0) size 200x100)
  (color #FFFFFF))
(set-state
  (change-flags [composite-mode])
  (composite-mode 
    (composite-operation source-over)
    (blend-mode normal)))
(fill-composited-rect
  (rect at (0,0) size 200x100)
  (color #FFFFFF)
  (composite-operation copy)
  (blend-mode normal))
(save)
(clip
  (rect at (0,0) size 40x20))
(begin-transparency-layer
  (opacity 0.50))
(fill-composited-rect
  (rect at (0,0) size 40x20)
  (color #FFFF00)
  (composite-operation source-over)
  (blend-mode normal))
(end-transparency-layer)
(restore))");
  }

  TEST_CASE("RenderLayer: paints with its transform", "[Rendering][RenderLayer][Painting]")
  {
    CHECK(Painted(u8"<style>body{margin:0}div{width:40px;height:20px;background:yellow;transform:translate("
                  u8"10px,5px)}</style><div></div>")
          == u8R"(
(set-state
  (change-flags [composite-mode])
  (composite-mode 
    (composite-operation copy)
    (blend-mode normal)))
(fill-rect-with-color
  (rect at (0,0) size 200x100)
  (color #FFFFFF))
(set-state
  (change-flags [composite-mode])
  (composite-mode 
    (composite-operation source-over)
    (blend-mode normal)))
(fill-composited-rect
  (rect at (0,0) size 200x100)
  (color #FFFFFF)
  (composite-operation copy)
  (blend-mode normal))
(concatenate-ctm
  (ctm {m=((1.00,0.00)(0.00,1.00)) t=(10.00,5.00)}))
(fill-composited-rect
  (rect at (0,0) size 40x20)
  (color #FFFF00)
  (composite-operation source-over)
  (blend-mode normal))
(set-ctm
  (set-ctm identity)))");

    CHECK(Painted(u8"<style>body{margin:0}div{width:40px;height:20px;background:yellow;transform:rotate("
                  u8"90deg)}</style><div></div>")
          == u8R"(
(set-state
  (change-flags [composite-mode])
  (composite-mode 
    (composite-operation copy)
    (blend-mode normal)))
(fill-rect-with-color
  (rect at (0,0) size 200x100)
  (color #FFFFFF))
(set-state
  (change-flags [composite-mode])
  (composite-mode 
    (composite-operation source-over)
    (blend-mode normal)))
(fill-composited-rect
  (rect at (0,0) size 200x100)
  (color #FFFFFF)
  (composite-operation copy)
  (blend-mode normal))
(concatenate-ctm
  (ctm {m=((0.00,1.00)(-1.00,0.00)) t=(30.00,-10.00)}))
(fill-composited-rect
  (rect at (0,0) size 40x20)
  (color #FFFF00)
  (composite-operation source-over)
  (blend-mode normal))
(set-ctm
  (set-ctm identity)))");
  }

  TEST_CASE("RenderLayer: clips overflow to the rounded padding box", "[Rendering][RenderLayer][Painting]")
  {
    CHECK(Painted(u8"<style>body{margin:0}#a{width:40px;height:40px;overflow:hidden;border-radius:10px}#b{"
                  u8"height:80px;background:blue}</style><div id=a><div id=b></div></div>")
          == u8R"(
(set-state
  (change-flags [composite-mode])
  (composite-mode 
    (composite-operation copy)
    (blend-mode normal)))
(fill-rect-with-color
  (rect at (0,0) size 200x100)
  (color #FFFFFF))
(set-state
  (change-flags [composite-mode])
  (composite-mode 
    (composite-operation source-over)
    (blend-mode normal)))
(fill-composited-rect
  (rect at (0,0) size 200x100)
  (color #FFFFFF)
  (composite-operation copy)
  (blend-mode normal))
(save)
(clip-rounded-rect
  (rect at (0,0) size 40x40
    (top-left width=10 height=10)
    (top-right width=10 height=10)
    (bottom-left width=10 height=10)
    (bottom-right width=10 height=10)))
(clip
  (rect at (0,0) size 40x40))
(fill-composited-rect
  (rect at (0,0) size 40x80)
  (color #0000FF)
  (composite-operation source-over)
  (blend-mode normal))
(restore)
(save)
(clip-rounded-rect
  (rect at (0,0) size 40x40
    (top-left width=10 height=10)
    (top-right width=10 height=10)
    (bottom-left width=10 height=10)
    (bottom-right width=10 height=10)))
(clip
  (rect at (0,0) size 40x40))
(restore)
(save)
(clip-rounded-rect
  (rect at (0,0) size 40x40
    (top-left width=10 height=10)
    (top-right width=10 height=10)
    (bottom-left width=10 height=10)
    (bottom-right width=10 height=10)))
(clip
  (rect at (0,0) size 40x40))
(restore)
(save)
(clip-rounded-rect
  (rect at (0,0) size 40x40
    (top-left width=10 height=10)
    (top-right width=10 height=10)
    (bottom-left width=10 height=10)
    (bottom-right width=10 height=10)))
(clip
  (rect at (0,0) size 40x40))
(restore))");
  }

  TEST_CASE("RenderLayer: paints negative z-index layers under normal flow and positive ones over it",
            "[Rendering][RenderLayer][Painting]")
  {
    CHECK(Painted(u8"<style>body{margin:0}#p{position:relative;width:40px;height:40px;background:yellow}#n{"
                  u8"position:absolute;z-index:-1;width:20px;height:20px;background:blue}#q{position:"
                  u8"absolute;z-index:1;left:10px;width:20px;height:20px;background:red}</style><div "
                  u8"id=p><div id=n></div><div id=q></div></div>")
          == u8R"(
(set-state
  (change-flags [composite-mode])
  (composite-mode 
    (composite-operation copy)
    (blend-mode normal)))
(fill-rect-with-color
  (rect at (0,0) size 200x100)
  (color #FFFFFF))
(set-state
  (change-flags [composite-mode])
  (composite-mode 
    (composite-operation source-over)
    (blend-mode normal)))
(fill-composited-rect
  (rect at (0,0) size 200x100)
  (color #FFFFFF)
  (composite-operation copy)
  (blend-mode normal))
(fill-composited-rect
  (rect at (0,0) size 20x20)
  (color #0000FF)
  (composite-operation source-over)
  (blend-mode normal))
(fill-composited-rect
  (rect at (0,0) size 40x40)
  (color #FFFF00)
  (composite-operation source-over)
  (blend-mode normal))
(fill-composited-rect
  (rect at (10,0) size 20x20)
  (color #FF0000)
  (composite-operation source-over)
  (blend-mode normal)))");
  }

  TEST_CASE("RenderLayer: isolates blend modes in the stacking context", "[Rendering][RenderLayer][Painting]")
  {
    CHECK(Painted(u8"<style>body{margin:0}div{width:40px;height:20px;background:red;mix-blend-mode:multiply}<"
                  u8"/style><div></div>")
          == u8R"(
(set-state
  (change-flags [composite-mode])
  (composite-mode 
    (composite-operation copy)
    (blend-mode normal)))
(fill-rect-with-color
  (rect at (0,0) size 200x100)
  (color #FFFFFF))
(set-state
  (change-flags [composite-mode])
  (composite-mode 
    (composite-operation source-over)
    (blend-mode normal)))
(save)
(clip
  (rect at (0,0) size 200x100))
(begin-transparency-layer
  (opacity 1.00))
(save)
(clip
  (rect at (0,0) size 40x20))
(set-state
  (change-flags [composite-mode])
  (composite-mode 
    (composite-operation source-over)
    (blend-mode multiply)))
(begin-transparency-layer
  (opacity 1.00))
(set-state
  (change-flags [composite-mode])
  (composite-mode 
    (composite-operation source-over)
    (blend-mode normal)))
(fill-composited-rect
  (rect at (0,0) size 40x20)
  (color #FF0000)
  (composite-operation source-over)
  (blend-mode normal))
(end-transparency-layer)
(restore)
(end-transparency-layer)
(restore))");
  }

  TEST_CASE("RenderLayer: clips to clip-path shapes", "[Rendering][RenderLayer][Painting]")
  {
    CHECK(Painted(u8"<style>body{margin:0}div{width:40px;height:40px;background:red;clip-path:inset(5px "
                  u8"round 4px)}</style><div></div>")
          == u8R"(
(set-state
  (change-flags [composite-mode])
  (composite-mode 
    (composite-operation copy)
    (blend-mode normal)))
(fill-rect-with-color
  (rect at (0,0) size 200x100)
  (color #FFFFFF))
(set-state
  (change-flags [composite-mode])
  (composite-mode 
    (composite-operation source-over)
    (blend-mode normal)))
(fill-composited-rect
  (rect at (0,0) size 200x100)
  (color #FFFFFF)
  (composite-operation copy)
  (blend-mode normal))
(save)
(clip-path
  (path add rounded rect at (5,5) size 30x30
    (top-left width=4 height=4)
    (top-right width=4 height=4)
    (bottom-left width=4 height=4)
    (bottom-right width=4 height=4))
  (wind-rule NON-ZERO))
(fill-composited-rect
  (rect at (0,0) size 40x40)
  (color #FF0000)
  (composite-operation source-over)
  (blend-mode normal))
(restore))");

    CHECK(Painted(u8"<style>body{margin:0}div{width:40px;height:40px;background:red;clip-path:circle(10px)}</"
                  u8"style><div></div>")
          == u8R"(
(set-state
  (change-flags [composite-mode])
  (composite-mode 
    (composite-operation copy)
    (blend-mode normal)))
(fill-rect-with-color
  (rect at (0,0) size 200x100)
  (color #FFFFFF))
(set-state
  (change-flags [composite-mode])
  (composite-mode 
    (composite-operation source-over)
    (blend-mode normal)))
(fill-composited-rect
  (rect at (0,0) size 200x100)
  (color #FFFFFF)
  (composite-operation copy)
  (blend-mode normal))
(save)
(clip-path
  (path add ellipse in rect at (10,10) size 20x20)
  (wind-rule NON-ZERO))
(fill-composited-rect
  (rect at (0,0) size 40x40)
  (color #FF0000)
  (composite-operation source-over)
  (blend-mode normal))
(restore))");
  }
}
