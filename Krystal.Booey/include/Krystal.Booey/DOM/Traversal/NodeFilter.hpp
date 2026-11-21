#pragma once

#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"

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

  consteval FilterResult MaxEnumValue(FilterResult) noexcept
  {
    return FilterResult::FILTER_SKIP;
  }

  /// @brief The node types a NodeIterator or TreeWalker shows: bit n is set for nodeType n + 1.
  /// @see https://dom.spec.whatwg.org/#concept-traversal-whattoshow
  /// @see https://dom.spec.whatwg.org/#interface-nodefilter
  enum class WhatToShowFlag : ulong
  {
    /// @see https://dom.spec.whatwg.org/#dom-nodefilter-show_element
    SHOW_ELEMENT,
    /// @see https://dom.spec.whatwg.org/#dom-nodefilter-show_attribute
    SHOW_ATTRIBUTE,
    /// @see https://dom.spec.whatwg.org/#dom-nodefilter-show_text
    SHOW_TEXT,
    /// @see https://dom.spec.whatwg.org/#dom-nodefilter-show_cdata_section
    SHOW_CDATA_SECTION,
    // Legacy.
    SHOW_ENTITY_REFERENCE,
    // Legacy.
    SHOW_ENTITY,
    /// @see https://dom.spec.whatwg.org/#dom-nodefilter-show_processing_instruction
    SHOW_PROCESSING_INSTRUCTION,
    /// @see https://dom.spec.whatwg.org/#dom-nodefilter-show_comment
    SHOW_COMMENT,
    /// @see https://dom.spec.whatwg.org/#dom-nodefilter-show_document
    SHOW_DOCUMENT,
    /// @see https://dom.spec.whatwg.org/#dom-nodefilter-show_document_type
    SHOW_DOCUMENT_TYPE,
    /// @see https://dom.spec.whatwg.org/#dom-nodefilter-show_document_fragment
    SHOW_DOCUMENT_FRAGMENT,
    // Legacy.
    SHOW_NOTATION,
  };

  consteval WhatToShowFlag MaxEnumValue(WhatToShowFlag) noexcept
  {
    return WhatToShowFlag::SHOW_NOTATION;
  }

  using WhatToShow = EnumSet<WhatToShowFlag>;

  /// @see https://dom.spec.whatwg.org/#dom-nodefilter-show_all
  constexpr WhatToShow ShowAll = WhatToShow::FromRaw(
    static_cast<WhatToShow::StorageType>((1u << (ToUnderlying(MaxEnumValue(WhatToShowFlag {})) + 1u)) - 1u));
}

namespace krys::boo
{
  class Node;
}

namespace krys::boo::dom
{
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
