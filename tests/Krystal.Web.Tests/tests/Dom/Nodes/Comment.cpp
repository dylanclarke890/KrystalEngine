#include "Krystal.Web/Dom/Nodes/Comment.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web/Dom/Nodes/Attr.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Dom/Nodes/Node.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Dom/Nodes/Text.hpp"
#include "Krystal.Web/Html/Elements/HTMLSlotElement.hpp"

namespace krys::web::dom::tests
{
  namespace
  {
    struct CommentTestData
    {
      Ref<web::Document> Document;
      Ref<Comment> Node;

      CommentTestData(DOMString &&value)
          : Document(CreateRef<web::Document>()), Node(CreateRef<Comment>(*Document, krys::move(value)))
      {
      }
    };
  }

  TEST_CASE("Comment: constructed, or created as a comment node, with the data in the document",
            "[DOM][Comment][dom:interface-comment][dom:dom-comment-comment][dom:create-a-comment-node]")
  {
    auto document = CreateRef<Document>();

    auto constructed = CreateRef<Comment>(*document, u8"data");
    CHECK(constructed->Data() == u8"data");
    CHECK(&constructed->NodeDocument() == document.get());
    CHECK(constructed->ParentNode() == nullptr);
    CHECK(CreateRef<Comment>(*document)->Data().empty());

    auto created = document->CreateComment(u8"created");
    CHECK(created->Data() == u8"created");
    CHECK(&created->NodeDocument() == document.get());
    // Any data is allowed, even what would end the comment in markup.
    CHECK(document->CreateComment(u8"a --> b")->Data() == u8"a --> b");
  }

  TEST_CASE("Node.nodeName: a Comment's is #comment", "[DOM][Comment][dom:dom-node-nodename]")
  {
    CommentTestData data(u8"Hello, world!");
    REQUIRE(data.Node->NodeName() == u8"#comment");
  }

  TEST_CASE("Node.nodeType: a Comment is a COMMENT_NODE",
            "[DOM][Comment][dom:dom-node-nodetype][dom:dom-node-comment_node]")
  {
    CommentTestData data(u8"Hello, world!");
    REQUIRE(data.Node->NodeType() == NodeType::COMMENT_NODE);
  }
}
