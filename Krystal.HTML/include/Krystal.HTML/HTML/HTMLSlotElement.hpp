#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.Lib/Pointers/WeakPtr.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlslotelement
  class HTMLSlotElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLSlotElement);

    friend class MutationAlgorithms;
    friend class SlotAlgorithms;

  private:
    List<Ref<Node>> _assignedNodes;
    List<WeakRef<Node>> _manuallyAssignedNodes;

  public:
    HTMLSlotElement(Document &document) noexcept;

#pragma region HTMLSlotElement - https://html.spec.whatwg.org/#htmlslotelement

    /// @see https://html.spec.whatwg.org/#dom-slot-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-slot-name
    void Name(DOMString &&name) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLSlotElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::Node &target) noexcept
  {
    return target.IsHTMLSlotElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();