#include "Krystal.Booey/HTML/Parser/OpenElementStack.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/DOM/TestElement.hpp"
#include "Krystal.Booey/DOM/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/Parser/DocumentParser.hpp"

namespace krys::boo::html::tests
{
  namespace
  {
    Ref<dom::HTMLDocument> Parse(utf8_string &&input) noexcept
    {
      auto document = CreateRef<dom::HTMLDocument>();
      auto parser = CreateRef<DocumentParser>(*document);
      parser->InputStream().Append(krys::move(input), IsEOF(true));
      (void)parser->PumpTokenizer();
      return document;
    }
  }

  TEST_CASE("OpenElementStack parsing children", "[HTML][OpenElementStack][html:the-stack-of-open-elements]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
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
    auto document = Parse(u8"<style>p { color: red }</style><p>a<b>b");

    for (auto &element : dom::InclusiveDescendantRange(*document))
    {
      if (auto *asElement = DynamicDowncast<dom::Element>(&element))
      {
        REQUIRE(asElement->IsFinishedParsingChildren());
      }
    }
  }

  TEST_CASE("Document quirks mode", "[HTML][Document][html:the-initial-insertion-mode]")
  {
    REQUIRE(Parse(u8"<p>")->InQuirksMode());
    REQUIRE(Parse(u8"<!DOCTYPE html><p>")->InNoQuirksMode());
    REQUIRE(
      Parse(u8"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"><p>")->InLimitedQuirksMode());
    REQUIRE(CreateRef<dom::HTMLDocument>()->InNoQuirksMode());
  }
}
