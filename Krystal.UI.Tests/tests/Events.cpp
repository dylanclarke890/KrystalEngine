#include "Krystal.UI/Layout/Event/Event.hpp"
#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include "utils/TestUtil.hpp"
#include <algorithm>
#include <catch_all.hpp>
#include <functional>
#include <memory>
#include <vector>

namespace Krys::Tests
{
  using namespace Krys::UI;

  template <Event::Type E>
  struct TypedEventTestData
  {
  };

  template <>
  struct TypedEventTestData<Event::LayoutPassEnd>
  {
    LayoutData LayoutData;
  };

  struct EventArgs
  {
    const NodeConstRef Node;
    Event::Type Type;
    std::unique_ptr<void, std::function<void(void *)>> DataPtr;
    std::unique_ptr<void, std::function<void(void *)>> EventTestDataPtr;

    template <Event::Type E>
    const Event::TypedData<E> &Data()
    {
      return *static_cast<Event::TypedData<E> *>(DataPtr.get());
    }

    template <Event::Type E>
    const TypedEventTestData<E> &EventTestData()
    {
      return *static_cast<TypedEventTestData<E> *>(EventTestDataPtr.get());
    }
  };

  class EventTest
  {
    ScopedEventSubscription Subscription {&EventTest::Listen};
    static void Listen(NodeConstRef /*node*/, Event::Type /*type*/, Event::Data /*data*/);

  public:
    static std::vector<EventArgs> Events;
    static EventArgs &LastEvent()
    {
      return Events.back();
    }

    ~EventTest()
    {
      Events.clear();
    }
  };

  TEST_CASE_METHOD(EventTest, "new_node_has_event", "[LayoutEngine][Events]")
  {
    auto c = ConfigGetDefault();
    auto n = NodeCreate();

    REQUIRE(LastEvent().Node == n);
    REQUIRE(LastEvent().Type == Event::NodeAllocation);
    REQUIRE(LastEvent().Data<Event::NodeAllocation>().Config == c);

    NodeDestroy(n);
  }

  TEST_CASE_METHOD(EventTest, "new_node_with_config_event", "[LayoutEngine][Events]")
  {
    auto c = ConfigCreate();
    auto n = NodeCreate(c);

    REQUIRE(LastEvent().Node == n);
    REQUIRE(LastEvent().Type == Event::NodeAllocation);
    REQUIRE(LastEvent().Data<Event::NodeAllocation>().Config == c);

    NodeDestroy(n);
    ConfigDestroy(c);
  }

  TEST_CASE_METHOD(EventTest, "clone_node_event", "[LayoutEngine][Events]")
  {
    auto c = ConfigCreate();
    auto n = NodeCreate(c);
    auto clone = NodeClone(n);

    REQUIRE(LastEvent().Node == clone);
    REQUIRE(LastEvent().Type == Event::NodeAllocation);
    REQUIRE(LastEvent().Data<Event::NodeAllocation>().Config == c);

    NodeDestroy(n);
    NodeDestroy(clone);
    ConfigDestroy(c);
  }

  TEST_CASE_METHOD(EventTest, "free_node_event", "[LayoutEngine][Events]")
  {
    auto c = ConfigCreate();
    auto n = NodeCreate(c);
    NodeDestroy(n);

    REQUIRE(LastEvent().Node == n);
    REQUIRE(LastEvent().Type == Event::NodeDeallocation);
    REQUIRE(LastEvent().Data<Event::NodeDeallocation>().Config == c);

    ConfigDestroy(c);
  }

  TEST_CASE_METHOD(EventTest, "layout_events", "[LayoutEngine][Events]")
  {
    auto root = NodeCreate();
    auto child = NodeCreate();
    NodeInsertChild(root, child, 0);

    NodeCalculateLayout(root, 123, 456, Direction::LTR);

    REQUIRE(Events[2].Node == root);
    REQUIRE(Events[2].Type == Event::LayoutPassStart);

    REQUIRE(Events[3].Node == child);
    REQUIRE(Events[3].Type == Event::NodeLayout);

    REQUIRE(Events[4].Node == child);
    REQUIRE(Events[4].Type == Event::NodeLayout);

    REQUIRE(Events[5].Node == child);
    REQUIRE(Events[5].Type == Event::NodeLayout);

    REQUIRE(Events[6].Node == root);
    REQUIRE(Events[6].Type == Event::NodeLayout);

    REQUIRE(Events[7].Node == root);
    REQUIRE(Events[7].Type == Event::LayoutPassEnd);

    NodeDestroyRecursive(root);
  }

