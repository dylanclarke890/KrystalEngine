#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML::Attributes;
  using namespace Krys::HTML;

  namespace
  {
    class TestHTMLElement : public HTMLElement
    {
    public:
      TestHTMLElement(Document &document) : HTMLElement(document)
      {
      }
    };

    DOMStringAtom TestAttribute = u8"test-attr";

    struct TestData
    {
      Ref<Document> Document;
      Ref<HTMLElement> Element;

      TestData(Maybe<DOMString> &&initialValue = Null)
          : Document(CreateRef<HTML::Document>()), Element(CreateRef<TestHTMLElement>(*Document))
      {
        if (initialValue != Null)
        {
          Element->SetAttribute(TestAttribute, std::move(initialValue.value()));
        }
      }
    };
  }

  TEST_CASE("Reflection::Reflect", "[Attributes]")
  {
    SECTION("[Reflect]")
    {
      SECTION("DOMString")
      {
        TestData data {u8"initial value"};
        REQUIRE(Reflection::Reflect<DOMString>(*data.Element, TestAttribute) == u8"initial value");

        Reflection::Reflect<DOMString>(*data.Element, TestAttribute, u8"new value");
        REQUIRE(Reflection::Reflect<DOMString>(*data.Element, TestAttribute) == u8"new value");
      }

      SECTION("Maybe<DOMString>")
      {
        TestData data;
        REQUIRE(Reflection::Reflect<Maybe<DOMString>>(*data.Element, TestAttribute) == Null);

        Reflection::Reflect<Maybe<DOMString>>(*data.Element, TestAttribute, u8"new value");
        REQUIRE(Reflection::Reflect<Maybe<DOMString>>(*data.Element, TestAttribute) == u8"new value");

        Reflection::Reflect<Maybe<DOMString>>(*data.Element, TestAttribute, Null);
        REQUIRE(Reflection::Reflect<Maybe<DOMString>>(*data.Element, TestAttribute) == Null);
      }

      SECTION("USVString")
      {
        // TODO(USVString) - implement when USVString is implemented.
      }

      SECTION("bool")
      {
        TestData data;
        REQUIRE_FALSE(Reflection::Reflect<bool>(*data.Element, TestAttribute));

        Reflection::Reflect<bool>(*data.Element, TestAttribute, true);
        REQUIRE(Reflection::Reflect<bool>(*data.Element, TestAttribute));

        Reflection::Reflect<bool>(*data.Element, TestAttribute, false);
        REQUIRE_FALSE(Reflection::Reflect<bool>(*data.Element, TestAttribute));
      }

      SECTION("int32")
      {
        TestData data;
        REQUIRE(Reflection::Reflect<int32>(*data.Element, TestAttribute) == 0);

        Reflection::Reflect<int32>(*data.Element, TestAttribute, 4);
        REQUIRE(Reflection::Reflect<int32>(*data.Element, TestAttribute) == 4);

        Reflection::Reflect<int32>(*data.Element, TestAttribute, -4);
        REQUIRE(Reflection::Reflect<int32>(*data.Element, TestAttribute) == -4);

        Reflection::Reflect<int32>(*data.Element, TestAttribute, std::numeric_limits<int32>::min());
        REQUIRE(Reflection::Reflect<int32>(*data.Element, TestAttribute)
                == std::numeric_limits<int32>::min());

        Reflection::Reflect<int32>(*data.Element, TestAttribute, std::numeric_limits<int32>::max());
        REQUIRE(Reflection::Reflect<int32>(*data.Element, TestAttribute)
                == std::numeric_limits<int32>::max());
      }

      SECTION("uint32")
      {
        TestData data;
        REQUIRE(Reflection::Reflect<uint32>(*data.Element, TestAttribute) == 0);

        Reflection::Reflect<uint32>(*data.Element, TestAttribute, 4);
        REQUIRE(Reflection::Reflect<uint32>(*data.Element, TestAttribute) == 4);

        Reflection::Reflect<uint32>(*data.Element, TestAttribute, 0);
        REQUIRE(Reflection::Reflect<uint32>(*data.Element, TestAttribute) == 0);

        Reflection::Reflect<uint32>(*data.Element, TestAttribute, std::numeric_limits<uint32>::min());
        REQUIRE(Reflection::Reflect<uint32>(*data.Element, TestAttribute)
                == std::numeric_limits<uint32>::min());

        constexpr auto MaxUnsignedLongValue = 2'147'483'647;

        Reflection::Reflect<uint32>(*data.Element, TestAttribute, std::numeric_limits<uint32>::max());
        REQUIRE(Reflection::Reflect<uint32>(*data.Element, TestAttribute) == 0);

        Reflection::Reflect<uint32>(*data.Element, TestAttribute, MaxUnsignedLongValue);
        REQUIRE(Reflection::Reflect<uint32>(*data.Element, TestAttribute) == MaxUnsignedLongValue);
      }

      SECTION("double")
      {
        TestData data;
        REQUIRE(Reflection::Reflect<double>(*data.Element, TestAttribute) == 0.0);

        Reflection::Reflect<double>(*data.Element, TestAttribute, 3.14159265359);
        REQUIRE_THAT(Reflection::Reflect<double>(*data.Element, TestAttribute),
                     Catch::Matchers::WithinRel(3.14159265359));

        Reflection::Reflect<double>(*data.Element, TestAttribute, -3.14159265359);
        REQUIRE_THAT(Reflection::Reflect<double>(*data.Element, TestAttribute),
                     Catch::Matchers::WithinRel(-3.14159265359));
      }

      SECTION("DOMTokenList")
      {
        // TODO(DOMTokenList) - implement when DOMTokenList reflection is implemented.
      }
    }
  }
}