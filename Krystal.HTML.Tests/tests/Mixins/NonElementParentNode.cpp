#include "Krystal.HTML/DOM/Mixins/NonElementParentNode.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/DocumentFragment.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/HTMLDocument.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  TEST_CASE("NonElementParentNode::GetElementById returns the element with the given id if it exists in the "
            "descendants of the node",
            "[NonElementParentNode]")
  {
    auto document = CreateRef<HTMLDocument>();

    auto parent = document->CreateElement(u8"parent");
    REQUIRE_FALSE(parent.HasException());

    auto div = document->CreateElement(u8"div");
    REQUIRE_FALSE(div.HasException());

    div->Id(u8"test-id");

    auto span = document->CreateElement(u8"span");
    REQUIRE_FALSE(span.HasException());

    SECTION("With Document node")
    {
      REQUIRE_FALSE(document->AppendChild(*parent.Value()).HasException());

      REQUIRE(document->GetElementById(u8"test-id") == nullptr);

      REQUIRE(parent->AppendChild(*div.Value()).HasValue());
      REQUIRE(document->GetElementById(u8"test-id") == div.Value());

      REQUIRE(parent->InsertBefore(*span.Value(), div.Value().get()).HasValue());
      REQUIRE(document->GetElementById(u8"test-id") == div.Value());

      span->Id(u8"test-id");
      REQUIRE(document->GetElementById(u8"test-id") == span.Value());

      REQUIRE_FALSE(document->RemoveChild(*parent.Value()).HasException());
      REQUIRE_FALSE(parent->RemoveChild(*div.Value()).HasException());
      REQUIRE_FALSE(parent->RemoveChild(*span.Value()).HasException());
    }

    SECTION("With DocumentFragment node")
    {
      auto fragment = document->CreateDocumentFragment();

      REQUIRE_FALSE(fragment->AppendChild(*parent.Value()).HasException());

      REQUIRE(fragment->GetElementById(u8"test-id") == nullptr);

      REQUIRE(parent->AppendChild(*div.Value()).HasValue());
      REQUIRE(fragment->GetElementById(u8"test-id") == div.Value());

      REQUIRE(parent->InsertBefore(*span.Value(), div.Value().get()).HasValue());
      REQUIRE(fragment->GetElementById(u8"test-id") == div.Value());

      span->Id(u8"test-id");
      REQUIRE(fragment->GetElementById(u8"test-id") == span.Value());

      REQUIRE_FALSE(fragment->RemoveChild(*parent.Value()).HasException());
    }
  }
}