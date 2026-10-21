#include "Krystal.Booey/Graphics/GraphicsLayer.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterOperation.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerAnimation.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerFactory.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerFilterAnimationValue.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerFloatAnimationValue.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerKeyframeValueList.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerTransform.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerTransformAnimationValue.hpp"
#include "Krystal.Booey/Graphics/NullGraphicsContext.hpp"
#include "Krystal.Booey/Graphics/Transforms/RotateTransformOperation.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief A layer that only counts what it's asked to display.
    class TestGraphicsLayer final : public GraphicsLayer
    {
    public:
      uint32 NeedsDisplayCount {0};
      List<FloatRect> NeedsDisplayRects;

      TestGraphicsLayer(Type type, GraphicsLayerClient &client) noexcept : GraphicsLayer(type, client)
      {
      }

      ~TestGraphicsLayer() noexcept final
      {
        WillBeDestroyed();
      }

      void SetNeedsDisplay() noexcept final
      {
        ++NeedsDisplayCount;
      }

      void SetNeedsDisplayInRect(const FloatRect &rect, ShouldClipToLayer) noexcept final
      {
        NeedsDisplayRects.push_back(rect);
      }

      using GraphicsLayer::ValidateFilterOperations;
    };

    class TestFactory final : public GraphicsLayerFactory
    {
    public:
      KRYS_NODISCARD Ref<GraphicsLayer> CreateGraphicsLayer(GraphicsLayerType type,
                                                            GraphicsLayerClient &client) noexcept final
      {
        return AdoptRef(*new TestGraphicsLayer(type, client));
      }
    };

    class TestClient final : public GraphicsLayerClient
    {
    public:
      FloatRect PaintedClip;

      void PaintContents(const GraphicsLayer &, GraphicsContext &, const FloatRect &clip,
                         EnumSet<GraphicsLayerPaintBehavior>) noexcept final
      {
        PaintedClip = clip;
      }
    };

    KRYS_NODISCARD Ref<TestGraphicsLayer>
      CreateLayer(GraphicsLayerClient &client, utf8_stringview name = u8"",
                  GraphicsLayerType type = GraphicsLayerType::Normal) noexcept
    {
      static TestFactory factory;
      Ref<GraphicsLayer> layer = GraphicsLayer::Create(&factory, client, type);
      layer->SetName(utf8_string(name));
      return ShareRef(Downcast<TestGraphicsLayer>(*layer));
    }

    KRYS_NODISCARD utf8_string Names(const GraphicsLayer &layer) noexcept
    {
      utf8_string names;
      for (const auto &child : layer.Children())
      {
        names += child->Name();
      }
      return names;
    }
  }
}

KRYS_SPECIALIZE_TYPE_TRAITS_GRAPHICSLAYER(krys::boo::tests::TestGraphicsLayer, IsMaskLayer() || true)

