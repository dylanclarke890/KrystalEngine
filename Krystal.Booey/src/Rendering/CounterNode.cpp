#include "Krystal.Booey/Rendering/CounterNode.hpp"
#include "Krystal.Booey/Rendering/RenderCounter.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"

namespace krys::boo
{
  namespace
  {
    // In case the sum overflows we need to ignore the operation instead of just clamping the result.
    KRYS_NODISCARD constexpr int32 SumIfNoOverflowOrFirstValue(int32 a, int32 b) noexcept
    {
      auto sum = static_cast<int64>(a) + b;
      if (sum < std::numeric_limits<int32>::min() || sum > std::numeric_limits<int32>::max())
      {
        return a;
      }
      return static_cast<int32>(sum);
    }
  }

  CounterNode::CounterNode(RenderElement &owner, EnumSet<Type> type, int32 value) noexcept
      : _type(type), _value(value), _owner(CreateWeakPtr(&owner))
  {
  }

  CounterNode::~CounterNode() noexcept
  {
    // Ideally this would be an assert and this would never be reached. In reality this happens a lot
    // so we need to handle these cases. The node is still connected to the tree so we need to detach it.
    if (_parent || _previousSibling || _nextSibling || _firstChild || _lastChild)
    {
      RefPtr<CounterNode> oldParent;
      RefPtr<CounterNode> oldPreviousSibling;
      // Instead of calling removeChild() we do this safely as the tree is likely broken if we get here.
      if (_parent)
      {
        if (_parent->_firstChild == this)
        {
          _parent->_firstChild = _nextSibling;
        }
        if (_parent->_lastChild == this)
        {
          _parent->_lastChild = _previousSibling;
        }
        oldParent = ShareRefPtr(_parent.get());
        _parent = nullptr;
      }
      if (_previousSibling)
      {
        if (_previousSibling->_nextSibling == this)
        {
          _previousSibling->_nextSibling = _nextSibling;
        }
        oldPreviousSibling = ShareRefPtr(_previousSibling.get());
        _previousSibling = nullptr;
      }
      if (_nextSibling)
      {
        if (_nextSibling->_previousSibling == this)
        {
          _nextSibling->_previousSibling = CreateWeakPtr(oldPreviousSibling.get());
        }
        _nextSibling = nullptr;
      }
      if (_firstChild)
      {
        // The node's children are reparented to the old parent.
        for (auto child = ShareRefPtr(_firstChild.get()); child;)
        {
          auto nextChild = ShareRefPtr(child->_nextSibling.get());
          RefPtr<CounterNode> nextSibling;
          child->_parent = CreateWeakPtr(oldParent.get());
          if (oldPreviousSibling)
          {
            nextSibling = ShareRefPtr(oldPreviousSibling->_nextSibling.get());
            child->_previousSibling = CreateWeakPtr(oldPreviousSibling.get());
            oldPreviousSibling->_nextSibling = CreateWeakPtr(child.get());
            child->_nextSibling = CreateWeakPtr(nextSibling.get());
            nextSibling->_previousSibling = CreateWeakPtr(child.get());
            oldPreviousSibling = child;
          }
          child = nextChild;
        }
      }
    }
    ResetRenderers();
  }

  RenderElement &CounterNode::Owner() const noexcept
  {
    return *_owner;
  }

  Ref<CounterNode> CounterNode::Create(RenderElement &owner, EnumSet<Type> type, int32 value) noexcept
  {
    return CreateRef<CounterNode>(owner, type, value);
  }

  CounterNode *CounterNode::NextInPreOrderAfterChildren(const CounterNode *stayWithin) const noexcept
  {
    if (this == stayWithin)
    {
      return nullptr;
    }

    const CounterNode *current = this;
    CounterNode *next;
    while (!(next = current->_nextSibling.get()))
    {
      current = current->_parent.get();
      if (!current || current == stayWithin)
      {
        return nullptr;
      }
    }
    return next;
  }

  CounterNode *CounterNode::NextInPreOrder(const CounterNode *stayWithin) const noexcept
  {
    if (auto *next = _firstChild.get())
    {
      return next;
    }

    return NextInPreOrderAfterChildren(stayWithin);
  }

