#include "Krystal.Booey/DOM/CDATASection.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Node.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include <catch_all.hpp>

namespace krys::boo::dom::tests
{
  namespace
  {
    struct CommonTestData
    {
      Ref<dom::Document> Document;
      Ref<CDATASection> Node;

      CommonTestData(dom::DOMString &&value)
          : Document(CreateRef<dom::Document>()), Node(CreateRef<CDATASection>(*Document, krys::move(value)))
      {
      }
    };
  }

  TEST_CASE("CDATASection::NodeName", "[HTML][CDATASection]")
  {
    CommonTestData data(u8"Hello, world!");
    REQUIRE(data.Node->NodeName() == u8"#cdata-section");
  }

  TEST_CASE("CDATASection::NodeType", "[HTML][CDATASection]")
  {
    CommonTestData data(u8"Hello, world!");
    REQUIRE(data.Node->NodeType() == NodeType::CDATA_SECTION_NODE);
  }
}
