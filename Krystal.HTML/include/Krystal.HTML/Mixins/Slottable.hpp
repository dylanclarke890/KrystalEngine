#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"

namespace Krys::HTML
{
  class Text;
  class Element;
  class HTMLSlotElement;
}

namespace Krys::HTML::Mixins
{
  /// @see https://dom.spec.whatwg.org/#mixin-slotable
  class Slottable
  {
  public:
    /// @see https://dom.spec.whatwg.org/#dom-slotable-assignedslot
    KRYS_NODISCARD static RawPtr<HTMLSlotElement> AssignedSlot(Text &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-slotable-assignedslot
    KRYS_NODISCARD static RawPtr<HTMLSlotElement> AssignedSlot(Element &node) noexcept;
  };
}