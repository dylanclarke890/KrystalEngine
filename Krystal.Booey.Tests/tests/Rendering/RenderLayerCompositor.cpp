#include "Krystal.Booey/Rendering/RenderLayerCompositor.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/Animation/CSSAnimation.hpp"
#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerFactory.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerKeyframeValueList.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
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
    using namespace std::chrono_literals;

    /// @brief An animation a layer runs.
    struct LayerAnimation
    {
      utf8_string Name;
      AnimatedProperty Property;
      bool Paused {false};
    };

    /// @brief A layer that records what it's asked to display and the animations it runs: transform and
    /// opacity animations, as Krystal.UI's layers run.
    class CompositingTestLayer final : public GraphicsLayer
    {
    public:
      List<FloatRect> NeedsDisplayRects;
      List<LayerAnimation> Animations;

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

      bool AddAnimation(const GraphicsLayerKeyframeValueList &values, const GraphicsLayerAnimation *,
                        const utf8_string &name, double) noexcept final
      {
        if (values.Property() != AnimatedProperty::Transform
            && values.Property() != AnimatedProperty::Opacity)
        {
          return false;
        }
        std::erase_if(Animations, [&](const auto &animation)
                      { return animation.Name == name && animation.Property == values.Property(); });
        Animations.push_back({name, values.Property()});
        return true;
      }

      void PauseAnimation(const utf8_string &name, double) noexcept final
      {
        for (auto &animation : Animations)
        {
          if (animation.Name == name)
          {
            animation.Paused = true;
          }
        }
      }

      void RemoveAnimation(const utf8_string &name, Maybe<AnimatedProperty>) noexcept final
      {
        std::erase_if(Animations, [&](const auto &animation) { return animation.Name == name; });
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

      KRYS_NODISCARD CompositingTestLayer &GraphicsLayerOf(utf8_stringview id) noexcept
      {
        auto &layer = LayerOf(id);
        REQUIRE(layer.Backing());
        return static_cast<CompositingTestLayer &>(*layer.Backing()->GetGraphicsLayer());
      }

      KRYS_NODISCARD KeyframeEffect &OnlyEffect(utf8_stringview id) noexcept
      {
        auto &animations = ById(id).AnimationsCreatedByMarkup(null);
        REQUIRE(animations.size() == 1);
        auto *effect = animations.first()->GetKeyframeEffect();
        REQUIRE(effect);
        return *effect;
      }

      /// @brief Runs the event loop @p delta after its last run, then the rendering update if one is
      /// scheduled.
      /// @return Whether there was a rendering update.
      bool RunFrame(MonotonicTime::duration delta = {}) noexcept
      {
        auto &loop = html::EventLoop::Current();
        loop.Run(loop.Now() + delta);
        if (!TestPage->IsRenderingUpdateScheduled())
        {
          return false;
        }
        TestPage->UpdateRendering();
        return true;
      }

      /// @brief Runs frames until the animations created by the markup are ready and started on layers.
      void StartAnimations() noexcept
      {
        (void)RunFrame();
        (void)RunFrame();
        (void)RunFrame();
      }
    };

    /// @brief `#box`, 20px square, with @p animation as its `animation`: `fade` (opacity), `spin` (transform)
    /// or `grow` (width).
    KRYS_NODISCARD utf8_string AnimatedBox(utf8_stringview animation) noexcept
    {
      utf8_string markup = u8"<style>@keyframes fade{from{opacity:0}to{opacity:1}}"
                           u8"@keyframes spin{from{transform:rotate(0)}to{transform:rotate(90deg)}}"
                           u8"@keyframes grow{from{width:20px}to{width:40px}}"
                           u8"#box{width:20px;height:20px;background:green;animation:";
      markup += animation;
      markup += u8"}</style><div id=box></div>";
      return markup;
    }
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

  TEST_CASE("RenderLayerCompositor: transform and opacity animations run on layers",
            "[Rendering][RenderLayerCompositor]")
  {
    SECTION("opacity")
    {
      CompositingFixture fixture(AnimatedBox(u8"fade 1s linear infinite"));
      fixture.StartAnimations();
      CHECK(fixture.LayerOf(u8"box").IsComposited());
      CHECK(fixture.OnlyEffect(u8"box").IsRunningAccelerated());
      auto &layer = fixture.GraphicsLayerOf(u8"box");
      REQUIRE(layer.Animations.size() == 1);
      CHECK(layer.Animations[0].Name == u8"fade");
      CHECK(layer.Animations[0].Property == AnimatedProperty::Opacity);

      // The layer animates: the page's style isn't updated each frame, but its animated style is.
      auto opacity = fixture.ById(u8"box").Renderer()->Style().GetOpacity().Value.Value;
      CHECK_FALSE(fixture.RunFrame(250ms));
      CHECK(fixture.ById(u8"box").Renderer()->Style().GetOpacity().Value.Value == opacity);
      CHECK(fixture.ById(u8"box").Renderer()->AnimatedStyle()->GetOpacity().Value.Value
            == Catch::Approx(opacity + 0.25f).margin(0.02f));
    }

    SECTION("transform")
    {
      CompositingFixture fixture(AnimatedBox(u8"spin 1s linear infinite"));
      fixture.StartAnimations();
      CHECK(fixture.LayerOf(u8"box").IsComposited());
      auto &layer = fixture.GraphicsLayerOf(u8"box");
      REQUIRE(layer.Animations.size() == 1);
      CHECK(layer.Animations[0].Property == AnimatedProperty::Transform);
    }
  }

  TEST_CASE("RenderLayerCompositor: pausing and ending an animation reach its layer",
            "[Rendering][RenderLayerCompositor]")
  {
    CompositingFixture fixture(AnimatedBox(u8"fade 1s linear infinite"));
    fixture.StartAnimations();
    auto &layer = fixture.GraphicsLayerOf(u8"box");
    REQUIRE(layer.Animations.size() == 1);

    fixture.ById(u8"box").SetAttribute(u8"style", u8"animation-play-state: paused");
    (void)fixture.RunFrame(16ms);
    (void)fixture.RunFrame(16ms);
    REQUIRE(layer.Animations.size() == 1);
    CHECK(layer.Animations[0].Paused);

    fixture.ById(u8"box").SetAttribute(u8"style", u8"animation: none");
    (void)fixture.RunFrame(16ms);
    (void)fixture.RunFrame(16ms);
    CHECK(layer.Animations.empty());
  }

  TEST_CASE("RenderLayerCompositor: animations that can't run on layers run on the page",
            "[Rendering][RenderLayerCompositor]")
  {
    SECTION("a steps() timing function")
    {
      CompositingFixture fixture(AnimatedBox(u8"fade 1s steps(4) infinite"));
      fixture.StartAnimations();
      CHECK_FALSE(fixture.OnlyEffect(u8"box").IsRunningAccelerated());
      CHECK(fixture.RunFrame(300ms));
      CHECK(fixture.ById(u8"box").Renderer()->Style().GetOpacity().Value.Value > 0);
    }

    SECTION("a property layers don't animate")
    {
      CompositingFixture fixture(AnimatedBox(u8"grow 1s linear infinite"));
      fixture.StartAnimations();
      CHECK_FALSE(fixture.OnlyEffect(u8"box").IsRunningAccelerated());
      CHECK_FALSE(fixture.ById(u8"box").Renderer()->IsComposited());
      CHECK(fixture.RunFrame(500ms));
    }
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
