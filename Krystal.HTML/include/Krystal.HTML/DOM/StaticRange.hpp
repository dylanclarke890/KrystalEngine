#pragma once

#include "Krystal.HTML/DOM/AbstractRange.hpp"
#include "Krystal.HTML/DOM/Dicts/StaticRangeInit.hpp"
#include "Krystal.Lib/Utils/StrongOrder.hpp"

namespace Krys::HTML
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