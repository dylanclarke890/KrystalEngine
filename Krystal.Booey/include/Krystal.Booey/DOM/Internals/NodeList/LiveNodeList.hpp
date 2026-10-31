#pragma once

#include "Krystal.Booey/DOM/NodeList.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Func.hpp"

namespace krys::boo::dom
{
  using LiveNodeListFilterFunc = Func<bool(const Node &node)>;

  /// @brief A live node list: a view of the root's descendants that match the filter, in tree order, worked
  /// out again on each access.
  /// @see https://dom.spec.whatwg.org/#concept-collection
  /// @see https://dom.spec.whatwg.org/#concept-collection-live
  /// @see https://dom.spec.whatwg.org/#represented-by-the-collection
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::LiveNodeList)
  static bool IsType(const krys::boo::dom::NodeList &target) noexcept
  {
    return target.IsLiveNodeList();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();