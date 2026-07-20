#pragma once

#include "Krystal.HTML/DOM/Text.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"

namespace Krys::HTML
{
  /// @see https://dom.spec.whatwg.org/#interface-cdatasection
  class CDATASection : public Text
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CDATASection);

  public:
#pragma region Text - https://dom.spec.whatwg.org/#text

    /// @see https://dom.spec.whatwg.org/#dom-text-text
    CDATASection(Document &document, DOMString &&data = u8"") noexcept;

#pragma endregion

#pragma region Node - https://dom.spec.whatwg.org/#node

    /// @see https://dom.spec.whatwg.org/#dom-node-nodename
    KRYS_NODISCARD DOMString NodeName() const noexcept final
    {
      return u8"#cdata-section";
    }

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::CDATASection)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::Node &node) noexcept
  {
    return node.IsCDATASectionNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();