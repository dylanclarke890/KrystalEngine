#include "Krystal.HTML/Node/Comment.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/Node.hpp"
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
      Ref<Comment> Node;

      CommonTestData(DOMString &&value)
          : Document(CreateRef<HTML::Document>()), Node(CreateRef<Comment>(*Document, Krys::Move(value)))
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
