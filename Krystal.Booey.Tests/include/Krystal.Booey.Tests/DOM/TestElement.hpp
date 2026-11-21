#pragma once

#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"

namespace krys::boo::dom::tests
{
  class TestElement : public Element
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(TestElement);

  public:
    TestElement(Document &document) noexcept
        : Element(document, {infra::namespaces::HTML, null, u8"test-element"}, dom::NodeFlags {})
    {
    }
  };

  /// @brief A document, and a test element of it that isn't inserted.
  struct TestElementFixture
  {
    Ref<Document> Document;
    Ref<TestElement> Node;

    TestElementFixture() : Document(CreateRef<HTMLDocument>()), Node(CreateRef<TestElement>(*Document))
    {
    }
  };

  constexpr const char8 *TestNamespace = u8"http://example.com/ns";
}
