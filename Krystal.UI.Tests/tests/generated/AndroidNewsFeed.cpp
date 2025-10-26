#include "../utils/TestUtil.hpp"
#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI;

  TEST_CASE("LayoutEngine::android_news_feed", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetAlignContent(root, Align::Stretch);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 1'080);

    NodeRef root_child0 = NodeCreate(config);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetAlignContent(root_child0_child0, Align::Stretch);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetAlignContent(root_child0_child0_child0, Align::Stretch);
    NodeInsertChild(root_child0_child0, root_child0_child0_child0, 0);

    NodeRef root_child0_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0_child0_child0_child0, FlexDirection::Row);
    NodeStyleSetAlignContent(root_child0_child0_child0_child0, Align::Stretch);
    NodeStyleSetAlignItems(root_child0_child0_child0_child0, Align::FlexStart);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Start, 36);
    NodeStyleSetMargin(root_child0_child0_child0_child0, Edge::Top, 24);
    NodeInsertChild(root_child0_child0_child0, root_child0_child0_child0_child0, 0);

    NodeRef root_child0_child0_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0_child0_child0_child0_child0, FlexDirection::Row);
    NodeStyleSetAlignContent(root_child0_child0_child0_child0_child0, Align::Stretch);
    NodeInsertChild(root_child0_child0_child0_child0, root_child0_child0_child0_child0_child0, 0);

    NodeRef root_child0_child0_child0_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetAlignContent(root_child0_child0_child0_child0_child0_child0, Align::Stretch);
    NodeStyleSetWidth(root_child0_child0_child0_child0_child0_child0, 120);
    NodeStyleSetHeight(root_child0_child0_child0_child0_child0_child0, 120);
    NodeInsertChild(root_child0_child0_child0_child0_child0, root_child0_child0_child0_child0_child0_child0,
                    0);

    NodeRef root_child0_child0_child0_child0_child1 = NodeCreate(config);
    NodeStyleSetAlignContent(root_child0_child0_child0_child0_child1, Align::Stretch);
    NodeStyleSetFlexShrink(root_child0_child0_child0_child0_child1, 1);
    NodeStyleSetMargin(root_child0_child0_child0_child0_child1, Edge::Right, 36);
    NodeStyleSetPadding(root_child0_child0_child0_child0_child1, Edge::Left, 36);
    NodeStyleSetPadding(root_child0_child0_child0_child0_child1, Edge::Top, 21);
    NodeStyleSetPadding(root_child0_child0_child0_child0_child1, Edge::Right, 36);
    NodeStyleSetPadding(root_child0_child0_child0_child0_child1, Edge::Bottom, 18);
    NodeInsertChild(root_child0_child0_child0_child0, root_child0_child0_child0_child0_child1, 1);

    NodeRef root_child0_child0_child0_child0_child1_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0_child0_child0_child0_child1_child0, FlexDirection::Row);
    NodeStyleSetAlignContent(root_child0_child0_child0_child0_child1_child0, Align::Stretch);
    NodeStyleSetFlexShrink(root_child0_child0_child0_child0_child1_child0, 1);
    NodeInsertChild(root_child0_child0_child0_child0_child1, root_child0_child0_child0_child0_child1_child0,
                    0);

    NodeRef root_child0_child0_child0_child0_child1_child1 = NodeCreate(config);
    NodeStyleSetAlignContent(root_child0_child0_child0_child0_child1_child1, Align::Stretch);
    NodeStyleSetFlexShrink(root_child0_child0_child0_child0_child1_child1, 1);
    NodeInsertChild(root_child0_child0_child0_child0_child1, root_child0_child0_child0_child0_child1_child1,
                    1);

    NodeRef root_child0_child0_child1 = NodeCreate(config);
    NodeStyleSetAlignContent(root_child0_child0_child1, Align::Stretch);
    NodeInsertChild(root_child0_child0, root_child0_child0_child1, 1);

    NodeRef root_child0_child0_child1_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0_child0_child1_child0, FlexDirection::Row);
    NodeStyleSetAlignContent(root_child0_child0_child1_child0, Align::Stretch);
    NodeStyleSetAlignItems(root_child0_child0_child1_child0, Align::FlexStart);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Start, 174);
    NodeStyleSetMargin(root_child0_child0_child1_child0, Edge::Top, 24);
    NodeInsertChild(root_child0_child0_child1, root_child0_child0_child1_child0, 0);

    NodeRef root_child0_child0_child1_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0_child0_child1_child0_child0, FlexDirection::Row);
    NodeStyleSetAlignContent(root_child0_child0_child1_child0_child0, Align::Stretch);
    NodeInsertChild(root_child0_child0_child1_child0, root_child0_child0_child1_child0_child0, 0);

    NodeRef root_child0_child0_child1_child0_child0_child0 = NodeCreate(config);
    NodeStyleSetAlignContent(root_child0_child0_child1_child0_child0_child0, Align::Stretch);
    NodeStyleSetWidth(root_child0_child0_child1_child0_child0_child0, 72);
    NodeStyleSetHeight(root_child0_child0_child1_child0_child0_child0, 72);
    NodeInsertChild(root_child0_child0_child1_child0_child0, root_child0_child0_child1_child0_child0_child0,
                    0);

    NodeRef root_child0_child0_child1_child0_child1 = NodeCreate(config);
    NodeStyleSetAlignContent(root_child0_child0_child1_child0_child1, Align::Stretch);
    NodeStyleSetFlexShrink(root_child0_child0_child1_child0_child1, 1);
    NodeStyleSetMargin(root_child0_child0_child1_child0_child1, Edge::Right, 36);
    NodeStyleSetPadding(root_child0_child0_child1_child0_child1, Edge::Left, 36);
    NodeStyleSetPadding(root_child0_child0_child1_child0_child1, Edge::Top, 21);
    NodeStyleSetPadding(root_child0_child0_child1_child0_child1, Edge::Right, 36);
    NodeStyleSetPadding(root_child0_child0_child1_child0_child1, Edge::Bottom, 18);
    NodeInsertChild(root_child0_child0_child1_child0, root_child0_child0_child1_child0_child1, 1);

    NodeRef root_child0_child0_child1_child0_child1_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0_child0_child1_child0_child1_child0, FlexDirection::Row);
    NodeStyleSetAlignContent(root_child0_child0_child1_child0_child1_child0, Align::Stretch);
    NodeStyleSetFlexShrink(root_child0_child0_child1_child0_child1_child0, 1);
    NodeInsertChild(root_child0_child0_child1_child0_child1, root_child0_child0_child1_child0_child1_child0,
                    0);

    NodeRef root_child0_child0_child1_child0_child1_child1 = NodeCreate(config);
    NodeStyleSetAlignContent(root_child0_child0_child1_child0_child1_child1, Align::Stretch);
    NodeStyleSetFlexShrink(root_child0_child0_child1_child0_child1_child1, 1);
    NodeInsertChild(root_child0_child0_child1_child0_child1, root_child0_child0_child1_child0_child1_child1,
                    1);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(1'080 == NodeLayoutGetWidth(root));
    REQUIRE(240 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(1'080 == NodeLayoutGetWidth(root_child0));
    REQUIRE(240 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(1'080 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(240 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(1'080 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(144 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(36 == NodeLayoutGetLeft(root_child0_child0_child0_child0));
    REQUIRE(24 == NodeLayoutGetTop(root_child0_child0_child0_child0));
    REQUIRE(1'044 == NodeLayoutGetWidth(root_child0_child0_child0_child0));
    REQUIRE(120 == NodeLayoutGetHeight(root_child0_child0_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0_child0_child0));
    REQUIRE(120 == NodeLayoutGetWidth(root_child0_child0_child0_child0_child0));
    REQUIRE(120 == NodeLayoutGetHeight(root_child0_child0_child0_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0_child0_child0_child0));
    REQUIRE(120 == NodeLayoutGetWidth(root_child0_child0_child0_child0_child0_child0));
    REQUIRE(120 == NodeLayoutGetHeight(root_child0_child0_child0_child0_child0_child0));

    REQUIRE(120 == NodeLayoutGetLeft(root_child0_child0_child0_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0_child0_child1));
    REQUIRE(72 == NodeLayoutGetWidth(root_child0_child0_child0_child0_child1));
    REQUIRE(39 == NodeLayoutGetHeight(root_child0_child0_child0_child0_child1));

    REQUIRE(36 == NodeLayoutGetLeft(root_child0_child0_child0_child0_child1_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0_child0_child0_child1_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0_child0_child0_child0_child1_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child0_child0_child0_child1_child0));

    REQUIRE(36 == NodeLayoutGetLeft(root_child0_child0_child0_child0_child1_child1));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0_child0_child0_child1_child1));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0_child0_child0_child0_child1_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child0_child0_child0_child1_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child1));
    REQUIRE(144 == NodeLayoutGetTop(root_child0_child0_child1));
    REQUIRE(1'080 == NodeLayoutGetWidth(root_child0_child0_child1));
    REQUIRE(96 == NodeLayoutGetHeight(root_child0_child0_child1));

    REQUIRE(174 == NodeLayoutGetLeft(root_child0_child0_child1_child0));
    REQUIRE(24 == NodeLayoutGetTop(root_child0_child0_child1_child0));
    REQUIRE(906 == NodeLayoutGetWidth(root_child0_child0_child1_child0));
    REQUIRE(72 == NodeLayoutGetHeight(root_child0_child0_child1_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child1_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child1_child0_child0));
    REQUIRE(72 == NodeLayoutGetWidth(root_child0_child0_child1_child0_child0));
    REQUIRE(72 == NodeLayoutGetHeight(root_child0_child0_child1_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child1_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child1_child0_child0_child0));
    REQUIRE(72 == NodeLayoutGetWidth(root_child0_child0_child1_child0_child0_child0));
    REQUIRE(72 == NodeLayoutGetHeight(root_child0_child0_child1_child0_child0_child0));

    REQUIRE(72 == NodeLayoutGetLeft(root_child0_child0_child1_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child1_child0_child1));
    REQUIRE(72 == NodeLayoutGetWidth(root_child0_child0_child1_child0_child1));
    REQUIRE(39 == NodeLayoutGetHeight(root_child0_child0_child1_child0_child1));

    REQUIRE(36 == NodeLayoutGetLeft(root_child0_child0_child1_child0_child1_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0_child1_child0_child1_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0_child0_child1_child0_child1_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child0_child1_child0_child1_child0));

    REQUIRE(36 == NodeLayoutGetLeft(root_child0_child0_child1_child0_child1_child1));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0_child1_child0_child1_child1));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0_child0_child1_child0_child1_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child0_child1_child0_child1_child1));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(1'080 == NodeLayoutGetWidth(root));
    REQUIRE(240 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(1'080 == NodeLayoutGetWidth(root_child0));
    REQUIRE(240 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(1'080 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(240 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0));
    REQUIRE(1'080 == NodeLayoutGetWidth(root_child0_child0_child0));
    REQUIRE(144 == NodeLayoutGetHeight(root_child0_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0_child0));
    REQUIRE(24 == NodeLayoutGetTop(root_child0_child0_child0_child0));
    REQUIRE(1'044 == NodeLayoutGetWidth(root_child0_child0_child0_child0));
    REQUIRE(120 == NodeLayoutGetHeight(root_child0_child0_child0_child0));

    REQUIRE(924 == NodeLayoutGetLeft(root_child0_child0_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0_child0_child0));
    REQUIRE(120 == NodeLayoutGetWidth(root_child0_child0_child0_child0_child0));
    REQUIRE(120 == NodeLayoutGetHeight(root_child0_child0_child0_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child0_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0_child0_child0_child0));
    REQUIRE(120 == NodeLayoutGetWidth(root_child0_child0_child0_child0_child0_child0));
    REQUIRE(120 == NodeLayoutGetHeight(root_child0_child0_child0_child0_child0_child0));

    REQUIRE(816 == NodeLayoutGetLeft(root_child0_child0_child0_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child0_child0_child1));
    REQUIRE(72 == NodeLayoutGetWidth(root_child0_child0_child0_child0_child1));
    REQUIRE(39 == NodeLayoutGetHeight(root_child0_child0_child0_child0_child1));

    REQUIRE(36 == NodeLayoutGetLeft(root_child0_child0_child0_child0_child1_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0_child0_child0_child1_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0_child0_child0_child0_child1_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child0_child0_child0_child1_child0));

    REQUIRE(36 == NodeLayoutGetLeft(root_child0_child0_child0_child0_child1_child1));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0_child0_child0_child1_child1));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0_child0_child0_child0_child1_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child0_child0_child0_child1_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child1));
    REQUIRE(144 == NodeLayoutGetTop(root_child0_child0_child1));
    REQUIRE(1'080 == NodeLayoutGetWidth(root_child0_child0_child1));
    REQUIRE(96 == NodeLayoutGetHeight(root_child0_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child1_child0));
    REQUIRE(24 == NodeLayoutGetTop(root_child0_child0_child1_child0));
    REQUIRE(906 == NodeLayoutGetWidth(root_child0_child0_child1_child0));
    REQUIRE(72 == NodeLayoutGetHeight(root_child0_child0_child1_child0));

    REQUIRE(834 == NodeLayoutGetLeft(root_child0_child0_child1_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child1_child0_child0));
    REQUIRE(72 == NodeLayoutGetWidth(root_child0_child0_child1_child0_child0));
    REQUIRE(72 == NodeLayoutGetHeight(root_child0_child0_child1_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0_child1_child0_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child1_child0_child0_child0));
    REQUIRE(72 == NodeLayoutGetWidth(root_child0_child0_child1_child0_child0_child0));
    REQUIRE(72 == NodeLayoutGetHeight(root_child0_child0_child1_child0_child0_child0));

    REQUIRE(726 == NodeLayoutGetLeft(root_child0_child0_child1_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0_child1_child0_child1));
    REQUIRE(72 == NodeLayoutGetWidth(root_child0_child0_child1_child0_child1));
    REQUIRE(39 == NodeLayoutGetHeight(root_child0_child0_child1_child0_child1));

    REQUIRE(36 == NodeLayoutGetLeft(root_child0_child0_child1_child0_child1_child0));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0_child1_child0_child1_child0));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0_child0_child1_child0_child1_child0));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child0_child1_child0_child1_child0));

    REQUIRE(36 == NodeLayoutGetLeft(root_child0_child0_child1_child0_child1_child1));
    REQUIRE(21 == NodeLayoutGetTop(root_child0_child0_child1_child0_child1_child1));
    REQUIRE(0 == NodeLayoutGetWidth(root_child0_child0_child1_child0_child1_child1));
    REQUIRE(0 == NodeLayoutGetHeight(root_child0_child0_child1_child0_child1_child1));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }
}
