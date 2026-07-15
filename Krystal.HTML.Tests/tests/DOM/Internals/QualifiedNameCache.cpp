#include "Krystal.HTML/DOM/Internals/QualifiedNameCache.hpp"
#include "Krystal.HTML/DOM/Types/QualifiedName.hpp"
#include "Krystal.HTML/DOM/Types/DOMString.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  TEST_CASE("QualifiedNameCache: Same namespace, prefix, and localName should share storage",
            "[HTML][QualifiedNameCache]")
  {
    auto qn1 = QualifiedName(DOMStringAtom(u8"http://www.w3.org/1999/xhtml"), DOMStringAtom::Null(),
                             DOMStringAtom(u8"div"));
    auto qn2 = QualifiedName(DOMStringAtom(u8"http://www.w3.org/1999/xhtml"), DOMStringAtom::Null(),
                             DOMStringAtom(u8"div"));

    REQUIRE(qn1 == qn2);
  }

  TEST_CASE("QualifiedNameCache: Different localNames should have different storage",
            "[HTML][QualifiedNameCache]")
  {
    auto qn1 = QualifiedName(DOMStringAtom(u8"http://www.w3.org/1999/xhtml"), DOMStringAtom::Null(),
                             DOMStringAtom(u8"div"));
    auto qn2 = QualifiedName(DOMStringAtom(u8"http://www.w3.org/1999/xhtml"), DOMStringAtom::Null(),
                             DOMStringAtom(u8"span"));

    REQUIRE(qn1 != qn2);
  }

  TEST_CASE("QualifiedNameCache: Different namespaces should have different storage",
            "[HTML][QualifiedNameCache]")
  {
    auto qn1 = QualifiedName(DOMStringAtom(u8"http://www.w3.org/1999/xhtml"), DOMStringAtom::Null(),
                             DOMStringAtom(u8"div"));
    auto qn2 = QualifiedName(DOMStringAtom(u8"http://www.w3.org/2000/svg"), DOMStringAtom::Null(),
                             DOMStringAtom(u8"div"));

    REQUIRE(qn1 != qn2);
  }

  TEST_CASE("QualifiedNameCache: Different prefixes should have different storage",
            "[HTML][QualifiedNameCache]")
  {
    auto qn1 = QualifiedName(DOMStringAtom(u8"http://www.w3.org/1999/xhtml"), DOMStringAtom::Null(),
                             DOMStringAtom(u8"div"));
    auto qn2 = QualifiedName(DOMStringAtom(u8"http://www.w3.org/1999/xhtml"), DOMStringAtom(u8"xhtml"),
                             DOMStringAtom(u8"div"));

    REQUIRE(qn1 != qn2);
  }

  TEST_CASE("QualifiedNameCache: Multiple creations with same parameters should be equal",
            "[HTML][QualifiedNameCache]")
  {
    auto qn1 = QualifiedName(DOMStringAtom(u8"http://www.w3.org/1999/xhtml"), DOMStringAtom::Null(),
                             DOMStringAtom(u8"div"));
    auto qn2 = QualifiedName(DOMStringAtom(u8"http://www.w3.org/1999/xhtml"), DOMStringAtom::Null(),
                             DOMStringAtom(u8"div"));
    auto qn3 = QualifiedName(DOMStringAtom(u8"http://www.w3.org/1999/xhtml"), DOMStringAtom::Null(),
                             DOMStringAtom(u8"div"));

    REQUIRE(qn1 == qn2);
    REQUIRE(qn2 == qn3);
    REQUIRE(qn1 == qn3);
  }

  TEST_CASE("QualifiedNameCache: QualifiedName with TagName and Namespace", "[HTML][QualifiedNameCache]")
  {
    auto qn1 = QualifiedName(DOMStringAtom(u8"http://www.w3.org/1999/xhtml"), DOMStringAtom::Null(),
                             DOMStringAtom(u8"div"), TagName::div, Namespace::HTML);
    auto qn2 = QualifiedName(DOMStringAtom(u8"http://www.w3.org/1999/xhtml"), DOMStringAtom::Null(),
                             DOMStringAtom(u8"div"), TagName::div, Namespace::HTML);

    REQUIRE(qn1 == qn2);
    REQUIRE(qn1.TagName() == TagName::div);
    REQUIRE(qn1.Namespace() == Namespace::HTML);
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
    REQUIRE(qn.LocalNameLower() == DOMStringAtom(u8"div"));
  }

  TEST_CASE("QualifiedNameCache: LocalNameLower is correctly computed", "[HTML][QualifiedNameCache]")
  {
    auto qn = QualifiedName(DOMStringAtom(u8"http://www.w3.org/1999/xhtml"), DOMStringAtom::Null(),
                            DOMStringAtom(u8"DIV"));

    REQUIRE(qn.LocalNameLower() == DOMStringAtom(u8"div"));
  }

  TEST_CASE("QualifiedNameCache: LocalNameUpper is correctly computed", "[HTML][QualifiedNameCache]")
  {
    auto qn = QualifiedName(DOMStringAtom(u8"http://www.w3.org/1999/xhtml"), DOMStringAtom::Null(),
                            DOMStringAtom(u8"div"));

    REQUIRE(qn.LocalNameUpper() == DOMStringAtom(u8"DIV"));
  }

  TEST_CASE("QualifiedNameCache: Null namespace and prefix", "[HTML][QualifiedNameCache]")
  {
    auto qn1 = QualifiedName(DOMStringAtom::Null(), DOMStringAtom::Null(), DOMStringAtom(u8"localName"));
    auto qn2 = QualifiedName(DOMStringAtom::Null(), DOMStringAtom::Null(), DOMStringAtom(u8"localName"));

    REQUIRE(qn1 == qn2);
    REQUIRE(qn1.NamespaceURI() == DOMStringAtom::Null());
    REQUIRE(qn1.NamespacePrefix() == DOMStringAtom::Null());
  }

  TEST_CASE("QualifiedNameCache: Cache deduplication works across multiple accesses",
            "[HTML][QualifiedNameCache]")
  {
    // Create first QualifiedName
    auto qn1 = QualifiedName(DOMStringAtom(u8"http://www.w3.org/2000/svg"), DOMStringAtom::Null(),
                             DOMStringAtom(u8"circle"));

    // Create same QualifiedName multiple times
    auto qn2 = QualifiedName(DOMStringAtom(u8"http://www.w3.org/2000/svg"), DOMStringAtom::Null(),
                             DOMStringAtom(u8"circle"));
    auto qn3 = QualifiedName(DOMStringAtom(u8"http://www.w3.org/2000/svg"), DOMStringAtom::Null(),
                             DOMStringAtom(u8"circle"));
    auto qn4 = QualifiedName(DOMStringAtom(u8"http://www.w3.org/2000/svg"), DOMStringAtom::Null(),
                             DOMStringAtom(u8"circle"));

    // All should point to the same storage
    REQUIRE(qn1 == qn2);
    REQUIRE(qn2 == qn3);
    REQUIRE(qn3 == qn4);
    REQUIRE(qn1 == qn4);
  }

  TEST_CASE("QualifiedNameCache: Name property with prefix", "[HTML][QualifiedNameCache]")
  {
    auto qn = QualifiedName(DOMStringAtom(u8"http://www.w3.org/1999/xhtml"), DOMStringAtom(u8"custom"),
                            DOMStringAtom(u8"element"));

    REQUIRE(qn.Name() == u8"custom:element");
  }

  TEST_CASE("QualifiedNameCache: Name property without prefix", "[HTML][QualifiedNameCache]")
  {
    auto qn = QualifiedName(DOMStringAtom(u8"http://www.w3.org/1999/xhtml"), DOMStringAtom::Null(),
                            DOMStringAtom(u8"element"));

    REQUIRE(qn.Name() == u8"element");
  }
}
