#pragma once

#include "Krystal.Booey/DOM/MutationObserver/MutationRecord.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class Node;
}

namespace krys::boo::dom
{
  class MutationObserver;

  /// @see https://dom.spec.whatwg.org/#dictdef-mutationobserverinit
  struct MutationObserverInit
  {
    bool ChildList {false};
    Maybe<bool> Attributes;
    Maybe<bool> CharacterData;
    bool Subtree {false};
    Maybe<bool> AttributeOldValue;
    Maybe<bool> CharacterDataOldValue;
    Maybe<List<DOMStringAtom>> AttributeFilter;
  };

  /// @see https://dom.spec.whatwg.org/#callbackdef-mutationcallback
  using MutationCallback = Func<void(const List<Ref<MutationRecord>> &records, MutationObserver &observer)>;

  /// @see https://dom.spec.whatwg.org/#interface-mutationobserver
  class MutationObserver : public RefCounted<MutationObserver>
  {
    friend class MutationObserverAlgorithms;

  private:
    /// @see https://dom.spec.whatwg.org/#concept-mo-callback
    MutationCallback _callback;
    /// @see https://dom.spec.whatwg.org/#mutationobserver-creation-index
    uint64 _creationIndex {0};
    /// @see https://dom.spec.whatwg.org/#mutationobserver-node-list
    List<WeakRef<Node>> _nodes;
    /// @see https://dom.spec.whatwg.org/#mutationobserver-transient-node-set
    List<WeakRef<Node>> _transientNodes;
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