  TEST_CASE_METHOD(EventTest, "layout_events_single_node", "[LayoutEngine][Events]")
  {
    auto root = NodeCreate();
    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(Events[1].Node == root);
    REQUIRE(Events[1].Type == Event::LayoutPassStart);

    REQUIRE(Events[2].Node == root);
    REQUIRE(Events[2].Type == Event::NodeLayout);

    REQUIRE(Events[3].Node == root);
    REQUIRE(Events[3].Type == Event::LayoutPassEnd);

    LayoutData layoutData = Events[3].EventTestData<Event::LayoutPassEnd>().LayoutData;

    REQUIRE(layoutData.Layouts == 1);
    REQUIRE(layoutData.Measures == 0);
    REQUIRE(layoutData.MaxMeasureCache == 1);
  }

  TEST_CASE_METHOD(EventTest, "layout_events_counts_multi_node_layout", "[LayoutEngine][Events]")
  {
    auto root = NodeCreate();
    auto childA = NodeCreate();
    NodeInsertChild(root, childA, 0);
    auto childB = NodeCreate();
    NodeInsertChild(root, childB, 1);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(Events[3].Node == root);
    REQUIRE(Events[3].Type == Event::LayoutPassStart);

    REQUIRE(Events[11].Node == root);
    REQUIRE(Events[11].Type == Event::LayoutPassEnd);

    LayoutData layoutData = Events[11].EventTestData<Event::LayoutPassEnd>().LayoutData;

    REQUIRE(layoutData.Layouts == 3);
    REQUIRE(layoutData.Measures == 4);
    REQUIRE(layoutData.MaxMeasureCache == 3);
  }

  TEST_CASE_METHOD(EventTest, "layout_events_counts_cache_hits_single_node_layout", "[LayoutEngine][Events]")
  {
    auto root = NodeCreate();

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(Events[4].Node == root);
    REQUIRE(Events[4].Type == Event::LayoutPassStart);

    REQUIRE(Events[6].Node == root);
    REQUIRE(Events[6].Type == Event::LayoutPassEnd);

    LayoutData layoutData = Events[6].EventTestData<Event::LayoutPassEnd>().LayoutData;

    REQUIRE(layoutData.Layouts == 0);
    REQUIRE(layoutData.Measures == 0);
    REQUIRE(layoutData.CachedLayouts == 1);
    REQUIRE(layoutData.CachedMeasures == 0);
  }

  TEST_CASE_METHOD(EventTest, "layout_events_counts_cache_hits_multi_node_layout", "[LayoutEngine][Events]")
  {
    auto root = NodeCreate();
    auto childA = NodeCreate();
    NodeInsertChild(root, childA, 0);
    auto childB = NodeCreate();
    NodeInsertChild(root, childB, 1);

    NodeCalculateLayout(root, 987, 654, Direction::LTR);
    NodeCalculateLayout(root, 123, 456, Direction::LTR);

    NodeCalculateLayout(root, 987, 654, Direction::LTR);

    REQUIRE(LastEvent().Node == root);
    REQUIRE(LastEvent().Type == Event::LayoutPassEnd);

    LayoutData layoutData = LastEvent().EventTestData<Event::LayoutPassEnd>().LayoutData;

    REQUIRE(layoutData.Layouts == 3);
    REQUIRE(layoutData.Measures == 0);
    REQUIRE(layoutData.MaxMeasureCache == 5);
    REQUIRE(layoutData.CachedLayouts == 0);
    REQUIRE(layoutData.CachedMeasures == 4);
  }

