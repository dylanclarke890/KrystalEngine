#include "Krystal.Booey/HTML/HTMLTemplateElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Text.hpp"

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
}
