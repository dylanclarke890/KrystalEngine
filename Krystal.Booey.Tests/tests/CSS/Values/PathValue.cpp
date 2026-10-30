#include "Krystal.Booey/CSS/Values/PathValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    PathFunction MakePath(CSSOMStringView d, Maybe<FillRule> fillRule = {})
    {
      return PathFunction {Path {.FillRule = krys::move(fillRule), .Data = {svg::SVGPathByteStream(d)}}};
    }
  }

  TEST_CASE_METHOD(ValueFixture, "PathValue::Create", "[CSS][Values][PathValue]")
  {
    auto value = PathValue::Create(MakePath(u8"M 0 0 L 10 10"));
    CHECK(value->IsPathValue());
    CHECK(value->GetPath() == MakePath(u8"M 0 0 L 10 10"));
  }

  TEST_CASE_METHOD(ValueFixture, "PathValue serialisation", "[CSS][Values][PathValue]")
  {
    CHECK(CssText(*PathValue::Create(MakePath(u8"M 0 0 L 10 10"))) == "path(\"M 0 0 L 10 10\")");
    CHECK(CssText(*PathValue::Create(MakePath(u8"M0,0l10-10z"))) == "path(\"M 0 0 l 10 -10 Z\")");
    CHECK(CssText(*PathValue::Create(MakePath(u8"M 0 0", FillRule {keywords::Evenodd {}})))
          == "path(evenodd, \"M 0 0\")");
  }

  TEST_CASE_METHOD(ValueFixture, "PathValue equality and hashing", "[CSS][Values][PathValue]")
  {
    auto a = PathValue::Create(MakePath(u8"M 0 0 L 10 10"));
    auto b = PathValue::Create(MakePath(u8"M0,0L10,10"));
    auto c = PathValue::Create(MakePath(u8"M 0 0 L 10 11"));
    auto evenodd = PathValue::Create(MakePath(u8"M 0 0 L 10 10", FillRule {keywords::Evenodd {}}));

    CHECK(*a == *b);
    CHECK_FALSE(*a == *c);
    CHECK_FALSE(*a == *evenodd);

    // `PathValue` has no hash of its own.
    size_t seed = 0uz;
    CHECK_FALSE(a->AddHash(seed));
  }

  TEST_CASE_METHOD(ValueFixture, "PathValue visitation", "[CSS][Values][PathValue]")
  {
    auto value = PathValue::Create(MakePath(u8"M 0 0 L 1 1"));
    CHECK(Children(*value).empty());
    CHECK_FALSE(value->MayDependOnBaseUrl());
  }
}
