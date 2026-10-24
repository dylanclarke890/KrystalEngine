#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    /// @brief A document with a div holding a span holding some text, as `document.body` would.
    Ref<HTMLDocument> DocumentWithTree() noexcept
    {
      auto document = CreateRef<HTMLDocument>();
      auto html = document->CreateElement(u8"html").ReleaseValue();
      auto div = document->CreateElement(u8"div").ReleaseValue();
      auto span = document->CreateElement(u8"span").ReleaseValue();
      REQUIRE_FALSE(span->AppendChild(*document->CreateTextNode(u8"text")).HasException());
      REQUIRE_FALSE(div->AppendChild(*span).HasException());
      REQUIRE_FALSE(html->AppendChild(*div).HasException());
      REQUIRE_FALSE(document->AppendChild(*html).HasException());
      return document;
    }
  }

  TEST_CASE("Node lifetime: an empty document is freed when its last reference goes", "[DOM][NodeLifetime]")
  {
    RefPtr<HTMLDocument> document = CreateRef<HTMLDocument>();
    auto weak = CreateWeakPtr<Document>(&*document);
    document = nullptr;
    CHECK_FALSE(weak);
  }

  TEST_CASE("Node lifetime: a document is freed with its tree", "[DOM][NodeLifetime]")
  {
    RefPtr<HTMLDocument> document = DocumentWithTree();
    auto weakDocument = CreateWeakPtr<Document>(&*document);
    auto weakText =
      CreateWeakPtr<Node>(&*document->DocumentElement()->FirstChild()->FirstChild()->FirstChild());
    document = nullptr;
    CHECK_FALSE(weakDocument);
    CHECK_FALSE(weakText);
  }

  TEST_CASE("Node lifetime: a detached subtree is freed with its descendants", "[DOM][NodeLifetime]")
  {
    auto document = DocumentWithTree();
    RefPtr<Node> div = ShareRefPtr(document->DocumentElement()->FirstChild());
    auto weakDiv = CreateWeakPtr<Node>(&*div);
    auto weakText = CreateWeakPtr<Node>(&*div->FirstChild()->FirstChild());
    REQUIRE_FALSE(document->DocumentElement()->RemoveChild(*div).HasException());
    div = nullptr;
    CHECK_FALSE(weakDiv);
    CHECK_FALSE(weakText);
    // The document is still whole.
    CHECK(document->DocumentElement());
  }

  TEST_CASE("Node lifetime: a node keeps its document alive", "[DOM][NodeLifetime]")
  {
    RefPtr<HTMLDocument> document = DocumentWithTree();
    auto weakDocument = CreateWeakPtr<Document>(&*document);
    RefPtr<Node> text = ShareRefPtr(document->DocumentElement()->FirstChild()->FirstChild()->FirstChild());
    document = nullptr;

    // The document's tree went with its last reference, but the node held and its document stay usable.
    REQUIRE(weakDocument);
    CHECK(&text->NodeDocument() == weakDocument.get());
    CHECK_FALSE(text->ParentNode());
    CHECK(text->TextContent() == u8"text");

    text = nullptr;
    CHECK_FALSE(weakDocument);
  }

  TEST_CASE("Node lifetime: adopting a node moves its document reference", "[DOM][NodeLifetime]")
  {
    RefPtr<HTMLDocument> first = DocumentWithTree();
    auto second = CreateRef<HTMLDocument>();
    auto weakFirst = CreateWeakPtr<Document>(&*first);
    RefPtr<Node> div = ShareRefPtr(first->DocumentElement()->FirstChild());
    REQUIRE_FALSE(second->AdoptNode(*div).HasException());
    CHECK(&div->NodeDocument() == &*second);

    // The first document no longer has a node of the adopted subtree to keep it.
    first = nullptr;
    CHECK_FALSE(weakFirst);
    CHECK(div->FirstChild()->FirstChild()->TextContent() == u8"text");
  }

  TEST_CASE("Node lifetime: a parsed document is freed", "[DOM][NodeLifetime]")
  {
    RefPtr<HTMLDocument> document = CreateRef<HTMLDocument>();
    {
      auto parser = CreateRef<html::HTMLDocumentParser>(*document);
      parser->InputStream().Append(u8"<!DOCTYPE html><title>T</title><p id=a>One<p>Two <b>three</b>",
                                   IsEOF(true));
      (void)parser->PumpTokenizer();
    }
    auto weak = CreateWeakPtr<Document>(&*document);
    auto paragraph = CreateWeakPtr<Node>(&*document->GetElementById(u8"a"));
    document = nullptr;
    CHECK_FALSE(weak);
    CHECK_FALSE(paragraph);
  }

  TEST_CASE("Node lifetime: documents with interactive elements are freed", "[DOM][NodeLifetime]")
  {
    const auto markup =
      GENERATE(as<std::string> {}, "<details><summary>S</summary>D</details>", "<details>D</details>",
               "<details name=a open></details><details name=a></details>", "<summary>S</summary>",
               "<dialog>D</dialog>", "<dialog closedby=any><form method=dialog></form></dialog>",
               "<div popover id=p></div><button popovertarget=p>B</button>",
               "<div popover=hint></div><div popover=manual></div>");
    CAPTURE(markup);
    RefPtr<HTMLDocument> document = CreateRef<HTMLDocument>();
    {
      auto parser = CreateRef<html::HTMLDocumentParser>(*document);
      parser->InputStream().Append(utf8_string(markup.begin(), markup.end()), IsEOF(true));
      (void)parser->PumpTokenizer();
    }
    // The signal slots hold slots whose assignment changed until the microtask checkpoint.
    html::EventLoop::Current().RunUntilIdle();
    auto weak = CreateWeakPtr<Document>(&*document);
    document = nullptr;
    CHECK_FALSE(weak);
  }

  TEST_CASE("Node lifetime: a document with slotted nodes is freed", "[DOM][NodeLifetime]")
  {
    RefPtr<HTMLDocument> document = DocumentWithTree();
    {
      auto &host = *document->DocumentElement()->FirstChild();
      auto shadow = Downcast<Element>(host).AttachShadow({}).ReleaseValue();
      REQUIRE_FALSE(shadow->AppendChild(*document->CreateElement(u8"slot").ReleaseValue()).HasException());
      REQUIRE(Downcast<Element>(*host.FirstChild()).AssignedSlot());
    }
    html::EventLoop::Current().RunUntilIdle();
    auto weak = CreateWeakPtr<Document>(&*document);
    document = nullptr;
    CHECK_FALSE(weak);
  }
}
