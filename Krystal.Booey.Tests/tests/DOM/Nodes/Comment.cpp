#include "Krystal.Booey/DOM/Nodes/Comment.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Abort/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/Node.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"

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
  TEST_CASE("Comment: constructed, or created as a comment node, with the data in the document",
            "[DOM][Comment][dom:interface-comment][dom:dom-comment-comment][dom:create-a-comment-node]")
  {
    auto document = CreateRef<dom::Document>();

    auto constructed = CreateRef<Comment>(*document, u8"data");
    CHECK(constructed->Data() == u8"data");
    CHECK(&constructed->NodeDocument() == document.get());
    CHECK(constructed->ParentNode() == nullptr);
    CHECK(CreateRef<Comment>(*document)->Data() == u8"");

    auto created = document->CreateComment(u8"created");
    CHECK(created->Data() == u8"created");
    CHECK(&created->NodeDocument() == document.get());
    // Any data is allowed, even what would end the comment in markup.
    CHECK(document->CreateComment(u8"a --> b")->Data() == u8"a --> b");
  }

  TEST_CASE("Node.nodeName: a Comment's is #comment", "[DOM][Comment][dom:dom-node-nodename]")
  {
    CommonTestData data(u8"Hello, world!");
    REQUIRE(data.Node->NodeName() == u8"#comment");
  }

  TEST_CASE("Node.nodeType: a Comment is a COMMENT_NODE",
            "[DOM][Comment][dom:dom-node-nodetype][dom:dom-node-comment_node]")
  {
    CommonTestData data(u8"Hello, world!");
    REQUIRE(data.Node->NodeType() == NodeType::COMMENT_NODE);
  }
}
