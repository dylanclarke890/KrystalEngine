#pragma once

#include "Krystal.HTML/DOM/NodeList.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Types/Func.hpp"

namespace Krys::HTML
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::LiveNodeList)
  static bool IsType(const Krys::HTML::NodeList &target) noexcept
  {
    return target.IsLiveNodeList();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();