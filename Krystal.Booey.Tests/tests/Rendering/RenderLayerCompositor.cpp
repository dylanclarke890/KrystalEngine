#include "Krystal.Booey/Rendering/RenderLayerCompositor.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerFactory.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayerBacking.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief A layer that records what it's asked to display.
    class CompositingTestLayer final : public GraphicsLayer
    {
    public:
      List<FloatRect> NeedsDisplayRects;

      CompositingTestLayer(Type type, GraphicsLayerClient &client) noexcept : GraphicsLayer(type, client)
      {
      }

      ~CompositingTestLayer() noexcept final
      {
        WillBeDestroyed();
      }

      void SetNeedsDisplay() noexcept final
      {
        NeedsDisplayRects.push_back(FloatRect(FloatPoint(), Size()));
      }

      void SetNeedsDisplayInRect(const FloatRect &rect, ShouldClipToLayer) noexcept final
      {
        NeedsDisplayRects.push_back(rect);
      }
    };

    class CompositingTestFactory final : public GraphicsLayerFactory
    {
    public:
      KRYS_NODISCARD Ref<GraphicsLayer> CreateGraphicsLayer(GraphicsLayerType type,
                                                            GraphicsLayerClient &client) noexcept final
      {
        return AdoptRef(*new CompositingTestLayer(type, client));
      }
    };

    class CompositingChromeClient final : public ChromeClient
    {
    public:
      CompositingTestFactory Factory;
      GraphicsLayer *RootLayer {nullptr};

      void AttachRootGraphicsLayer(LocalFrame &, GraphicsLayer *layer) noexcept final
      {
        RootLayer = layer;
      }

      KRYS_NODISCARD GraphicsLayerFactory *GetGraphicsLayerFactory() const noexcept final
      {
        return const_cast<CompositingTestFactory *>(&Factory);
      }
    };

    /// @brief A document in a 200x100 view of a page whose host composites it.
    struct CompositingFixture : style::tests::ResolverFixture
    {
      CompositingChromeClient *Chrome {nullptr};
      RefPtr<Page> TestPage;

      explicit CompositingFixture(utf8_stringview markup) noexcept
          : ResolverFixture(u8"<!DOCTYPE html><style>body{margin:0}</style>" + utf8_string(markup),
                            style::tests::WithView::No)
      {
        auto chrome = CreateUnique<CompositingChromeClient>();
        Chrome = chrome.get();
        TestPage = Page::Create(krys::move(chrome));
        Frame = ShareRefPtr(&TestPage->MainFrame());
        Frame->CreateView({200, 100});
        Frame->SetDocument(ShareRefPtr<dom::Document>(Document.get()));
        TestPage->UpdateRendering();
      }

      KRYS_NODISCARD RenderLayerCompositor &Compositor() noexcept
      {
        return Frame->View()->GetRenderViewForUpdate()->Compositor();
      }

      KRYS_NODISCARD RenderLayer &LayerOf(utf8_stringview id) noexcept
      {
        auto *layer = Downcast<RenderLayerModelObject>(*ById(id).Renderer()).Layer();
        krys_assert(layer);
        return *layer;
      }

      KRYS_NODISCARD utf8_string LayerTree() noexcept
      {
        return Compositor().LayerTreeAsText();
      }
    };
  }

  TEST_CASE("RenderLayerCompositor: the root is composited when the host composites",
            "[Rendering][RenderLayerCompositor]")
  {
    CompositingFixture fixture(u8"<div style='width:50px;height:50px;background:green'></div>");
    CHECK(fixture.Compositor().UsesCompositing());
    CHECK(fixture.Chrome->RootLayer);
    CHECK(fixture.Frame->View()->GetRenderView()->Layer()->IsComposited());
    CHECK(fixture.LayerTree()
          == u8"(GraphicsLayer\n  (anchor 0.00 0.00)\n  (bounds 200.00 100.00)\n  (children 1\n    "
             u8"(GraphicsLayer\n"
             u8"      (bounds 200.00 100.00)\n      (contentsOpaque 1)\n    )\n  )\n)\n");
  }

  TEST_CASE("RenderLayerCompositor: filters and 3D transforms composite",
            "[Rendering][RenderLayerCompositor]")
  {
    CompositingFixture fixture(u8"<style>div{width:30px;height:30px;margin-bottom:30px}</style>"
                               u8"<div id=blurred style='background:green;filter:blur(2px)'></div>"
                               u8"<div id=flat style='background:red;transform:rotate(10deg)'></div>"
                               u8"<div id=turned style='background:blue;transform:rotateY(10deg)'></div>");
    CHECK(fixture.LayerOf(u8"blurred").IsComposited());
    CHECK_FALSE(fixture.LayerOf(u8"flat").IsComposited());
    CHECK(fixture.LayerOf(u8"turned").IsComposited());
  }

  TEST_CASE("RenderLayerCompositor: layers above a composited layer composite for overlap",
            "[Rendering][RenderLayerCompositor]")
  {
    CompositingFixture fixture(
      u8"<div id=blurred "
      u8"style='position:absolute;left:0;top:0;width:50px;height:50px;filter:blur(2px)'></div>"
      u8"<div id=above "
      u8"style='position:absolute;left:20px;top:20px;width:50px;height:50px;background:red'></div>"
      u8"<div id=apart "
      u8"style='position:absolute;left:120px;top:0;width:50px;height:50px;background:blue'></div>");
    CHECK(fixture.LayerOf(u8"blurred").IsComposited());
    CHECK(fixture.LayerOf(u8"above").IsComposited());
    CHECK(fixture.LayerOf(u8"above").GetIndirectCompositingReason() == IndirectCompositingReason::Overlap);
    CHECK_FALSE(fixture.LayerOf(u8"apart").IsComposited());
  }

  TEST_CASE("RenderLayerCompositor: repaints go to the composited layer's backing",
            "[Rendering][RenderLayerCompositor]")
  {
    CompositingFixture fixture(
      u8"<div id=blurred "
      u8"style='position:absolute;left:10px;top:10px;width:50px;height:50px;filter:blur(2px)'>"
      u8"<div id=inner style='width:10px;height:10px;background:red'></div></div>");
    auto &backing = *fixture.LayerOf(u8"blurred").Backing();
    auto &graphicsLayer = static_cast<CompositingTestLayer &>(*backing.GetGraphicsLayer());
    graphicsLayer.NeedsDisplayRects.clear();

    fixture.ById(u8"inner").SetAttribute(u8"style", u8"width:10px;height:10px;background:green");
    fixture.TestPage->UpdateRendering();
    CHECK_FALSE(graphicsLayer.NeedsDisplayRects.empty());
  }
}
