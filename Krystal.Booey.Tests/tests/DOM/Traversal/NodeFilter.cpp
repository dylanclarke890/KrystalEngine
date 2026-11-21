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
    STATIC_CHECK(ShowAll.ToRaw() == 0xFFFul);
    STATIC_CHECK(WhatToShow {WhatToShowFlag::SHOW_ELEMENT}.ToRaw() == bit(NodeType::ELEMENT_NODE));
    STATIC_CHECK(WhatToShow {WhatToShowFlag::SHOW_ATTRIBUTE}.ToRaw() == bit(NodeType::ATTRIBUTE_NODE));
    STATIC_CHECK(WhatToShow {WhatToShowFlag::SHOW_TEXT}.ToRaw() == bit(NodeType::TEXT_NODE));
    STATIC_CHECK(WhatToShow {WhatToShowFlag::SHOW_CDATA_SECTION}.ToRaw()
                 == bit(NodeType::CDATA_SECTION_NODE));
    STATIC_CHECK(WhatToShow {WhatToShowFlag::SHOW_ENTITY_REFERENCE}.ToRaw() == 0x10ul);
    STATIC_CHECK(WhatToShow {WhatToShowFlag::SHOW_ENTITY}.ToRaw() == 0x20ul);
    STATIC_CHECK(WhatToShow {WhatToShowFlag::SHOW_PROCESSING_INSTRUCTION}.ToRaw()
                 == bit(NodeType::PROCESSING_INSTRUCTION_NODE));
    STATIC_CHECK(WhatToShow {WhatToShowFlag::SHOW_COMMENT}.ToRaw() == bit(NodeType::COMMENT_NODE));
    STATIC_CHECK(WhatToShow {WhatToShowFlag::SHOW_DOCUMENT}.ToRaw() == bit(NodeType::DOCUMENT_NODE));
    STATIC_CHECK(WhatToShow {WhatToShowFlag::SHOW_DOCUMENT_TYPE}.ToRaw()
                 == bit(NodeType::DOCUMENT_TYPE_NODE));
    STATIC_CHECK(WhatToShow {WhatToShowFlag::SHOW_DOCUMENT_FRAGMENT}.ToRaw()
                 == bit(NodeType::DOCUMENT_FRAGMENT_NODE));
    STATIC_CHECK(WhatToShow {WhatToShowFlag::SHOW_NOTATION}.ToRaw() == 0x800ul);
  }
}
