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
    }

    SECTION("[Reflect][ReflectDefault]")
    {
      SECTION("int32")
      {
        TestData data;
        REQUIRE(Reflection::Reflect<int32, ReflectDefault<int32>(42)>(*data.Element, TestAttribute) == 42);

        Reflection::Reflect<int32>(*data.Element, TestAttribute, 7);
        REQUIRE(Reflection::Reflect<int32, ReflectDefault<int32>(42)>(*data.Element, TestAttribute) == 7);
      }

      SECTION("uint32")
      {
        TestData data;
        REQUIRE(Reflection::Reflect<uint32, ReflectDefault<uint32>(10)>(*data.Element, TestAttribute)
                == 10);

        Reflection::Reflect<uint32, ReflectDefault<uint32>(10)>(*data.Element, TestAttribute, 5u);
        REQUIRE(Reflection::Reflect<uint32, ReflectDefault<uint32>(10)>(*data.Element, TestAttribute)
                == 5);

        // Values above MaxUnsignedLongValue should fall back to default.
        constexpr auto MaxUnsignedLongValue = 2'147'483'647u;
        Reflection::Reflect<uint32, ReflectDefault<uint32>(10)>(*data.Element, TestAttribute,
                                                                MaxUnsignedLongValue + 1u);
        REQUIRE(Reflection::Reflect<uint32, ReflectDefault<uint32>(10)>(*data.Element, TestAttribute)
                == 10);
      }

      SECTION("double")
      {
        TestData data;
        auto val1 = Reflection::Reflect<double, ReflectDefault<double>(1.5)>(*data.Element, TestAttribute);
        REQUIRE_THAT(val1, Catch::Matchers::WithinRel(1.5));

        Reflection::Reflect<double>(*data.Element, TestAttribute, 2.5);
        auto val2 = Reflection::Reflect<double, ReflectDefault<double>(1.5)>(*data.Element, TestAttribute);
        REQUIRE_THAT(val2, Catch::Matchers::WithinRel(2.5));
      }
    }

    SECTION("[Reflect][ReflectRange]")
    {
      SECTION("uint32")
      {
        constexpr ReflectRange<uint32> Range {10, 20};

        TestData data;
        // No attribute set: falls back to minimum of range.
        REQUIRE(Reflection::Reflect<uint32, Range>(*data.Element, TestAttribute) == 10);

        // In-range value is returned as-is.
        Reflection::Reflect<uint32>(*data.Element, TestAttribute, 15u);
        REQUIRE(Reflection::Reflect<uint32, Range>(*data.Element, TestAttribute) == 15);

        // Below-range value is clamped to min.
        Reflection::Reflect<uint32>(*data.Element, TestAttribute, 5u);
        REQUIRE(Reflection::Reflect<uint32, Range>(*data.Element, TestAttribute) == 10);

        // Above-range value is clamped to max.
        Reflection::Reflect<uint32>(*data.Element, TestAttribute, 25u);
        REQUIRE(Reflection::Reflect<uint32, Range>(*data.Element, TestAttribute) == 20);
      }
    }

    SECTION("[Reflect][ReflectDefault][ReflectRange]")
    {
      SECTION("uint32")
      {
        constexpr ReflectDefault<uint32> Default {15};
        constexpr ReflectRange<uint32> Range {10, 20};

        TestData data;
        // No attribute set: returns default.
        REQUIRE(Reflection::Reflect<uint32, Default, Range>(*data.Element, TestAttribute) == 15);

        // In-range value is returned as-is.
        Reflection::Reflect<uint32>(*data.Element, TestAttribute, 18u);
        REQUIRE(Reflection::Reflect<uint32, Default, Range>(*data.Element, TestAttribute) == 18);

        // Out-of-range value is clamped.
        Reflection::Reflect<uint32>(*data.Element, TestAttribute, 25u);
        REQUIRE(Reflection::Reflect<uint32, Default, Range>(*data.Element, TestAttribute) == 20);
      }
    }
  }

  TEST_CASE("Reflection::ReflectURL", "[Attributes]")
  {
    SECTION("USVString")
    {
      // TODO(USVString) - implement when USVString is implemented.
    }
  }

  TEST_CASE("Reflection::ReflectNonNegative", "[Attributes]")
  {
    SECTION("int32")
    {
      TestData data;
      REQUIRE(Reflection::ReflectNonNegative<int32>(*data.Element, TestAttribute) == -1);

      auto set5 = Reflection::ReflectNonNegative<int32>(*data.Element, TestAttribute, 5);
      REQUIRE_FALSE(set5.HasException());
      REQUIRE(Reflection::ReflectNonNegative<int32>(*data.Element, TestAttribute) == 5);

      // Negative value: setter returns an exception, attribute is unchanged.
      auto result = Reflection::ReflectNonNegative<int32>(*data.Element, TestAttribute, -1);
      REQUIRE(result.HasException());
      REQUIRE(Reflection::ReflectNonNegative<int32>(*data.Element, TestAttribute) == 5);

      auto set0 = Reflection::ReflectNonNegative<int32>(*data.Element, TestAttribute, 0);
      REQUIRE_FALSE(set0.HasException());
      REQUIRE(Reflection::ReflectNonNegative<int32>(*data.Element, TestAttribute) == 0);
    }

    SECTION("[ReflectNonNegative][ReflectDefault] int32")
    {
      TestData data;
      REQUIRE(Reflection::ReflectNonNegative<int32, ReflectDefault<int32>(99)>(*data.Element,
                                                                               TestAttribute)
              == 99);

      auto set3 = Reflection::ReflectNonNegative<int32>(*data.Element, TestAttribute, 3);
      REQUIRE_FALSE(set3.HasException());
      REQUIRE(Reflection::ReflectNonNegative<int32, ReflectDefault<int32>(99)>(*data.Element,
                                                                               TestAttribute)
              == 3);
    }
  }

  TEST_CASE("Reflection::ReflectPositive", "[Attributes]")
  {
    SECTION("uint32")
    {
      TestData data;
      // No attribute: minimum for positive is 1.
      REQUIRE(Reflection::ReflectPositive<uint32>(*data.Element, TestAttribute) == 1);

      auto set8 = Reflection::ReflectPositive<uint32>(*data.Element, TestAttribute, 8u);
      REQUIRE_FALSE(set8.HasException());
      REQUIRE(Reflection::ReflectPositive<uint32>(*data.Element, TestAttribute) == 8);

      // Zero is not a valid positive value: setter returns an exception.
      auto result = Reflection::ReflectPositive<uint32>(*data.Element, TestAttribute, 0u);
      REQUIRE(result.HasException());
      REQUIRE(Reflection::ReflectPositive<uint32>(*data.Element, TestAttribute) == 8);
    }

    SECTION("double")
    {
      TestData data;
      REQUIRE(Reflection::ReflectPositive<double>(*data.Element, TestAttribute) == 0.0);

      Reflection::ReflectPositive<double>(*data.Element, TestAttribute, 2.5);
      REQUIRE_THAT(Reflection::ReflectPositive<double>(*data.Element, TestAttribute),
                   Catch::Matchers::WithinRel(2.5));

      // Non-positive value is ignored, attribute is unchanged.
      Reflection::ReflectPositive<double>(*data.Element, TestAttribute, -1.0);
      REQUIRE_THAT(Reflection::ReflectPositive<double>(*data.Element, TestAttribute),
                   Catch::Matchers::WithinRel(2.5));

      Reflection::ReflectPositive<double>(*data.Element, TestAttribute, 0.0);
      REQUIRE_THAT(Reflection::ReflectPositive<double>(*data.Element, TestAttribute),
                   Catch::Matchers::WithinRel(2.5));
    }

    SECTION("[ReflectPositive][ReflectDefault] uint32")
    {
      TestData data;
      REQUIRE(Reflection::ReflectPositive<uint32, ReflectDefault<uint32>(5)>(*data.Element,
                                                                             TestAttribute)
              == 5);

      auto set3 = Reflection::ReflectPositive<uint32, ReflectDefault<uint32>(5)>(*data.Element, TestAttribute, 3u);
      REQUIRE_FALSE(set3.HasException());
      REQUIRE(Reflection::ReflectPositive<uint32, ReflectDefault<uint32>(5)>(*data.Element,
                                                                             TestAttribute)
              == 3);
    }

    SECTION("[ReflectPositive][ReflectDefault] double")
    {
      TestData data;
      auto defVal = Reflection::ReflectPositive<double, ReflectDefault<double>(3.0)>(*data.Element, TestAttribute);
      REQUIRE_THAT(defVal, Catch::Matchers::WithinRel(3.0));

      Reflection::ReflectPositive<double>(*data.Element, TestAttribute, 7.5);
      auto newVal = Reflection::ReflectPositive<double, ReflectDefault<double>(3.0)>(*data.Element, TestAttribute);
      REQUIRE_THAT(newVal, Catch::Matchers::WithinRel(7.5));
    }
  }

  TEST_CASE("Reflection::ReflectPositiveWithFallback", "[Attributes]")
  {
    SECTION("uint32")
    {
      TestData data;
      // No attribute: minimum for positive-with-fallback is 1.
      REQUIRE(Reflection::ReflectPositiveWithFallback<uint32>(*data.Element, TestAttribute) == 1);

      Reflection::ReflectPositiveWithFallback<uint32>(*data.Element, TestAttribute, 6u);
      REQUIRE(Reflection::ReflectPositiveWithFallback<uint32>(*data.Element, TestAttribute) == 6);

      // Zero is not valid: stored but getter falls back to minimum.
      Reflection::ReflectPositiveWithFallback<uint32>(*data.Element, TestAttribute, 0u);
      REQUIRE(Reflection::ReflectPositiveWithFallback<uint32>(*data.Element, TestAttribute) == 1);
    }

    SECTION("[ReflectPositiveWithFallback][ReflectDefault] uint32")
    {
      TestData data;
      REQUIRE(Reflection::ReflectPositiveWithFallback<uint32, ReflectDefault<uint32>(8)>(
                *data.Element, TestAttribute)
              == 8);

      Reflection::ReflectPositiveWithFallback<uint32, ReflectDefault<uint32>(8)>(*data.Element,
                                                                                 TestAttribute, 4u);
      REQUIRE(Reflection::ReflectPositiveWithFallback<uint32, ReflectDefault<uint32>(8)>(
                *data.Element, TestAttribute)
              == 4);
    }
  }

  TEST_CASE("Reflection::ReflectSetter", "[Attributes]")
  {
    SECTION("DOMString")
    {
      TestData data;
      Reflection::ReflectSetter<DOMString>(*data.Element, TestAttribute, u8"setter value");
      REQUIRE(Reflection::Reflect<DOMString>(*data.Element, TestAttribute) == u8"setter value");
    }

    SECTION("bool")
    {
      TestData data;
      Reflection::ReflectSetter<bool>(*data.Element, TestAttribute, true);
      REQUIRE(Reflection::Reflect<bool>(*data.Element, TestAttribute));

      Reflection::ReflectSetter<bool>(*data.Element, TestAttribute, false);
      REQUIRE_FALSE(Reflection::Reflect<bool>(*data.Element, TestAttribute));
    }

    SECTION("int32")
    {
      TestData data;
      Reflection::ReflectSetter<int32>(*data.Element, TestAttribute, 12);
      REQUIRE(Reflection::Reflect<int32>(*data.Element, TestAttribute) == 12);
    }

    SECTION("uint32")
    {
      TestData data;
      Reflection::ReflectSetter<uint32>(*data.Element, TestAttribute, 7u);
      REQUIRE(Reflection::Reflect<uint32>(*data.Element, TestAttribute) == 7);
    }

    SECTION("double")
    {
      TestData data;
      Reflection::ReflectSetter<double>(*data.Element, TestAttribute, 9.99);
      REQUIRE_THAT(Reflection::Reflect<double>(*data.Element, TestAttribute),
                   Catch::Matchers::WithinRel(9.99));
    }
  }
}