#include "Krystal.Booey/SVG/SVGTextContentElement.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Dicts/DOMPointInit.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"
#include "Krystal.Booey/SVG/SVGPoint.hpp"
#include "Krystal.Booey/SVG/SVGRect.hpp"
#include "Krystal.Booey/SVG/SVGTextElement.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::svg::tests
{
  using Catch::Approx;
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD SVGTextContentElement &TextById(dom::Document &document, dom::DOMStringView id) noexcept
    {
      auto element = document.GetElementById(id);
      krys_assert(element);
      return Downcast<SVGTextContentElement>(Downcast<SVGElement>(*element));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGTextContentElement: the queries",
                   "[SVG][SVGTextContentElement]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100>)html"
      u8R"html(<text id=t x=10 y=20 rotate="0 30">a&#xE9;&#x20AC;b</text></svg></body>)html"};
    auto &text = TextById(*fixture.Document, u8"t");

    // Characters are counted as the DOM counts them, in UTF-8 code units: "é" is two and "€" three.
    CHECK(text.GetNumberOfChars() == 7);
    CHECK(text.GetComputedTextLength() == Approx(35.59375));
    CHECK(text.GetSubStringLength(1, 2).ReleaseValue() == Approx(8.8984375));

    auto start = text.GetStartPositionOfChar(0).ReleaseValue();
    auto end = text.GetEndPositionOfChar(0).ReleaseValue();
    CHECK(start->X() == Approx(10));
    CHECK(start->Y() == Approx(20));
    CHECK(end->X() == Approx(18.8984375));
    CHECK(end->Y() == Approx(20));
    CHECK(text.GetRotationOfChar(0).ReleaseValue() == 0);

    auto extent = text.GetExtentOfChar(0).ReleaseValue();
    CHECK(extent->Value().X() == Approx(10));
    CHECK(extent->Value().Width() == Approx(8.8984375));
    CHECK(extent->Value().Height() == Approx(17.875));

    // The rotate list has a value for each character, the last one repeating for the rest. A code unit inside
    // a character answers for the whole character.
    for (uint32 charnum : {1u, 2u})
    {
      CAPTURE(charnum);
      CHECK(text.GetRotationOfChar(charnum).ReleaseValue() == 30);
      auto rotatedStart = text.GetStartPositionOfChar(charnum).ReleaseValue();
      auto rotatedEnd = text.GetEndPositionOfChar(charnum).ReleaseValue();
      CHECK(rotatedStart->X() == Approx(18.8984375));
      CHECK(rotatedStart->Y() == Approx(20));
      CHECK(rotatedEnd->X() == Approx(26.60471));
      CHECK(rotatedEnd->Y() == Approx(24.449219));
    }
    CHECK(text.GetRotationOfChar(6).ReleaseValue() == 30);
    CHECK(text.GetStartPositionOfChar(3).ReleaseValue()->X() == Approx(27.796875));
    CHECK(text.GetStartPositionOfChar(6).ReleaseValue()->X() == Approx(36.6953125));

    CHECK(text.GetCharNumAtPosition({15, 15}) == 0);
    CHECK(text.GetCharNumAtPosition({0, 0}) == -1);

    // Character numbers past the text throw.
    CHECK(text.GetStartPositionOfChar(7).GetException().Code() == dom::ExceptionCode::IndexSizeError);
    CHECK(text.GetEndPositionOfChar(7).GetException().Code() == dom::ExceptionCode::IndexSizeError);
    CHECK(text.GetExtentOfChar(7).GetException().Code() == dom::ExceptionCode::IndexSizeError);
    CHECK(text.GetRotationOfChar(7).GetException().Code() == dom::ExceptionCode::IndexSizeError);
    CHECK(text.GetSubStringLength(7, 1).GetException().Code() == dom::ExceptionCode::IndexSizeError);
    CHECK(text.SelectSubString(7, 1).GetException().Code() == dom::ExceptionCode::IndexSizeError);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGTextContentElement: selectSubString",
                   "[SVG][SVGTextContentElement]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100>)html"
                             u8R"html(<text id=t x=10 y=20>abcd</text></svg></body>)html"};
    auto &text = TextById(*fixture.Document, u8"t");

    REQUIRE_FALSE(text.SelectSubString(1, 2).HasException());
    auto &selection = fixture.Document->Selection().Selection();
    CHECK(selection.IsRange());
    CHECK(selection.Start().DeprecatedEditingOffset() == 1);
    CHECK(selection.End().DeprecatedEditingOffset() == 3);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGTextContentElement: textLength and lengthAdjust",
                   "[SVG][SVGTextContentElement]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100>)html"
      u8R"html(<text id=a x=10 y=20>abc</text>)html"
      u8R"html(<text id=b x=10 y=40 textLength=100 lengthAdjust=spacingAndGlyphs>abc</text></svg></body>)html"};

    // Without textLength, the textLength property reads as the computed length; lengthAdjust defaults to
    // spacing.
    auto &a = TextById(*fixture.Document, u8"a");
    CHECK(a.TextLengthAnimated().BaseVal()->Value().Value(SVGLengthContext(&a))
          == Approx(a.GetComputedTextLength()));
    CHECK(a.LengthAdjust() == SVGLengthAdjustSpacing);

    // With it, the glyphs are stretched to fill it, but the computed length is still the unstretched
    // advances'.
    auto &b = TextById(*fixture.Document, u8"b");
    CHECK(b.SpecifiedTextLength().Value(SVGLengthContext(&b)) == Approx(100));
    CHECK(b.LengthAdjust() == SVGLengthAdjustSpacingAndGlyphs);
    CHECK(b.GetComputedTextLength() == Approx(a.GetComputedTextLength()));
  }
}