  TEST_CASE_METHOD(EventTest, "layout_events_has_max_measure_cache", "[LayoutEngine][Events]")
  {
    auto root = NodeCreate();
    auto a = NodeCreate();
    NodeInsertChild(root, a, 0);
    auto b = NodeCreate();
    NodeInsertChild(root, b, 1);
    NodeStyleSetFlexBasis(a, 10.0f);

    for (auto s : {20.0f, 30.0f, 40.0f})
    {
      NodeCalculateLayout(root, s, s, Direction::LTR);
    }

    REQUIRE(LastEvent().Node == root);
    REQUIRE(LastEvent().Type == Event::LayoutPassEnd);

    LayoutData layoutData = LastEvent().EventTestData<Event::LayoutPassEnd>().LayoutData;

    REQUIRE(layoutData.Layouts == 3);
    REQUIRE(layoutData.Measures == 3);
    REQUIRE(layoutData.MaxMeasureCache == 7);
  }

  TEST_CASE_METHOD(EventTest, "measure_functions_get_wrapped", "[LayoutEngine][Events]")
  {
    auto root = NodeCreate();
    NodeSetMeasureFunc(root, [](NodeConstRef, float, MeasureMode, float, MeasureMode) { return Size {}; });

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(Events[2].Node == root);
    REQUIRE(Events[2].Type == Event::MeasureCallbackStart);

    REQUIRE(Events[Events.size() - 1].Node == root);
    REQUIRE(Events[Events.size() - 1].Type == Event::LayoutPassEnd);
  }

  TEST_CASE_METHOD(EventTest, "baseline_functions_get_wrapped", "[LayoutEngine][Events]")
  {
    auto root = NodeCreate();
    auto child = NodeCreate();
    NodeInsertChild(root, child, 0);

    NodeSetBaselineFunc(child, [](NodeConstRef, float, float) { return 0.0f; });
    NodeStyleSetFlexDirection(root, FlexDirection::Row);
    NodeStyleSetAlignItems(root, Align::Baseline);

    NodeCalculateLayout(root, Undefined, Undefined, Direction::LTR);

    REQUIRE(Events[5].Node == child);
    REQUIRE(Events[5].Type == Event::NodeBaselineStart);

    REQUIRE(Events[Events.size() - 1].Node == root);
    REQUIRE(Events[Events.size() - 1].Type == Event::LayoutPassEnd);
  }

  namespace
  {

    template <Event::Type E>
    EventArgs CreateArgs(NodeConstRef node, const Event::Data data)
    {
      using Data = Event::TypedData<E>;
      auto deleteData = [](void *x)
      {
        delete static_cast<Data *>(x);
      };

      return {node, E, {new Data {(data.Get<E>())}, deleteData}, nullptr};
    }

    template <Event::Type E>
    EventArgs CreateArgs(NodeConstRef node, const Event::Data data, TypedEventTestData<E> eventTestData)
    {
      using EventTestData = TypedEventTestData<E>;
      auto deleteEventTestData = [](void *x)
      {
        delete static_cast<EventTestData *>(x);
      };

      EventArgs args = CreateArgs<E>(node, data);
      args.EventTestDataPtr = {new EventTestData {eventTestData}, deleteEventTestData};
      return args;
    }

  }

  void EventTest::Listen(NodeConstRef node, Event::Type type, Event::Data data)
  {
    switch (type)
    {
      case Event::NodeAllocation:   Events.push_back(CreateArgs<Event::NodeAllocation>(node, data)); break;
      case Event::NodeDeallocation: Events.push_back(CreateArgs<Event::NodeDeallocation>(node, data)); break;
      case Event::NodeLayout:       Events.push_back(CreateArgs<Event::NodeLayout>(node, data)); break;
      case Event::LayoutPassStart:  Events.push_back(CreateArgs<Event::LayoutPassStart>(node, data)); break;
      case Event::LayoutPassEnd:
      {
        auto &eventData = data.Get<Event::LayoutPassEnd>();
        Events.push_back(CreateArgs<Event::LayoutPassEnd>(node, data, {*eventData.LayoutData}));
        break;
      }

      case Event::MeasureCallbackStart:
        Events.push_back(CreateArgs<Event::MeasureCallbackStart>(node, data));
        break;
      case Event::MeasureCallbackEnd:
        Events.push_back(CreateArgs<Event::MeasureCallbackEnd>(node, data));
        break;
      case Event::NodeBaselineStart:
        Events.push_back(CreateArgs<Event::NodeBaselineStart>(node, data));
        break;
      case Event::NodeBaselineEnd: Events.push_back(CreateArgs<Event::NodeBaselineEnd>(node, data)); break;
    }
  }

  std::vector<EventArgs> EventTest::Events {};
}