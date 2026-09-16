#pragma once

#include "Krystal.Booey/DOM/NodeList.hpp"
#include "Krystal.Booey/DOM/Types/SmallNodeList.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::dom
{
  class StaticNodeList final : public NodeList
  {
  private:
    SmallNodeList _nodes;

  public:
    StaticNodeList(SmallNodeList &&nodes = {}) noexcept;

    KRYS_NODISCARD RefPtr<Node> Item(size_t index) noexcept override;
    KRYS_NODISCARD RefPtr<const Node> Item(size_t index) const noexcept override;

    KRYS_NODISCARD RefPtr<Node> operator[](size_t index) noexcept override;
    KRYS_NODISCARD RefPtr<const Node> operator[](size_t index) const noexcept override;

    KRYS_NODISCARD size_t Length() const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::StaticNodeList)
  static bool IsType(const krys::boo::dom::NodeList &target) noexcept
  {
    return !target.IsLiveNodeList();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();