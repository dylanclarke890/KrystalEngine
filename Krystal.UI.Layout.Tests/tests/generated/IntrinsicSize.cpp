#include "../utils/TestUtil.hpp"
#include "Krystal.UI.Layout/LayoutEngine.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::UI::Layout;
  using namespace Krys::UI::Styles;

  TEST_CASE("LayoutEngine::contains_inner_text_long_word", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 2'000);
    NodeStyleSetHeight(root, 2'000);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeInsertChild(root, root_child0, 0);
    NodeSetContext(root_child0, (void *)"LoremipsumdolorsitametconsecteturadipiscingelitSedeleifasdfettorto"
                                        "racauctorFuscerhoncusipsumtemporerosaliquamconsequatPraesentsoda");
    NodeSetMeasureFunc(root_child0, &IntrinsicSizeMeasure);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(2'000 == NodeLayoutGetWidth(root));
    REQUIRE(2'000 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(1'300 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(2'000 == NodeLayoutGetWidth(root));
    REQUIRE(2'000 == NodeLayoutGetHeight(root));

    REQUIRE(700 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(1'300 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::contains_inner_text_no_width_no_height", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 2'000);
    NodeStyleSetHeight(root, 2'000);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeInsertChild(root, root_child0, 0);
    NodeSetContext(
      root_child0,
      (void
         *)"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed eleifasd et tortor ac auctor. "
           "Integer at volutpat libero, sed elementum dui interdum id. Aliquam consectetur massa vel neque "
           "aliquet, quis consequat risus fringilla. Fusce rhoncus ipsum tempor eros aliquam, vel tempus "
           "metus ullamcorper. Nam at nulla sed tellus vestibulum fringilla vel sit amet ligula. Proin velit "
           "lectus, euismod sit amet quam vel ultricies dolor, vitae finibus lorem ipsum. Pellentesque "
           "molestie at mi sit amet dictum. Donec vehicula lacinia felis sit amet consectetur. Praesent "
           "sodales enim sapien, sed varius ipsum pellentesque vel. Aenean eu mi eu justo tincidunt finibus "
           "vel sit amet ipsum. Sed bibasdum purus vel ipsum sagittis, quis fermentum dolor lobortis. Etiam "
           "vulputate eleifasd lectus vel varius. Phasellus imperdiet lectus sit amet ipsum egestas, ut "
           "bibasdum ipsum malesuada. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices "
           "posuere cubilia Curae; Sed mollis eros sit amet elit porttitor, vel venenatis turpis venenatis. "
           "Nulla tempus tortor at eros efficitur, sit amet dapibus ipsum malesuada. Ut at mauris sed nunc "
           "malesuada convallis. Duis id sem vel magna varius eleifasd vel at est. Donec eget orci a ipsum "
           "tempor lobortis. Sed at consectetur ipsum.");
    NodeSetMeasureFunc(root_child0, &IntrinsicSizeMeasure);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(2'000 == NodeLayoutGetWidth(root));
    REQUIRE(2'000 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(2'000 == NodeLayoutGetWidth(root_child0));
    REQUIRE(70 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(2'000 == NodeLayoutGetWidth(root));
    REQUIRE(2'000 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(2'000 == NodeLayoutGetWidth(root_child0));
    REQUIRE(70 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::contains_inner_text_no_width_no_height_long_word_in_paragraph", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 2'000);
    NodeStyleSetHeight(root, 2'000);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeInsertChild(root, root_child0, 0);
    NodeSetContext(
      root_child0,
      (void
         *)"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed eleifasd et tortor ac auctor. "
           "Integer at volutpat libero, sed elementum dui interdum id. Aliquam consectetur massa vel neque "
           "aliquet, quis consequat risus fringilla. Fusce rhoncus ipsum tempor eros aliquam, vel tempus "
           "metus ullamcorper. Nam at nulla sed tellus vestibulum fringilla vel sit amet ligula. Proin velit "
           "lectus, euismod sit amet quam vel ultricies dolor, vitae finibus "
           "loremipsumloremipsumloremipsumloremipsumloremipsumloremipsumloremipsumloremipsumloremipsumloremip"
           "sumloremipsumloremipsumloremipsumlorem Pellentesque molestie at mi sit amet dictum. Donec "
           "vehicula lacinia felis sit amet consectetur. Praesent sodales enim sapien, sed varius ipsum "
           "pellentesque vel. Aenean eu mi eu justo tincidunt finibus vel sit amet ipsum. Sed bibasdum purus "
           "vel ipsum sagittis, quis fermentum dolor lobortis. Etiam vulputate eleifasd lectus vel varius. "
           "Phasellus imperdiet lectus sit amet ipsum egestas, ut bibasdum ipsum malesuada. Vestibulum ante "
           "ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Sed mollis eros sit amet "
           "elit porttitor, vel venenatis turpis venenatis. Nulla tempus tortor at eros efficitur, sit amet "
           "dapibus ipsum malesuada. Ut at mauris sed nunc malesuada convallis. Duis id sem vel magna varius "
           "eleifasd vel at est. Donec eget orci a ipsum tempor lobortis. Sed at consectetur ipsum.");
    NodeSetMeasureFunc(root_child0, &IntrinsicSizeMeasure);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(2'000 == NodeLayoutGetWidth(root));
    REQUIRE(2'000 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(2'000 == NodeLayoutGetWidth(root_child0));
    REQUIRE(70 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(2'000 == NodeLayoutGetWidth(root));
    REQUIRE(2'000 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(2'000 == NodeLayoutGetWidth(root_child0));
    REQUIRE(70 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::contains_inner_text_fixed_width", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 2'000);
    NodeStyleSetHeight(root, 2'000);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetWidth(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);
    NodeSetContext(
      root_child0,
      (void
         *)"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed eleifasd et tortor ac auctor. "
           "Integer at volutpat libero, sed elementum dui interdum id. Aliquam consectetur massa vel neque "
           "aliquet, quis consequat risus fringilla. Fusce rhoncus ipsum tempor eros aliquam, vel tempus "
           "metus ullamcorper. Nam at nulla sed tellus vestibulum fringilla vel sit amet ligula. Proin velit "
           "lectus, euismod sit amet quam vel ultricies dolor, vitae finibus lorem ipsum. Pellentesque "
           "molestie at mi sit amet dictum. Donec vehicula lacinia felis sit amet consectetur. Praesent "
           "sodales enim sapien, sed varius ipsum pellentesque vel. Aenean eu mi eu justo tincidunt finibus "
           "vel sit amet ipsum. Sed bibasdum purus vel ipsum sagittis, quis fermentum dolor lobortis. Etiam "
           "vulputate eleifasd lectus vel varius. Phasellus imperdiet lectus sit amet ipsum egestas, ut "
           "bibasdum ipsum malesuada. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices "
           "posuere cubilia Curae; Sed mollis eros sit amet elit porttitor, vel venenatis turpis venenatis. "
           "Nulla tempus tortor at eros efficitur, sit amet dapibus ipsum malesuada. Ut at mauris sed nunc "
           "malesuada convallis. Duis id sem vel magna varius eleifasd vel at est. Donec eget orci a ipsum "
           "tempor lobortis. Sed at consectetur ipsum.");
    NodeSetMeasureFunc(root_child0, &IntrinsicSizeMeasure);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(2'000 == NodeLayoutGetWidth(root));
    REQUIRE(2'000 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(1'290 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(2'000 == NodeLayoutGetWidth(root));
    REQUIRE(2'000 == NodeLayoutGetHeight(root));

    REQUIRE(1'900 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(1'290 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::contains_inner_text_no_width_fixed_height", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 2'000);
    NodeStyleSetHeight(root, 2'000);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetHeight(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);
    NodeSetContext(
      root_child0,
      (void
         *)"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed eleifasd et tortor ac auctor. "
           "Integer at volutpat libero, sed elementum dui interdum id. Aliquam consectetur massa vel neque "
           "aliquet, quis consequat risus fringilla. Fusce rhoncus ipsum tempor eros aliquam, vel tempus "
           "metus ullamcorper. Nam at nulla sed tellus vestibulum fringilla vel sit amet ligula. Proin velit "
           "lectus, euismod sit amet quam vel ultricies dolor, vitae finibus lorem ipsum. Pellentesque "
           "molestie at mi sit amet dictum. Donec vehicula lacinia felis sit amet consectetur. Praesent "
           "sodales enim sapien, sed varius ipsum pellentesque vel. Aenean eu mi eu justo tincidunt finibus "
           "vel sit amet ipsum. Sed bibasdum purus vel ipsum sagittis, quis fermentum dolor lobortis. Etiam "
           "vulputate eleifasd lectus vel varius. Phasellus imperdiet lectus sit amet ipsum egestas, ut "
           "bibasdum ipsum malesuada. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices "
           "posuere cubilia Curae; Sed mollis eros sit amet elit porttitor, vel venenatis turpis venenatis. "
           "Nulla tempus tortor at eros efficitur, sit amet dapibus ipsum malesuada. Ut at mauris sed nunc "
           "malesuada convallis. Duis id sem vel magna varius eleifasd vel at est. Donec eget orci a ipsum "
           "tempor lobortis. Sed at consectetur ipsum.");
    NodeSetMeasureFunc(root_child0, &IntrinsicSizeMeasure);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(2'000 == NodeLayoutGetWidth(root));
    REQUIRE(2'000 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(2'000 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(2'000 == NodeLayoutGetWidth(root));
    REQUIRE(2'000 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(2'000 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::contains_inner_text_fixed_width_fixed_height", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 2'000);
    NodeStyleSetHeight(root, 2'000);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);
    NodeSetContext(
      root_child0,
      (void
         *)"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed eleifasd et tortor ac auctor. "
           "Integer at volutpat libero, sed elementum dui interdum id. Aliquam consectetur massa vel neque "
           "aliquet, quis consequat risus fringilla. Fusce rhoncus ipsum tempor eros aliquam, vel tempus "
           "metus ullamcorper. Nam at nulla sed tellus vestibulum fringilla vel sit amet ligula. Proin velit "
           "lectus, euismod sit amet quam vel ultricies dolor, vitae finibus lorem ipsum. Pellentesque "
           "molestie at mi sit amet dictum. Donec vehicula lacinia felis sit amet consectetur. Praesent "
           "sodales enim sapien, sed varius ipsum pellentesque vel. Aenean eu mi eu justo tincidunt finibus "
           "vel sit amet ipsum. Sed bibasdum purus vel ipsum sagittis, quis fermentum dolor lobortis. Etiam "
           "vulputate eleifasd lectus vel varius. Phasellus imperdiet lectus sit amet ipsum egestas, ut "
           "bibasdum ipsum malesuada. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices "
           "posuere cubilia Curae; Sed mollis eros sit amet elit porttitor, vel venenatis turpis venenatis. "
           "Nulla tempus tortor at eros efficitur, sit amet dapibus ipsum malesuada. Ut at mauris sed nunc "
           "malesuada convallis. Duis id sem vel magna varius eleifasd vel at est. Donec eget orci a ipsum "
           "tempor lobortis. Sed at consectetur ipsum.");
    NodeSetMeasureFunc(root_child0, &IntrinsicSizeMeasure);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(2'000 == NodeLayoutGetWidth(root));
    REQUIRE(2'000 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(2'000 == NodeLayoutGetWidth(root));
    REQUIRE(2'000 == NodeLayoutGetHeight(root));

    REQUIRE(1'950 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::contains_inner_text_max_width_max_height", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 2'000);
    NodeStyleSetHeight(root, 2'000);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetMaxWidth(root_child0, 50);
    NodeStyleSetMaxHeight(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);
    NodeSetContext(
      root_child0,
      (void
         *)"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed eleifasd et tortor ac auctor. "
           "Integer at volutpat libero, sed elementum dui interdum id. Aliquam consectetur massa vel neque "
           "aliquet, quis consequat risus fringilla. Fusce rhoncus ipsum tempor eros aliquam, vel tempus "
           "metus ullamcorper. Nam at nulla sed tellus vestibulum fringilla vel sit amet ligula. Proin velit "
           "lectus, euismod sit amet quam vel ultricies dolor, vitae finibus lorem ipsum. Pellentesque "
           "molestie at mi sit amet dictum. Donec vehicula lacinia felis sit amet consectetur. Praesent "
           "sodales enim sapien, sed varius ipsum pellentesque vel. Aenean eu mi eu justo tincidunt finibus "
           "vel sit amet ipsum. Sed bibasdum purus vel ipsum sagittis, quis fermentum dolor lobortis. Etiam "
           "vulputate eleifasd lectus vel varius. Phasellus imperdiet lectus sit amet ipsum egestas, ut "
           "bibasdum ipsum malesuada. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices "
           "posuere cubilia Curae; Sed mollis eros sit amet elit porttitor, vel venenatis turpis venenatis. "
           "Nulla tempus tortor at eros efficitur, sit amet dapibus ipsum malesuada. Ut at mauris sed nunc "
           "malesuada convallis. Duis id sem vel magna varius eleifasd vel at est. Donec eget orci a ipsum "
           "tempor lobortis. Sed at consectetur ipsum.");
    NodeSetMeasureFunc(root_child0, &IntrinsicSizeMeasure);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(2'000 == NodeLayoutGetWidth(root));
    REQUIRE(2'000 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(2'000 == NodeLayoutGetWidth(root));
    REQUIRE(2'000 == NodeLayoutGetHeight(root));

    REQUIRE(1'950 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::contains_inner_text_max_width_max_height_column", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 2'000);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetMaxWidth(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);
    NodeSetContext(
      root_child0,
      (void
         *)"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed eleifasd et tortor ac auctor. "
           "Integer at volutpat libero, sed elementum dui interdum id. Aliquam consectetur massa vel neque "
           "aliquet, quis consequat risus fringilla. Fusce rhoncus ipsum tempor eros aliquam, vel tempus "
           "metus ullamcorper. Nam at nulla sed tellus vestibulum fringilla vel sit amet ligula. Proin velit "
           "lectus, euismod sit amet quam vel ultricies dolor, vitae finibus lorem ipsum. Pellentesque "
           "molestie at mi sit amet dictum. Donec vehicula lacinia felis sit amet consectetur. Praesent "
           "sodales enim sapien, sed varius ipsum pellentesque vel. Aenean eu mi eu justo tincidunt finibus "
           "vel sit amet ipsum. Sed bibasdum purus vel ipsum sagittis, quis fermentum dolor lobortis. Etiam "
           "vulputate eleifasd lectus vel varius. Phasellus imperdiet lectus sit amet ipsum egestas, ut "
           "bibasdum ipsum malesuada. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices "
           "posuere cubilia Curae; Sed mollis eros sit amet elit porttitor, vel venenatis turpis venenatis. "
           "Nulla tempus tortor at eros efficitur, sit amet dapibus ipsum malesuada. Ut at mauris sed nunc "
           "malesuada convallis. Duis id sem vel magna varius eleifasd vel at est. Donec eget orci a ipsum "
           "tempor lobortis. Sed at consectetur ipsum.");
    NodeSetMeasureFunc(root_child0, &IntrinsicSizeMeasure);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(2'000 == NodeLayoutGetWidth(root));
    REQUIRE(1'890 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(1'890 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(2'000 == NodeLayoutGetWidth(root));
    REQUIRE(1'890 == NodeLayoutGetHeight(root));

    REQUIRE(1'950 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(1'890 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::contains_inner_text_max_width", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 2'000);
    NodeStyleSetHeight(root, 2'000);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetMaxWidth(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);
    NodeSetContext(
      root_child0,
      (void
         *)"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed eleifasd et tortor ac auctor. "
           "Integer at volutpat libero, sed elementum dui interdum id. Aliquam consectetur massa vel neque "
           "aliquet, quis consequat risus fringilla. Fusce rhoncus ipsum tempor eros aliquam, vel tempus "
           "metus ullamcorper. Nam at nulla sed tellus vestibulum fringilla vel sit amet ligula. Proin velit "
           "lectus, euismod sit amet quam vel ultricies dolor, vitae finibus lorem ipsum. Pellentesque "
           "molestie at mi sit amet dictum. Donec vehicula lacinia felis sit amet consectetur. Praesent "
           "sodales enim sapien, sed varius ipsum pellentesque vel. Aenean eu mi eu justo tincidunt finibus "
           "vel sit amet ipsum. Sed bibasdum purus vel ipsum sagittis, quis fermentum dolor lobortis. Etiam "
           "vulputate eleifasd lectus vel varius. Phasellus imperdiet lectus sit amet ipsum egestas, ut "
           "bibasdum ipsum malesuada. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices "
           "posuere cubilia Curae; Sed mollis eros sit amet elit porttitor, vel venenatis turpis venenatis. "
           "Nulla tempus tortor at eros efficitur, sit amet dapibus ipsum malesuada. Ut at mauris sed nunc "
           "malesuada convallis. Duis id sem vel magna varius eleifasd vel at est. Donec eget orci a ipsum "
           "tempor lobortis. Sed at consectetur ipsum.");
    NodeSetMeasureFunc(root_child0, &IntrinsicSizeMeasure);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(2'000 == NodeLayoutGetWidth(root));
    REQUIRE(2'000 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(1'290 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(2'000 == NodeLayoutGetWidth(root));
    REQUIRE(2'000 == NodeLayoutGetHeight(root));

    REQUIRE(1'900 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(1'290 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::contains_inner_text_fixed_width_shorter_text", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 2'000);
    NodeStyleSetHeight(root, 2'000);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetWidth(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);
    NodeSetContext(root_child0, (void *)"Lorem ipsum");
    NodeSetMeasureFunc(root_child0, &IntrinsicSizeMeasure);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(2'000 == NodeLayoutGetWidth(root));
    REQUIRE(2'000 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(2'000 == NodeLayoutGetWidth(root));
    REQUIRE(2'000 == NodeLayoutGetHeight(root));

    REQUIRE(1'900 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::contains_inner_text_fixed_height_shorter_text", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 2'000);
    NodeStyleSetHeight(root, 2'000);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetHeight(root_child0, 100);
    NodeInsertChild(root, root_child0, 0);
    NodeSetContext(root_child0, (void *)"Lorem ipsum");
    NodeSetMeasureFunc(root_child0, &IntrinsicSizeMeasure);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(2'000 == NodeLayoutGetWidth(root));
    REQUIRE(2'000 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(110 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(2'000 == NodeLayoutGetWidth(root));
    REQUIRE(2'000 == NodeLayoutGetHeight(root));

    REQUIRE(1'890 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(110 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::contains_inner_text_max_height", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetAlignItems(root, Align::FlexStart);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 2'000);
    NodeStyleSetHeight(root, 2'000);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetMaxHeight(root_child0, 20);
    NodeInsertChild(root, root_child0, 0);
    NodeSetContext(
      root_child0,
      (void
         *)"Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed eleifasd et tortor ac auctor. "
           "Integer at volutpat libero, sed elementum dui interdum id. Aliquam consectetur massa vel neque "
           "aliquet, quis consequat risus fringilla. Fusce rhoncus ipsum tempor eros aliquam, vel tempus "
           "metus ullamcorper. Nam at nulla sed tellus vestibulum fringilla vel sit amet ligula. Proin velit "
           "lectus, euismod sit amet quam vel ultricies dolor, vitae finibus lorem ipsum. Pellentesque "
           "molestie at mi sit amet dictum. Donec vehicula lacinia felis sit amet consectetur. Praesent "
           "sodales enim sapien, sed varius ipsum pellentesque vel. Aenean eu mi eu justo tincidunt finibus "
           "vel sit amet ipsum. Sed bibasdum purus vel ipsum sagittis, quis fermentum dolor lobortis. Etiam "
           "vulputate eleifasd lectus vel varius. Phasellus imperdiet lectus sit amet ipsum egestas, ut "
           "bibasdum ipsum malesuada. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices "
           "posuere cubilia Curae; Sed mollis eros sit amet elit porttitor, vel venenatis turpis venenatis. "
           "Nulla tempus tortor at eros efficitur, sit amet dapibus ipsum malesuada. Ut at mauris sed nunc "
           "malesuada convallis. Duis id sem vel magna varius eleifasd vel at est. Donec eget orci a ipsum "
           "tempor lobortis. Sed at consectetur ipsum.");
    NodeSetMeasureFunc(root_child0, &IntrinsicSizeMeasure);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(2'000 == NodeLayoutGetWidth(root));
    REQUIRE(2'000 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(2'000 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(2'000 == NodeLayoutGetWidth(root));
    REQUIRE(2'000 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(2'000 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::max_content_width", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidthMaxContent(root);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 100);
    NodeStyleSetHeight(root_child1, 50);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 25);
    NodeStyleSetHeight(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(175 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child1));

    REQUIRE(150 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(175 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(125 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(25 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::fit_content_width", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 90);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetWidthFitContent(root_child0);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0, 50);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 100);
    NodeStyleSetHeight(root_child0_child1, 50);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 25);
    NodeStyleSetHeight(root_child0_child2, 50);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(90 == NodeLayoutGetWidth(root));
    REQUIRE(150 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(150 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(90 == NodeLayoutGetWidth(root));
    REQUIRE(150 == NodeLayoutGetHeight(root));

    REQUIRE(-10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(150 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(75 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::stretch_width", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 500);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetWidthStretch(root_child0);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0, 50);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 100);
    NodeStyleSetHeight(root_child0_child1, 50);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 25);
    NodeStyleSetHeight(root_child0_child2, 50);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(500 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(150 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(500 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(450 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(350 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(325 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::max_content_height", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetHeightMaxContent(root);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 100);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 50);
    NodeStyleSetHeight(root_child2, 25);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(175 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(150 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(175 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(150 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::fit_content_height", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetHeight(root, 90);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetHeightFitContent(root_child0);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0, 50);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 50);
    NodeStyleSetHeight(root_child0_child1, 100);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 50);
    NodeStyleSetHeight(root_child0_child2, 25);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(90 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(175 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(150 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(90 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(175 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(150 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::stretch_height", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetHeight(root, 500);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetHeightStretch(root_child0);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0, 50);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 50);
    NodeStyleSetHeight(root_child0_child1, 100);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 50);
    NodeStyleSetHeight(root_child0_child2, 25);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(500 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(500 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(150 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(500 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(500 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(150 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::max_content_flex_basis_column", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetFlexBasisMaxContent(root);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 100);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 50);
    NodeStyleSetHeight(root_child2, 25);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(175 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(150 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(175 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(150 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::fit_content_flex_basis_column", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetHeight(root, 90);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetFlexBasisFitContent(root_child0);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0, 50);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 50);
    NodeStyleSetHeight(root_child0_child1, 100);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 50);
    NodeStyleSetHeight(root_child0_child2, 25);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(90 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(175 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(150 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(90 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(175 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(150 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::stretch_flex_basis_column", "[LayoutEngine]")
  {
    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetHeight(root, 500);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0, 50);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 50);
    NodeStyleSetHeight(root_child0_child1, 100);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 50);
    NodeStyleSetHeight(root_child0_child2, 25);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(500 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(175 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(150 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(500 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(175 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(150 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::max_content_flex_basis_row", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetFlexBasisMaxContent(root);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 100);
    NodeStyleSetHeight(root_child1, 500);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 25);
    NodeStyleSetHeight(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(600 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(500 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(550 == NodeLayoutGetTop(root_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(100 == NodeLayoutGetWidth(root));
    REQUIRE(600 == NodeLayoutGetHeight(root));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(500 == NodeLayoutGetHeight(root_child1));

    REQUIRE(75 == NodeLayoutGetLeft(root_child2));
    REQUIRE(550 == NodeLayoutGetTop(root_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::fit_content_flex_basis_row", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 90);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetFlexBasisFitContent(root_child0);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0, 50);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 100);
    NodeStyleSetHeight(root_child0_child1, 50);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 25);
    NodeStyleSetHeight(root_child0_child2, 50);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(90 == NodeLayoutGetWidth(root));
    REQUIRE(150 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(90 == NodeLayoutGetWidth(root_child0));
    REQUIRE(150 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(90 == NodeLayoutGetWidth(root));
    REQUIRE(150 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(90 == NodeLayoutGetWidth(root_child0));
    REQUIRE(150 == NodeLayoutGetHeight(root_child0));

    REQUIRE(40 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(-10 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(65 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::stretch_flex_basis_row", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 500);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0, 50);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 100);
    NodeStyleSetHeight(root_child0_child1, 50);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 25);
    NodeStyleSetHeight(root_child0_child2, 50);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(500 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(150 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(500 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(450 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(350 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(325 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::max_content_max_width", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 200);
    NodeStyleSetMaxWidthMaxContent(root);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 100);
    NodeStyleSetHeight(root_child1, 50);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 25);
    NodeStyleSetHeight(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(175 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child1));

    REQUIRE(150 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(175 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(125 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(25 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::fit_content_max_width", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 90);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetWidth(root_child0, 110);
    NodeStyleSetMaxWidthFitContent(root_child0);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0, 50);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 100);
    NodeStyleSetHeight(root_child0_child1, 50);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 25);
    NodeStyleSetHeight(root_child0_child2, 50);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(90 == NodeLayoutGetWidth(root));
    REQUIRE(150 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(150 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(90 == NodeLayoutGetWidth(root));
    REQUIRE(150 == NodeLayoutGetHeight(root));

    REQUIRE(-10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(150 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(75 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::stretch_max_width", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 500);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetWidth(root_child0, 600);
    NodeStyleSetMaxWidthStretch(root_child0);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0, 50);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 100);
    NodeStyleSetHeight(root_child0_child1, 50);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 25);
    NodeStyleSetHeight(root_child0_child2, 50);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(500 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(150 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(500 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(450 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(350 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(325 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::max_content_min_width", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetWidth(root, 100);
    NodeStyleSetMinWidthMaxContent(root);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 100);
    NodeStyleSetHeight(root_child1, 50);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 25);
    NodeStyleSetHeight(root_child2, 50);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(175 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child1));

    REQUIRE(150 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(175 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(125 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(25 == NodeLayoutGetLeft(root_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::fit_content_min_width", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 90);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetWidth(root_child0, 90);
    NodeStyleSetMinWidthFitContent(root_child0);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0, 50);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 100);
    NodeStyleSetHeight(root_child0_child1, 50);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 25);
    NodeStyleSetHeight(root_child0_child2, 50);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(90 == NodeLayoutGetWidth(root));
    REQUIRE(150 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(150 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(90 == NodeLayoutGetWidth(root));
    REQUIRE(150 == NodeLayoutGetHeight(root));

    REQUIRE(-10 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0));
    REQUIRE(150 == NodeLayoutGetHeight(root_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(75 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(100 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::stretch_min_width", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 500);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0, FlexDirection::Row);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetWidth(root_child0, 400);
    NodeStyleSetMinWidthStretch(root_child0);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0, 50);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 100);
    NodeStyleSetHeight(root_child0_child1, 50);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 25);
    NodeStyleSetHeight(root_child0_child2, 50);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(500 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(150 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(500 == NodeLayoutGetWidth(root));
    REQUIRE(50 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(500 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(450 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(350 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(325 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::max_content_max_height", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetHeight(root, 200);
    NodeStyleSetMaxHeightMaxContent(root);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 100);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 50);
    NodeStyleSetHeight(root_child2, 25);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(175 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(150 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(175 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(150 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::fit_content_max_height", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetHeight(root, 90);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetHeight(root_child0, 110);
    NodeStyleSetMaxHeightFitContent(root_child0);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0, 50);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 50);
    NodeStyleSetHeight(root_child0_child1, 100);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 50);
    NodeStyleSetHeight(root_child0_child2, 25);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(90 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(50 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(100 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(90 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(-50 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(-100 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::stretch_max_height", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetHeight(root, 500);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetHeight(root_child0, 600);
    NodeStyleSetMaxHeightStretch(root_child0);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0, 50);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 50);
    NodeStyleSetHeight(root_child0_child1, 100);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 50);
    NodeStyleSetHeight(root_child0_child2, 25);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(500 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(500 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(150 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(500 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(500 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(150 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::max_content_min_height", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetFlexWrap(root, Wrap::Wrap);
    NodeStyleSetHeight(root, 100);
    NodeStyleSetMinHeightMaxContent(root);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 50);
    NodeStyleSetHeight(root_child0, 50);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child1, 50);
    NodeStyleSetHeight(root_child1, 100);
    NodeInsertChild(root, root_child1, 1);

    NodeRef root_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child2, 50);
    NodeStyleSetHeight(root_child2, 25);
    NodeInsertChild(root, root_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(175 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(150 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(175 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child2));
    REQUIRE(150 == NodeLayoutGetTop(root_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::fit_content_min_height", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetHeight(root, 90);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetHeight(root_child0, 90);
    NodeStyleSetMinHeightFitContent(root_child0);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0, 50);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 50);
    NodeStyleSetHeight(root_child0_child1, 100);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 50);
    NodeStyleSetHeight(root_child0_child2, 25);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(90 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(175 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(150 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(90 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(175 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(150 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::stretch_min_height", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetHeight(root, 500);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetFlexWrap(root_child0, Wrap::Wrap);
    NodeStyleSetHeight(root_child0, 400);
    NodeStyleSetMinHeightStretch(root_child0);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child0, 50);
    NodeStyleSetHeight(root_child0_child0, 50);
    NodeInsertChild(root_child0, root_child0_child0, 0);

    NodeRef root_child0_child1 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child1, 50);
    NodeStyleSetHeight(root_child0_child1, 100);
    NodeInsertChild(root_child0, root_child0_child1, 1);

    NodeRef root_child0_child2 = NodeCreate(config);
    NodeStyleSetWidth(root_child0_child2, 50);
    NodeStyleSetHeight(root_child0_child2, 25);
    NodeInsertChild(root_child0, root_child0_child2, 2);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(500 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(500 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(150 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0_child2));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(50 == NodeLayoutGetWidth(root));
    REQUIRE(500 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0));
    REQUIRE(500 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(50 == NodeLayoutGetHeight(root_child0_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetTop(root_child0_child1));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child1));
    REQUIRE(100 == NodeLayoutGetHeight(root_child0_child1));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child2));
    REQUIRE(150 == NodeLayoutGetTop(root_child0_child2));
    REQUIRE(50 == NodeLayoutGetWidth(root_child0_child2));
    REQUIRE(25 == NodeLayoutGetHeight(root_child0_child2));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::text_max_content_width", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 200);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidthMaxContent(root_child0);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0_child0, FlexDirection::Row);
    NodeInsertChild(root_child0, root_child0_child0, 0);
    NodeSetContext(root_child0_child0, (void *)"Lorem ipsum "
                                               "sdafhasdfkjlasdhlkajsfhasldkfhasdlkahsdflkjasdhflaksdfasdlk"
                                               "jhasdlfjahsdfljkasdhalsdfhas dolor sit amet");
    NodeSetMeasureFunc(root_child0_child0, &IntrinsicSizeMeasure);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(10 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(1'140 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(1'140 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(10 == NodeLayoutGetHeight(root));

    REQUIRE(-940 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(1'140 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(1'140 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::text_stretch_width", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 200);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidthStretch(root_child0);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0_child0, FlexDirection::Row);
    NodeInsertChild(root_child0, root_child0_child0, 0);
    NodeSetContext(root_child0_child0, (void *)"Lorem ipsum dolor sit amet");
    NodeSetMeasureFunc(root_child0_child0, &IntrinsicSizeMeasure);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(20 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(20 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::text_fit_content_width", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 200);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidthFitContent(root_child0);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0_child0, FlexDirection::Row);
    NodeInsertChild(root_child0, root_child0_child0, 0);
    NodeSetContext(root_child0_child0, (void *)"Lorem ipsum "
                                               "sdafhasdfkjlasdhlkajsfhasldkfhasdlkahsdflkjasdhflaksdfasdlk"
                                               "jhasdlfjahsdfljkasdhalsdfhas dolor sit amet");
    NodeSetMeasureFunc(root_child0_child0, &IntrinsicSizeMeasure);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(30 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(870 == NodeLayoutGetWidth(root_child0));
    REQUIRE(30 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(870 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(30 == NodeLayoutGetHeight(root_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(30 == NodeLayoutGetHeight(root));

    REQUIRE(-670 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(870 == NodeLayoutGetWidth(root_child0));
    REQUIRE(30 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(870 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(30 == NodeLayoutGetHeight(root_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::text_max_content_min_width", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 200);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 200);
    NodeStyleSetMinWidthMaxContent(root_child0);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0_child0, FlexDirection::Row);
    NodeInsertChild(root_child0, root_child0_child0, 0);
    NodeSetContext(root_child0_child0, (void *)"Lorem ipsum "
                                               "sdafhasdfkjlasdhlkajsfhasldkfhasdlkahsdflkjasdhflaksdfasdlk"
                                               "jhasdlfjahsdfljkasdhalsdfhas dolor sit amet");
    NodeSetMeasureFunc(root_child0_child0, &IntrinsicSizeMeasure);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(10 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(1'140 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(1'140 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(10 == NodeLayoutGetHeight(root));

    REQUIRE(-940 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(1'140 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(1'140 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::text_stretch_min_width", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 200);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 100);
    NodeStyleSetMinWidthStretch(root_child0);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0_child0, FlexDirection::Row);
    NodeInsertChild(root_child0, root_child0_child0, 0);
    NodeSetContext(root_child0_child0, (void *)"Lorem ipsum dolor sit amet");
    NodeSetMeasureFunc(root_child0_child0, &IntrinsicSizeMeasure);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(20 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(20 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::text_fit_content_min_width", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 200);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 300);
    NodeStyleSetMinWidthFitContent(root_child0);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0_child0, FlexDirection::Row);
    NodeInsertChild(root_child0, root_child0_child0, 0);
    NodeSetContext(root_child0_child0, (void *)"Lorem ipsum "
                                               "sdafhasdfkjlasdhlkajsfhasldkfhasdlkahsdflkjasdhflaksdfasdlk"
                                               "jhasdlfjahsdfljkasdhalsdfhas dolor sit amet");
    NodeSetMeasureFunc(root_child0_child0, &IntrinsicSizeMeasure);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(30 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(870 == NodeLayoutGetWidth(root_child0));
    REQUIRE(30 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(870 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(30 == NodeLayoutGetHeight(root_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(30 == NodeLayoutGetHeight(root));

    REQUIRE(-670 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(870 == NodeLayoutGetWidth(root_child0));
    REQUIRE(30 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(870 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(30 == NodeLayoutGetHeight(root_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::text_max_content_max_width", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 200);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 2'000);
    NodeStyleSetMaxWidthMaxContent(root_child0);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0_child0, FlexDirection::Row);
    NodeInsertChild(root_child0, root_child0_child0, 0);
    NodeSetContext(root_child0_child0, (void *)"Lorem ipsum "
                                               "sdafhasdfkjlasdhlkajsfhasldkfhasdlkahsdflkjasdhflaksdfasdlk"
                                               "jhasdlfjahsdfljkasdhalsdfhas dolor sit amet");
    NodeSetMeasureFunc(root_child0_child0, &IntrinsicSizeMeasure);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(10 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(1'140 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(1'140 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(10 == NodeLayoutGetHeight(root));

    REQUIRE(-940 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(1'140 == NodeLayoutGetWidth(root_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(1'140 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(10 == NodeLayoutGetHeight(root_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::text_stretch_max_width", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 200);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 300);
    NodeStyleSetMaxWidthStretch(root_child0);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0_child0, FlexDirection::Row);
    NodeInsertChild(root_child0, root_child0_child0, 0);
    NodeSetContext(root_child0_child0, (void *)"Lorem ipsum dolor sit amet");
    NodeSetMeasureFunc(root_child0_child0, &IntrinsicSizeMeasure);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(20 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(20 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(200 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(20 == NodeLayoutGetHeight(root_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }

  TEST_CASE("LayoutEngine::text_fit_content_max_width", "[LayoutEngine]")
  {
    SKIP();

    ConfigRef config = ConfigCreate();

    NodeRef root = NodeCreate(config);
    NodeStyleSetPositionType(root, PositionType::Absolute);
    NodeStyleSetWidth(root, 200);

    NodeRef root_child0 = NodeCreate(config);
    NodeStyleSetWidth(root_child0, 1'000);
    NodeStyleSetMaxWidthFitContent(root_child0);
    NodeInsertChild(root, root_child0, 0);

    NodeRef root_child0_child0 = NodeCreate(config);
    NodeStyleSetFlexDirection(root_child0_child0, FlexDirection::Row);
    NodeInsertChild(root_child0, root_child0_child0, 0);
    NodeSetContext(root_child0_child0, (void *)"Lorem ipsum "
                                               "sdafhasdfkjlasdhlkajsfhasldkfhasdlkahsdflkjasdhflaksdfasdlk"
                                               "jhasdlfjahsdfljkasdhalsdfhas dolor sit amet");
    NodeSetMeasureFunc(root_child0_child0, &IntrinsicSizeMeasure);
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(30 == NodeLayoutGetHeight(root));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(870 == NodeLayoutGetWidth(root_child0));
    REQUIRE(30 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(870 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(30 == NodeLayoutGetHeight(root_child0_child0));

    NodeCalculateLayout(root, Undefined, Undefined, Direction::RTL);

    REQUIRE(0 == NodeLayoutGetLeft(root));
    REQUIRE(0 == NodeLayoutGetTop(root));
    REQUIRE(200 == NodeLayoutGetWidth(root));
    REQUIRE(30 == NodeLayoutGetHeight(root));

    REQUIRE(-670 == NodeLayoutGetLeft(root_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0));
    REQUIRE(870 == NodeLayoutGetWidth(root_child0));
    REQUIRE(30 == NodeLayoutGetHeight(root_child0));

    REQUIRE(0 == NodeLayoutGetLeft(root_child0_child0));
    REQUIRE(0 == NodeLayoutGetTop(root_child0_child0));
    REQUIRE(870 == NodeLayoutGetWidth(root_child0_child0));
    REQUIRE(30 == NodeLayoutGetHeight(root_child0_child0));

    NodeDestroyRecursive(root);

    ConfigDestroy(config);
  }
}
