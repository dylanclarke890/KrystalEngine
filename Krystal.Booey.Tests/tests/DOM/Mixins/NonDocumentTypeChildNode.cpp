#include "Krystal.Booey/DOM/Mixins/NonDocumentTypeChildNode.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/CharacterData.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
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
      Ref<Document> Document;
      Ref<Element> Parent;
      Ref<Element> Child1;
      Ref<krys::boo::dom::Text> Child2;
      Ref<Element> Child3;

      CommonTestData()
          : Document(CreateRef<HTMLDocument>()), Parent(Document->CreateElement(u8"div").Value()),
            Child1(Document->CreateElement(u8"p").Value()), Child2(Document->CreateTextNode(u8"Hello")),
            Child3(Document->CreateElement(u8"span").Value())
      {
        REQUIRE_FALSE(Parent->AppendChild(*Child1).HasException());
        REQUIRE_FALSE(Parent->AppendChild(*Child2).HasException());
        REQUIRE_FALSE(Parent->AppendChild(*Child3).HasException());
      }

      ~CommonTestData() noexcept
      {
        REQUIRE_FALSE(Parent->RemoveChild(*Child1).HasException());
        REQUIRE_FALSE(Parent->RemoveChild(*Child2).HasException());
        REQUIRE_FALSE(Parent->RemoveChild(*Child3).HasException());
      }
    };
  }

  TEST_CASE("NonDocumentTypeChildNode::PreviousElementSibling returns the first preceding sibling that is an "
            "element; otherwise null",
            "[Mixins][NonDocumentTypeChildNode]")
  {
    CommonTestData data;

    REQUIRE(mixins::NonDocumentTypeChildNode::PreviousElementSibling(*data.Child1) == nullptr);
    REQUIRE(mixins::NonDocumentTypeChildNode::PreviousElementSibling(*data.Child2) == data.Child1);
    REQUIRE(mixins::NonDocumentTypeChildNode::PreviousElementSibling(*data.Child3) == data.Child1);
  }

  TEST_CASE("NonDocumentTypeChildNode::NextElementSibling returns the first following sibling that is an "
            "element; otherwise null",
            "[Mixins][NonDocumentTypeChildNode]")
  {
    CommonTestData data;

    REQUIRE(mixins::NonDocumentTypeChildNode::NextElementSibling(*data.Child1) == data.Child3);
    REQUIRE(mixins::NonDocumentTypeChildNode::NextElementSibling(*data.Child2) == data.Child3);
    REQUIRE(mixins::NonDocumentTypeChildNode::NextElementSibling(*data.Child3) == nullptr);
  }
}
