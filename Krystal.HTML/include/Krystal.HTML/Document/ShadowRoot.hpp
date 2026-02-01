#pragma once

#include "Krystal.HTML/Document/DocumentFragment.hpp"
#include "Krystal.HTML/Element/Element.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Pointers/Ref.hpp"
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
  class ShadowRoot : public DocumentFragment
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
    ShadowRoot(Document &document, NodeFlags flags = NodeFlags::None) noexcept;
  };
}