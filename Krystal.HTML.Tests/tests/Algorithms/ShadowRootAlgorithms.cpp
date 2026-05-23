#include "Krystal.HTML/Algorithms/ShadowRootAlgorithms.hpp"
#include "Krystal.HTML.Tests/TestNode.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  TEST_CASE("ShadowRootAlgorithms::ShadowIncludingRoot", "[HTML][ShadowRootAlgorithms]")
  {
    auto doc = CreateRef<Document>();
    auto parent = CreateRef<TestNode>(*doc);
    auto child = CreateRef<TestNode>(*doc);

    REQUIRE(&ShadowRootAlgorithms::ShadowIncludingRoot(*doc) == doc.get());
    REQUIRE(&ShadowRootAlgorithms::ShadowIncludingRoot(*parent) == parent.get());
    REQUIRE(&ShadowRootAlgorithms::ShadowIncludingRoot(*child) == child.get());

    auto appendResult = parent->AppendChild(*child);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(&ShadowRootAlgorithms::ShadowIncludingRoot(*doc) == doc.get());
    REQUIRE(&ShadowRootAlgorithms::ShadowIncludingRoot(*parent) == parent.get());
    REQUIRE(&ShadowRootAlgorithms::ShadowIncludingRoot(*child) == parent.get());

    appendResult = doc->AppendChild(*parent);
    REQUIRE_FALSE(appendResult.HasException());

    REQUIRE(&ShadowRootAlgorithms::ShadowIncludingRoot(*doc) == doc.get());
    REQUIRE(&ShadowRootAlgorithms::ShadowIncludingRoot(*parent) == doc.get());
    REQUIRE(&ShadowRootAlgorithms::ShadowIncludingRoot(*child) == doc.get());

    // TODO(test): Test with ShadowRoot when implemented

    auto removeResult = parent->RemoveChild(*child);
    REQUIRE_FALSE(removeResult.HasException());
    removeResult = doc->RemoveChild(*parent);
    REQUIRE_FALSE(removeResult.HasException());
  }
}