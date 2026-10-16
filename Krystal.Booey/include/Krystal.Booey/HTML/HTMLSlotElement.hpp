#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class MutationAlgorithms;
  class SlotAlgorithms;
}

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlslotelement
  class HTMLSlotElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLSlotElement);

    friend class ::krys::boo::dom::MutationAlgorithms;
    friend class ::krys::boo::dom::SlotAlgorithms;

  private:
    List<Ref<dom::Node>> _assignedNodes;
    List<WeakRef<dom::Node>> _manuallyAssignedNodes;

  public:
    HTMLSlotElement(dom::Document &document) noexcept;

#pragma region HTMLSlotElement - https://html.spec.whatwg.org/#htmlslotelement

    /// @see https://html.spec.whatwg.org/#dom-slot-name
    KRYS_NODISCARD dom::DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-slot-name
    void Name(dom::DOMString &&name) noexcept;

#pragma endregion

    /// @brief The slot's assigned nodes, or null if it has none.
    /// @see https://dom.spec.whatwg.org/#slot-assigned-nodes
    KRYS_NODISCARD const List<Ref<dom::Node>> *AssignedNodes() const noexcept
    {
      return _assignedNodes.empty() ? nullptr : &_assignedNodes;
    }

  protected:
    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value,
                            dom::DOMStringAtom namespaceURI) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLSlotElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsHTMLSlotElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();