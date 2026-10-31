#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::dom
{
  /// @brief The bits `compareDocumentPosition()` returns.
  /// @see https://dom.spec.whatwg.org/#dom-node-comparedocumentposition
  enum class DocumentPosition : uint8
  {
    DOCUMENT_POSITION_EQUIVALENT = 0x00,
    /// @see https://dom.spec.whatwg.org/#dom-node-document_position_disconnected
    DOCUMENT_POSITION_DISCONNECTED = 0x01,
    /// @see https://dom.spec.whatwg.org/#dom-node-document_position_preceding
    DOCUMENT_POSITION_PRECEDING = 0x02,
    /// @see https://dom.spec.whatwg.org/#dom-node-document_position_following
    DOCUMENT_POSITION_FOLLOWING = 0x04,
    /// @see https://dom.spec.whatwg.org/#dom-node-document_position_contains
    DOCUMENT_POSITION_CONTAINS = 0x08,
    /// @see https://dom.spec.whatwg.org/#dom-node-document_position_contained_by
    DOCUMENT_POSITION_CONTAINED_BY = 0x10,
    /// @see https://dom.spec.whatwg.org/#dom-node-document_position_implementation_specific
    DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC = 0x20,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::dom::DocumentPosition, 7u);
