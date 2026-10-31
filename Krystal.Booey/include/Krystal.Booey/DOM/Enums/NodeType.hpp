#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::dom
{
  /// @see https://dom.spec.whatwg.org/#dom-node-nodetype
  enum class NodeType : uint8
  {
    NONE = 0,
    /// @see https://dom.spec.whatwg.org/#dom-node-element_node
    ELEMENT_NODE = 1,
    /// @see https://dom.spec.whatwg.org/#dom-node-attribute_node
    ATTRIBUTE_NODE = 2,
    /// @see https://dom.spec.whatwg.org/#dom-node-text_node
    TEXT_NODE = 3,
    /// @see https://dom.spec.whatwg.org/#dom-node-cdata_section_node
    CDATA_SECTION_NODE = 4,
    ENTITY_REFERENCE_NODE = 5, // legacy
    ENTITY_NODE = 6,           // legacy
    /// @see https://dom.spec.whatwg.org/#dom-node-processing_instruction_node
    PROCESSING_INSTRUCTION_NODE = 7,
    /// @see https://dom.spec.whatwg.org/#dom-node-comment_node
    COMMENT_NODE = 8,
    /// @see https://dom.spec.whatwg.org/#dom-node-document_node
    DOCUMENT_NODE = 9,
    /// @see https://dom.spec.whatwg.org/#dom-node-document_type_node
    DOCUMENT_TYPE_NODE = 10,
    /// @see https://dom.spec.whatwg.org/#dom-node-document_fragment_node
    DOCUMENT_FRAGMENT_NODE = 11,
    NOTATION_NODE = 12, // legacy
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::dom::NodeType, 13u);
