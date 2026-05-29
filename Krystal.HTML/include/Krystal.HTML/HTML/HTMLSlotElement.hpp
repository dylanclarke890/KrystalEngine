#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.Lib/Pointers/WeakPtr.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  class HTMLSlotElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLSlotElement);

    friend class MutationAlgorithms;
    friend class SlotAlgorithms;

  private:
    DOMString _name;
    List<Ref<Node>> _assignedNodes;
    List<WeakRef<Node>> _manuallyAssignedNodes;

  public:
    HTMLSlotElement(Document &document) noexcept;

    KRYS_NODISCARD const DOMString &Name() const noexcept
    {
      return _name;
    }

    void Name(DOMString &&name) noexcept
    {
      _name = Krys::Move(name);
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLSlotElement)
  static bool IsType(const Krys::HTML::Node &target) noexcept
  {
    return target.IsHTMLSlotElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();