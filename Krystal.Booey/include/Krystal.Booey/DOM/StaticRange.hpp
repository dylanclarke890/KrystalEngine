#pragma once

#include "Krystal.Booey/DOM/AbstractRange.hpp"
#include "Krystal.Booey/DOM/Dicts/StaticRangeInit.hpp"
#include "Krystal.Core/Utils/StrongOrder.hpp"

namespace krys::boo::dom
{
  /// @brief A range that doesn't follow changes to the tree.
  /// @see https://dom.spec.whatwg.org/#interface-staticrange
  class StaticRange : public AbstractRange
  {
  public:
    /// @brief Creates a static range from @p init, whose nodes `StaticRangeInit::Create` has checked.
    /// @see https://dom.spec.whatwg.org/#dom-staticrange-staticrange
    explicit StaticRange(const StaticRangeInit &init) noexcept : AbstractRange(init.Start(), init.End())
    {
      // 2. Set this's start to (init["startContainer"], init["startOffset"]) and end to
      // (init["endContainer"],
      //    init["endOffset"]).
    }

    /// @brief Whether its start and end are in the same tree, within their nodes, and in order.
    /// @see https://dom.spec.whatwg.org/#staticrange-valid
    KRYS_NODISCARD bool IsValid() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::StaticRange)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::AbstractRange &range) noexcept
  {
    return !range.IsLiveRange();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
