#include "Krystal.Booey/DOM/Traversal/NodeFilter.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Nodes/Node.hpp"

namespace krys::boo::dom::tests
{
  TEST_CASE("NodeFilter: the acceptNode() results",
            "[DOM][NodeFilter][dom:interface-nodefilter][dom:dom-nodefilter-filter_accept]"
            "[dom:dom-nodefilter-filter_reject][dom:dom-nodefilter-filter_skip]")
  {
    STATIC_CHECK(std::to_underlying(FilterResult::FILTER_ACCEPT) == 1);
    STATIC_CHECK(std::to_underlying(FilterResult::FILTER_REJECT) == 2);
    STATIC_CHECK(std::to_underlying(FilterResult::FILTER_SKIP) == 3);
  }

  TEST_CASE("NodeFilter: the whatToShow constants are one bit per node type, at nodeType minus one",
            "[DOM][NodeFilter][dom:interface-nodefilter][dom:dom-nodefilter-show_all]"
            "[dom:dom-nodefilter-show_element][dom:dom-nodefilter-show_attribute]"
            "[dom:dom-nodefilter-show_text][dom:dom-nodefilter-show_cdata_section]"
            "[dom:dom-nodefilter-show_processing_instruction][dom:dom-nodefilter-show_comment]"
            "[dom:dom-nodefilter-show_document][dom:dom-nodefilter-show_document_type]"
            "[dom:dom-nodefilter-show_document_fragment]")
  {
    constexpr auto bit = [](NodeType type)
    {
      return 1ul << (std::to_underlying(type) - 1u);
    };
    STATIC_CHECK(std::to_underlying(WhatToShow::SHOW_ALL) == 0xFF'FF'FF'FFul);
    STATIC_CHECK(std::to_underlying(WhatToShow::SHOW_ELEMENT) == bit(NodeType::ELEMENT_NODE));
    STATIC_CHECK(std::to_underlying(WhatToShow::SHOW_ATTRIBUTE) == bit(NodeType::ATTRIBUTE_NODE));
    STATIC_CHECK(std::to_underlying(WhatToShow::SHOW_TEXT) == bit(NodeType::TEXT_NODE));
    STATIC_CHECK(std::to_underlying(WhatToShow::SHOW_CDATA_SECTION) == bit(NodeType::CDATA_SECTION_NODE));
    STATIC_CHECK(std::to_underlying(WhatToShow::SHOW_ENTITY_REFERENCE) == 0x10ul);
    STATIC_CHECK(std::to_underlying(WhatToShow::SHOW_ENTITY) == 0x20ul);
    STATIC_CHECK(std::to_underlying(WhatToShow::SHOW_PROCESSING_INSTRUCTION)
                 == bit(NodeType::PROCESSING_INSTRUCTION_NODE));
    STATIC_CHECK(std::to_underlying(WhatToShow::SHOW_COMMENT) == bit(NodeType::COMMENT_NODE));
    STATIC_CHECK(std::to_underlying(WhatToShow::SHOW_DOCUMENT) == bit(NodeType::DOCUMENT_NODE));
    STATIC_CHECK(std::to_underlying(WhatToShow::SHOW_DOCUMENT_TYPE) == bit(NodeType::DOCUMENT_TYPE_NODE));
    STATIC_CHECK(std::to_underlying(WhatToShow::SHOW_DOCUMENT_FRAGMENT)
                 == bit(NodeType::DOCUMENT_FRAGMENT_NODE));
    STATIC_CHECK(std::to_underlying(WhatToShow::SHOW_NOTATION) == 0x800ul);
  }
}
