#include "Krystal.HTML/DOM/MutationObserver.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
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

    if (!options.ChildList && !options.Attributes.value_or(false) && !options.CharacterData.value_or(false))
    {
      return Exception {ExceptionCode::TypeError};
    }

    if (options.AttributeOldValue == true && !options.Attributes.value_or(false))
    {
      return Exception {ExceptionCode::TypeError};
    }

    if (options.AttributeFilter.has_value() && !options.Attributes.value_or(false))
    {
      return Exception {ExceptionCode::TypeError};
    }

    if (options.CharacterDataOldValue == true && !options.CharacterData.value_or(false))
    {
      return Exception {ExceptionCode::TypeError};
    }

    for (auto &registered : target.RegisteredObservers())
    {
      if (registered->Observer() == this)
      {
        for (auto &n : _nodes)
        {
          if (auto node = n.lock())
          {
            std::erase_if(node->TransientRegisteredObservers(), [source = registered.get()](auto &observer)
                          { return observer->Source() == source; });
          }
        }

        registered->Options(options);
        return {};
      }
    }

    auto registered = CreateRef<RegisteredObserver>(ShareRef(*this), options);
    target.RegisteredObservers().emplace_back(Krys::Move(registered));
    _nodes.emplace_back(CreateWeakRef(target));

    return {};
  }

  void MutationObserver::Disconnect() noexcept
  {
    for (auto &n : _nodes)
    {
      if (auto node = n.lock())
      {
        std::erase_if(node->RegisteredObservers(), [self = this](Ref<RegisteredObserver> &observer)
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