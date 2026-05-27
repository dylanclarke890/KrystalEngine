#pragma once

#include "Krystal.HTML/Node/DocumentFragment.hpp"
#include "Krystal.HTML/Node/Enums/ShadowRootMode.hpp"
#include "Krystal.HTML/Node/Enums/SlotAssignmentMode.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::HTML
{
  class Element;
  class CustomElementRegistry;

  class ShadowRoot : public DocumentFragment
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ShadowRoot);

    friend class DocumentAlgorithms;
    friend class NodeAlgorithms;
    friend class ShadowRootAlgorithms;

  private:
    ShadowRootMode _mode : BitCount<ShadowRootMode>() {ShadowRootMode::Open};
    bool _delegatesFocus : 1 {false};
    bool _availableToElementInternals : 1 {false};
    bool _declarative : 1 {false};
    SlotAssignmentMode _slotAssignment : BitCount<SlotAssignmentMode>() {SlotAssignmentMode::Manual};
    bool _clonable : 1 {false};
    bool _serializable : 1 {false};
    bool _keepCustomElementRegistryNull : 1 {false};
    RefPtr<CustomElementRegistry> _customElementRegistry;

  public:
    ShadowRoot(Document &document, RefPtr<CustomElementRegistry> &&registry = nullptr,
               NodeFlags flags = NodeFlags::None) noexcept;

#pragma region ShadowRoot

    KRYS_NODISCARD ShadowRootMode Mode() const noexcept
    {
      return _mode;
    }

    KRYS_NODISCARD bool DelegatesFocus() const noexcept
    {
      return _delegatesFocus;
    }

    KRYS_NODISCARD SlotAssignmentMode SlotAssignment() const noexcept
    {
      return _slotAssignment;
    }

    KRYS_NODISCARD bool Clonable() const noexcept
    {
      return _clonable;
    }

    KRYS_NODISCARD bool Serializable() const noexcept
    {
      return _serializable;
    }

    KRYS_NODISCARD RawPtr<Element> Host() const noexcept;

    // TODO(impl): MINOR - The onslotchange attribute is an event handler IDL attribute for the onslotchange
    // event handler, whose event handler event type is slotchange.

#pragma endregion

#pragma region DocumentOrShadowRoot Mixin - https://dom.spec.whatwg.org/#mixin-documentorshadowroot

    RefPtr<CustomElementRegistry> CustomElementRegistry() const noexcept
    {
      return _customElementRegistry;
    }

#pragma endregion

  protected:
    /// @see https://dom.spec.whatwg.org/#get-the-parent
    KRYS_NODISCARD RawPtr<EventTarget> GetParent(Event &event) const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::ShadowRoot)
  static bool IsType(const Krys::HTML::Node &node)
  {
    return node.IsShadowRootNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();