#pragma once

#include "Krystal.HTML/Node/Text.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"

namespace Krys::HTML
{
  class CDATASection : public Text
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CDATASection);

  public:
    CDATASection(Document &document, DOMString &&data = u8"") noexcept;

    KRYS_NODISCARD DOMString NodeName() const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CDATASection)
  static bool IsType(const Krys::HTML::Node &node)
  {
    return node.IsCDATASectionNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();