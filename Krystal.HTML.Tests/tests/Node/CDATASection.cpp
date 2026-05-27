#include "Krystal.HTML/Node/CDATASection.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/DOM/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  namespace
  {
    struct CommonTestData
    {
      Ref<Document> Document;
      Ref<CDATASection> Node;

      CommonTestData(DOMString &&value)
          : Document(CreateRef<HTML::Document>()), Node(CreateRef<CDATASection>(*Document, Krys::Move(value)))
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
