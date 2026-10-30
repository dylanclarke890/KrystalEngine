#include "Krystal.Booey/Rendering/LayerOverlapMap.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/Rendering/LayerAncestorClippingStack.hpp"
#include "Krystal.Booey/Rendering/RenderGeometryMap.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"

namespace krys::boo::tests
{
  namespace
  {
    struct LayersFixture : PaintFixture
    {
      explicit LayersFixture(utf8_stringview markup) noexcept
          : PaintFixture(u8"<!DOCTYPE html><style>body{margin:0}div{position:absolute}</style>"
                         + utf8_string(markup))
      {
        (void)Document->UpdateLayout();
      }

      KRYS_NODISCARD RenderLayer &LayerOf(utf8_stringview id) noexcept
      {
        auto *layer = Downcast<RenderLayerModelObject>(*ById(id).Renderer()).Layer();
        krys_assert(layer);
        return *layer;
      }

      KRYS_NODISCARD RenderLayer &ViewLayer() noexcept
      {
        return *Frame->View()->GetRenderView()->Layer();
      }
    };
  }

  TEST_CASE("RenderGeometryMap: maps as the renderers do", "[Rendering][RenderGeometryMap]")
  {
    LayersFixture fixture(
      u8"<div id=outer style='left:10px;top:20px;width:100px;height:100px'>"
      u8"<div id=inner style='left:5px;top:6px;width:10px;height:10px'></div></div>"
      u8"<div id=turned style='left:50px;top:50px;width:20px;height:20px;transform:rotate(90deg)'>"
      u8"<div id=inTurned style='left:2px;top:0;width:4px;height:4px'></div></div>");

    for (auto id : {u8"inner", u8"inTurned"})
    {
      auto &layer = fixture.LayerOf(id);
      RenderGeometryMap geometryMap;
      geometryMap.PushMappingsToAncestor(&layer, nullptr);
      auto expected = layer.Renderer().LocalToAbsoluteQuad(FloatQuad(FloatRect(0, 0, 10, 10))).BoundingBox();
      auto mapped = geometryMap.MapToContainer(FloatRect(0, 0, 10, 10), nullptr).BoundingBox();
      INFO(reinterpret_cast<const char *>(id));
      CHECK(AreEssentiallyEqual(mapped.Location(), expected.Location()));
      CHECK(AreEssentiallyEqual(FloatPoint(mapped.MaxX(), mapped.MaxY()),
                                FloatPoint(expected.MaxX(), expected.MaxY())));
      geometryMap.PopMappingsToAncestor(static_cast<const RenderLayer *>(nullptr));
    }

    // Through the layers: offsets alone.
    auto &inner = fixture.LayerOf(u8"inner");
    RenderGeometryMap geometryMap;
    geometryMap.PushMappingsToAncestor(&fixture.LayerOf(u8"outer"), nullptr);
    geometryMap.PushMappingsToAncestor(&inner, &fixture.LayerOf(u8"outer"));
    CHECK(geometryMap.MapToContainer(FloatPoint(1, 1), nullptr) == FloatPoint(16, 27));
  }

  TEST_CASE("LayerOverlapMap: layers overlap what's composited before them", "[Rendering][LayerOverlapMap]")
  {
    LayersFixture fixture(u8"<div id=a style='left:0;top:0;width:50px;height:50px'></div>"
                          u8"<div id=b style='left:100px;top:0;width:50px;height:50px'></div>");
    auto &view = fixture.ViewLayer();
    auto &a = fixture.LayerOf(u8"a");
    auto &b = fixture.LayerOf(u8"b");
    LayerOverlapMap::LayerAndBoundsVector clipping {{&view, LayoutRect(0, 0, 200, 100)}};

    LayerOverlapMap overlapMap(view);
    CHECK(overlapMap.IsEmpty());

    // A layer contributes to overlap once its composited container is popped.
    overlapMap.PushCompositingContainer(a);
    overlapMap.Add(a, LayoutRect(0, 0, 50, 50), clipping);
    CHECK_FALSE(overlapMap.IsEmpty());
    overlapMap.PopCompositingContainer(a);

    CHECK(overlapMap.OverlapsLayers(b, LayoutRect(40, 40, 20, 20), clipping));
    CHECK_FALSE(overlapMap.OverlapsLayers(b, LayoutRect(100, 0, 50, 50), clipping));

    // A speculative container that isn't needed is thrown away with what was added to it.
    overlapMap.PushSpeculativeCompositingContainer(b);
    CHECK(overlapMap.MaybePopSpeculativeCompositingContainer());
    CHECK(overlapMap.OverlapsLayers(b, LayoutRect(40, 40, 20, 20), clipping));
  }

  TEST_CASE("LayerAncestorClippingStack: compares and updates its clips",
            "[Rendering][LayerAncestorClippingStack]")
  {
    LayersFixture fixture(u8"<div id=a style='left:0;top:0;width:50px;height:50px;overflow:hidden'></div>");
    auto &a = fixture.LayerOf(u8"a");
    List<CompositedClipData> clips {{&a, LayoutRoundedRect(LayoutRect(0, 0, 50, 50)), false}};

    LayerAncestorClippingStack stack {List<CompositedClipData>(clips)};
    CHECK(stack.EqualToClipData(clips));
    CHECK_FALSE(stack.HasAnyScrollingLayers());
    CHECK_FALSE(stack.UpdateWithClipData(List<CompositedClipData>(clips)));

    clips.push_back({&a, LayoutRoundedRect(LayoutRect(10, 10, 20, 20)), true});
    CHECK(stack.UpdateWithClipData(List<CompositedClipData>(clips)));
    CHECK(stack.HasAnyScrollingLayers());
    CHECK(stack.Stack().size() == 2u);

    clips.pop_back();
    CHECK(stack.UpdateWithClipData(List<CompositedClipData>(clips)));
    CHECK(stack.Stack().size() == 1u);
    CHECK(stack.GetCompositedClipData().size() == 1u);
  }
}
