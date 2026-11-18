#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class Element;
  class HTMLSlotElement;
  class Text;
}

namespace krys::boo::dom::mixins
{
  /// @see https://dom.spec.whatwg.org/#mixin-slotable
  class Slottable
  {
  public:
    /// @see https://dom.spec.whatwg.org/#dom-slotable-assignedslot
    KRYS_NODISCARD static RefPtr<HTMLSlotElement> AssignedSlot(Text &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-slotable-assignedslot
    KRYS_NODISCARD static RefPtr<HTMLSlotElement> AssignedSlot(Element &node) noexcept;
  };
}
