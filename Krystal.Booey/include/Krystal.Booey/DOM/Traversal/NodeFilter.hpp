#pragma once

#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  /// @brief What a filter returns for a node.
  /// @see https://dom.spec.whatwg.org/#interface-nodefilter
  enum class FilterResult : uint8
  {
    /// @see https://dom.spec.whatwg.org/#dom-nodefilter-filter_accept
    FILTER_ACCEPT = 1,
    /// @see https://dom.spec.whatwg.org/#dom-nodefilter-filter_reject
    FILTER_REJECT = 2,
    /// @see https://dom.spec.whatwg.org/#dom-nodefilter-filter_skip
    FILTER_SKIP = 3,
  };

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

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::dom::FilterResult, 3u);
KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::dom::WhatToShow, 13u);

namespace krys::boo::dom
{
  class Node;

  /// @brief Decides which nodes a NodeIterator or TreeWalker returns.
  /// @see https://dom.spec.whatwg.org/#interface-nodefilter
  class NodeFilter : public RefCounted<NodeFilter>
  {
  public:
    virtual ~NodeFilter() noexcept = default;

    /// @brief Whether to accept, reject or skip the node.
    KRYS_NODISCARD virtual ExceptionOr<FilterResult> AcceptNode(const Node &node) const noexcept = 0;
  };
}