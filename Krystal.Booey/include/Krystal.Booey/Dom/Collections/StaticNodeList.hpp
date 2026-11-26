#pragma once

#include "Krystal.Booey/Dom/Collections/NodeList.hpp"
#include "Krystal.Booey/Dom/Internals/SmallNodeList.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  /// @brief A static node list: the nodes it was made with.
  /// @see https://dom.spec.whatwg.org/#concept-collection-static
  class StaticNodeList final : public NodeList
  {
  private:
    dom::SmallNodeList _nodes;

  public:
    StaticNodeList(dom::SmallNodeList nodes = {}) noexcept;

#pragma region NodeList - https://dom.spec.whatwg.org/#nodelist

    /// @see https://dom.spec.whatwg.org/#dom-nodelist-item
    KRYS_NODISCARD RefPtr<Node> Item(size_t index) noexcept override;

    /// @see https://dom.spec.whatwg.org/#dom-nodelist-item
    KRYS_NODISCARD RefPtr<const Node> Item(size_t index) const noexcept override;

    /// @see https://dom.spec.whatwg.org/#dom-nodelist-item
    KRYS_NODISCARD RefPtr<Node> operator[](size_t index) noexcept override;

    /// @see https://dom.spec.whatwg.org/#dom-nodelist-item
    KRYS_NODISCARD RefPtr<const Node> operator[](size_t index) const noexcept override;

    /// @see https://dom.spec.whatwg.org/#dom-nodelist-length
    KRYS_NODISCARD size_t Length() const noexcept override;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::StaticNodeList)
  static bool IsType(const krys::boo::NodeList &target) noexcept
  {
    return !target.IsLiveNodeList();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
