#include "Krystal.Booey/HTML/Elements/HTMLTemplateElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/HTML/Parser/DocumentParser.hpp"

namespace krys::boo::html::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<dom::HTMLDocument> ParseTemplateDocument(utf8_string &&input) noexcept
    {
      auto document = CreateRef<dom::HTMLDocument>();
      auto parser = CreateRef<DocumentParser>(*document);
      parser->InputStream().Append(krys::move(input), IsEOF(true));
      (void)parser->PumpTokenizer();
      return document;
    }

    KRYS_NODISCARD std::string InnerHTMLOf(dom::Document &document, dom::DOMStringView id) noexcept
    {
      auto html = document.GetElementById(id)->InnerHTML().ReleaseValue();
      return std::string(html.begin(), html.end());
    }
  }

  TEST_CASE("HTMLTemplateElement: a template for a marker puts its children in the marker's place",
            "[HTML][HTMLTemplateElement][html:prepare-content-patching][html:find-markers]")
  {
    auto document = ParseTemplateDocument(
      u8"<!DOCTYPE html><div id=t>a<?marker name=\"x\"?>b</div><template for=x><p>new</p></template>");
    CHECK(InnerHTMLOf(*document, u8"t") == "a<p>new</p>b");
    CHECK_FALSE(document->QuerySelector(u8"template").ReleaseValue());
  }

  TEST_CASE("HTMLTemplateElement: a template for a start marker replaces what's up to its end marker",
            "[HTML][HTMLTemplateElement][html:prepare-content-patching][html:find-markers]")
  {
    SECTION("The end marker after the start ends the range")
    {
      auto document =
        ParseTemplateDocument(u8"<!DOCTYPE html><ul id=t><li>keep</li><?start name=\"list\"?><li>old</li>"
                              u8"<?end?><li>after</li></ul><template for=list><li>new</li></template>");
      CHECK(InnerHTMLOf(*document, u8"t") == "<li>keep</li><li>new</li><li>after</li>");
    }

    SECTION("Nested start and end markers are skipped over")
    {
      auto document = ParseTemplateDocument(
        u8"<!DOCTYPE html><div id=t><?start "
        u8"name=\"x\"?>a<?start?>b<?end?>c<?end?>d</div><template for=x>new</template>");
      CHECK(InnerHTMLOf(*document, u8"t") == "newd");
    }

    SECTION("Without an end marker, everything after the start is replaced")
    {
      auto document =
        ParseTemplateDocument(u8"<!DOCTYPE html><div id=t><?start "
                              u8"name=\"x\"?>old<span>old</span></div><template for=x>new</template>");
      CHECK(InnerHTMLOf(*document, u8"t") == "new");
    }
  }

  TEST_CASE("HTMLTemplateElement: a template for no marker is an ordinary template",
            "[HTML][HTMLTemplateElement][html:prepare-content-patching]")
  {
    auto check = [](utf8_string &&markup)
    {
      auto document =
        ParseTemplateDocument(u8"<!DOCTYPE html><div id=t><?marker name=\"x\"?></div>" + markup);
      auto templateElement = document->QuerySelector(u8"template").ReleaseValue();
      REQUIRE(templateElement);
      CHECK(Downcast<HTMLTemplateElement>(*templateElement).Content()->FirstChild());
      CHECK(InnerHTMLOf(*document, u8"t") == "<?marker name=\"x\">");
    };

    SECTION("A name with no marker")
    {
      check(u8"<template for=missing><p>x</p></template>");
    }

    SECTION("An empty name")
    {
      check(u8"<template for><p>x</p></template>");
    }
  }

  TEST_CASE("HTMLTemplateElement: a deep clone clones the template contents",
            "[HTML][HTMLTemplateElement][dom:concept-node-clone-ext]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    auto element = document->CreateElement(u8"template").ReleaseValue();
    auto &templateElement = Downcast<HTMLTemplateElement>(*element);
    auto paragraph = document->CreateElement(u8"p").ReleaseValue();
    REQUIRE_FALSE(paragraph->AppendChild(*document->CreateTextNode(u8"text")).HasException());
    REQUIRE_FALSE(templateElement.Content()->AppendChild(*paragraph).HasException());

    auto shallow = element->CloneNode().ReleaseValue();
    CHECK_FALSE(Downcast<HTMLTemplateElement>(*shallow).Content()->HasChildNodes());

    auto deep = element->CloneNode(true).ReleaseValue();
    auto deepContent = Downcast<HTMLTemplateElement>(*deep).Content();
    REQUIRE(deepContent->FirstChild() != nullptr);
    CHECK(deepContent->FirstChild() != paragraph.get());
    CHECK(deepContent->IsEqualNode(templateElement.Content().get()));
    CHECK(&deepContent->FirstChild()->NodeDocument() == &deepContent->NodeDocument());
  }

  TEST_CASE("HTMLTemplateElement: content is a fragment hosted by the template",
            "[HTML][HTMLTemplateElement][html:template-contents][html:dom-template-content]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    auto element = document->CreateElement(u8"template").ReleaseValue();
    auto content = Downcast<HTMLTemplateElement>(*element).Content();
    REQUIRE(content);
    CHECK(content->Host() == element.get());
    CHECK(&content->NodeDocument() == document.get());
    CHECK_FALSE(content->ParentNode());
  }

  TEST_CASE("HTMLTemplateElement: adopting a template adopts its contents",
            "[HTML][HTMLTemplateElement][html:insertion-end-marker]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    auto other = CreateRef<dom::HTMLDocument>();
    auto element = document->CreateElement(u8"template").ReleaseValue();
    auto content = Downcast<HTMLTemplateElement>(*element).Content();
    REQUIRE_FALSE(content->AppendChild(*document->CreateElement(u8"p").ReleaseValue()).HasException());

    REQUIRE_FALSE(other->AdoptNode(*element).HasException());
    CHECK(&content->NodeDocument() == other.get());
    CHECK(&content->FirstChild()->NodeDocument() == other.get());
  }

  TEST_CASE("HTMLTemplateElement: the shadow root attributes reflect only known values",
            "[HTML][HTMLTemplateElement][html:dom-template-shadowrootmode]"
            "[html:dom-template-shadowrootslotassignment]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    auto element = document->CreateElement(u8"template").ReleaseValue();
    auto &templateElement = Downcast<HTMLTemplateElement>(*element);

    CHECK(templateElement.ShadowRootMode().empty());
    templateElement.ShadowRootMode(u8"OPEN");
    CHECK(templateElement.ShadowRootMode() == u8"open");
    templateElement.ShadowRootMode(u8"closed");
    CHECK(templateElement.ShadowRootMode() == u8"closed");
    templateElement.ShadowRootMode(u8"nope");
    CHECK(templateElement.ShadowRootMode().empty());
    CHECK(templateElement.GetAttribute(u8"shadowrootmode") == u8"nope");

    CHECK(templateElement.ShadowRootSlotAssignment() == u8"named");
    templateElement.ShadowRootSlotAssignment(u8"Manual");
    CHECK(templateElement.ShadowRootSlotAssignment() == u8"manual");
    templateElement.ShadowRootSlotAssignment(u8"nope");
    CHECK(templateElement.ShadowRootSlotAssignment() == u8"named");

    // shadowRootCustomElementRegistry and htmlFor reflect any value.
    templateElement.ShadowRootCustomElementRegistry(u8"x");
    CHECK(templateElement.ShadowRootCustomElementRegistry() == u8"x");
    templateElement.HtmlFor(u8"marker");
    CHECK(templateElement.GetAttribute(u8"for") == u8"marker");
    CHECK(templateElement.HtmlFor() == u8"marker");
  }
}