  CounterNode *CounterNode::LastDescendant() const noexcept
  {
    auto *last = _lastChild.get();
    if (!last)
    {
      return nullptr;
    }

    while (auto *lastChild = last->_lastChild.get())
    {
      last = lastChild;
    }

    return last;
  }

  CounterNode *CounterNode::PreviousInPreOrder() const noexcept
  {
    auto *previous = _previousSibling.get();
    if (!previous)
    {
      return _parent.get();
    }

    while (auto *lastChild = previous->_lastChild.get())
    {
      previous = lastChild;
    }

    return previous;
  }

  int32 CounterNode::ComputeCountInParent() const noexcept
  {
    if (HasSetType())
    {
      return _value;
    }

    int32 increment = ActsAsReset() ? 0 : _value;
    // In case the sum overflows we need to ignore the operation instead
    // of just clamping the result, as per spec resolution.
    // See https://github.com/w3c/csswg-drafts/issues/9029
    if (_previousSibling)
    {
      return SumIfNoOverflowOrFirstValue(_previousSibling->_countInParent, increment);
    }
    krys_debug_assert(_parent->_firstChild.get() == this);
    return SumIfNoOverflowOrFirstValue(_parent->_value, increment);
  }

  void CounterNode::AddRenderer(RenderCounter &renderer) noexcept
  {
    krys_debug_assert(!renderer._counterNode);
    krys_debug_assert(!renderer._nextForSameCounter);
    renderer._nextForSameCounter = _rootRenderer;
    _rootRenderer = CreateWeakPtr(&renderer);
    renderer._counterNode = CreateWeakPtr(this);
  }

  void CounterNode::RemoveRenderer(RenderCounter &renderer) noexcept
  {
    krys_debug_assert(renderer._counterNode && renderer._counterNode == this);
    RenderCounter *previous = nullptr;
    for (auto *current = _rootRenderer.get(); current;
         previous = current, current = current->_nextForSameCounter.get())
    {
      if (current != &renderer)
      {
        continue;
      }

      if (previous)
      {
        previous->_nextForSameCounter = renderer._nextForSameCounter;
      }
      else
      {
        _rootRenderer = renderer._nextForSameCounter;
      }
      renderer._nextForSameCounter = nullptr;
      renderer._counterNode = nullptr;
      return;
    }
    krys_debug_assert(false);
  }

  void CounterNode::ResetRenderers() noexcept
  {
    if (!_rootRenderer)
    {
      return;
    }
    auto *current = _rootRenderer.get();
    while (current)
    {
      auto *next = current->_nextForSameCounter.get();
      current->_nextForSameCounter = nullptr;
      current->_counterNode = nullptr;
      current->View().AddCounterNeedingUpdate(*current);
      current = next;
    }
    _rootRenderer = nullptr;
  }

  void CounterNode::ResetThisAndDescendantsRenderers() noexcept
  {
    RefPtr node = ShareRefPtr(this);
    do
    {
      node->ResetRenderers();
      node = ShareRefPtr(node->NextInPreOrder(this));
    } while (node);
  }

  void CounterNode::Recount() noexcept
  {
    for (auto node = ShareRefPtr(this); node; node = ShareRefPtr(node->_nextSibling.get()))
    {
      int32 oldCount = node->_countInParent;
      int32 newCount = node->ComputeCountInParent();
      if (oldCount == newCount)
      {
        break;
      }
      node->_countInParent = newCount;
      node->ResetThisAndDescendantsRenderers();
    }
  }

