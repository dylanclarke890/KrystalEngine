#pragma once

#include "Krystal.Lib/Array.hpp"
#include "Krystal.Lib/Func.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.UI/Api/Forward.hpp"

namespace Krys::UI
{
  enum struct LayoutType : int
  {
    kLayout = 0,
    kMeasure = 1,
    kCachedLayout = 2,
    kCachedMeasure = 3
  };

  enum struct LayoutPassReason : int
  {
    kInitial = 0,
    kAbsLayout = 1,
    kStretch = 2,
    kMultilineStretch = 3,
    kFlexLayout = 4,
    kMeasureChild = 5,
    kAbsMeasureChild = 6,
    kFlexMeasure = 7,
    COUNT
  };

  struct LayoutData
  {
    int Layouts = 0;
    int Measures = 0;
    uint32 MaxMeasureCache = 0;
    int CachedLayouts = 0;
    int CachedMeasures = 0;
    int MeasureCallbacks = 0;
    Array<int, static_cast<uint8>(LayoutPassReason::COUNT)> MeasureCallbackReasonsCount;
  };

  const char *LayoutPassReasonToString(LayoutPassReason value);

  struct Event
  {
    enum Type
    {
      NodeAllocation,
      NodeDeallocation,
      NodeLayout,
      LayoutPassStart,
      LayoutPassEnd,
      MeasureCallbackStart,
      MeasureCallbackEnd,
      NodeBaselineStart,
      NodeBaselineEnd,
    };
    class Data;
    using Subscriber = void(NodeConstRef, Type, Data);
    using Subscribers = List<Func<Subscriber>>;

    template <Type E>
    struct TypedData
    {
    };

    class Data
    {
      const void *_data;

    public:
      template <Type E>
      explicit Data(const TypedData<E> &data) : _data {&data}
      {
      }

      template <Type E>
      const TypedData<E> &Get() const
      {
        return *static_cast<const TypedData<E> *>(_data);
      }
    };

    static void Reset();

    static void Subscribe(std::function<Subscriber> &&subscriber);

    template <Type E>
    static void Publish(NodeConstRef node, const TypedData<E> &eventData = {})
    {
      Publish(node, E, Data {eventData});
    }

  private:
    static void Publish(NodeConstRef /*node*/, Type /*eventType*/, const Data & /*eventData*/);
  };

  template <>
  struct Event::TypedData<Event::NodeAllocation>
  {
    ConfigConstRef Config;
  };

  template <>
  struct Event::TypedData<Event::NodeDeallocation>
  {
    ConfigConstRef Config;
  };

  template <>
  struct Event::TypedData<Event::LayoutPassEnd>
  {
    LayoutData *LayoutData;
  };

  template <>
  struct Event::TypedData<Event::MeasureCallbackEnd>
  {
    float Width;
    MeasureMode WidthMeasureMode;
    float Height;
    MeasureMode HeightMeasureMode;
    float MeasuredWidth;
    float MeasuredHeight;
    const LayoutPassReason Reason;
  };

  template <>
  struct Event::TypedData<Event::NodeLayout>
  {
    LayoutType LayoutType;
  };
}