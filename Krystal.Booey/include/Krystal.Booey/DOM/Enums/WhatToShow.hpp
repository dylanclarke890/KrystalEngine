#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::dom
{
  /// @brief The node types a NodeIterator or TreeWalker shows: bit n is set for nodeType n + 1.
  /// @see https://dom.spec.whatwg.org/#concept-traversal-whattoshow
  /// @see https://dom.spec.whatwg.org/#interface-nodefilter
  enum class WhatToShow : ulong
  {
    /// @see https://dom.spec.whatwg.org/#dom-nodefilter-show_all
    SHOW_ALL = 0xFF'FF'FF'FF,
    /// @see https://dom.spec.whatwg.org/#dom-nodefilter-show_element
    SHOW_ELEMENT = 0x00'00'00'01,
    /// @see https://dom.spec.whatwg.org/#dom-nodefilter-show_attribute
    SHOW_ATTRIBUTE = 0x00'00'00'02,
    /// @see https://dom.spec.whatwg.org/#dom-nodefilter-show_text
    SHOW_TEXT = 0x00'00'00'04,
    /// @see https://dom.spec.whatwg.org/#dom-nodefilter-show_cdata_section
    SHOW_CDATA_SECTION = 0x00'00'00'08,
    // Legacy.
    SHOW_ENTITY_REFERENCE = 0x00'00'00'10,
    // Legacy.
    SHOW_ENTITY = 0x00'00'00'20,
    /// @see https://dom.spec.whatwg.org/#dom-nodefilter-show_processing_instruction
    SHOW_PROCESSING_INSTRUCTION = 0x00'00'00'40,
    /// @see https://dom.spec.whatwg.org/#dom-nodefilter-show_comment
    SHOW_COMMENT = 0x00'00'00'80,
    /// @see https://dom.spec.whatwg.org/#dom-nodefilter-show_document
    SHOW_DOCUMENT = 0x00'00'01'00,
    /// @see https://dom.spec.whatwg.org/#dom-nodefilter-show_document_type
    SHOW_DOCUMENT_TYPE = 0x00'00'02'00,
    /// @see https://dom.spec.whatwg.org/#dom-nodefilter-show_document_fragment
    SHOW_DOCUMENT_FRAGMENT = 0x00'00'04'00,
    // Legacy.
    SHOW_NOTATION = 0x00'00'08'00
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::dom::WhatToShow, 13u);