#include "Krystal.Booey/DOM/Comment.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Node.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    struct CommonTestData
    {
      Ref<dom::Document> Document;
      Ref<dom::Comment> Node;

      CommonTestData(dom::DOMString &&value)
          : Document(CreateRef<dom::Document>()), Node(CreateRef<dom::Comment>(*Document, krys::move(value)))
      {
      }
    };

  }
  TEST_CASE("Comment::NodeName", "[HTML][Comment]")
  {
    CommonTestData data(u8"Hello, world!");
    REQUIRE(data.Node->NodeName() == u8"#comment");
  }

  TEST_CASE("Comment::NodeType", "[HTML][Comment]")
  {
    CommonTestData data(u8"Hello, world!");
    REQUIRE(data.Node->NodeType() == NodeType::COMMENT_NODE);
  }
}
