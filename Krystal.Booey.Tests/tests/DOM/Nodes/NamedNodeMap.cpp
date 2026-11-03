#include "Krystal.Booey/DOM/Nodes/NamedNodeMap.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Nodes/XMLDocument.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    constexpr DOMStringView TestNamespace = u8"http://example.com/ns";

    struct CommonTestData
    {
      Ref<Document> Document;
      Ref<Element> Node;

      CommonTestData()
          : Document(CreateRef<HTMLDocument>()), Node(Document->CreateElement(u8"test-element").Value())
      {
      }

      NamedNodeMap &Attributes() noexcept
      {
        return Node->Attributes();
      }

      Ref<Attr> NewAttribute(DOMStringAtom name, DOMString &&value) noexcept
      {
        auto attr = Document->CreateAttribute(name).ReleaseValue();
        attr->Value(krys::move(value));
        return attr;
      }

      Ref<Attr> NewAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom name, DOMString &&value) noexcept
      {
        auto attr = Document->CreateAttributeNS(namespaceURI, name).ReleaseValue();
        attr->Value(krys::move(value));
        return attr;
      }
    };
  }

  TEST_CASE("NamedNodeMap.length: the size of its element's attribute list, which it follows",
            "[DOM][NamedNodeMap][dom:interface-namednodemap][dom:dom-namednodemap-length]"
            "[dom:concept-namednodemap-element][dom:concept-namednodemap-attribute]")
  {
    CommonTestData data;
    auto &attributes = data.Attributes();
    CHECK(attributes.Length() == 0uz);

    REQUIRE_FALSE(data.Node->SetAttribute(u8"a", u8"1").HasException());
    REQUIRE_FALSE(data.Node->SetAttributeNS(TestNamespace, u8"x:a", u8"2").HasException());
    CHECK(attributes.Length() == 2uz);

    data.Node->RemoveAttribute(u8"a");
    CHECK(attributes.Length() == 1uz);
  }

  TEST_CASE("NamedNodeMap.item(): the attribute at the index, or null past the end",
            "[DOM][NamedNodeMap][dom:dom-namednodemap-item]")
  {
    CommonTestData data;
    CHECK(data.Attributes().Item(0) == nullptr);

    auto attr1 = data.NewAttribute(u8"test-attr1", u8"value1");
    auto attr2 = data.NewAttribute(u8"test-attr2", u8"value2");
    REQUIRE_FALSE(data.Attributes().SetNamedItem(*attr1).HasException());
    REQUIRE_FALSE(data.Attributes().SetNamedItem(*attr2).HasException());

    CHECK(data.Attributes().Item(0) == attr1);
    CHECK(data.Attributes().Item(1) == attr2);
    CHECK(data.Attributes().Item(2) == nullptr);
    CHECK(data.Attributes()[1] == attr2);
    CHECK(data.Attributes().IsSupportedPropertyIndex(1));
    CHECK_FALSE(data.Attributes().IsSupportedPropertyIndex(2));
  }

  TEST_CASE("NamedNodeMap: its supported property names are its qualified names, without duplicates",
            "[DOM][NamedNodeMap][dom:dom-namednodemap-item]")
  {
    SECTION("Names with uppercase letters are left out for an HTML element in an HTML document")
    {
      CommonTestData data;
      REQUIRE_FALSE(data.Node->SetAttribute(u8"b", u8"").HasException());
      REQUIRE_FALSE(data.Node->SetAttributeNS(u8"urn:a", u8"x:y", u8"").HasException());
      REQUIRE_FALSE(data.Node->SetAttributeNS(DOMStringAtom::Null(), u8"Upper", u8"").HasException());
      REQUIRE_FALSE(data.Node->SetAttributeNS(u8"urn:b", u8"x:y", u8"").HasException());

      CHECK(data.Attributes().SupportedPropertyNames() == List<DOMString> {u8"b", u8"x:y"});
      CHECK(data.Attributes().IsSupportedPropertyName(u8"x:y"));
      CHECK_FALSE(data.Attributes().IsSupportedPropertyName(u8"Upper"));
      // The attribute can still be got by its name.
      CHECK(data.Attributes().GetNamedItem(u8"x:y") != nullptr);
    }

    SECTION("Otherwise they're all kept")
    {
      auto document = CreateRef<XMLDocument>();
      auto element =
        document->CreateElementNS(DOMStringAtom(infra::Namespaces::HTML), u8"div").ReleaseValue();
      REQUIRE_FALSE(element->SetAttribute(u8"Upper", u8"").HasException());
      REQUIRE_FALSE(element->SetAttribute(u8"lower", u8"").HasException());
      CHECK(element->Attributes().SupportedPropertyNames() == List<DOMString> {u8"Upper", u8"lower"});
    }
  }

  TEST_CASE("NamedNodeMap.getNamedItem() and getNamedItemNS(): get an attribute by name, or by namespace",
            "[DOM][NamedNodeMap][dom:dom-namednodemap-getnameditem][dom:dom-namednodemap-getnameditemns]")
  {
    CommonTestData data;
    CHECK(data.Attributes().GetNamedItem(u8"test-attr") == nullptr);
    CHECK(data.Attributes().GetNamedItemNS(TestNamespace, u8"test-attr") == nullptr);

    auto attr = data.NewAttribute(u8"test-attr", u8"value");
    auto nsAttr = data.NewAttributeNS(TestNamespace, u8"x:test-attr", u8"ns");
    REQUIRE_FALSE(data.Attributes().SetNamedItem(*attr).HasException());
    REQUIRE_FALSE(data.Attributes().SetNamedItemNS(*nsAttr).HasException());

    CHECK(data.Attributes().GetNamedItem(u8"TEST-ATTR") == attr);
    CHECK(data.Attributes().GetNamedItem(u8"x:test-attr") == nsAttr);
    CHECK(data.Attributes()[DOMStringAtom(u8"test-attr")] == attr);
    CHECK(data.Attributes().GetNamedItemNS(DOMStringAtom::Empty(), u8"test-attr") == attr);
    CHECK(data.Attributes().GetNamedItemNS(TestNamespace, u8"test-attr") == nsAttr);
  }

  TEST_CASE("NamedNodeMap.setNamedItem() and setNamedItemNS(): set an attribute, returning the one replaced",
            "[DOM][NamedNodeMap][dom:dom-namednodemap-setnameditem][dom:dom-namednodemap-setnameditemns]")
  {
    CommonTestData data;
    using SetNamedItem = ExceptionOr<RefPtr<Attr>> (NamedNodeMap::*)(Attr &) noexcept;
    for (auto set : {SetNamedItem(&NamedNodeMap::SetNamedItem), SetNamedItem(&NamedNodeMap::SetNamedItemNS)})
    {
      data.Node = data.Document->CreateElement(u8"div").ReleaseValue();
      auto attr1 = data.NewAttributeNS(TestNamespace, u8"x:test-attr", u8"value1");
      auto result = (data.Attributes().*set)(*attr1);
      REQUIRE_FALSE(result.HasException());
      CHECK(result.Value() == nullptr);
      CHECK(attr1->OwnerElement() == data.Node.get());

      auto attr2 = data.NewAttributeNS(TestNamespace, u8"y:test-attr", u8"value2");
      result = (data.Attributes().*set)(*attr2);
      REQUIRE_FALSE(result.HasException());
      CHECK(result.Value() == attr1);
      CHECK(data.Attributes().GetNamedItemNS(TestNamespace, u8"test-attr") == attr2);

      auto other = data.Document->CreateElement(u8"div").ReleaseValue();
      result = (other->Attributes().*set)(*attr2);
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::InUseAttributeError);
    }
  }

  TEST_CASE(
    "NamedNodeMap.removeNamedItem() and removeNamedItemNS(): remove the attribute, or throw if none",
    "[DOM][NamedNodeMap][dom:dom-namednodemap-removenameditem][dom:dom-namednodemap-removenameditemns]")
  {
    CommonTestData data;
    auto attr = data.NewAttribute(u8"test-attr", u8"value");
    auto nsAttr = data.NewAttributeNS(TestNamespace, u8"x:test-attr", u8"ns");
    REQUIRE_FALSE(data.Attributes().SetNamedItem(*attr).HasException());
    REQUIRE_FALSE(data.Attributes().SetNamedItemNS(*nsAttr).HasException());

    auto result = data.Attributes().RemoveNamedItem(u8"TEST-ATTR");
    REQUIRE_FALSE(result.HasException());
    CHECK(result.Value() == attr);
    CHECK(attr->OwnerElement() == nullptr);

    result = data.Attributes().RemoveNamedItemNS(TestNamespace, u8"test-attr");
    REQUIRE_FALSE(result.HasException());
    CHECK(result.Value() == nsAttr);
    CHECK(data.Attributes().Length() == 0uz);

    result = data.Attributes().RemoveNamedItem(u8"test-attr");
    REQUIRE(result.HasException());
    CHECK(result.GetException().Code() == ExceptionCode::NotFoundError);
    result = data.Attributes().RemoveNamedItemNS(TestNamespace, u8"test-attr");
    REQUIRE(result.HasException());
    CHECK(result.GetException().Code() == ExceptionCode::NotFoundError);
  }
}
