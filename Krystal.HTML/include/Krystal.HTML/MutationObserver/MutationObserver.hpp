#pragma once

#include "Krystal.HTML/DOM/MutationRecord.hpp"
#include "Krystal.HTML/MutationObserver/MutationCallback.hpp"
#include "Krystal.HTML/MutationObserver/MutationObserverInit.hpp"
#include "Krystal.HTML/Types/ExceptionOr.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/WeakPtr.hpp"
#include "Krystal.Lib/Types/Func.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  class Node;

  class MutationObserver : public RefCounted<MutationObserver>
  {
    using NodeList = List<WeakRef<Node>>;
    using RecordQueue = List<Ref<MutationRecord>>;

  private:
    MutationCallback _callback;
    NodeList _nodes;
    RecordQueue _recordQueue;

  public:
    explicit MutationObserver(MutationCallback &&callback) noexcept;

    ExceptionOr<void> Observe(Node &target, const MutationObserverInit &options) noexcept;

    void Disconnect() noexcept;

    KRYS_NODISCARD RecordQueue TakeRecords() noexcept;
  };
}