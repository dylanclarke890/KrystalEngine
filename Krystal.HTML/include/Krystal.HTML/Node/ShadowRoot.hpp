#pragma once

#include "Krystal.HTML/Node/DocumentFragment.hpp"
#include "Krystal.HTML/Node/TreeScope.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::HTML
{
  enum class ShadowRootMode : uint8
  {
    Open,
    Closed
  };

  enum class SlotAssignmentMode : uint8
  {
    Manual,
    Named
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::ShadowRootMode, 2u);
KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::SlotAssignmentMode, 2u);

namespace Krys::HTML
{
  class Element;
  class CustomElementRegistry;

  class ShadowRoot : public DocumentFragment, public TreeScope
  {
  private:
    ShadowRootMode _mode : BitCount<ShadowRootMode>() {ShadowRootMode::Open};
    bool _delegatesFocus : 1 {false};
    SlotAssignmentMode _slotAssignment : BitCount<SlotAssignmentMode>() {SlotAssignmentMode::Manual};
    bool _clonable : 1 {false};
    bool _serializable : 1 {false};
    RawPtr<Element> _host;

    // TODO: most of the fecking implementation lol
  public:
    ShadowRoot(Document &document, RefPtr<CustomElementRegistry> &&registry,
               NodeFlag flags = NodeFlag::None) noexcept;

    KRYS_NODISCARD ShadowRootMode Mode() const noexcept
    {
      return _mode;
    }

    KRYS_NODISCARD RawPtr<Element> Host() const noexcept
    {
      return _host;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::ShadowRoot)
  static bool IsType(const Krys::HTML::Node &node)
  {
    return node.IsShadowRootNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()