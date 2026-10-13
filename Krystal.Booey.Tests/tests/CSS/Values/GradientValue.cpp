#include "Krystal.Booey/CSS/Values/GradientValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/Images/Gradient.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    using Method = GradientColorInterpolationMethod;

    Color Keyword(ValueId id)
    {
      return Color {KeywordColor {id}};
    }

    Method Standard()
    {
      return Method {.Method = {ColorInterpolationMethod::OKLab {}, AlphaPremultiplication::Premultiplied},
                     .DefaultMethod = Method::Default::OKLab};
    }

    Gradient Linear(LinearGradient::GradientLineType line, ValueId from = ValueId::Red,
                    ValueId to = ValueId::Blue)
    {
      return FunctionNotation<ValueId::LinearGradient, LinearGradient> {
        LinearGradient {.ColorInterpolationMethod = Standard(),
                        .GradientLine = krys::move(line),
                        .Stops = {GradientLinearColorStop {.Color = Keyword(from), .Position = {}},
                                  GradientLinearColorStop {.Color = Keyword(to), .Position = {}}}}};
    }

    Gradient RepeatingLinear(LinearGradient::GradientLineType line)
    {
      return FunctionNotation<ValueId::RepeatingLinearGradient, LinearGradient> {LinearGradient {
        .ColorInterpolationMethod = Standard(),
        .GradientLine = krys::move(line),
        .Stops = {GradientLinearColorStop {.Color = Keyword(ValueId::Red), .Position = {}},
                  GradientLinearColorStop {.Color = Keyword(ValueId::Blue), .Position = {}}}}};
    }
  }

  TEST_CASE_METHOD(ValueFixture, "GradientValue::Create", "[CSS][Values][GradientValue]")
  {
    auto value = GradientValue::Create(Linear(Vertical {keywords::Bottom {}}));
    CHECK(value->IsGradientValue());
    CHECK(value->IsImageGeneratorValue());
    CHECK(value->IsImage());
    CHECK_FALSE(value->IsImageValue());
  }

  TEST_CASE_METHOD(ValueFixture, "GradientValue serialisation", "[CSS][Values][GradientValue]")
  {
    // The gradient serialises itself; the default direction is omitted.
    CHECK(CssText(*GradientValue::Create(Linear(Vertical {keywords::Bottom {}})))
          == "linear-gradient(red, blue)");
    CHECK(CssText(*GradientValue::Create(Linear(Vertical {keywords::Top {}})))
          == "linear-gradient(to top, red, blue)");
    CHECK(CssText(*GradientValue::Create(RepeatingLinear(Angle<> {90_css_deg})))
          == "repeating-linear-gradient(90deg, red, blue)");
  }

  TEST_CASE_METHOD(ValueFixture, "GradientValue equality", "[CSS][Values][GradientValue]")
  {
    auto a = GradientValue::Create(Linear(Vertical {keywords::Top {}}));
    CHECK(*a == *GradientValue::Create(Linear(Vertical {keywords::Top {}})));
    CHECK_FALSE(*a == *GradientValue::Create(Linear(Vertical {keywords::Bottom {}})));
    CHECK_FALSE(*a
                == *GradientValue::Create(Linear(Vertical {keywords::Top {}}, ValueId::Red, ValueId::Green)));
    CHECK_FALSE(*a == *GradientValue::Create(RepeatingLinear(Vertical {keywords::Top {}})));

    size_t seed = 0uz;
    CHECK_FALSE(a->AddHash(seed));
  }

  TEST_CASE_METHOD(ValueFixture, "GradientValue children", "[CSS][Values][GradientValue]")
  {
    // Gradients hold no values to visit.
    auto value = GradientValue::Create(Linear(Angle<> {90_css_deg}));
    CHECK(Children(*value).empty());
    CHECK_FALSE(value->MayDependOnBaseUrl());
  }
}
