#pragma once

#include "Krystal.HTML/NodeList/NodeList.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Mixins/CanMakeWeakPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class Node;

  class LiveNodeList final : public NodeList
  {
  private:
    Ref<Node> _owner;

  public:
    explicit LiveNodeList(Node &owner) noexcept;

    KRYS_NODISCARD RawPtr<Node> Item(size_t) const noexcept override
    {
      return nullptr;
    }

    KRYS_NODISCARD size_t Length() const noexcept override
    {
      return 0;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::LiveNodeList)
  static bool IsType(const Krys::HTML::NodeList &target) noexcept
  {
    return target.IsLiveNodeList();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();