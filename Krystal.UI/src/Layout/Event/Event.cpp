#include "Krystal.UI/Layout/Event/Event.hpp"
#include <atomic>
#include <memory>

namespace Krys::UI
{
  const char *LayoutPassReasonToString(const LayoutPassReason value)
  {
    switch (value)
    {
      case LayoutPassReason::kInitial:          return "initial";
      case LayoutPassReason::kAbsLayout:        return "abs_layout";
      case LayoutPassReason::kStretch:          return "stretch";
      case LayoutPassReason::kMultilineStretch: return "multiline_stretch";
      case LayoutPassReason::kFlexLayout:       return "flex_layout";
      case LayoutPassReason::kMeasureChild:     return "measure";
      case LayoutPassReason::kAbsMeasureChild:  return "abs_measure";
      case LayoutPassReason::kFlexMeasure:      return "flex_measure";
      default:                                  return "unknown";
    }
  }

  namespace
  {
    using namespace Krys;

    struct InternalNode
    {
      Func<Event::Subscriber> Subscriber = nullptr;
      InternalNode *Next = nullptr;

      explicit InternalNode(Func<Event::Subscriber> &&subscriber) : Subscriber(std::move(subscriber))
      {
      }
    };

    std::atomic<InternalNode *> SubscribersAtomic {nullptr};

    InternalNode *Push(InternalNode *newHead)
    {
      InternalNode *oldHead = nullptr;
      do
      {
        oldHead = SubscribersAtomic.load(std::memory_order_relaxed);
        if (newHead != nullptr)
        {
          newHead->Next = oldHead;
        }
      } while (!SubscribersAtomic.compare_exchange_weak(oldHead, newHead, std::memory_order_release,
                                                        std::memory_order_relaxed));
      return oldHead;
    }

  }

  void Event::Reset()
  {
    auto head = Push(nullptr);
    while (head != nullptr)
    {
      auto current = head;
      head = head->Next;
      delete current;
    }
  }

  void Event::Subscribe(std::function<Subscriber> &&subscriber)
  {
    Push(new InternalNode {std::move(subscriber)});
  }

  void Event::Publish(NodeConstRef node, Type eventType, const Data &eventData)
  {
    for (auto subscriber = SubscribersAtomic.load(std::memory_order_relaxed); subscriber != nullptr;
         subscriber = subscriber->Next)
    {
      subscriber->Subscriber(node, eventType, eventData);
    }
  }

}