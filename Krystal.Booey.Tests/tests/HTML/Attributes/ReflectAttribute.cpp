#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html::tests
{
  namespace
  {
    class TestHTMLElement : public HTMLElement
    {
    public:
      TestHTMLElement(dom::Document &document) : HTMLElement(document)
      {
      }
    };

    dom::DOMStringAtom TestAttribute = u8"test-attr";

    struct TestData
    {
      Ref<dom::Document> Document;
      Ref<html::HTMLElement> Element;

      TestData(Maybe<dom::DOMString> &&initialValue = null)
          : Document(CreateRef<dom::Document>()), Element(CreateRef<TestHTMLElement>(*Document))
      {
        if (initialValue != null)
        {
          Element->SetAttribute(TestAttribute, std::move(initialValue.value()));
        }
      }
    };
  }

  TEST_CASE(
    "Reflect: each type's getter and setter, with defaults and ranges",
    "[HTML][Attributes][html:xattr-reflect][html:xattr-reflectdefault][html:xattr-reflectrange]"
    "[html:limited-to-only-known-values][html:default-value][html:clamped-to-the-range][html:reflect]")
  {
    SECTION("[Reflect]")
    {
      SECTION("DOMString")
      {
        TestData data {u8"initial value"};
        REQUIRE(Reflect<dom::DOMString>(*data.Element, TestAttribute) == u8"initial value");

        Reflect<dom::DOMString>(*data.Element, TestAttribute, u8"new value");
        REQUIRE(Reflect<dom::DOMString>(*data.Element, TestAttribute) == u8"new value");
      }

      SECTION("Maybe<dom::DOMString>")
      {
        TestData data;
        REQUIRE(Reflect<Maybe<dom::DOMString>>(*data.Element, TestAttribute) == null);

        Reflect<Maybe<dom::DOMString>>(*data.Element, TestAttribute, u8"new value");
        REQUIRE(Reflect<Maybe<dom::DOMString>>(*data.Element, TestAttribute) == u8"new value");

        Reflect<Maybe<dom::DOMString>>(*data.Element, TestAttribute, null);
        REQUIRE(Reflect<Maybe<dom::DOMString>>(*data.Element, TestAttribute) == null);
      }

      SECTION("USVString")
      {
        // FIXME: Implement when USVString is implemented.
      }

      SECTION("bool")
      {
        TestData data;
        REQUIRE_FALSE(Reflect<bool>(*data.Element, TestAttribute));

        Reflect<bool>(*data.Element, TestAttribute, true);
        REQUIRE(Reflect<bool>(*data.Element, TestAttribute));

        Reflect<bool>(*data.Element, TestAttribute, false);
        REQUIRE_FALSE(Reflect<bool>(*data.Element, TestAttribute));
      }

      SECTION("int32")
      {
        TestData data;
        REQUIRE(Reflect<int32>(*data.Element, TestAttribute) == 0);

        Reflect<int32>(*data.Element, TestAttribute, 4);
        REQUIRE(Reflect<int32>(*data.Element, TestAttribute) == 4);

        Reflect<int32>(*data.Element, TestAttribute, -4);
        REQUIRE(Reflect<int32>(*data.Element, TestAttribute) == -4);

        Reflect<int32>(*data.Element, TestAttribute, std::numeric_limits<int32>::min());
        REQUIRE(Reflect<int32>(*data.Element, TestAttribute) == std::numeric_limits<int32>::min());

        Reflect<int32>(*data.Element, TestAttribute, std::numeric_limits<int32>::max());
        REQUIRE(Reflect<int32>(*data.Element, TestAttribute) == std::numeric_limits<int32>::max());
      }

      SECTION("uint32")
      {
        TestData data;
        REQUIRE(Reflect<uint32>(*data.Element, TestAttribute) == 0);

        Reflect<uint32>(*data.Element, TestAttribute, 4);
        REQUIRE(Reflect<uint32>(*data.Element, TestAttribute) == 4);

        Reflect<uint32>(*data.Element, TestAttribute, 0);
        REQUIRE(Reflect<uint32>(*data.Element, TestAttribute) == 0);

        Reflect<uint32>(*data.Element, TestAttribute, std::numeric_limits<uint32>::min());
        REQUIRE(Reflect<uint32>(*data.Element, TestAttribute) == std::numeric_limits<uint32>::min());

        constexpr auto MaxUnsignedLongValue = 2'147'483'647;

        Reflect<uint32>(*data.Element, TestAttribute, std::numeric_limits<uint32>::max());
        REQUIRE(Reflect<uint32>(*data.Element, TestAttribute) == 0);

        Reflect<uint32>(*data.Element, TestAttribute, MaxUnsignedLongValue);
        REQUIRE(Reflect<uint32>(*data.Element, TestAttribute) == MaxUnsignedLongValue);
      }

      SECTION("double")
      {
        TestData data;
        REQUIRE(Reflect<double>(*data.Element, TestAttribute) == 0.0);

        Reflect<double>(*data.Element, TestAttribute, 3.14159265359);
        REQUIRE_THAT(Reflect<double>(*data.Element, TestAttribute),
                     Catch::Matchers::WithinRel(3.14159265359));

        Reflect<double>(*data.Element, TestAttribute, -3.14159265359);
        REQUIRE_THAT(Reflect<double>(*data.Element, TestAttribute),
                     Catch::Matchers::WithinRel(-3.14159265359));
      }
    }

    SECTION("[Reflect][ReflectDefault]")
    {
      SECTION("int32")
      {
        TestData data;
        REQUIRE(Reflect<int32, ReflectDefault<int32>(42)>(*data.Element, TestAttribute) == 42);

        Reflect<int32>(*data.Element, TestAttribute, 7);
        REQUIRE(Reflect<int32, ReflectDefault<int32>(42)>(*data.Element, TestAttribute) == 7);
      }

      SECTION("uint32")
      {
        TestData data;
        REQUIRE(Reflect<uint32, ReflectDefault<uint32>(10)>(*data.Element, TestAttribute) == 10);

        Reflect<uint32, ReflectDefault<uint32>(10)>(*data.Element, TestAttribute, 5u);
        REQUIRE(Reflect<uint32, ReflectDefault<uint32>(10)>(*data.Element, TestAttribute) == 5);

        // Values above MaxUnsignedLongValue should fall back to default.
        constexpr auto MaxUnsignedLongValue = 2'147'483'647u;
        Reflect<uint32, ReflectDefault<uint32>(10)>(*data.Element, TestAttribute, MaxUnsignedLongValue + 1u);
        REQUIRE(Reflect<uint32, ReflectDefault<uint32>(10)>(*data.Element, TestAttribute) == 10);
      }

      SECTION("double")
      {
        TestData data;
        auto val1 = Reflect<double, ReflectDefault<double>(1.5)>(*data.Element, TestAttribute);
        REQUIRE_THAT(val1, Catch::Matchers::WithinRel(1.5));

        Reflect<double>(*data.Element, TestAttribute, 2.5);
        auto val2 = Reflect<double, ReflectDefault<double>(1.5)>(*data.Element, TestAttribute);
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
        REQUIRE(Reflect<uint32, Range>(*data.Element, TestAttribute) == 10);

        // In-range value is returned as-is.
        Reflect<uint32>(*data.Element, TestAttribute, 15u);
        REQUIRE(Reflect<uint32, Range>(*data.Element, TestAttribute) == 15);

        // Below-range value is clamped to min.
        Reflect<uint32>(*data.Element, TestAttribute, 5u);
        REQUIRE(Reflect<uint32, Range>(*data.Element, TestAttribute) == 10);

        // Above-range value is clamped to max.
        Reflect<uint32>(*data.Element, TestAttribute, 25u);
        REQUIRE(Reflect<uint32, Range>(*data.Element, TestAttribute) == 20);
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
        REQUIRE(Reflect<uint32, Default, Range>(*data.Element, TestAttribute) == 15);

        // In-range value is returned as-is.
        Reflect<uint32>(*data.Element, TestAttribute, 18u);
        REQUIRE(Reflect<uint32, Default, Range>(*data.Element, TestAttribute) == 18);

        // Out-of-range value is clamped.
        Reflect<uint32>(*data.Element, TestAttribute, 25u);
        REQUIRE(Reflect<uint32, Default, Range>(*data.Element, TestAttribute) == 20);
      }
    }
  }

  TEST_CASE("Reflect<int32>: integers outside the long range are the default",
            "[HTML][Attributes][html:default-value]")
  {
    for (auto value : {u8"-2147483649", u8"2147483648", u8"99999999999999999999", u8"x"})
    {
      INFO(reinterpret_cast<const char *>(value));
      TestData data {dom::DOMString(value)};
      CHECK(Reflect<int32>(*data.Element, TestAttribute) == 0);
      CHECK(ReflectNonNegative<int32>(*data.Element, TestAttribute) == -1);
    }
    TestData data {dom::DOMString(u8" -2147483648")};
    CHECK(Reflect<int32>(*data.Element, TestAttribute) == -2'147'483'648);
    // Negative values aren't non-negative, but negative zero is zero.
    CHECK(ReflectNonNegative<int32>(*data.Element, TestAttribute) == -1);
    TestData zero {dom::DOMString(u8"-0")};
    CHECK(ReflectNonNegative<int32>(*zero.Element, TestAttribute) == 0);
  }

  TEST_CASE("Reflect<uint32>: setting a value past 2147483647 sets the default or minimum",
            "[HTML][Attributes][html:clamped-to-the-range]")
  {
    TestData data;
    Reflect<uint32>(*data.Element, TestAttribute, 2'147'483'648u);
    CHECK(data.Element->GetAttribute(TestAttribute) == u8"0");
    Reflect<uint32, ReflectDefault<uint32>(10)>(*data.Element, TestAttribute, 4'000'000'000u);
    CHECK(data.Element->GetAttribute(TestAttribute) == u8"10");
    ReflectPositiveWithFallback<uint32>(*data.Element, TestAttribute, 0u);
    CHECK(data.Element->GetAttribute(TestAttribute) == u8"1");
    CHECK(ReflectPositive<uint32>(*data.Element, TestAttribute, 0u).HasException());
  }

  TEST_CASE("Reflect<uint32>: a range clamps values past it, however large",
            "[HTML][Attributes][html:clamped-to-the-range]")
  {
    constexpr ReflectRange<uint32> Range {1, 1'000};
    for (auto [value, expected] : {std::pair {u8"5000", 1'000u}, std::pair {u8"99999999999999999999", 1'000u},
                                   std::pair {u8"0", 1u}, std::pair {u8"-5", 1u}, std::pair {u8"x", 1u}})
    {
      INFO(reinterpret_cast<const char *>(value));
      TestData data {dom::DOMString(value)};
      CHECK(Reflect<uint32, Range>(*data.Element, TestAttribute) == expected);
    }
  }

  TEST_CASE("Reflect<double>: setting writes the best representation of the number",
            "[HTML][Attributes][html:best-representation-of-the-number-as-a-floating-point-number]")
  {
    TestData data;
    for (auto [value, expected] :
         {std::pair {0.000001, u8"0.000001"}, std::pair {1e21, u8"1e+21"}, std::pair {1.5, u8"1.5"},
          std::pair {-0.0, u8"0"}, std::pair {123456789012345680000.0, u8"123456789012345680000"},
          std::pair {1e-7, u8"1e-7"}})
    {
      INFO(reinterpret_cast<const char *>(expected));
      Reflect<double>(*data.Element, TestAttribute, double(value));
      CHECK(data.Element->GetAttribute(TestAttribute) == expected);
    }
    // Positive-only values ignore what isn't positive.
    ReflectPositive<double>(*data.Element, TestAttribute, -1.0);
    CHECK(data.Element->GetAttribute(TestAttribute) == u8"1e-7");
  }

  TEST_CASE("Reflect<bool>: a boolean attribute is true when present, whatever its value",
            "[HTML][Attributes][html:boolean-attribute]")
  {
    for (auto value : {u8"", u8"false", u8"test-attr", u8"no"})
    {
      TestData data {dom::DOMString(value)};
      CHECK(Reflect<bool>(*data.Element, TestAttribute));
    }

    // Setting it to true sets it to the empty string.
    TestData data;
    Reflect<bool>(*data.Element, TestAttribute, true);
    CHECK(data.Element->GetAttribute(TestAttribute) == u8"");
  }

  TEST_CASE("ReflectURL", "[HTML][Attributes][html:xattr-reflecturl][html:treated-as-a-url]")
  {
    SECTION("USVString")
    {
      // FIXME: Implement when USVString is implemented.
    }
  }

  TEST_CASE("ReflectNonNegative", "[HTML][Attributes][html:xattr-reflectnonnegative]")
  {
    SECTION("int32")
    {
      TestData data;
      REQUIRE(ReflectNonNegative<int32>(*data.Element, TestAttribute) == -1);

      auto set5 = ReflectNonNegative<int32>(*data.Element, TestAttribute, 5);
      REQUIRE_FALSE(set5.HasException());
      REQUIRE(ReflectNonNegative<int32>(*data.Element, TestAttribute) == 5);

      // Negative value: setter returns an exception, attribute is unchanged.
      auto result = ReflectNonNegative<int32>(*data.Element, TestAttribute, -1);
      REQUIRE(result.HasException());
      REQUIRE(ReflectNonNegative<int32>(*data.Element, TestAttribute) == 5);

      auto set0 = ReflectNonNegative<int32>(*data.Element, TestAttribute, 0);
      REQUIRE_FALSE(set0.HasException());
      REQUIRE(ReflectNonNegative<int32>(*data.Element, TestAttribute) == 0);
    }

    SECTION("[ReflectNonNegative][ReflectDefault] int32")
    {
      TestData data;
      REQUIRE(ReflectNonNegative<int32, ReflectDefault<int32>(99)>(*data.Element, TestAttribute) == 99);

      auto set3 = ReflectNonNegative<int32>(*data.Element, TestAttribute, 3);
      REQUIRE_FALSE(set3.HasException());
      REQUIRE(ReflectNonNegative<int32, ReflectDefault<int32>(99)>(*data.Element, TestAttribute) == 3);
    }
  }

  TEST_CASE("ReflectPositive", "[HTML][Attributes][html:xattr-reflectpositive]")
  {
    SECTION("uint32")
    {
      TestData data;
      // No attribute: minimum for positive is 1.
      REQUIRE(ReflectPositive<uint32>(*data.Element, TestAttribute) == 1);

      auto set8 = ReflectPositive<uint32>(*data.Element, TestAttribute, 8u);
      REQUIRE_FALSE(set8.HasException());
      REQUIRE(ReflectPositive<uint32>(*data.Element, TestAttribute) == 8);

      // Zero is not a valid positive value: setter returns an exception.
      auto result = ReflectPositive<uint32>(*data.Element, TestAttribute, 0u);
      REQUIRE(result.HasException());
      REQUIRE(ReflectPositive<uint32>(*data.Element, TestAttribute) == 8);
    }

    SECTION("double")
    {
      TestData data;
      REQUIRE(ReflectPositive<double>(*data.Element, TestAttribute) == 0.0);

      ReflectPositive<double>(*data.Element, TestAttribute, 2.5);
      REQUIRE_THAT(ReflectPositive<double>(*data.Element, TestAttribute), Catch::Matchers::WithinRel(2.5));

      // Non-positive value is ignored, attribute is unchanged.
      ReflectPositive<double>(*data.Element, TestAttribute, -1.0);
      REQUIRE_THAT(ReflectPositive<double>(*data.Element, TestAttribute), Catch::Matchers::WithinRel(2.5));

      ReflectPositive<double>(*data.Element, TestAttribute, 0.0);
      REQUIRE_THAT(ReflectPositive<double>(*data.Element, TestAttribute), Catch::Matchers::WithinRel(2.5));
    }

    SECTION("[ReflectPositive][ReflectDefault] uint32")
    {
      TestData data;
      REQUIRE(ReflectPositive<uint32, ReflectDefault<uint32>(5)>(*data.Element, TestAttribute) == 5);

      auto set3 = ReflectPositive<uint32, ReflectDefault<uint32>(5)>(*data.Element, TestAttribute, 3u);
      REQUIRE_FALSE(set3.HasException());
      REQUIRE(ReflectPositive<uint32, ReflectDefault<uint32>(5)>(*data.Element, TestAttribute) == 3);
    }

    SECTION("[ReflectPositive][ReflectDefault] double")
    {
      TestData data;
      auto defVal = ReflectPositive<double, ReflectDefault<double>(3.0)>(*data.Element, TestAttribute);
      REQUIRE_THAT(defVal, Catch::Matchers::WithinRel(3.0));

      ReflectPositive<double>(*data.Element, TestAttribute, 7.5);
      auto newVal = ReflectPositive<double, ReflectDefault<double>(3.0)>(*data.Element, TestAttribute);
      REQUIRE_THAT(newVal, Catch::Matchers::WithinRel(7.5));
    }
  }

  TEST_CASE("ReflectPositiveWithFallback", "[HTML][Attributes][html:xattr-reflectpositivewithfallback]")
  {
    SECTION("uint32")
    {
      TestData data;
      // No attribute: minimum for positive-with-fallback is 1.
      REQUIRE(ReflectPositiveWithFallback<uint32>(*data.Element, TestAttribute) == 1);

      ReflectPositiveWithFallback<uint32>(*data.Element, TestAttribute, 6u);
      REQUIRE(ReflectPositiveWithFallback<uint32>(*data.Element, TestAttribute) == 6);

      // Zero is not valid: stored but getter falls back to minimum.
      ReflectPositiveWithFallback<uint32>(*data.Element, TestAttribute, 0u);
      REQUIRE(ReflectPositiveWithFallback<uint32>(*data.Element, TestAttribute) == 1);
    }

    SECTION("[ReflectPositiveWithFallback][ReflectDefault] uint32")
    {
      TestData data;
      REQUIRE(ReflectPositiveWithFallback<uint32, ReflectDefault<uint32>(8)>(*data.Element, TestAttribute)
              == 8);

      ReflectPositiveWithFallback<uint32, ReflectDefault<uint32>(8)>(*data.Element, TestAttribute, 4u);
      REQUIRE(ReflectPositiveWithFallback<uint32, ReflectDefault<uint32>(8)>(*data.Element, TestAttribute)
              == 4);
    }
  }

  TEST_CASE("ReflectSetter", "[HTML][Attributes][html:xattr-reflectsetter]")
  {
    SECTION("DOMString")
    {
      TestData data;
      ReflectSetter<dom::DOMString>(*data.Element, TestAttribute, u8"setter value");
      REQUIRE(Reflect<dom::DOMString>(*data.Element, TestAttribute) == u8"setter value");
    }

    SECTION("bool")
    {
      TestData data;
      ReflectSetter<bool>(*data.Element, TestAttribute, true);
      REQUIRE(Reflect<bool>(*data.Element, TestAttribute));

      ReflectSetter<bool>(*data.Element, TestAttribute, false);
      REQUIRE_FALSE(Reflect<bool>(*data.Element, TestAttribute));
    }

    SECTION("int32")
    {
      TestData data;
      ReflectSetter<int32>(*data.Element, TestAttribute, 12);
      REQUIRE(Reflect<int32>(*data.Element, TestAttribute) == 12);
    }

    SECTION("uint32")
    {
      TestData data;
      ReflectSetter<uint32>(*data.Element, TestAttribute, 7u);
      REQUIRE(Reflect<uint32>(*data.Element, TestAttribute) == 7);
    }

    SECTION("double")
    {
      TestData data;
      ReflectSetter<double>(*data.Element, TestAttribute, 9.99);
      REQUIRE_THAT(Reflect<double>(*data.Element, TestAttribute), Catch::Matchers::WithinRel(9.99));
    }
  }
}