#include "Krystal.Booey/MathML/MathMLPresentationElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/MathML/MathMLFractionElement.hpp"
#include "Krystal.Booey/MathML/MathMLMencloseElement.hpp"
#include "Krystal.Booey/MathML/MathMLPaddedElement.hpp"
#include "Krystal.Booey/MathML/MathMLSpaceElement.hpp"
#include "Krystal.Booey/MathML/MathMLUnderOverElement.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::mathml::tests
{
  using style::tests::ResolverFixture;
  using Length = MathMLElement::Length;
  using LengthType = MathMLElement::LengthType;
  using BooleanValue = MathMLElement::BooleanValue;

  TEST_CASE("MathMLPresentationElement: parsing lengths", "[MathML][MathMLPresentationElement]")
  {
    auto parse = [](dom::DOMStringView string)
    {
      return MathMLPresentationElement::ParseMathMLLength(string);
    };

    CHECK(parse(u8"12px") == Length {.Type = LengthType::Px, .Value = 12});
    CHECK(parse(u8" -1.5em\t") == Length {.Type = LengthType::Em, .Value = -1.5f});
    CHECK(parse(u8".5ex") == Length {.Type = LengthType::Ex, .Value = 0.5f});
    CHECK(parse(u8"2in") == Length {.Type = LengthType::In, .Value = 2});
    CHECK(parse(u8"3cm") == Length {.Type = LengthType::Cm, .Value = 3});
    CHECK(parse(u8"4mm") == Length {.Type = LengthType::Mm, .Value = 4});
    CHECK(parse(u8"5pt") == Length {.Type = LengthType::Pt, .Value = 5});
    CHECK(parse(u8"6pc") == Length {.Type = LengthType::Pc, .Value = 6});
    CHECK(parse(u8"50%") == Length {.Type = LengthType::Percentage, .Value = 50});
    CHECK(parse(u8"0") == Length {.Type = LengthType::UnitLess, .Value = 0});

    // MathML Core has no unitless values other than 0, no named spaces, and no CSS-only units.
    for (auto string : {u8"", u8"  ", u8"2", u8"1.5", u8"thinmathspace", u8"negativemediummathspace", u8"px",
                        u8"1rem", u8"1 px", u8"x1px", u8"+1px", u8"-infpx", u8"1e"})
    {
      CAPTURE(css::tests::ToNarrow(string));
      CHECK(parse(string).Type == LengthType::ParsingFailed);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "MathMLPresentationElement: lengths and booleans of elements",
                   "[MathML][MathMLPresentationElement]")
  {
    auto document = ResolverFixture::Parse(
      u8"<math><mfrac id=f linethickness=2px></mfrac><mspace id=s width=1em height=2ex depth=bad></mspace>"
      u8"<mpadded id=p width=10% height=0 depth=3px lspace=-1em voffset=4pt></mpadded>"
      u8"<munderover id=u accent=TRUE accentunder=no></munderover></math>");

    auto &fraction = Downcast<MathMLFractionElement>(*document->GetElementById(u8"f"));
    CHECK(fraction.LineThickness() == Length {.Type = LengthType::Px, .Value = 2});

    auto &space = Downcast<MathMLSpaceElement>(*document->GetElementById(u8"s"));
    CHECK(space.Width() == Length {.Type = LengthType::Em, .Value = 1});
    CHECK(space.Height() == Length {.Type = LengthType::Ex, .Value = 2});
    CHECK(space.Depth().Type == LengthType::ParsingFailed);

    auto &padded = Downcast<MathMLPaddedElement>(*document->GetElementById(u8"p"));
    CHECK(padded.Width() == Length {.Type = LengthType::Percentage, .Value = 10});
    CHECK(padded.Height() == Length {.Type = LengthType::UnitLess, .Value = 0});
    CHECK(padded.Depth() == Length {.Type = LengthType::Px, .Value = 3});
    CHECK(padded.Lspace() == Length {.Type = LengthType::Em, .Value = -1});
    CHECK(padded.Voffset() == Length {.Type = LengthType::Pt, .Value = 4});

    auto &underOver = Downcast<MathMLUnderOverElement>(*document->GetElementById(u8"u"));
    // Booleans are ASCII case-insensitive, and anything else is the default.
    CHECK(underOver.Accent() == BooleanValue::True);
    CHECK(underOver.AccentUnder() == BooleanValue::Default);

    SECTION("Changing an attribute drops its cached value")
    {
      REQUIRE_FALSE(fraction.SetAttribute(u8"linethickness", u8"3em").HasException());
      CHECK(fraction.LineThickness() == Length {.Type = LengthType::Em, .Value = 3});
      fraction.RemoveAttribute(u8"linethickness");
      CHECK(fraction.LineThickness().Type == LengthType::ParsingFailed);

      REQUIRE_FALSE(space.SetAttribute(u8"depth", u8"1px").HasException());
      CHECK(space.Depth() == Length {.Type = LengthType::Px, .Value = 1});

      REQUIRE_FALSE(padded.SetAttribute(u8"voffset", u8"2px").HasException());
      CHECK(padded.Voffset() == Length {.Type = LengthType::Px, .Value = 2});

      REQUIRE_FALSE(underOver.SetAttribute(u8"accentunder", u8"false").HasException());
      CHECK(underOver.AccentUnder() == BooleanValue::False);
      REQUIRE_FALSE(underOver.SetAttribute(u8"accent", u8"").HasException());
      CHECK(underOver.Accent() == BooleanValue::Default);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "MathMLMencloseElement: notations",
                   "[MathML][MathMLMencloseElement]")
  {
    using enum MathMLMencloseElement::MencloseNotationFlag;

    auto document = ResolverFixture::Parse(u8"<math><menclose id=d></menclose>"
                                           u8"<menclose id=e notation=\" circle\tbox  updiagonalstrike \">"
                                           u8"</menclose><menclose id=n notation=\"\"></menclose></math>");

    // The default notation is longdiv.
    auto &byDefault = Downcast<MathMLMencloseElement>(*document->GetElementById(u8"d"));
    CHECK(byDefault.HasNotation(LongDiv));
    CHECK_FALSE(byDefault.HasNotation(Circle));

    auto &element = Downcast<MathMLMencloseElement>(*document->GetElementById(u8"e"));
    for (auto notation : {Circle, Left, Right, Top, Bottom, UpDiagonalStrike})
    {
      CHECK(element.HasNotation(notation));
    }
    CHECK_FALSE(element.HasNotation(LongDiv));
    CHECK_FALSE(element.HasNotation(RoundedBox));

    // An empty notation draws nothing.
    auto &none = Downcast<MathMLMencloseElement>(*document->GetElementById(u8"n"));
    CHECK_FALSE(none.HasNotation(LongDiv));

    SECTION("Changing the notation reparses it")
    {
      REQUIRE_FALSE(element.SetAttribute(u8"notation", u8"actuarial madruwb unknown").HasException());
      CHECK(element.HasNotation(Right));
      CHECK(element.HasNotation(Top));
      CHECK(element.HasNotation(Bottom));
      CHECK_FALSE(element.HasNotation(Left));
      CHECK_FALSE(element.HasNotation(Circle));

      element.RemoveAttribute(u8"notation");
      CHECK(element.HasNotation(LongDiv));
    }
  }
}
