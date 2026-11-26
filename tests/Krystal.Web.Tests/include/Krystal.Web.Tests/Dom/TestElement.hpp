#pragma once

#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Dom/Nodes/HTMLDocument.hpp"
#include "Krystal.Web/Dom/Types/QualifiedName.hpp"
#include "Krystal.Web/Infra/Namespaces.hpp"

namespace krys::web::dom::tests
{
  class TestElement : public Element
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(TestElement);

  public:
    TestElement(Document &document) noexcept
        : Element(document, {infra::namespaces::HTML, null, u8"test-element"}, {})
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
