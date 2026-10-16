#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

// This implements a counter tree that is used for finding parents in counters() lookup,
// and for propagating count changes when nodes are added or removed.

// Parents represent unique counters and their scope, which are created either explicitly
// by "counter-reset" style rules or implicitly by referring to a counter that is not in scope.
// Such nodes are tagged as "reset" nodes, although they are not all due to "counter-reset".

// Not that render tree children are often counter tree siblings due to counter scoping rules.

namespace krys::boo
{
  class RenderCounter;
  class RenderElement;

  enum class CounterNodeType : uint8
  {
    Increment,
    Reset,
    Set
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::CounterNodeType, 3uz);

namespace krys::boo
{
  /// @brief A counter reference (a reset, increment or set of a counter, or a use of it) in a counter tree.
  class CounterNode : public RefCounted<CounterNode>, public CanMakeWeakPtr<CounterNode>
  {
    KRYS_NON_COPY_MOVABLE(CounterNode);

  public:
    using Type = CounterNodeType;

  private:
    EnumSet<Type> _type {};
    int32 _value;
    int32 _countInParent {0};
    WeakPtr<RenderElement> _owner;
    WeakPtr<RenderCounter> _rootRenderer;

    WeakPtr<CounterNode> _parent;
    WeakPtr<CounterNode> _previousSibling;
    WeakPtr<CounterNode> _nextSibling;
    WeakPtr<CounterNode> _firstChild;
    WeakPtr<CounterNode> _lastChild;

  public:
    CounterNode(RenderElement &owner, EnumSet<Type> type, int32 value) noexcept;
    ~CounterNode() noexcept;

    KRYS_NODISCARD static Ref<CounterNode> Create(RenderElement &owner, EnumSet<Type> type,
                                                  int32 value) noexcept;

    KRYS_NODISCARD bool ActsAsReset() const noexcept
    {
      return HasResetType() || !_parent;
    }
    KRYS_NODISCARD bool HasResetType() const noexcept
    {
      return _type.Contains(Type::Reset);
    }
    KRYS_NODISCARD bool HasSetType() const noexcept
    {
      return _type.Contains(Type::Set);
    }
    KRYS_NODISCARD int32 Value() const noexcept
    {
      return _value;
    }
    KRYS_NODISCARD int32 CountInParent() const noexcept
    {
      return _countInParent;
    }
    KRYS_NODISCARD RenderElement &Owner() const noexcept;
    void AddRenderer(RenderCounter &renderer) noexcept;
    void RemoveRenderer(RenderCounter &renderer) noexcept;

    /// @brief Invalidates the text in the renderers of this counter, if any.
    void ResetRenderers() noexcept;

    KRYS_NODISCARD CounterNode *Parent() const noexcept
    {
      return _parent.get();
    }
    KRYS_NODISCARD CounterNode *PreviousSibling() const noexcept
    {
      return _previousSibling.get();
    }
    KRYS_NODISCARD CounterNode *NextSibling() const noexcept
    {
      return _nextSibling.get();
    }
    KRYS_NODISCARD CounterNode *FirstChild() const noexcept
    {
      return _firstChild.get();
    }
    KRYS_NODISCARD CounterNode *LastChild() const noexcept
    {
      return _lastChild.get();
    }
    KRYS_NODISCARD CounterNode *LastDescendant() const noexcept;
    KRYS_NODISCARD CounterNode *PreviousInPreOrder() const noexcept;
    KRYS_NODISCARD CounterNode *NextInPreOrder(const CounterNode *stayWithin = nullptr) const noexcept;
    KRYS_NODISCARD CounterNode *
      NextInPreOrderAfterChildren(const CounterNode *stayWithin = nullptr) const noexcept;

    void InsertAfter(CounterNode &newChild, CounterNode *beforeChild,
                     const css::CSSOMStringAtom &identifier) noexcept;
    /// @note @p oldChild's identifier must match the identifier of this counter.
    void RemoveChild(CounterNode &oldChild) noexcept;

  private:
    KRYS_NODISCARD int32 ComputeCountInParent() const noexcept;
    // Invalidates the text in the renderer of this counter, if any,
    // and in the renderers of all descendants of this counter, if any.
    void ResetThisAndDescendantsRenderers() noexcept;
    void Recount() noexcept;
  };
}