namespace krys::boo::tests
{
  TEST_CASE("GraphicsLayer: children, masks and replicas", "[Graphics][GraphicsLayer]")
  {
    TestClient client;
    auto root = CreateLayer(client, u8"r");
    auto a = CreateLayer(client, u8"a");
    auto b = CreateLayer(client, u8"b");
    auto c = CreateLayer(client, u8"c");
    auto d = CreateLayer(client, u8"d");

    root->AddChild(Ref<GraphicsLayer> {a});
    root->AddChild(Ref<GraphicsLayer> {c});
    root->AddChildAbove(Ref<GraphicsLayer> {b}, a.get());
    CHECK(Names(*root) == u8"abc");
    root->AddChildBelow(Ref<GraphicsLayer> {d}, a.get());
    CHECK(Names(*root) == u8"dabc");
    CHECK(d->Parent() == root.get());
    CHECK(d->HasAncestor(root.get()));
    CHECK_FALSE(root->HasAncestor(d.get()));

    // Adding a child that has a parent moves it.
    a->AddChild(Ref<GraphicsLayer> {d});
    CHECK(Names(*root) == u8"abc");
    CHECK(Names(*a) == u8"d");

    auto e = CreateLayer(client, u8"e");
    CHECK(root->ReplaceChild(b.get(), Ref<GraphicsLayer> {e}));
    CHECK(Names(*root) == u8"aec");
    CHECK_FALSE(b->Parent());

    root->AddChildAtIndex(Ref<GraphicsLayer> {b}, 0);
    CHECK(Names(*root) == u8"baec");

    c->RemoveFromParent();
    CHECK(Names(*root) == u8"bae");
    CHECK_FALSE(root->SetChildren({Ref<GraphicsLayer> {b}, Ref<GraphicsLayer> {a}, Ref<GraphicsLayer> {e}}));
    CHECK(root->SetChildren({Ref<GraphicsLayer> {e}}));
    CHECK(Names(*root) == u8"e");
    CHECK_FALSE(a->Parent());

    auto mask = CreateLayer(client, u8"m");
    root->SetMaskLayer(Ref<GraphicsLayer> {mask});
    CHECK(mask->IsMaskLayer());
    CHECK(mask->Parent() == root.get());
    root->SetMaskLayer(nullptr);
    CHECK_FALSE(mask->IsMaskLayer());
    CHECK_FALSE(mask->Parent());

    auto replica = CreateLayer(client, u8"p");
    root->SetReplicatedByLayer(Ref<GraphicsLayer> {replica});
    CHECK(root->IsReplicated());
    CHECK(root->ReplicaLayer() == replica.get());

    root->SetMaskLayer(Ref<GraphicsLayer> {mask});
    utf8_string order;
    GraphicsLayer::Traverse(*root, [&](GraphicsLayer &layer) { order += layer.Name(); });
    // Children, then the replica, then the mask.
    CHECK(order == u8"repm");

    RefPtr<GraphicsLayer> cleared = e;
    GraphicsLayer::UnparentAndClear(cleared);
    CHECK_FALSE(cleared);
    CHECK(root->Children().empty());
  }

  TEST_CASE("GraphicsLayer: changes that need display", "[Graphics][GraphicsLayer]")
  {
    TestClient client;
    auto layer = CreateLayer(client);

    // Layers that don't draw content needn't repaint when they're resized.
    layer->SetSize({10, 10});
    CHECK(layer->NeedsDisplayCount == 0u);
    layer->SetDrawsContent(true);
    layer->SetSize({20, 10});
    CHECK(layer->NeedsDisplayCount == 1u);
    layer->SetSize({20, 10});
    CHECK(layer->NeedsDisplayCount == 1u);

    layer->SetOffsetFromRenderer({5, 5});
    CHECK(layer->NeedsDisplayCount == 2u);
    layer->SetOffsetFromRenderer({6, 6}, GraphicsLayer::ShouldSetNeedsDisplay::DoNotSet);
    CHECK(layer->NeedsDisplayCount == 2u);

    layer->SetPaintingPhase({GraphicsLayerPaintingPhase::Foreground});
    CHECK(layer->NeedsDisplayCount == 3u);

    layer->SetScrollOffset({0, 4});
    CHECK(layer->NeedsDisplayCount == 4u);
  }

  TEST_CASE("GraphicsLayer: contents paint at the renderer's origin", "[Graphics][GraphicsLayer]")
  {
    TestClient client;
    auto layer = CreateLayer(client);
    layer->SetOffsetFromRenderer({10, 20}, GraphicsLayer::ShouldSetNeedsDisplay::DoNotSet);
    layer->SetScrollOffset({0, 5}, GraphicsLayer::ShouldSetNeedsDisplay::DoNotSet);

    NullGraphicsContext context;
    layer->PaintGraphicsLayerContents(context, {0, 0, 50, 40});
    // The clip moves by the offset from the renderer, less the scroll offset.
    CHECK(client.PaintedClip == FloatRect {10, 15, 50, 40});
  }

