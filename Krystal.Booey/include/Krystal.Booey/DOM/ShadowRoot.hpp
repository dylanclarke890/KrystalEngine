#pragma once

#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Enums/ShadowRootMode.hpp"
#include "Krystal.Booey/DOM/Enums/SlotAssignmentMode.hpp"

namespace krys::boo::dom
{
  /// @see https://dom.spec.whatwg.org/#interface-shadowroot
  class ShadowRoot : public DocumentFragment
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ShadowRoot);

    friend class DocumentAlgorithms;
    friend class ElementAlgorithms;
    friend class NodeAlgorithms;

  private:
    ShadowRootMode _mode : BitCount<ShadowRootMode>() {ShadowRootMode::Open};
    bool _delegatesFocus : 1 {false};
    bool _availableToElementInternals : 1 {false};
    bool _declarative : 1 {false};
    SlotAssignmentMode _slotAssignment : BitCount<SlotAssignmentMode>() {SlotAssignmentMode::Manual};
    bool _clonable : 1 {false};
    bool _serializable : 1 {false};
    bool _keepCustomElementRegistryNull : 1 {false};
    RefPtr<html::CustomElementRegistry> _customElementRegistry;

  public:
    ShadowRoot(Document &document, RefPtr<html::CustomElementRegistry> &&registry = nullptr,
               NodeFlags flags = NodeFlags::None) noexcept;

#pragma region ShadowRoot - https://dom.spec.whatwg.org/#shadowroot

    /// @see https://dom.spec.whatwg.org/#dom-shadowroot-mode
    KRYS_NODISCARD ShadowRootMode Mode() const noexcept
    {
      return _mode;
    }

    /// @see https://dom.spec.whatwg.org/#dom-shadowroot-delegatesfocus
    KRYS_NODISCARD bool DelegatesFocus() const noexcept
    {
      return _delegatesFocus;
    }

    /// @see https://dom.spec.whatwg.org/#dom-shadowroot-slotassignment
    KRYS_NODISCARD SlotAssignmentMode SlotAssignment() const noexcept
    {
      return _slotAssignment;
    }

    /// @see https://dom.spec.whatwg.org/#dom-shadowroot-clonable
    KRYS_NODISCARD bool Clonable() const noexcept
    {
      return _clonable;
    }

    /// @see https://dom.spec.whatwg.org/#dom-shadowroot-serializable
    KRYS_NODISCARD bool Serializable() const noexcept
    {
      return _serializable;
    }

    /// @see https://dom.spec.whatwg.org/#dom-shadowroot-host
    KRYS_NODISCARD Element *Host() const noexcept;

    // TODO(impl): SHADOW-ROOT - The onslotchange attribute is an event handler IDL attribute for the
    // onslotchange event handler, whose event handler event type is slotchange.

#pragma endregion

#pragma region DocumentOrShadowRoot Mixin - https://dom.spec.whatwg.org/#mixin-documentorshadowroot

    /// @brief Returns node's CustomElementRegistry object, if any; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-documentorshadowroot-customelementregistry
    RefPtr<html::CustomElementRegistry> CustomElementRegistry() const noexcept;

#pragma endregion

  protected:
    /// @see https://dom.spec.whatwg.org/#get-the-parent
    KRYS_NODISCARD EventTarget *GetParent(Event &event) const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::ShadowRoot)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    return node.IsShadowRootNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();