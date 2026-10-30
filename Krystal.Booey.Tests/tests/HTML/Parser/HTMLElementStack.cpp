#include "Krystal.Booey/HTML/Parser/HTMLElementStack.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/DOM/TestElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"

namespace krys::boo::html::tests
{
  namespace
  {
    Ref<dom::HTMLDocument> Parse(utf8_string &&input) noexcept
    {
      auto document = CreateRef<dom::HTMLDocument>();
      auto parser = CreateRef<HTMLDocumentParser>(*document);
      parser->InputStream().Append(krys::move(input), IsEOF(true));
      (void)parser->PumpTokenizer();
      return document;
    }
  }

  TEST_CASE("HTMLElementStack parsing children", "[HTML][HTMLElementStack]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    auto element = CreateRef<dom::tests::TestElement>(*document);
    REQUIRE(element->IsFinishedParsingChildren());

    HTMLElementStack stack;

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

  TEST_CASE("HTMLDocumentParser finishes every element", "[HTML][HTMLElementStack]")
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

  TEST_CASE("Document quirks mode", "[HTML][Document]")
  {
    REQUIRE(Parse(u8"<p>")->InQuirksMode());
    REQUIRE(Parse(u8"<!DOCTYPE html><p>")->InNoQuirksMode());
    REQUIRE(
      Parse(u8"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"><p>")->InLimitedQuirksMode());
    REQUIRE(CreateRef<dom::HTMLDocument>()->InNoQuirksMode());
  }
}
