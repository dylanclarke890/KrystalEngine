#pragma once

#include "Krystal.Booey/DOM/NodeList.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Func.hpp"

namespace krys::boo::dom
{
  using LiveNodeListFilterFunc = Func<bool(const Node &node)>;

  class LiveNodeList final : public NodeList
  {
  private:
    Ref<Node> _root;
    LiveNodeListFilterFunc _filter;

  public:
    LiveNodeList(Node &root, LiveNodeListFilterFunc &&filter) noexcept;

    KRYS_NODISCARD RefPtr<Node> Item(size_t index) noexcept override;
    KRYS_NODISCARD RefPtr<const Node> Item(size_t index) const noexcept override;

    KRYS_NODISCARD RefPtr<Node> operator[](size_t index) noexcept override;
    KRYS_NODISCARD RefPtr<const Node> operator[](size_t index) const noexcept override;

    KRYS_NODISCARD size_t Length() const noexcept override;

  protected:
    KRYS_NODISCARD bool IsLiveNodeList() const noexcept override
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LiveNodeList)
  static bool IsType(const krys::boo::NodeList &target) noexcept
  {
    return target.IsLiveNodeList();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();