#pragma once

#include "Krystal.HTML/DOM/Callbacks/MutationCallback.hpp"
#include "Krystal.HTML/DOM/Dicts/MutationObserverInit.hpp"
#include "Krystal.HTML/DOM/MutationRecord.hpp"
#include "Krystal.HTML/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/WeakPtr.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  class Node;

  /// @see https://dom.spec.whatwg.org/#interface-mutationobserver
  class MutationObserver : public RefCounted<MutationObserver>
  {
    friend class MutationObserverAlgorithms;

  private:
    /// @see https://dom.spec.whatwg.org/#concept-mo-callback
    MutationCallback _callback;
    /// @see https://dom.spec.whatwg.org/#mutationobserver-node-list
    List<WeakRef<Node>> _nodes;
    /// @see https://dom.spec.whatwg.org/#concept-mo-queue
    List<Ref<MutationRecord>> _recordQueue;

  public:
#pragma region MutationObserver - https://dom.spec.whatwg.org/#mutationobserver

    /// @see https://dom.spec.whatwg.org/#dom-mutationobserver-mutationobserver
    explicit MutationObserver(MutationCallback &&callback) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-mutationobserver-observe
    ExceptionOr<void> Observe(Node &target, const MutationObserverInit &options) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-mutationobserver-disconnect
    void Disconnect() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-mutationobserver-takerecords
    KRYS_NODISCARD List<Ref<MutationRecord>> TakeRecords() noexcept;

#pragma endregion
  };
}