  TEST_CASE("GraphicsLayer: layer tree as text", "[Graphics][GraphicsLayer]")
  {
    TestClient client;
    auto root = CreateLayer(client);
    root->SetSize({800, 600});
    root->SetMasksToBounds(true);
    root->SetPaintingPhase({GraphicsLayerPaintingPhase::Background, GraphicsLayerPaintingPhase::Foreground});

    auto child = CreateLayer(client);
    child->SetPosition({8, 16});
    child->SetSize({100, 50});
    child->SetOpacity(0.5f);
    child->SetBlendMode(BlendMode::Multiply);
    child->SetDrawsContent(true);
    child->SetBackgroundColor(Color {SRGBA<uint8> {255, 0, 0}});
    TransformationMatrix transform;
    transform.Translate(2, 3);
    child->SetTransform(transform);
    root->AddChild(Ref<GraphicsLayer> {child});

    auto mask = CreateLayer(client);
    mask->SetSize({100, 50});
    child->SetMaskLayer(Ref<GraphicsLayer> {mask});

    CHECK(root->LayerTreeAsText({LayerTreeAsTextOptions::IncludeClipping}) == u8R"((GraphicsLayer
  (anchor 0.50 0.50)
  (bounds 800.00 600.00)
  (clips 1)
  (children 1
    (GraphicsLayer
      (position 8.00 16.00)
      (anchor 0.50 0.50)
      (bounds 100.00 50.00)
      (opacity 0.50)
      (blendMode multiply)
      (drawsContent 1)
      (backgroundColor #FF0000)
      (transform [1.00 0.00 0.00 0.00] [0.00 1.00 0.00 0.00] [0.00 0.00 1.00 0.00] [2.00 3.00 0.00 1.00])
      (mask layer)
        (GraphicsLayer
          (anchor 0.50 0.50)
          (bounds 100.00 50.00)
        )
    )
  )
)
)");

    CHECK(root->LayerTreeAsText({LayerTreeAsTextOptions::IncludePaintingPhases})
            .find(u8"(paintingPhases [background, foreground])")
          != utf8_string::npos);
  }

  TEST_CASE("GraphicsLayer: coverage grows in the direction it moves", "[Graphics][GraphicsLayer]")
  {
    // No previous visible rect: just the current one.
    CHECK(GraphicsLayer::AdjustCoverageRectForMovement({}, {}, {0, 0, 100, 100})
          == FloatRect {0, 0, 100, 100});

    // Moving down by 10 pads the bottom by twice that.
    CHECK(GraphicsLayer::AdjustCoverageRectForMovement({}, {0, 0, 100, 100}, {0, 10, 100, 100})
          == FloatRect {0, 10, 100, 120});

    // Moving left by 10 pads the left by twice that.
    CHECK(GraphicsLayer::AdjustCoverageRectForMovement({}, {50, 0, 100, 100}, {40, 0, 100, 100})
          == FloatRect {20, 0, 120, 100});
  }

