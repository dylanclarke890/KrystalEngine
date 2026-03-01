#include "Krystal.HTML/MutationObserver/MutationObserver.hpp"
#include "Krystal.HTML/MutationObserver/MutationRecord.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.Lib/Core/Move.hpp"

namespace Krys::HTML
{
  MutationObserver::MutationObserver(MutationCallback &&callback) noexcept : _callback(Krys::Move(callback))
  {
  }

  ExceptionOr<void> MutationObserver::Observe(Node &target, const MutationObserverInit &initOptions) noexcept
  {
    MutationObserverInit options = initOptions;

    if ((options.AttributeOldValue == true || options.AttributeFilter.has_value())
        && !options.Attributes.has_value())
    {
      options.Attributes = true;
    }

    if (options.CharacterDataOldValue == true && !options.CharacterData.has_value())
    {
      options.CharacterData = true;
    }

    if (!options.ChildList && !(*options.Attributes) && !(*options.CharacterData))
    {
      return Exception {ExceptionCode::TypeError};
    }

    if ((options.AttributeOldValue == true || options.AttributeFilter.has_value())
        && options.Attributes == false)
    {
      return Exception {ExceptionCode::TypeError};
    }

    if (options.CharacterDataOldValue == true && options.CharacterData == false)
    {
      return Exception {ExceptionCode::TypeError};
    }

    for (auto &registered : target.RegisteredObserverList())
    {
      if (registered->Observer().get() == this)
      {
        for (auto &n : _nodes)
        {
          if (auto node = ShareRefPtr(n.get()))
          {
            std::erase_if(node->TransientRegisteredObservers(),
                          [source = registered](Ref<TransientRegisteredObserver> &observer)
                          { return observer->Source() == source; });
          }
        }

        registered->SetOptions(options);
        return {};
      }
    }

    Ref<RegisteredObserver> registered = CreateRef<RegisteredObserver>(ShareRef(*this), options);
    target.RegisteredObserverList().emplace_back(Krys::Move(registered));
    _nodes.emplace_back(ShareRef(target));

    return {};
  }

  void MutationObserver::Disconnect() noexcept
  {
    for (auto &n : _nodes)
    {
      if (auto node = ShareRefPtr(n.get()))
      {
        std::erase_if(node->RegisteredObserverList(), [self = this](Ref<RegisteredObserver> &observer)
                      { return observer->Observer() == self; });
      }
    }
    _recordQueue.clear();
  }

  List<Ref<MutationRecord>> MutationObserver::TakeRecords() noexcept
  {
    List<Ref<MutationRecord>> records = _recordQueue;
    _recordQueue.clear();
    return records;
  }
}