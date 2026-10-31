#pragma once

#include "Krystal.Booey/DOM/Types/ElementOrText.hpp"
#include "Krystal.Booey/HTML/Dicts/AssignedNodesOptions.hpp"
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
  /// @see https://dom.spec.whatwg.org/#concept-slot
  class HTMLSlotElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLSlotElement);

    friend class ::krys::boo::dom::MutationAlgorithms;
    friend class ::krys::boo::dom::SlotAlgorithms;

  private:
    /// @see https://dom.spec.whatwg.org/#slot-assigned-nodes
    List<WeakRef<dom::Node>> _assignedNodes;
    /// @see https://html.spec.whatwg.org/#manually-assigned-nodes
    List<WeakRef<dom::Node>> _manuallyAssignedNodes;

  public:
    HTMLSlotElement(dom::Document &document) noexcept;

#pragma region HTMLSlotElement - https://html.spec.whatwg.org/#htmlslotelement

    /// @brief The slot's name: its `name` attribute in no namespace, or the empty string without one.
    /// @see https://html.spec.whatwg.org/#dom-slot-name
    /// @see https://dom.spec.whatwg.org/#slot-name
    KRYS_NODISCARD dom::DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-slot-name
    void Name(dom::DOMString &&name) noexcept;

    /// @brief The slot's assigned nodes, or with @p options' `Flatten`, its flattened slottables.
    /// @see https://html.spec.whatwg.org/#dom-slot-assignednodes
    KRYS_NODISCARD List<Ref<dom::Node>> AssignedNodes(const AssignedNodesOptions &options) noexcept;

    /// @brief `AssignedNodes(options)`, filtered to contain only elements.
    /// @see https://html.spec.whatwg.org/#dom-slot-assignedelements
    KRYS_NODISCARD List<Ref<dom::Element>>
      AssignedElements(const AssignedNodesOptions &options = {}) noexcept;

    /// @brief Sets the slot's manually assigned nodes to @p nodes.
    /// @see https://html.spec.whatwg.org/#dom-slot-assign
    void Assign(const List<dom::ElementOrText> &nodes) noexcept;

#pragma endregion

    /// @brief The slot's assigned nodes, or null if it has none.
    /// @see https://dom.spec.whatwg.org/#slot-assigned-nodes
    KRYS_NODISCARD const List<WeakRef<dom::Node>> *AssignedNodes() const noexcept
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