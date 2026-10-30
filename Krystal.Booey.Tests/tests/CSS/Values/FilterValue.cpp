#include "Krystal.Booey/CSS/Values/FilterValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    FilterReference Reference(utf8_stringview url)
    {
      ParserContext context {ParserMode::HTMLStandard};
      context.BaseURL = url::URL {u8"https://example.com/"};
      return FilterReference {*CompleteURL(CSSOMString {url}, context)};
    }

    Filter Filters(FilterValueList::Container &&values)
    {
      return Filter {FilterValueList {krys::move(values)}};
    }

    BlurFunction MakeBlur(Blur::Parameter parameter)
    {
      return BlurFunction {.Parameters = {krys::move(parameter)}};
    }
  }

  TEST_CASE_METHOD(ValueFixture, "FilterValue::Create", "[CSS][Values][FilterValue]")
  {
    auto value = FilterValue::Create(Filters({MakeBlur(2_css_px)}));
    CHECK(value->IsFilterValue());
    CHECK(value->GetFilter() == Filters({MakeBlur(2_css_px)}));
  }

  TEST_CASE_METHOD(ValueFixture, "FilterValue serialisation", "[CSS][Values][FilterValue]")
  {
    CHECK(CssText(*FilterValue::Create(Filter {keywords::None {}})) == "none");
    CHECK(CssText(*FilterValue::Create(Filters({MakeBlur(2_css_px)}))) == "blur(2px)");
    CHECK(CssText(*FilterValue::Create(Filters({Reference(u8"#a"), SepiaFunction {}, MakeBlur(1_css_px)})))
          == "url(\"#a\") sepia() blur(1px)");
  }

  TEST_CASE_METHOD(ValueFixture, "FilterValue equality and hashing", "[CSS][Values][FilterValue]")
  {
    auto a = FilterValue::Create(Filters({SepiaFunction {}, MakeBlur(1_css_px)}));
    auto b = FilterValue::Create(Filters({SepiaFunction {}, MakeBlur(1_css_px)}));
    auto reordered = FilterValue::Create(Filters({MakeBlur(1_css_px), SepiaFunction {}}));
    auto none = FilterValue::Create(Filter {keywords::None {}});

    CHECK(*a == *b);
    CHECK_FALSE(*a == *reordered);
    CHECK_FALSE(*a == *none);
    CHECK(*none == *FilterValue::Create(Filter {keywords::None {}}));

    // `FilterValue` has no hash of its own.
    size_t seed = 0uz;
    CHECK_FALSE(a->AddHash(seed));
  }

  TEST_CASE_METHOD(ValueFixture, "FilterValue visitation", "[CSS][Values][FilterValue]")
  {
    auto value = FilterValue::Create(Filters({Reference(u8"#a"), MakeBlur(1_css_px)}));
    CHECK(Children(*value).empty());
    // Not customised, like upstream.
    CHECK_FALSE(value->MayDependOnBaseUrl());
  }
}
