#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Animation/AnimationTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD List<dom::Element *> TopLayer(const dom::Document &document)
    {
      List<dom::Element *> elements;
      for (auto &element : document.TopLayerElements())
      {
        elements.push_back(element.get());
      }
      return elements;
    }

    KRYS_NODISCARD RenderLayer *LayerOf(const RenderElement *renderer)
    {
      auto *layerModelObject = DynamicDowncast<RenderLayerModelObject>(renderer);
      return layerModelObject ? layerModelObject->Layer() : nullptr;
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "TopLayer: the document's top layer", "[DOM][TopLayer]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><div id=a></div><div id=b><div id=c></div></div>)html"};
    auto &document = *fixture.Document;
    auto &a = fixture.ById(u8"a");
    auto &b = fixture.ById(u8"b");
    auto &c = fixture.ById(u8"c");

    CHECK_FALSE(document.HasTopLayerElement());
    a.AddToTopLayer();
    c.AddToTopLayer();
    CHECK(a.IsInTopLayer());
    CHECK_FALSE(b.IsInTopLayer());
    CHECK(TopLayer(document) == List<dom::Element *> {&a, &c});

    SECTION("Removing and adding again puts an element on top")
    {
      a.RemoveFromTopLayer();
      CHECK_FALSE(a.IsInTopLayer());
      CHECK(TopLayer(document) == List<dom::Element *> {&c});
      a.AddToTopLayer();
      CHECK(TopLayer(document) == List<dom::Element *> {&c, &a});
    }

    SECTION("Elements leave the top layer, and stop being the target, when they're disconnected")
    {
      document.SetCSSTarget(&c);
      RefPtr<dom::Element> keep = ShareRefPtr(&b);
      REQUIRE_FALSE(dom::MutationAlgorithms::Remove(b).HasException());
      CHECK_FALSE(c.IsInTopLayer());
      CHECK(TopLayer(document) == List<dom::Element *> {&a});
      CHECK_FALSE(document.CSSTarget());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "TopLayer: rendering and ::backdrop", "[DOM][TopLayer]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      body { margin: 0 }
      #z { position: relative; z-index: 1000; width: 100px; height: 100px }
      #top { left: 200px; top: 0; width: 50px; height: 50px }
      #top::backdrop { background: red }
    </style><div id=z></div><div id=top></div>)html"};
    auto &document = *fixture.Document;
    auto &z = fixture.ById(u8"z");
    auto &top = fixture.ById(u8"top");
    auto &viewLayer = *document.GetRenderView()->Layer();
    (void)document.UpdateLayout();
    CHECK(document.ElementFromPoint(10, 10).get() == &z);

    top.AddToTopLayer();
    (void)document.UpdateLayout();

    auto *renderer = top.Renderer();
    REQUIRE(renderer);
    CHECK(renderer->Style().GetPosition() == PositionType::Absolute);
    auto *backdrop = renderer->BackdropRenderer();
    REQUIRE(backdrop);
    CHECK(backdrop->Style().GetPosition() == PositionType::Fixed);

    // The backdrop and the element are the view's last layers, above the z-index.
    REQUIRE(LayerOf(renderer));
    REQUIRE(LayerOf(backdrop));
    CHECK(LayerOf(renderer)->Parent() == &viewLayer);
    CHECK(LayerOf(backdrop)->Parent() == &viewLayer);
    CHECK(RenderLayer::TopLayerRenderLayers(*document.GetRenderView())
          == List<RenderLayer *> {LayerOf(backdrop), LayerOf(renderer)});
    viewLayer.UpdateLayerListsIfNeeded();
    auto zOrder = viewLayer.PositiveZOrderLayers();
    List<RenderLayer *> layers {zOrder.begin(), zOrder.end()};
    REQUIRE(layers.size() >= 2);
    CHECK(layers[layers.size() - 2] == LayerOf(backdrop));
    CHECK(layers.back() == LayerOf(renderer));

    // The element is placed in the viewport, and its backdrop hit tests to it.
    CHECK(document.ElementFromPoint(210, 10).get() == &top);
    CHECK(document.ElementFromPoint(10, 10).get() == &top);

    top.RemoveFromTopLayer();
    (void)document.UpdateLayout();
    REQUIRE(top.Renderer());
    CHECK_FALSE(top.Renderer()->BackdropRenderer());
    CHECK(top.Renderer()->Style().GetPosition() == PositionType::Static);
    CHECK(document.ElementFromPoint(10, 10).get() == &z);
    CHECK(document.ElementFromPoint(210, 10).get() != &top);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "TopLayer: overlay and pending removals", "[DOM][TopLayer]")
  {
    using namespace std::chrono_literals;

    AnimationPageFixture fixture {u8R"html(<!DOCTYPE html><style>
      div { overlay: auto !important }
    </style><div id=top></div><div id=fade></div>)html"};
    auto &document = *fixture.Document;
    auto &viewLayer = *document.GetRenderView()->Layer();
    auto overlayOf = [&](dom::DOMStringView id)
    {
      auto *renderer = fixture.ById(id).Renderer();
      REQUIRE(renderer);
      return renderer->Style().GetOverlay();
    };
    auto isRenderedInTopLayer = [&](dom::DOMStringView id)
    {
      auto *layer = LayerOf(fixture.ById(id).Renderer());
      return layer && layer->Parent() == &viewLayer;
    };

    // Authors can't set overlay.
    CHECK(overlayOf(u8"top") == Overlay::None);

    auto &top = fixture.ById(u8"top");
    top.AddToTopLayer();
    fixture.Frame();
    CHECK(overlayOf(u8"top") == Overlay::Auto);
    CHECK(isRenderedInTopLayer(u8"top"));

    SECTION("An element leaves at the next rendering update")
    {
      top.RequestRemoveFromTopLayer();
      CHECK(top.IsInTopLayer());
      CHECK(document.IsPendingTopLayerRemoval(top));
      fixture.Frame();
      CHECK_FALSE(top.IsInTopLayer());
      CHECK_FALSE(document.IsPendingTopLayerRemoval(top));
      CHECK(overlayOf(u8"top") == Overlay::None);
      CHECK_FALSE(isRenderedInTopLayer(u8"top"));
    }

    SECTION("Adding an element that's leaving moves it to the top")
    {
      auto &fade = fixture.ById(u8"fade");
      fade.AddToTopLayer();
      top.RequestRemoveFromTopLayer();
      top.AddToTopLayer();
      CHECK_FALSE(document.IsPendingTopLayerRemoval(top));
      CHECK(TopLayer(document) == List<dom::Element *> {&fade, &top});
      fixture.Frame();
      CHECK(top.IsInTopLayer());
      CHECK(isRenderedInTopLayer(u8"top"));
    }

    SECTION("A transition on overlay keeps an element in the top layer until it ends")
    {
      auto &fade = fixture.ById(u8"fade");
      fade.AddToTopLayer();
      fixture.Frame();
      CHECK(isRenderedInTopLayer(u8"fade"));

      fixture.SetStyle(u8"fade", u8"transition: overlay 1s allow-discrete");
      fixture.Frame();
      fade.RequestRemoveFromTopLayer();
      fixture.StartAnimations();
      fixture.Frame(500ms);
      CHECK(fade.IsInTopLayer());
      CHECK(overlayOf(u8"fade") == Overlay::Auto);
      CHECK(isRenderedInTopLayer(u8"fade"));

      fixture.Frame(600ms);
      fixture.Frame();
      CHECK_FALSE(fade.IsInTopLayer());
      CHECK(overlayOf(u8"fade") == Overlay::None);
      CHECK_FALSE(isRenderedInTopLayer(u8"fade"));
    }
  }
}
