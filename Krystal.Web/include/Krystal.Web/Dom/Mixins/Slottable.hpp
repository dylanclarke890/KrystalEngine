#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"

namespace krys::web
{
  class Element;
  class HTMLSlotElement;
  class Text;
}

namespace krys::web::dom::mixins
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
