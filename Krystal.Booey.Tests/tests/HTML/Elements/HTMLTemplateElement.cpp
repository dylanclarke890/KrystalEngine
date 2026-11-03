#include "Krystal.Booey/HTML/Elements/HTMLTemplateElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"

namespace krys::boo::html::tests
{
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
