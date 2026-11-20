#include "Krystal.Booey/HTML/Parser/OpenElementStack.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/DOM/DOMTestUtils.hpp"
#include "Krystal.Booey.Tests/DOM/TestElement.hpp"
#include "Krystal.Booey/DOM/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/Parser/DocumentParser.hpp"

namespace krys::boo::html::tests
{
  using boo::tests::ParseDocument;

  TEST_CASE("OpenElementStack parsing children", "[HTML][OpenElementStack][html:the-stack-of-open-elements]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto element = CreateRef<dom::tests::TestElement>(*document);
    REQUIRE(element->IsFinishedParsingChildren());

    OpenElementStack stack;

    SECTION("Pushing begins and popping finishes")
    {
      stack.Push({TagName::Unknown, Namespace::HTML, *element, {}});
      REQUIRE_FALSE(element->IsFinishedParsingChildren());

      stack.Pop();
      REQUIRE(element->IsFinishedParsingChildren());
    }

    SECTION("Removing finishes")
    {
      auto other = CreateRef<dom::tests::TestElement>(*document);
      stack.Push({TagName::Unknown, Namespace::HTML, *element, {}});
      stack.Push({TagName::Unknown, Namespace::HTML, *other, {}});

      stack.Remove(*element);
      REQUIRE(element->IsFinishedParsingChildren());
      REQUIRE_FALSE(other->IsFinishedParsingChildren());
    }

    SECTION("Inserting below begins")
    {
      auto other = CreateRef<dom::tests::TestElement>(*document);
      stack.Push({TagName::Unknown, Namespace::HTML, *element, {}});

      stack.InsertBelow({TagName::Unknown, Namespace::HTML, *other, {}}, *element);
      REQUIRE_FALSE(other->IsFinishedParsingChildren());
    }
  }

  TEST_CASE("DocumentParser finishes every element",
            "[HTML][OpenElementStack][html:the-stack-of-open-elements]")
  {
    auto document = ParseDocument(u8"<style>p { color: red }</style><p>a<b>b");

    for (auto &element : dom::InclusiveDescendantRange(*document))
    {
      if (auto *asElement = DynamicDowncast<Element>(&element))
      {
        REQUIRE(asElement->IsFinishedParsingChildren());
      }
    }
  }

  TEST_CASE("Document quirks mode", "[HTML][Document][html:the-initial-insertion-mode]")
  {
    REQUIRE(ParseDocument(u8"<p>")->InQuirksMode());
    REQUIRE(ParseDocument(u8"<!DOCTYPE html><p>")->InNoQuirksMode());
    REQUIRE(ParseDocument(u8"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"><p>")
              ->InLimitedQuirksMode());
    REQUIRE(CreateRef<HTMLDocument>()->InNoQuirksMode());
  }
}
