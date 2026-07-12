#pragma once

#include "Krystal.HTML/DOM/NodeList.hpp"
#include "Krystal.HTML/DOM/Types/SmallNodeList.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"

namespace Krys::HTML
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::StaticNodeList)
  static bool IsType(const Krys::HTML::NodeList &target) noexcept
  {
    return !target.IsLiveNodeList();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();