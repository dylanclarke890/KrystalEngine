#include "Krystal.HTML/Node/NamedNodeMap.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/HTMLDocument.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  namespace
  {
    struct CommonTestData
    {
      Ref<Document> Document;
      Ref<Element> Node;
      CommonTestData()
          : Document(CreateRef<HTMLDocument>()), Node(Document->CreateElement(u8"test-element").Value())
      {
      }
    };
  }

  TEST_CASE("NamedNodeMap::Length", "[HTML][NamedNodeMap]")
  {
    CommonTestData data;

    REQUIRE(data.Node->Attributes().Length() == 0uz);

    auto attr1 = data.Document->CreateAttribute(u8"test-attr1");
    REQUIRE_FALSE(attr1.HasException());
    attr1.Value()->Value(u8"value1");
    REQUIRE_FALSE(data.Node->Attributes().SetNamedItem(*attr1.Value()).HasException());
    REQUIRE(data.Node->Attributes().Length() == 1uz);

    auto attr2 = data.Document->CreateAttribute(u8"test-attr2");
    REQUIRE_FALSE(attr2.HasException());
    attr2.Value()->Value(u8"value2");
    REQUIRE_FALSE(data.Node->Attributes().SetNamedItem(*attr2.Value()).HasException());
    REQUIRE(data.Node->Attributes().Length() == 2uz);

    REQUIRE_FALSE(data.Node->Attributes().RemoveNamedItem(u8"test-attr1").HasException());
    REQUIRE(data.Node->Attributes().Length() == 1uz);
    REQUIRE_FALSE(data.Node->Attributes().RemoveNamedItem(u8"test-attr2").HasException());
    REQUIRE(data.Node->Attributes().Length() == 0uz);
  }

  TEST_CASE("NamedNodeMap::Item", "[HTML][NamedNodeMap]")
  {
    CommonTestData data;

    REQUIRE(data.Node->Attributes().Item(0) == nullptr);
    REQUIRE(data.Node->Attributes().Item(1) == nullptr);

    auto attr1 = data.Document->CreateAttribute(u8"test-attr1");
    REQUIRE_FALSE(attr1.HasException());
    attr1.Value()->Value(u8"value1");
    REQUIRE_FALSE(data.Node->Attributes().SetNamedItem(*attr1.Value()).HasException());

    auto attr2 = data.Document->CreateAttribute(u8"test-attr2");
    REQUIRE_FALSE(attr2.HasException());
    attr2.Value()->Value(u8"value2");
    REQUIRE_FALSE(data.Node->Attributes().SetNamedItem(*attr2.Value()).HasException());

    REQUIRE(data.Node->Attributes().Item(0)->Name() == u8"test-attr1");
    REQUIRE(data.Node->Attributes().Item(0)->Value() == u8"value1");
    REQUIRE(data.Node->Attributes().Item(1)->Name() == u8"test-attr2");
    REQUIRE(data.Node->Attributes().Item(1)->Value() == u8"value2");
  }

  TEST_CASE("NamedNodeMap::GetNamedItem", "[HTML][NamedNodeMap]")
  {
    CommonTestData data;

    REQUIRE(data.Node->Attributes().GetNamedItem(u8"test-attr1") == nullptr);
    REQUIRE(data.Node->Attributes().GetNamedItem(u8"test-attr2") == nullptr);

    auto attr1 = data.Document->CreateAttribute(u8"test-attr1");
    REQUIRE_FALSE(attr1.HasException());
    attr1.Value()->Value(u8"value1");
    REQUIRE_FALSE(data.Node->Attributes().SetNamedItem(*attr1.Value()).HasException());

    auto attr2 = data.Document->CreateAttribute(u8"test-attr2");
    REQUIRE_FALSE(attr2.HasException());
    attr2.Value()->Value(u8"value2");
    REQUIRE_FALSE(data.Node->Attributes().SetNamedItem(*attr2.Value()).HasException());

    REQUIRE(data.Node->Attributes().GetNamedItem(u8"test-attr1") == *attr1);
    REQUIRE(data.Node->Attributes().GetNamedItem(u8"test-attr2") == *attr2);
  }

  TEST_CASE("NamedNodeMap::GetNamedItemNS", "[HTML][NamedNodeMap]")
  {
    CommonTestData data;

    REQUIRE(data.Node->Attributes().GetNamedItemNS(u8"http://example.com/ns", u8"test-attr") == nullptr);
    REQUIRE(data.Node->Attributes().GetNamedItemNS(u8"http://example.com/ns", u8"another-attr") == nullptr);

    auto attr1 = data.Document->CreateAttributeNS(u8"http://example.com/ns", u8"test-attr");
    REQUIRE_FALSE(attr1.HasException());
    attr1.Value()->Value(u8"value1");
    REQUIRE_FALSE(data.Node->Attributes().SetNamedItemNS(*attr1.Value()).HasException());

    auto attr2 = data.Document->CreateAttributeNS(u8"http://example.com/ns", u8"another-attr");
    REQUIRE_FALSE(attr2.HasException());
    attr2.Value()->Value(u8"value2");
    REQUIRE_FALSE(data.Node->Attributes().SetNamedItemNS(*attr2.Value()).HasException());

    REQUIRE(data.Node->Attributes().GetNamedItemNS(u8"http://example.com/ns", u8"test-attr") == *attr1);
    REQUIRE(data.Node->Attributes().GetNamedItemNS(u8"http://example.com/ns", u8"another-attr") == *attr2);
  }

  TEST_CASE("NamedNodeMap::SetNamedItem", "[HTML][NamedNodeMap]")
  {
    CommonTestData data;

    auto attr1 = data.Document->CreateAttribute(u8"test-attr1");
    REQUIRE_FALSE(attr1.HasException());
    attr1.Value()->Value(u8"value1");

    auto result = data.Node->Attributes().SetNamedItem(*attr1.Value());
    REQUIRE_FALSE(result.HasException());
    REQUIRE(result.Value() == nullptr);
    REQUIRE(data.Node->Attributes().GetNamedItem(u8"test-attr1") == *attr1);

    auto attr2 = data.Document->CreateAttribute(u8"test-attr1");
    REQUIRE_FALSE(attr2.HasException());
    attr2.Value()->Value(u8"value2");

    result = data.Node->Attributes().SetNamedItem(*attr2.Value());
    REQUIRE_FALSE(result.HasException());
    REQUIRE(result.Value() == *attr1);
    REQUIRE(data.Node->Attributes().GetNamedItem(u8"test-attr1") == *attr2);
  }

  TEST_CASE("NamedNodeMap::SetNamedItemNS", "[HTML][NamedNodeMap]")
  {
    CommonTestData data;

    auto attr1 = data.Document->CreateAttributeNS(u8"http://example.com/ns", u8"test-attr");
    REQUIRE_FALSE(attr1.HasException());
    attr1.Value()->Value(u8"value1");

    auto result = data.Node->Attributes().SetNamedItemNS(*attr1.Value());
    REQUIRE_FALSE(result.HasException());
    REQUIRE(result.Value() == nullptr);
    REQUIRE(data.Node->Attributes().GetNamedItemNS(u8"http://example.com/ns", u8"test-attr") == *attr1);

    auto attr2 = data.Document->CreateAttributeNS(u8"http://example.com/ns", u8"test-attr");
    REQUIRE_FALSE(attr2.HasException());
    attr2.Value()->Value(u8"value2");
    result = data.Node->Attributes().SetNamedItemNS(*attr2.Value());
    REQUIRE_FALSE(result.HasException());
    REQUIRE(result.Value() == *attr1);
    REQUIRE(data.Node->Attributes().GetNamedItemNS(u8"http://example.com/ns", u8"test-attr") == *attr2);
  }

  TEST_CASE("NamedNodeMap::RemoveNamedItem", "[HTML][NamedNodeMap]")
  {
    CommonTestData data;

    auto attr = data.Document->CreateAttribute(u8"test-attr");
    REQUIRE_FALSE(attr.HasException());
    attr.Value()->Value(u8"value");
    REQUIRE_FALSE(data.Node->Attributes().SetNamedItem(*attr.Value()).HasException());

    auto result = data.Node->Attributes().RemoveNamedItem(u8"test-attr");
    REQUIRE_FALSE(result.HasException());
    REQUIRE(result.Value() == *attr);
    REQUIRE(data.Node->Attributes().GetNamedItem(u8"test-attr") == nullptr);
  }

  TEST_CASE("NamedNodeMap::RemoveNamedItemNS", "[HTML][NamedNodeMap]")
  {
    CommonTestData data;

    auto attr = data.Document->CreateAttributeNS(u8"http://example.com/ns", u8"test-attr");
    REQUIRE_FALSE(attr.HasException());
    attr.Value()->Value(u8"value");
    REQUIRE_FALSE(data.Node->Attributes().SetNamedItemNS(*attr.Value()).HasException());

    auto result = data.Node->Attributes().RemoveNamedItemNS(u8"http://example.com/ns", u8"test-attr");
    REQUIRE_FALSE(result.HasException());
    REQUIRE(result.Value() == *attr);
    REQUIRE(data.Node->Attributes().GetNamedItemNS(u8"http://example.com/ns", u8"test-attr") == nullptr);
  }

  TEST_CASE("NamedNodeMap::operator[]", "[HTML][NamedNodeMap]")
  {
    CommonTestData data;
    
    auto attr1 = data.Document->CreateAttribute(u8"test-attr1");
    REQUIRE_FALSE(attr1.HasException());
    attr1.Value()->Value(u8"value1");
    REQUIRE_FALSE(data.Node->Attributes().SetNamedItem(*attr1.Value()).HasException());
    
    auto attr2 = data.Document->CreateAttribute(u8"test-attr2");
    REQUIRE_FALSE(attr2.HasException());
    attr2.Value()->Value(u8"value2");
    REQUIRE_FALSE(data.Node->Attributes().SetNamedItem(*attr2.Value()).HasException());
    
    REQUIRE(data.Node->Attributes()[0]->Name() == u8"test-attr1");
    REQUIRE(data.Node->Attributes()[0]->Value() == u8"value1");
    REQUIRE(data.Node->Attributes()[1]->Name() == u8"test-attr2");
    REQUIRE(data.Node->Attributes()[1]->Value() == u8"value2");
    REQUIRE(data.Node->Attributes()[u8"test-attr1"] == *attr1);
    REQUIRE(data.Node->Attributes()[u8"test-attr2"] == *attr2);
  }
}