#include "Krystal.Web/Css/TypedOM/Color/CSSColorValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Css/TypedOMTestUtils.hpp"
#include "Krystal.Web.Tests/Css/ValueTestUtils.hpp"
#include "Krystal.Web/Css/TypedOM/Color/CSSColor.hpp"
#include "Krystal.Web/Css/TypedOM/Color/CSSHSL.hpp"
#include "Krystal.Web/Css/TypedOM/Color/CSSHWB.hpp"
#include "Krystal.Web/Css/TypedOM/Color/CSSLab.hpp"
#include "Krystal.Web/Css/TypedOM/Color/CSSLCH.hpp"
#include "Krystal.Web/Css/TypedOM/Color/CSSOKLab.hpp"
#include "Krystal.Web/Css/TypedOM/Color/CSSOKLCH.hpp"
#include "Krystal.Web/Css/TypedOM/Color/CSSRGB.hpp"
#include "Krystal.Web/Css/TypedOM/CSSKeywordValue.hpp"
#include "Krystal.Web/Css/TypedOM/CSSNumericFactory.hpp"
#include "Krystal.Web/Css/TypedOM/CSSUnitValue.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"

namespace krys::web::css::tests
{
  namespace
  {
    using Factory = CSSNumericFactory;

    /// @brief The component as text: a numeric value's serialisation, or the keyword.
    KRYS_NODISCARD std::string ComponentText(const CSSColorPercent &component)
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

    KRYS_NODISCARD std::string Space(const CSSColor &color)
    {
      return ToNarrow(color.ColorSpace()->Value());
    }

    KRYS_NODISCARD std::string Function(const CSSColorValue &color)
    {
      return ToNarrow(color.ColorFunctionText());
    }

    KRYS_NODISCARD CSSColorPercent AsColorPercent(Ref<CSSUnitValue> value)
    {
      return RefPtr<CSSNumericValue>(krys::move(value));
    }
  }

  TEST_CASE_METHOD(ValueFixture, "CSSColorValue: rectifying components",
                   "[CSS][TypedOM][css-typed-om:rectify-a-csscolorpercent]")
  {
    // A number is a fraction of a percentage, an angle in degrees, or a number.
    CHECK(ComponentText(
            *std::get_if<RefPtr<CSSNumericValue>>(&CSSColorValue::RectifyCSSColorPercent(0.5).Value()))
          == "50%");
    CHECK(ComponentText(CSSColorValue::ToCSSColorComponent(CSSColorValue::RectifyCSSColorAngle(30.0).Value()))
          == "30deg");
    CHECK(ComponentText(CSSColorValue::ToCSSColorComponent(CSSColorValue::RectifyCSSColorNumber(2.0).Value()))
          == "2");

    // `none`, in any case, is the only keyword.
    CHECK(ComponentText(CSSColorValue::ToCSSColorComponent(
            CSSColorValue::RectifyCSSColorPercent(CSSOMString(u8"NONE")).Value()))
          == "NONE");
    CHECK(CSSColorValue::RectifyCSSColorPercent(CSSOMString(u8"auto")).GetException().Code()
          == ExceptionCode::SyntaxError);
    CHECK(CSSColorValue::RectifyCSSColorAngle(AsColorPercent(Factory::Px(1))).GetException().Code()
          == ExceptionCode::SyntaxError);
  }

