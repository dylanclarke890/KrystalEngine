#include "Krystal.Booey/CSS/TypedOM/Color/CSSColorValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/TypedOM/Color/CSSColor.hpp"
#include "Krystal.Booey/CSS/TypedOM/Color/CSSHSL.hpp"
#include "Krystal.Booey/CSS/TypedOM/Color/CSSHWB.hpp"
#include "Krystal.Booey/CSS/TypedOM/Color/CSSLab.hpp"
#include "Krystal.Booey/CSS/TypedOM/Color/CSSLCH.hpp"
#include "Krystal.Booey/CSS/TypedOM/Color/CSSOKLab.hpp"
#include "Krystal.Booey/CSS/TypedOM/Color/CSSOKLCH.hpp"
#include "Krystal.Booey/CSS/TypedOM/Color/CSSRGB.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSNumericFactory.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSUnitValue.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    using Factory = CSSNumericFactory;

    template <typename T>
    KRYS_NODISCARD Ref<T> Value(dom::ExceptionOr<Ref<T>> &&result)
    {
      REQUIRE(result.HasValue());
      return result.ReleaseValue();
    }

    /// @brief The component as text: a numeric value's serialisation, or the keyword.
    KRYS_NODISCARD std::string Text(const CSSColorPercent &component)
    {
      if (const auto *numeric = std::get_if<RefPtr<CSSNumericValue>>(&component))
      {
        return ToNarrow((*numeric)->ToString());
      }
      if (const auto *keyword = std::get_if<RefPtr<CSSKeywordValue>>(&component))
      {
        return ToNarrow((*keyword)->Value());
      }
      return "?";
    }

    KRYS_NODISCARD std::string Space(const CSSColorValue &color)
    {
      return ToNarrow(color.ColorSpace()->Value());
    }

    KRYS_NODISCARD CSSColorPercent Numeric(Ref<CSSUnitValue> value)
    {
      return RefPtr<CSSNumericValue>(krys::move(value));
    }
  }

  TEST_CASE_METHOD(ValueFixture, "CSSColorValue: rectifying components",
                   "[CSS][TypedOM][css-typed-om:rectify-a-csscolorpercent]")
  {
    // A number is a fraction of a percentage, an angle in degrees, or a number.
    CHECK(Text(*std::get_if<RefPtr<CSSNumericValue>>(&CSSColorValue::RectifyCSSColorPercent(0.5).Value()))
          == "50%");
    CHECK(Text(CSSColorValue::ToCSSColorComponent(CSSColorValue::RectifyCSSColorAngle(30.0).Value()))
          == "30deg");
    CHECK(Text(CSSColorValue::ToCSSColorComponent(CSSColorValue::RectifyCSSColorNumber(2.0).Value())) == "2");

    // `none`, in any case, is the only keyword.
    CHECK(Text(CSSColorValue::ToCSSColorComponent(
            CSSColorValue::RectifyCSSColorPercent(CSSOMString(u8"NONE")).Value()))
          == "NONE");
    CHECK(CSSColorValue::RectifyCSSColorPercent(CSSOMString(u8"auto")).GetException().Code()
          == dom::ExceptionCode::SyntaxError);
    CHECK(CSSColorValue::RectifyCSSColorAngle(Numeric(Factory::Px(1))).GetException().Code()
          == dom::ExceptionCode::SyntaxError);
  }

  TEST_CASE_METHOD(ValueFixture, "CSSColorValue: colour spaces", "[CSS][TypedOM][css-typed-om:csscolorvalue]")
  {
    auto rgb = Value(CSSRGB::Create(0.5, Numeric(Factory::Number(255)), CSSOMString(u8"none")));
    CHECK(Space(*rgb) == "srgb");
    CHECK(Text(rgb->R()) == "50%");
    CHECK(Text(rgb->G()) == "255");
    CHECK(Text(rgb->B()) == "none");
    CHECK(Text(rgb->Alpha()) == "100%");
    CHECK(rgb->SetR(Numeric(Factory::Px(1))).GetException().Code() == dom::ExceptionCode::SyntaxError);
    CHECK(Is<CSSColorValue>(static_cast<const CSSStyleValue &>(*rgb)));

    auto hsl = Value(CSSHSL::Create(30.0, 0.5, 0.25));
    CHECK(Space(*hsl) == "hsl");
    CHECK(Text(hsl->H()) == "30deg");
    CHECK(Text(hsl->L()) == "25%");
    CHECK(hsl->SetS(Numeric(Factory::Number(1))).HasException());

    auto hwb = Value(CSSHWB::Create(Factory::Deg(30), 0.2, 0.3));
    CHECK(Space(*hwb) == "hwb");
    CHECK(Text(CSSColorValue::ToCSSColorPercent(hwb->W())) == "20%");
    CHECK(CSSHWB::Create(Factory::Px(1), 0.2, 0.3).GetException().Code() == dom::ExceptionCode::SyntaxError);

    CHECK(Space(*Value(CSSLab::Create(0.5, 20.0, -20.0))) == "lab");
    CHECK(Space(*Value(CSSLCH::Create(0.5, 0.2, 90.0))) == "lch");
    CHECK(Space(*Value(CSSOKLab::Create(0.5, 0.1, -0.1))) == "oklab");
    CHECK(Space(*Value(CSSOKLCH::Create(0.5, 0.1, 90.0))) == "oklch");

    auto color = Value(CSSColor::Create(CSSOMString(u8"display-p3"), {0.1, 0.2, CSSOMString(u8"none")}));
    CHECK(Space(*color) == "display-p3");
    REQUIRE(color->Channels().size() == 3);
    CHECK(Text(color->Channels()[2]) == "none");
    color->SetColorSpace(CSSOMString(u8"rec2020"));
    CHECK(Space(*color) == "rec2020");
    CHECK(CSSColor::Create(CSSOMString(u8"srgb"), {CSSOMString(u8"auto")}).HasException());
  }
}
