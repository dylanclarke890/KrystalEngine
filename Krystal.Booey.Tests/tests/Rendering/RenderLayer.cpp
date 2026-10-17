#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "catch2/catch_test_macros.hpp"
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
}
