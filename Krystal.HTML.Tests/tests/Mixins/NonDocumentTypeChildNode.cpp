#include "Krystal.HTML/DOM/Mixins/NonDocumentTypeChildNode.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CharacterData.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/DOM/HTMLDocument.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
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
      Ref<Element> Parent;
      Ref<Element> Child1;
      Ref<HTML::Text> Child2;
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

    REQUIRE(Mixins::NonDocumentTypeChildNode::PreviousElementSibling(*data.Child1) == nullptr);
    REQUIRE(Mixins::NonDocumentTypeChildNode::PreviousElementSibling(*data.Child2) == data.Child1);
    REQUIRE(Mixins::NonDocumentTypeChildNode::PreviousElementSibling(*data.Child3) == data.Child1);
  }

  TEST_CASE("NonDocumentTypeChildNode::NextElementSibling returns the first following sibling that is an "
            "element; otherwise null",
            "[Mixins][NonDocumentTypeChildNode]")
  {
    CommonTestData data;

    REQUIRE(Mixins::NonDocumentTypeChildNode::NextElementSibling(*data.Child1) == data.Child3);
    REQUIRE(Mixins::NonDocumentTypeChildNode::NextElementSibling(*data.Child2) == data.Child3);
    REQUIRE(Mixins::NonDocumentTypeChildNode::NextElementSibling(*data.Child3) == nullptr);
  }
}