  TEST_CASE_METHOD(ValueFixture, "CSSColorValue: colour functions",
                   "[CSS][TypedOM][css-typed-om:csscolorvalue]")
  {
    auto rgb = RequireValue(CSSRGB::Create(0.5, AsColorPercent(Factory::Number(255)), CSSOMString(u8"none")));
    CHECK(Function(*rgb) == "rgb(50% 255 none / 100%)");
    CHECK(ComponentText(rgb->R()) == "50%");
    CHECK(ComponentText(rgb->G()) == "255");
    CHECK(ComponentText(rgb->B()) == "none");
    CHECK(ComponentText(rgb->Alpha()) == "100%");
    CHECK(rgb->SetR(AsColorPercent(Factory::Px(1))).GetException().Code() == ExceptionCode::SyntaxError);
    CHECK(Is<CSSColorValue>(static_cast<const CSSStyleValue &>(*rgb)));

    auto hsl = RequireValue(CSSHSL::Create(30.0, 0.5, 0.25));
    CHECK(Function(*hsl) == "hsl(30deg 50% 25% / 100%)");
    CHECK(ComponentText(hsl->H()) == "30deg");
    CHECK(ComponentText(hsl->L()) == "25%");
    CHECK(hsl->SetS(AsColorPercent(Factory::Number(1))).HasException());

    auto hwb = RequireValue(CSSHWB::Create(Factory::Deg(30), 0.2, 0.3));
    CHECK(Function(*hwb) == "hwb(30deg 20% 30% / 100%)");
    CHECK(ComponentText(CSSColorValue::ToCSSColorPercent(hwb->W())) == "20%");
    CHECK(CSSHWB::Create(Factory::Px(1), 0.2, 0.3).GetException().Code() == ExceptionCode::SyntaxError);

    CHECK(Function(*RequireValue(CSSLab::Create(0.5, 20.0, -20.0))) == "lab(50% 20 -20 / 100%)");
    CHECK(Function(*RequireValue(CSSLCH::Create(0.5, 0.2, 90.0))) == "lch(50% 20% 90deg / 100%)");
    CHECK(Function(*RequireValue(CSSOKLab::Create(0.5, 0.1, -0.1))) == "oklab(50% 0.1 -0.1 / 100%)");
    CHECK(Function(*RequireValue(CSSOKLCH::Create(0.5, 0.1, 90.0))) == "oklch(50% 10% 90deg / 100%)");

    auto color =
      RequireValue(CSSColor::Create(CSSOMString(u8"display-p3"), {0.1, 0.2, CSSOMString(u8"none")}));
    CHECK(Space(*color) == "display-p3");
    CHECK(Function(*color) == "color(display-p3 10% 20% none / 1)");
    REQUIRE(color->Channels().size() == 3);
    CHECK(ComponentText(color->Channels()[2]) == "none");
    color->SetColorSpace(CSSOMString(u8"rec2020"));
    CHECK(Space(*color) == "rec2020");
    CHECK(CSSColor::Create(CSSOMString(u8"srgb"), {CSSOMString(u8"auto")}).HasException());
  }

  TEST_CASE_METHOD(ValueFixture,
                   "CSSColorValue: colour values serialise, and become CSS values, as their functions",
                   "[CSS][TypedOM][css-typed-om:csscolorvalue]")
  {
    auto rgb = RequireValue(
      CSSRGB::Create(1.0, AsColorPercent(Factory::Number(0)), AsColorPercent(Factory::Number(0)), 0.5));
    CHECK(ToNarrow(rgb->ToString()) == "rgba(255, 0, 0, 0.5)");
    CHECK(rgb->ToCSSValue());
    auto lab = RequireValue(CSSLab::Create(0.5, 20.0, CSSOMString(u8"none")));
    CHECK(ToNarrow(lab->ToString()) == "lab(50 20 none)");
    // Components that don't make a colour make no CSS value.
    auto color = RequireValue(CSSColor::Create(CSSOMString(u8"not-a-space"), {0.1, 0.2, 0.3}));
    CHECK_FALSE(color->ToCSSValue());
    CHECK(color->ToString().empty());
  }

