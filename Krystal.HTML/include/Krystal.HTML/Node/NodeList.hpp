#pragma once

#include "Krystal.HTML/Node/Enums/NodeCollectionLiveness.hpp"
#include "Krystal.HTML/Utils/SmallNodeList.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Mixins/CanMakeWeakPtr.hpp"
#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Types/Func.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::HTML
{
  class Node;

  using LiveNodeListFilterFunc = Func<bool(const Node &)>;

  class NodeList : public RefCounted<NodeList>, public CanMakeWeakPtr<NodeList>
  {
    KRYS_TYPE_CAST_TRAITS_ACCESS();

  private:
    NodeCollectionLiveness _liveness;

  protected:
    explicit NodeList(NodeCollectionLiveness type) noexcept : _liveness(type)
    {
    }

  public:
    virtual ~NodeList() noexcept = default;

    KRYS_NODISCARD virtual RawPtr<Node> Item(size_t index) noexcept = 0;
    KRYS_NODISCARD virtual RawPtr<const Node> Item(size_t index) const noexcept = 0;

    KRYS_NODISCARD virtual RawPtr<Node> operator[](size_t index) noexcept = 0;
    KRYS_NODISCARD virtual RawPtr<const Node> operator[](size_t index) const noexcept = 0;

    KRYS_NODISCARD virtual size_t Length() const noexcept = 0;

    KRYS_NODISCARD bool IsSupportedPropertyIndex(size_t index) const noexcept
    {
      return index < Length();
    }

  protected:
#pragma region Type Checks

    KRYS_NODISCARD bool IsLiveNodeList() const noexcept
    {
      return _liveness == NodeCollectionLiveness::Live;
    }

    KRYS_NODISCARD bool IsStaticNodeList() const noexcept
    {
      return _liveness == NodeCollectionLiveness::Static;
    }

#pragma endregion
  };

  class LiveNodeList final : public NodeList
  {
  private:
    WeakRef<Node> _root;
    LiveNodeListFilterFunc _filter;

  public:
    LiveNodeList(WeakRef<Node> &&root, LiveNodeListFilterFunc &&filter) noexcept;

    KRYS_NODISCARD RawPtr<Node> Item(size_t index) noexcept override;
    KRYS_NODISCARD RawPtr<const Node> Item(size_t index) const noexcept override;

    KRYS_NODISCARD RawPtr<Node> operator[](size_t index) noexcept override;
    KRYS_NODISCARD RawPtr<const Node> operator[](size_t index) const noexcept override;

    KRYS_NODISCARD size_t Length() const noexcept override;
  };

  class StaticNodeList final : public NodeList
  {
  private:
    SmallNodeList _nodes;

  public:
    StaticNodeList(SmallNodeList &&nodes = {}) noexcept;

    KRYS_NODISCARD RawPtr<Node> Item(size_t index) noexcept override;
    KRYS_NODISCARD RawPtr<const Node> Item(size_t index) const noexcept override;

    KRYS_NODISCARD RawPtr<Node> operator[](size_t index) noexcept override;
    KRYS_NODISCARD RawPtr<const Node> operator[](size_t index) const noexcept override;

    KRYS_NODISCARD size_t Length() const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::LiveNodeList)
  static bool IsType(const Krys::HTML::NodeList &target) noexcept
  {
    return target.IsLiveNodeList();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::StaticNodeList)
  static bool IsType(const Krys::HTML::NodeList &target) noexcept
  {
    return target.IsStaticNodeList();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