  TEST_CASE("GraphicsLayer: animations", "[Graphics][GraphicsLayer]")
  {
    SECTION("names")
    {
      CHECK(GraphicsLayer::AnimationNameForTransition(AnimatedProperty::Opacity) == u8"-|transition5-");
      CHECK(AnimatedPropertyIDAsString(AnimatedProperty::Rotate) == u8"transform");
      CHECK(AnimatedPropertyIDAsString(AnimatedProperty::WebkitBackdropFilter) == u8"backdrop-filter");
    }

    SECTION("keyframe value lists are sorted by key time and copy their values")
    {
      GraphicsLayerKeyframeValueList list(AnimatedProperty::Opacity);
      list.Insert(CreateUnique<GraphicsLayerFloatAnimationValue>(1, 1.f));
      list.Insert(
        CreateUnique<GraphicsLayerFloatAnimationValue>(0, 0.f, CubicBezierTimingFunction::Create().get()));
      list.Insert(CreateUnique<GraphicsLayerFloatAnimationValue>(0.5, 0.25f));
      REQUIRE(list.Size() == 3u);
      CHECK(list.At(0).KeyTime() == 0);
      CHECK(list.At(1).KeyTime() == 0.5);
      CHECK(list.At(2).KeyTime() == 1);

      GraphicsLayerKeyframeValueList copy(list);
      REQUIRE(copy.Size() == 3u);
      CHECK(&copy.At(0) != &list.At(0));
      CHECK(static_cast<const GraphicsLayerFloatAnimationValue &>(copy.At(1)).Value() == 0.25f);
      // Timing functions are cloned.
      REQUIRE(copy.At(0).GetTimingFunction());
      CHECK(copy.At(0).GetTimingFunction() != list.At(0).GetTimingFunction());
      CHECK(*copy.At(0).GetTimingFunction() == *list.At(0).GetTimingFunction());

      GraphicsLayerKeyframeValueList transforms(AnimatedProperty::Transform);
      auto rotate = RotateTransformOperation::Create(45, TransformOperation::Type::Rotate);
      transforms.Insert(CreateUnique<GraphicsLayerTransformAnimationValue>(0, rotate.get()));
      transforms.Insert(CreateUnique<GraphicsLayerTransformAnimationValue>(1, TransformOperations {}));
      GraphicsLayerKeyframeValueList transformsCopy(AnimatedProperty::Invalid);
      transformsCopy = transforms;
      CHECK(transformsCopy.Property() == AnimatedProperty::Transform);
      CHECK(static_cast<const GraphicsLayerTransformAnimationValue &>(transformsCopy.At(0)).Value().size()
            == 1u);
    }

    SECTION("filter keyframes must match, except where empty")
    {
      GraphicsLayerKeyframeValueList list(AnimatedProperty::Filter);
      list.Insert(CreateUnique<GraphicsLayerFilterAnimationValue>(0, FilterOperations {}));
      list.Insert(CreateUnique<GraphicsLayerFilterAnimationValue>(
        0.5, FilterOperations {{BlurFilterOperation::Create(2)}}));
      list.Insert(CreateUnique<GraphicsLayerFilterAnimationValue>(
        1, FilterOperations {{BlurFilterOperation::Create(4)}}));
      CHECK(TestGraphicsLayer::ValidateFilterOperations(list) == 1);

      list.Insert(CreateUnique<GraphicsLayerFilterAnimationValue>(
        2,
        FilterOperations {{BasicColorMatrixFilterOperation::Create(1, FilterOperation::Type::Grayscale)}}));
      CHECK(TestGraphicsLayer::ValidateFilterOperations(list) == -1);
    }

    SECTION("timing")
    {
      auto animation = GraphicsLayerAnimation::Create();
      CHECK(animation->IsZeroDuration());
      animation->SetDuration(2);
      animation->SetFillMode(GraphicsLayerAnimation::FillMode::Both);
      animation->SetDirection(GraphicsLayerAnimation::Direction::AlternateReverse);
      CHECK_FALSE(animation->IsZeroDuration());
      CHECK(animation->FillsBackwards());
      CHECK(animation->FillsForwards());
      CHECK_FALSE(animation->DirectionIsForwards());

      auto copy = GraphicsLayerAnimation::Create(*animation);
      CHECK(*copy == *animation);
      copy->SetDelay(1);
      CHECK_FALSE(*copy == *animation);

      TextStream ts(TextStream::LineMode::SingleLine);
      ts << *animation;
      CHECK(
        ts.Release()
        == u8" (delay 0.00) (direction alternate-reverse) (duration 2.00) (fill-mode both) (iteration count "
           u8"1.00) (play-state running) (timing function cubic-bezier(0.25, 0.1, 0.25, 1))");
    }
  }

  TEST_CASE("GraphicsLayerTransform: combines position, anchor point and transforms",
            "[Graphics][GraphicsLayer]")
  {
    GraphicsLayerTransform transform;
    transform.SetPosition({10, 20});
    transform.SetSize({100, 50});
    TransformationMatrix scale;
    scale.Scale(2);
    transform.SetLocalTransform(scale);
    transform.CombineTransforms({});

    // Scaled about the centre (50, 25), then moved to (10, 20).
    CHECK(transform.Combined().MapPoint(FloatPoint {0, 0}) == FloatPoint {-40, -5});
    CHECK(transform.Combined().MapPoint(FloatPoint {50, 25}) == FloatPoint {60, 45});

    // Children are placed in the layer's coordinates, transformed with it.
    CHECK(transform.CombinedForChildren().MapPoint(FloatPoint {0, 0}) == FloatPoint {-40, -5});
  }
}
