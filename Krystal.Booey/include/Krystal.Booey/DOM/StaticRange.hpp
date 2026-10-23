#pragma once

#include "Krystal.Booey/DOM/AbstractRange.hpp"
#include "Krystal.Booey/DOM/Dicts/StaticRangeInit.hpp"
#include "Krystal.Core/Utils/StrongOrder.hpp"

namespace krys::boo::dom
{
  /// @see https://dom.spec.whatwg.org/#interface-staticrange
  class StaticRange : public AbstractRange
  {
  public:
    /// @see https://dom.spec.whatwg.org/#staticrange-staticrange
    StaticRange(const StaticRangeInit &init) noexcept : AbstractRange(init.Start(), init.End())
    {
    }

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
