#include "Krystal.Booey/Dom/Mixins/NonDocumentTypeChildNode.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Dom/Nodes/Attr.hpp"
#include "Krystal.Booey/Dom/Nodes/CharacterData.hpp"
#include "Krystal.Booey/Dom/Nodes/Element.hpp"
#include "Krystal.Booey/Dom/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/Dom/Nodes/Text.hpp"
#include "Krystal.Booey/Html/Elements/HTMLSlotElement.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    struct NonDocumentTypeChildNodeTestData
    {
      Ref<Document> Document;
      Ref<Element> Parent;
      Ref<Element> Child1;
      Ref<krys::boo::Text> Child2;
      Ref<Element> Child3;

      NonDocumentTypeChildNodeTestData()
          : Document(CreateRef<HTMLDocument>()), Parent(Document->CreateElement(u8"div").Value()),
            Child1(Document->CreateElement(u8"p").Value()), Child2(Document->CreateTextNode(u8"Hello")),
            Child3(Document->CreateElement(u8"span").Value())
      {
        REQUIRE_FALSE(Parent->AppendChild(*Child1).HasException());
        REQUIRE_FALSE(Parent->AppendChild(*Child2).HasException());
        REQUIRE_FALSE(Parent->AppendChild(*Child3).HasException());
      }

      ~NonDocumentTypeChildNodeTestData() noexcept
      {
        REQUIRE_FALSE(Parent->RemoveChild(*Child1).HasException());
        REQUIRE_FALSE(Parent->RemoveChild(*Child2).HasException());
        REQUIRE_FALSE(Parent->RemoveChild(*Child3).HasException());
      }
    };
  }

  TEST_CASE("previousElementSibling: the first preceding sibling that is an element; otherwise null",
            "[DOM][NonDocumentTypeChildNode][dom:interface-nondocumenttypechildnode]"
            "[dom:dom-nondocumenttypechildnode-previouselementsibling]")
  {
    NonDocumentTypeChildNodeTestData data;

    // The nearest one, and none without a parent.
    auto nearest = data.Document->CreateElement(u8"b").Value();
    REQUIRE_FALSE(data.Parent->InsertBefore(*nearest, data.Child2.get()).HasException());
    CHECK(mixins::NonDocumentTypeChildNode::PreviousElementSibling(*data.Child3) == nearest);
    REQUIRE_FALSE(nearest->Remove().HasException());
    CHECK(mixins::NonDocumentTypeChildNode::PreviousElementSibling(*nearest) == nullptr);

    REQUIRE(mixins::NonDocumentTypeChildNode::PreviousElementSibling(*data.Child1) == nullptr);
    REQUIRE(mixins::NonDocumentTypeChildNode::PreviousElementSibling(*data.Child2) == data.Child1);
    REQUIRE(mixins::NonDocumentTypeChildNode::PreviousElementSibling(*data.Child3) == data.Child1);
  }

  TEST_CASE("nextElementSibling: the first following sibling that is an element; otherwise null",
            "[DOM][NonDocumentTypeChildNode][dom:dom-nondocumenttypechildnode-nextelementsibling]")
  {
    NonDocumentTypeChildNodeTestData data;

    // The nearest one, and none without a parent.
    auto nearest = data.Document->CreateElement(u8"b").Value();
    REQUIRE_FALSE(data.Parent->InsertBefore(*nearest, data.Child2.get()).HasException());
    CHECK(mixins::NonDocumentTypeChildNode::NextElementSibling(*data.Child1) == nearest);
    REQUIRE_FALSE(nearest->Remove().HasException());
    CHECK(mixins::NonDocumentTypeChildNode::NextElementSibling(*nearest) == nullptr);

    REQUIRE(mixins::NonDocumentTypeChildNode::NextElementSibling(*data.Child1) == data.Child3);
    REQUIRE(mixins::NonDocumentTypeChildNode::NextElementSibling(*data.Child2) == data.Child3);
    REQUIRE(mixins::NonDocumentTypeChildNode::NextElementSibling(*data.Child3) == nullptr);
  }
}
