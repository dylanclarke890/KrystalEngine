#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo
{
  class Text;
}

namespace krys::boo::dom
{
  class MutationAlgorithms;
  class SlotAlgorithms;

  /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-slot-assign
  using ElementOrText = Variant<Ref<Element>, Ref<Text>>;
}

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/multipage/scripting.html#assignednodesoptions
  struct AssignedNodesOptions
  {
    bool Flatten {false};
  };
}

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/multipage/scripting.html#htmlslotelement
  /// @see https://dom.spec.whatwg.org/#concept-slot
  class HTMLSlotElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLSlotElement);

    friend class ::krys::boo::dom::MutationAlgorithms;
    friend class ::krys::boo::dom::SlotAlgorithms;

  private:
    /// @see https://dom.spec.whatwg.org/#slot-assigned-nodes
    List<WeakRef<Node>> _assignedNodes;
    /// @see https://html.spec.whatwg.org/multipage/scripting.html#manually-assigned-nodes
    List<WeakRef<Node>> _manuallyAssignedNodes;

  public:
    HTMLSlotElement(Document &document) noexcept;

#pragma region HTMLSlotElement - https://html.spec.whatwg.org/multipage/scripting.html#htmlslotelement

    /// @brief The slot's name: its `name` attribute in no namespace, or the empty string without one.
    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-slot-name
    /// @see https://dom.spec.whatwg.org/#slot-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-slot-name
    void Name(DOMString name) noexcept;

    /// @brief The slot's assigned nodes, or with @p options' `Flatten`, its flattened slottables.
    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-slot-assignednodes
    KRYS_NODISCARD List<Ref<Node>> AssignedNodes(const html::AssignedNodesOptions &options) noexcept;

    /// @brief `AssignedNodes(options)`, filtered to contain only elements.
    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-slot-assignedelements
    KRYS_NODISCARD List<Ref<Element>>
      AssignedElements(const html::AssignedNodesOptions &options = {}) noexcept;

    /// @brief Sets the slot's manually assigned nodes to @p nodes.
    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-slot-assign
    void Assign(const List<dom::ElementOrText> &nodes) noexcept;

#pragma endregion

    /// @brief The slot's assigned nodes, or null if it has none.
    /// @see https://dom.spec.whatwg.org/#slot-assigned-nodes
    KRYS_NODISCARD const List<WeakRef<Node>> *AssignedNodes() const noexcept
    {
      return _assignedNodes.empty() ? nullptr : &_assignedNodes;
    }

  protected:
    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLSlotElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    return target.IsHTMLSlotElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