  TEST_CASE_METHOD(ValueFixture, "CSSColorValue: parsing reifies colours",
                   "[CSS][TypedOM][css-typed-om:dom-csscolorvalue-parse][css-typed-om:reify-a-color-value]")
  {
    auto document = CreateRef<Document>();
    auto parse = [&](utf8_stringview text) -> Ref<CSSStyleValue>
    {
      auto parsed = CSSColorValue::Parse(*document, text);
      REQUIRE_FALSE(parsed.HasException());
      return parsed.ReleaseValue();
    };
    auto function = [&](utf8_stringview text) -> std::string
    {
      auto parsed = parse(text);
      auto *color = DynamicDowncast<CSSColorValue>(parsed.get());
      return color ? Function(*color) : "not a colour value: " + ToNarrow(parsed->ToString());
    };

    SECTION("Hex and named colours, and rgb(), are CSSRGBs")
    {
      CHECK(Is<CSSRGB>(*parse(u8"#ff0000")));
      CHECK(function(u8"#ff000080") == "rgb(255 0 0 / 50.196078%)");
      CHECK(function(u8"red") == "rgb(255 0 0 / 100%)");
      CHECK(function(u8"transparent") == "rgb(0 0 0 / 0%)");
      CHECK(function(u8"rgb(255 50% none / 0.25)") == "rgb(255 50% none / 25%)");
      CHECK(function(u8"rgba(1, 2, 3, 40%)") == "rgb(1 2 3 / 40%)");
    }

    SECTION(
      "The other colour functions are their colour values, numbers converted to their components' types")
    {
      CHECK(function(u8"hsl(120 50 25)") == "hsl(120deg 50% 25% / 100%)");
      CHECK(function(u8"hsla(0.5turn, 10%, 20%, 0.5)") == "hsl(0.5turn 10% 20% / 50%)");
      CHECK(function(u8"hwb(90deg 10% 20%)") == "hwb(90deg 10% 20% / 100%)");
      CHECK(function(u8"lab(50 50% -25)") == "lab(50% 62.5 -25 / 100%)");
      CHECK(function(u8"lch(50% 75 30)") == "lch(50% 50% 30deg / 100%)");
      CHECK(function(u8"oklab(0.5 0.1 -50%)") == "oklab(50% 0.1 -0.2 / 100%)");
      CHECK(function(u8"oklch(0.5 0.2 none)") == "oklch(50% 50% none / 100%)");
      CHECK(function(u8"color(display-p3 1 0.5 none / 0.5)") == "color(display-p3 100% 50% none / 50%)");
      auto srgb = parse(u8"color(srgb 0 0 0)");
      CHECK(Space(Downcast<CSSColor>(*srgb)) == "srgb");
      CHECK(function(u8"rgb(calc(50% + 10%) 0 0)") == "rgb(60% 0 0 / 100%)");
    }

    SECTION("Other colour keywords are keywords")
    {
      for (auto [text, keyword] :
           {std::pair {u8"currentcolor", "currentcolor"}, std::pair {u8"Canvas", "Canvas"}})
      {
        auto parsed = parse(text);
        REQUIRE(Is<CSSKeywordValue>(*parsed));
        CHECK(ToNarrow(Downcast<CSSKeywordValue>(*parsed).Value()) == keyword);
      }
    }

    SECTION("Colours the colour values can't hold are plain style values")
    {
      for (auto text : {u8"color-mix(in srgb, red, blue)", u8"rgb(from red r g b)", u8"light-dark(red, blue)",
                        u8"hsl(calc(10) 50% 50%)", u8"hwb(none 10% 20%)"})
      {
        INFO(ToNarrow(text));
        auto parsed = parse(text);
        CHECK_FALSE(Is<CSSColorValue>(*parsed));
        CHECK_FALSE(parsed->ToString().empty());
      }
    }

    SECTION("What isn't a colour is a SyntaxError")
    {
      for (auto text : {u8"", u8"10px", u8"red blue", u8"rgb(1 2)", u8"notacolour"})
      {
        INFO(ToNarrow(text));
        auto parsed = CSSColorValue::Parse(*document, text);
        REQUIRE(parsed.HasException());
        CHECK(parsed.GetException().Code() == ExceptionCode::SyntaxError);
      }
    }
  }
}