  void CounterNode::InsertAfter(CounterNode &newChild, CounterNode *beforeChild,
                                const css::CSSOMStringAtom &identifier) noexcept
  {
    krys_debug_assert(!newChild._parent);
    krys_debug_assert(!newChild._previousSibling);
    krys_debug_assert(!newChild._nextSibling);
    // If the beforeChild is not our child we can not complete the request. This hardens against bugs in
    // RenderCounter. When renderers are reparented it may request that we insert counter nodes improperly.
    if (beforeChild && beforeChild->_parent != this)
    {
      return;
    }

    if (newChild.HasResetType())
    {
      while (_lastChild != beforeChild)
      {
        RenderCounter::DestroyCounterNode(_lastChild->Owner(), identifier);
      }
    }

    RefPtr<CounterNode> next;

    if (beforeChild)
    {
      next = ShareRefPtr(beforeChild->_nextSibling.get());
      beforeChild->_nextSibling = CreateWeakPtr(&newChild);
    }
    else
    {
      next = ShareRefPtr(_firstChild.get());
      _firstChild = CreateWeakPtr(&newChild);
    }

    newChild._parent = CreateWeakPtr(this);
    newChild._previousSibling = CreateWeakPtr(beforeChild);

    if (next)
    {
      krys_debug_assert(next->_previousSibling == beforeChild);
      next->_previousSibling = CreateWeakPtr(&newChild);
      newChild._nextSibling = CreateWeakPtr(next.get());
    }
    else
    {
      krys_debug_assert(_lastChild == beforeChild);
      _lastChild = CreateWeakPtr(&newChild);
    }

    if (!newChild._firstChild || newChild.HasResetType())
    {
      newChild._countInParent = newChild.ComputeCountInParent();
      newChild.ResetThisAndDescendantsRenderers();
      if (next)
      {
        next->Recount();
      }
      return;
    }

    // The code below handles the case when a formerly root increment counter is loosing its root position
    // and therefore its children become next siblings.
    auto last = ShareRefPtr(newChild._lastChild.get());
    auto first = ShareRefPtr(newChild._firstChild.get());

    if (first)
    {
      krys_debug_assert(last);
      newChild._nextSibling = CreateWeakPtr(first.get());
      if (_lastChild == &newChild)
      {
        _lastChild = CreateWeakPtr(last.get());
      }

      first->_previousSibling = CreateWeakPtr(&newChild);

      // The case when the original next sibling of the inserted node becomes a child of
      // one of the former children of the inserted node is not handled as it is believed
      // to be impossible since:
      // 1. if the increment counter node lost it's root position as a result of another
      //    counter node being created, it will be inserted as the last child so next is null.
      // 2. if the increment counter node lost it's root position as a result of a renderer being
      //    inserted into the document's render tree, all its former children counters are attached
      //    to children of the inserted renderer and hence cannot be in scope for counter nodes
      //    attached to renderers that were already in the document's render tree.
      last->_nextSibling = CreateWeakPtr(next.get());
      if (next)
      {
        krys_debug_assert(next->_previousSibling == &newChild);
        next->_previousSibling = CreateWeakPtr(last.get());
      }
      else
      {
        _lastChild = CreateWeakPtr(last.get());
      }
      for (next = first;; next = ShareRefPtr(next->_nextSibling.get()))
      {
        next->_parent = CreateWeakPtr(this);
        if (last == next)
        {
          break;
        }
      }
    }
    newChild._firstChild = nullptr;
    newChild._lastChild = nullptr;
    newChild._countInParent = newChild.ComputeCountInParent();
    newChild.ResetRenderers();
    first->Recount();
  }

  void CounterNode::RemoveChild(CounterNode &oldChild) noexcept
  {
    krys_debug_assert(!oldChild._firstChild);
    krys_debug_assert(!oldChild._lastChild);

    auto next = ShareRefPtr(oldChild._nextSibling.get());
    auto previous = ShareRefPtr(oldChild._previousSibling.get());

    oldChild._nextSibling = nullptr;
    oldChild._previousSibling = nullptr;
    oldChild._parent = nullptr;

    if (previous)
    {
      previous->_nextSibling = CreateWeakPtr(next.get());
    }
    else
    {
      krys_debug_assert(_firstChild == &oldChild);
      _firstChild = CreateWeakPtr(next.get());
    }

    if (next)
    {
      next->_previousSibling = CreateWeakPtr(previous.get());
    }
    else
    {
      krys_debug_assert(_lastChild == &oldChild);
      _lastChild = CreateWeakPtr(previous.get());
    }

    if (next)
    {
      next->Recount();
    }
  }
}
