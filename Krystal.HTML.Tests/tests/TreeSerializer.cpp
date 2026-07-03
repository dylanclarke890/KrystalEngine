#include "Krystal.HTML.Tests/TreeSerializer.hpp"
#include <catch_all.hpp>

namespace Krys::HTML::Tests
{
  namespace
  {
    Ref<Element> CreateElement(Document &document, const DOMString &tagName)
    {
      auto element = document.CreateElement(tagName);
      REQUIRE(element.HasValue());
      return *element;
    }
  }

  TEST_CASE("DumpNode should serialize a simple HTML document correctly",
            "[HTML][HTMLTestUtils][TreeSerializer]")
  {
    Ref<Document> document = CreateRef<Document>();
    auto html = CreateElement(*document, u8"html");
    auto head = CreateElement(*document, u8"head");
    auto body = CreateElement(*document, u8"body");
    auto title = CreateElement(*document, u8"title");
    auto text = document->CreateTextNode(u8"Hello, World!");

    REQUIRE_FALSE(title->AppendChild(*text).HasException());
    REQUIRE_FALSE(head->AppendChild(*title).HasException());
    REQUIRE_FALSE(html->AppendChild(*head).HasException());
    REQUIRE_FALSE(html->AppendChild(*body).HasException());
    REQUIRE_FALSE(document->AppendChild(*html).HasException());

    DOMString output = Dump(*document);

    REQUIRE_FALSE(title->RemoveChild(*text).HasException());
    REQUIRE_FALSE(head->RemoveChild(*title).HasException());
    REQUIRE_FALSE(html->RemoveChild(*head).HasException());
    REQUIRE_FALSE(html->RemoveChild(*body).HasException());
    REQUIRE_FALSE(document->RemoveChild(*html).HasException());

    DOMString expectedOutput = u8"#document\n"
                               u8"| <html>\n"
                               u8"|   <head>\n"
                               u8"|     <title>\n"
                               u8"|       \"Hello, World!\"\n"
                               u8"|   <body>\n";

    REQUIRE(output == expectedOutput);
  }
}