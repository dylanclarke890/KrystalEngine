#include "Krystal.HTML/DOM/StaticRange.hpp"
#include "Krystal.HTML.Tests/HTML/TestElement.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/DOM/DocumentType.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  TEST_CASE("StaticRangeInit::Create", "[HTML][StaticRange]")
  {
    auto document = CreateRef<Document>();
    auto parent = CreateRef<TestElement>(*document);

    auto doctype = document->Implementation().CreateDocumentType(u8"html", u8"", u8"");
    auto attr = document->CreateAttribute(u8"test");
    REQUIRE_FALSE(doctype.HasException());
    REQUIRE_FALSE(attr.HasException());

    auto childA = CreateRef<TestElement>(*document);
    auto childB = CreateRef<TestElement>(*document);

    REQUIRE_FALSE(parent->AppendChild(*childA).HasException());
    REQUIRE_FALSE(parent->AppendChild(*childB).HasException());

    auto validRangeInit = StaticRangeInit::Create({childA, 0}, {childB, 0});
    REQUIRE_FALSE(validRangeInit.HasException());

    auto invalidRangeAttrLeft = StaticRangeInit::Create({attr.Value(), 0}, {childB, 0});
    REQUIRE(invalidRangeAttrLeft.HasException());

    auto invalidRangeAttrRight = StaticRangeInit::Create({childA, 0}, {attr.Value(), 0});
    REQUIRE(invalidRangeAttrRight.HasException());

    auto invalidRangeDocTypeLeft = StaticRangeInit::Create({doctype.Value(), 0}, {childB, 0});
    REQUIRE(invalidRangeDocTypeLeft.HasException());

    auto invalidRangeDocTypeRight = StaticRangeInit::Create({childA, 0}, {doctype.Value(), 0});
    REQUIRE(invalidRangeDocTypeRight.HasException());

    REQUIRE_FALSE(parent->RemoveChild(*childA).HasException());
    REQUIRE_FALSE(parent->RemoveChild(*childB).HasException());
  }

  TEST_CASE("StaticRange::IsValid", "[HTML][StaticRange]")
  {
    auto document = CreateRef<Document>();
    auto parent = CreateRef<TestElement>(*document);
    auto childA = CreateRef<TestElement>(*document);
    auto childB = CreateRef<TestElement>(*document);

    REQUIRE_FALSE(parent->AppendChild(*childA).HasException());
    REQUIRE_FALSE(parent->AppendChild(*childB).HasException());

    SECTION("valid range")
    {
      auto validRangeInit = StaticRangeInit::Create({childA, 0}, {childB, 0});
      REQUIRE_FALSE(validRangeInit.HasException());

      auto range = CreateRef<StaticRange>(validRangeInit.Value());
      REQUIRE(range->IsValid());
    }

    SECTION("invalid range - different trees")
    {
      auto otherParent = CreateRef<TestElement>(*document);
      auto otherChild = CreateRef<TestElement>(*document);

      REQUIRE_FALSE(otherParent->AppendChild(*otherChild).HasException());

      auto invalidRangeInit = StaticRangeInit::Create({childA, 0}, {otherChild, 0});
      REQUIRE_FALSE(invalidRangeInit.HasException());

      auto range = CreateRef<StaticRange>(invalidRangeInit.Value());
      REQUIRE_FALSE(range->IsValid());
    }

    SECTION("invalid range - offset out of bounds")
    {
      auto invalidRangeInit = StaticRangeInit::Create({childA, 0}, {childB, 1});
      REQUIRE_FALSE(invalidRangeInit.HasException());

      auto range = CreateRef<StaticRange>(invalidRangeInit.Value());
      REQUIRE_FALSE(range->IsValid());
    }

    REQUIRE_FALSE(parent->RemoveChild(*childA).HasException());
    REQUIRE_FALSE(parent->RemoveChild(*childB).HasException());
  }
}