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
#include <catch_all.hpp>

namespace krys::tests
{
  using namespace krys::boo;

  namespace
  {
    struct CommonTestData
    {
      Ref<Document> Document;
      Ref<Comment> Node;

      CommonTestData(dom::DOMString &&value)
          : Document(CreateRef<HTML::Document>()), Node(CreateRef<Comment>(*Document, krys::move(value)))
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
