#include "Krystal.Web/Dom/Internals/QualifiedNameCache.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web/Dom/Types/DOMString.hpp"
#include "Krystal.Web/Dom/Types/QualifiedName.hpp"

namespace krys::web::dom::tests
{
  TEST_CASE("QualifiedNameCache: Same namespace, prefix, and localName should share storage",
            "[HTML][QualifiedNameCache]")
  {
    auto qn1 = QualifiedName(u8"http://www.w3.org/1999/xhtml", null, u8"div");
    auto qn2 = QualifiedName(u8"http://www.w3.org/1999/xhtml", null, u8"div");

    REQUIRE(qn1 == qn2);
  }

  TEST_CASE("QualifiedNameCache: Different localNames should have different storage",
            "[HTML][QualifiedNameCache]")
  {
    auto qn1 = QualifiedName(u8"http://www.w3.org/1999/xhtml", null, u8"div");
    auto qn2 = QualifiedName(u8"http://www.w3.org/1999/xhtml", null, u8"span");

    REQUIRE(qn1 != qn2);
  }

  TEST_CASE("QualifiedNameCache: Different namespaces should have different storage",
            "[HTML][QualifiedNameCache]")
  {
    auto qn1 = QualifiedName(u8"http://www.w3.org/1999/xhtml", null, u8"div");
    auto qn2 = QualifiedName(u8"http://www.w3.org/2000/svg", null, u8"div");

    REQUIRE(qn1 != qn2);
  }

  TEST_CASE("QualifiedNameCache: Different prefixes should have different storage",
            "[HTML][QualifiedNameCache]")
  {
    auto qn1 = QualifiedName(u8"http://www.w3.org/1999/xhtml", null, u8"div");
    auto qn2 = QualifiedName(u8"http://www.w3.org/1999/xhtml", u8"xhtml", u8"div");

    REQUIRE(qn1 != qn2);
  }

  TEST_CASE("QualifiedNameCache: Multiple creations with same parameters should be equal",
            "[HTML][QualifiedNameCache]")
  {
    auto qn1 = QualifiedName(u8"http://www.w3.org/1999/xhtml", null, u8"div");
    auto qn2 = QualifiedName(u8"http://www.w3.org/1999/xhtml", null, u8"div");
    auto qn3 = QualifiedName(u8"http://www.w3.org/1999/xhtml", null, u8"div");

    REQUIRE(qn1 == qn2);
    REQUIRE(qn2 == qn3);
    REQUIRE(qn1 == qn3);
  }

  TEST_CASE("QualifiedNameCache: QualifiedName with TagName and Namespace", "[HTML][QualifiedNameCache]")
  {
    auto qn1 = QualifiedName(u8"http://www.w3.org/1999/xhtml", null, u8"div", html::TagName::div,
                             html::Namespace::HTML);
    auto qn2 = QualifiedName(u8"http://www.w3.org/1999/xhtml", null, u8"div", html::TagName::div,
                             html::Namespace::HTML);

    REQUIRE(qn1 == qn2);
    REQUIRE(qn1.TagName() == html::TagName::div);
    REQUIRE(qn1.Namespace() == html::Namespace::HTML);
  }

  TEST_CASE("QualifiedNameCache: Storage properties are accessible", "[HTML][QualifiedNameCache]")
  {
    auto namespaceURI = DOMStringAtom(u8"http://www.w3.org/1999/xhtml");
    auto prefix = DOMStringAtom(u8"xhtml");
    auto localName = DOMStringAtom(u8"div");

    auto qn = QualifiedName(namespaceURI, prefix, localName);

    REQUIRE(qn.NamespaceURI() == namespaceURI);
    REQUIRE(qn.NamespacePrefix() == prefix);
    REQUIRE(qn.LocalName() == localName);
    REQUIRE(qn.LocalNameLower() == u8"div");
  }

  TEST_CASE("QualifiedNameCache: LocalNameLower is correctly computed", "[HTML][QualifiedNameCache]")
  {
    auto qn = QualifiedName(u8"http://www.w3.org/1999/xhtml", null, u8"DIV");

    REQUIRE(qn.LocalNameLower() == u8"div");
  }

  TEST_CASE("QualifiedNameCache: null namespace and prefix", "[HTML][QualifiedNameCache]")
  {
    auto qn1 = QualifiedName(null, null, u8"localName");
    auto qn2 = QualifiedName(null, null, u8"localName");

    REQUIRE(qn1 == qn2);
    REQUIRE(qn1.NamespaceURI() == null);
    REQUIRE(qn1.NamespacePrefix() == null);
  }

  TEST_CASE("QualifiedNameCache: Cache deduplication works across multiple accesses",
            "[HTML][QualifiedNameCache]")
  {
    // Create first QualifiedName
    auto qn1 = QualifiedName(u8"http://www.w3.org/2000/svg", null, u8"circle");

    // Create same QualifiedName multiple times
    auto qn2 = QualifiedName(u8"http://www.w3.org/2000/svg", null, u8"circle");
    auto qn3 = QualifiedName(u8"http://www.w3.org/2000/svg", null, u8"circle");
    auto qn4 = QualifiedName(u8"http://www.w3.org/2000/svg", null, u8"circle");

    // All should point to the same storage
    REQUIRE(qn1 == qn2);
    REQUIRE(qn2 == qn3);
    REQUIRE(qn3 == qn4);
    REQUIRE(qn1 == qn4);
  }

  TEST_CASE("QualifiedNameCache: Name property with prefix", "[HTML][QualifiedNameCache]")
  {
    auto qn = QualifiedName(u8"http://www.w3.org/1999/xhtml", u8"custom", u8"element");

    REQUIRE(qn.Name() == u8"custom:element");
  }

  TEST_CASE("QualifiedNameCache: Name property without prefix", "[HTML][QualifiedNameCache]")
  {
    auto qn = QualifiedName(u8"http://www.w3.org/1999/xhtml", null, u8"element");

    REQUIRE(qn.Name() == u8"element");
  }

  TEST_CASE("AnyQualifiedName: no prefix, any namespace and any local name", "[HTML][QualifiedNameCache]")
  {
    auto any = AnyQualifiedName();
    CHECK(any.NamespacePrefix() == null);
    CHECK(any.NamespaceURI() == StarAtom());
    CHECK(any.LocalName() == StarAtom());
    CHECK(any == AnyQualifiedName());
  }

  TEST_CASE("QualifiedName: an empty namespace is no namespace", "[HTML][QualifiedNameCache]")
  {
    auto empty = QualifiedName(DOMStringAtom::Empty(), null, u8"p");
    CHECK(empty.NamespaceURI() == null);
    CHECK(empty == QualifiedName(null, null, u8"p"));
  }
}
