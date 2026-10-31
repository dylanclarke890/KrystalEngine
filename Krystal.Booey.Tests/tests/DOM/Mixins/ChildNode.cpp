#include "Krystal.Booey/DOM/Mixins/ChildNode.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/CharacterData.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

namespace krys::boo::dom::mixins::tests
{
  namespace
  {
    /// @brief A parent with the children a, b and c, and a spare element x.
    struct ChildNodeTestData
    {
      Ref<HTMLDocument> Document {CreateRef<HTMLDocument>()};
      Ref<Element> Parent {Document->CreateElement(u8"div").Value()};
      Ref<Element> A {NewElement(u8"a")};
      Ref<Element> B {NewElement(u8"b")};
      Ref<Element> C {NewElement(u8"c")};
      Ref<Element> X {NewElement(u8"x")};

      ChildNodeTestData() noexcept
      {
        for (auto *child : {A.get(), B.get(), C.get()})
        {
          REQUIRE_FALSE(Parent->AppendChild(*child).HasException());
        }
      }

      KRYS_NODISCARD Ref<Element> NewElement(DOMStringView id) noexcept
      {
        auto element = Document->CreateElement(u8"span").ReleaseValue();
        element->Id(DOMString(id));
        return element;
      }

      /// @brief The parent's children's IDs, or `#text` for text, comma-separated.
      KRYS_NODISCARD DOMString Children() const noexcept
      {
        DOMString result;
        for (auto *child = Parent->FirstChild(); child; child = child->NextSibling())
        {
          if (!result.empty())
          {
            result += u8",";
          }
          auto *element = DynamicDowncast<Element>(*child);
          result += element ? DOMString(element->Id()) : DOMString(u8"#text");
        }
        return result;
      }
    };
  }

  TEST_CASE("before(): the nodes go before this, after its first preceding sibling not among them",
            "[DOM][ChildNode][dom:interface-childnode][dom:dom-childnode-before]")
  {
    ChildNodeTestData data;

    SECTION("Before this, with strings as text")
    {
      REQUIRE_FALSE(mixins::ChildNode::Before(*data.B, {data.X, u8"text"}).HasException());
      CHECK(data.Children() == u8"a,x,#text,b,c");
    }

    SECTION("A preceding sibling among the nodes is skipped")
    {
      REQUIRE_FALSE(mixins::ChildNode::Before(*data.B, {data.X, data.A}).HasException());
      CHECK(data.Children() == u8"x,a,b,c");
    }

    SECTION("Without a parent, nothing happens")
    {
      REQUIRE_FALSE(mixins::ChildNode::Before(*data.X, {u8"text"}).HasException());
      CHECK(data.Children() == u8"a,b,c");
    }
  }

  TEST_CASE("after(): the nodes go after this, before its first following sibling not among them",
            "[DOM][ChildNode][dom:dom-childnode-after]")
  {
    ChildNodeTestData data;

    SECTION("After this, with strings as text")
    {
      REQUIRE_FALSE(mixins::ChildNode::After(*data.B, {data.X, u8"text"}).HasException());
      CHECK(data.Children() == u8"a,b,x,#text,c");
    }

    SECTION("A following sibling among the nodes is skipped")
    {
      REQUIRE_FALSE(mixins::ChildNode::After(*data.B, {data.C, data.X}).HasException());
      CHECK(data.Children() == u8"a,b,c,x");
    }

    SECTION("Without a parent, nothing happens")
    {
      REQUIRE_FALSE(mixins::ChildNode::After(*data.X, {u8"text"}).HasException());
      CHECK(data.Children() == u8"a,b,c");
    }
  }

  TEST_CASE("replaceWith(): the nodes replace this, or go before its first following sibling not among them",
            "[DOM][ChildNode][dom:dom-childnode-replacewith]")
  {
    ChildNodeTestData data;

    SECTION("In place of this, with strings as text")
    {
      REQUIRE_FALSE(mixins::ChildNode::ReplaceWith(*data.B, {data.X, u8"text"}).HasException());
      CHECK(data.Children() == u8"a,x,#text,c");
      CHECK(data.B->ParentNode() == nullptr);
    }

    SECTION("Among the nodes itself, this is moved with them")
    {
      REQUIRE_FALSE(mixins::ChildNode::ReplaceWith(*data.B, {data.X, data.B}).HasException());
      CHECK(data.Children() == u8"a,x,b,c");
    }

    SECTION("A following sibling among the nodes is skipped")
    {
      REQUIRE_FALSE(mixins::ChildNode::ReplaceWith(*data.B, {data.C, data.B}).HasException());
      CHECK(data.Children() == u8"a,c,b");
    }

    SECTION("Without a parent, nothing happens")
    {
      REQUIRE_FALSE(mixins::ChildNode::ReplaceWith(*data.X, {u8"text"}).HasException());
      CHECK(data.Children() == u8"a,b,c");
    }
  }

  TEST_CASE("remove(): removes this from its parent, if it has one",
            "[DOM][ChildNode][dom:dom-childnode-remove]")
  {
    ChildNodeTestData data;

    REQUIRE_FALSE(mixins::ChildNode::Remove(*data.B).HasException());
    CHECK(data.Children() == u8"a,c");
    CHECK(data.B->ParentNode() == nullptr);

    REQUIRE_FALSE(mixins::ChildNode::Remove(*data.B).HasException());
  }
}
