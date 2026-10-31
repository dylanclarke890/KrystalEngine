#pragma once

#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::dom
{
  /// @brief A CDATA section in an XML document: a Text node that isn't exclusive.
  /// @see https://dom.spec.whatwg.org/#interface-cdatasection
  class CDATASection : public Text
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(CDATASection);

  public:
    /// @brief Creates a CDATA section node in @p document with the data.
    /// @see https://dom.spec.whatwg.org/#create-a-cdata-section-node
    CDATASection(Document &document, DOMString &&data = u8"") noexcept;

#pragma region Node - https://dom.spec.whatwg.org/#node

    /// @see https://dom.spec.whatwg.org/#dom-node-nodename
    KRYS_NODISCARD DOMString NodeName() const noexcept final
    {
      return u8"#cdata-section";
    }

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::CDATASection)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    return node.IsCDATASectionNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();