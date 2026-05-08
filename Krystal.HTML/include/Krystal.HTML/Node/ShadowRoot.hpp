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

  private:
    ShadowRootMode _mode : BitCount<ShadowRootMode>() {ShadowRootMode::Open};
    bool _delegatesFocus : 1 {false};
    SlotAssignmentMode _slotAssignment : BitCount<SlotAssignmentMode>() {SlotAssignmentMode::Manual};
    bool _clonable : 1 {false};
    bool _serializable : 1 {false};
    RawPtr<Element> _host {nullptr};
    RefPtr<CustomElementRegistry> _customElementRegistry;

  public:
    ShadowRoot(Document &document, RefPtr<CustomElementRegistry> &&registry,
               NodeFlag flags = NodeFlag::None) noexcept;

#pragma region ShadowRoot

    KRYS_NODISCARD ShadowRootMode Mode() const noexcept
    {
      return _mode;
    }

    KRYS_NODISCARD SlotAssignmentMode SlotAssignment() const noexcept
    {
      return _slotAssignment;
    }

    KRYS_NODISCARD RawPtr<Element> Host() const noexcept
    {
      return _host;
    }

    KRYS_NODISCARD bool Clonable() const noexcept
    {
      return _clonable;
    }

#pragma endregion

#pragma region DocumentOrShadowRoot Mixin - https://dom.spec.whatwg.org/#mixin-documentorshadowroot

    RefPtr<CustomElementRegistry> CustomElementRegistry() const noexcept
    {
      return _customElementRegistry;
    }

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::ShadowRoot)
  static bool IsType(const Krys::HTML::Node &node)
  {
    return node.IsShadowRootNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();