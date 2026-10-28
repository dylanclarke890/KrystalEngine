#include "Krystal.Booey/MathML/MathMLOperatorElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::mathml::tests
{
  using style::tests::ResolverFixture;
  using Dictionary = MathMLOperatorDictionary;
  using Form = MathMLOperatorDictionary::Form;
  using Length = MathMLElement::Length;
  using LengthType = MathMLElement::LengthType;

  TEST_CASE("MathMLOperatorDictionary: searching", "[MathML][MathMLOperatorDictionary]")
  {
    // An entry in the form searched for.
    auto plus = Dictionary::Search(U'+', Form::Infix, true);
    REQUIRE(plus);
    CHECK(*plus
          == Dictionary::Property {
            .Form = Form::Infix, .LeadingSpaceInMathUnit = 4, .TrailingSpaceInMathUnit = 4, .Flags = 0});
    auto prefixPlus = Dictionary::Search(U'+', Form::Prefix, true);
    REQUIRE(prefixPlus);
    CHECK(prefixPlus->LeadingSpaceInMathUnit == 0);
    CHECK(prefixPlus->TrailingSpaceInMathUnit == 1);

    auto parenthesis = Dictionary::Search(U'(', Form::Prefix, true);
    REQUIRE(parenthesis);
    CHECK(parenthesis->Flags == (Dictionary::Symmetric | Dictionary::Fence | Dictionary::Stretchy));

    auto sum = Dictionary::Search(U'∑', Form::Prefix, false);
    REQUIRE(sum);
    CHECK(sum->Flags == (Dictionary::Symmetric | Dictionary::LargeOp | Dictionary::MovableLimits));

    // A missing form isn't found if it was explicit...
    CHECK_FALSE(Dictionary::Search(U'(', Form::Postfix, true));
    // ...and otherwise falls back to the first of the operator's forms: infix, prefix, postfix.
    auto fallback = Dictionary::Search(U'(', Form::Postfix, false);
    REQUIRE(fallback);
    CHECK(fallback->Form == Form::Prefix);
    auto bar = Dictionary::Search(U'|', Form::Infix, true);
    REQUIRE(bar);
    CHECK(bar->LeadingSpaceInMathUnit == 2);

    // Operators the dictionary doesn't have.
    CHECK_FALSE(Dictionary::Search(U'a', Form::Infix, false));
    CHECK_FALSE(Dictionary::Search(0, Form::Infix, false));

    SECTION("The infix circumflex and low line are found")
    {
      // These were listed after their postfix forms, so a binary search over the pairs missed them.
      for (auto character : {U'^', U'_'})
      {
        auto infix = Dictionary::Search(character, Form::Infix, true);
        REQUIRE(infix);
        CHECK(infix->Form == Form::Infix);
        CHECK(infix->LeadingSpaceInMathUnit == 1);
        CHECK(infix->Flags == 0);

        auto postfix = Dictionary::Search(character, Form::Postfix, true);
        REQUIRE(postfix);
        CHECK(postfix->Flags == (Dictionary::Accent | Dictionary::Stretchy));

        // The fallback is the infix form.
        auto infixFallback = Dictionary::Search(character, Form::Prefix, false);
        REQUIRE(infixFallback);
        CHECK(infixFallback->Form == Form::Infix);
      }
    }
  }

  TEST_CASE("MathMLOperatorDictionary: stretch direction", "[MathML][MathMLOperatorDictionary]")
  {
    CHECK(Dictionary::IsVertical(U'('));
    CHECK(Dictionary::IsVertical(U'|'));
    CHECK(Dictionary::IsVertical(U'∫'));
    CHECK_FALSE(Dictionary::IsVertical(U'='));
    CHECK_FALSE(Dictionary::IsVertical(U'→'));
    CHECK_FALSE(Dictionary::IsVertical(U'\U0001EEF1'));
  }

  TEST_CASE("MathMLOperatorElement: the operator's character", "[MathML][MathMLOperatorElement]")
  {
    using Element = MathMLOperatorElement;

    CHECK(Element::ParseOperatorChar(u8"+").Character == U'+');
    CHECK(Element::ParseOperatorChar(u8" \n(\t").Character == U'(');
    CHECK(Element::ParseOperatorChar(u8"(").IsVertical);
    CHECK_FALSE(Element::ParseOperatorChar(u8"→").IsVertical);
    // The hyphen is drawn as the minus sign.
    CHECK(Element::ParseOperatorChar(u8"-").Character == U'−');
    CHECK(Element::ParseOperatorChar(u8"∑").Character == U'∑');

    // Only one character is an operator.
    CHECK(Element::ParseOperatorChar(u8"").Character == 0);
    CHECK(Element::ParseOperatorChar(u8"sin").Character == 0);
    CHECK(Element::ParseOperatorChar(utf8_string(1, char8 {0xFF})).Character == 0);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "MathMLOperatorElement: form and properties",
                   "[MathML][MathMLOperatorElement]")
  {
    auto document = ResolverFixture::Parse(
      u8"<math><mrow><mo id=open>(</mo><mi>x</mi><mo id=plus>+</mo>"
      u8"<mi>y</mi><mo id=close>)</mo></mrow><mo id=alone>-</mo>"
      u8"<mrow><mo id=minus>-</mo><mi>a</mi></mrow><mrow><mo id=sum>∑</mo><mi>i</mi></mrow></math>");
    auto mo = [&](dom::DOMStringView id) -> MathMLOperatorElement &
    {
      return Downcast<MathMLOperatorElement>(*document->GetElementById(id));
    };

    // The form comes from the operator's place in its row.
    CHECK(mo(u8"open").Form() == Form::Prefix);
    CHECK(mo(u8"plus").Form() == Form::Infix);
    CHECK(mo(u8"close").Form() == Form::Postfix);

    CHECK(mo(u8"open").HasProperty(Dictionary::Stretchy));
    CHECK(mo(u8"open").HasProperty(Dictionary::Fence));
    CHECK_FALSE(mo(u8"plus").HasProperty(Dictionary::Stretchy));
    CHECK(mo(u8"sum").HasProperty(Dictionary::LargeOp));
    CHECK(mo(u8"sum").HasProperty(Dictionary::MovableLimits));

    // The dictionary's spacing.
    CHECK(mo(u8"plus").DefaultLeadingSpace() == Length {.Type = LengthType::MathUnit, .Value = 4});
    CHECK(mo(u8"plus").DefaultTrailingSpace() == Length {.Type = LengthType::MathUnit, .Value = 4});
    CHECK(mo(u8"sum").DefaultLeadingSpace() == Length {.Type = LengthType::MathUnit, .Value = 1});
    CHECK(mo(u8"sum").DefaultTrailingSpace() == Length {.Type = LengthType::MathUnit, .Value = 2});
    CHECK(mo(u8"plus").LeadingSpace().Type == LengthType::ParsingFailed);

    // A prefix minus.
    REQUIRE_FALSE(mo(u8"alone").SetAttribute(u8"form", u8"prefix").HasException());
    CHECK(mo(u8"alone").GetOperatorChar().Character == U'−');
    CHECK(mo(u8"alone").DefaultLeadingSpace().Value == 0);
    CHECK(mo(u8"alone").DefaultTrailingSpace().Value == 1);

    SECTION("Attributes override the dictionary")
    {
      auto &plus = mo(u8"plus");
      REQUIRE_FALSE(plus.SetAttribute(u8"stretchy", u8"true").HasException());
      REQUIRE_FALSE(plus.SetAttribute(u8"lspace", u8"2px").HasException());
      REQUIRE_FALSE(plus.SetAttribute(u8"rspace", u8"1em").HasException());
      REQUIRE_FALSE(plus.SetAttribute(u8"minsize", u8"50%").HasException());
      REQUIRE_FALSE(plus.SetAttribute(u8"maxsize", u8"3ex").HasException());
      CHECK(plus.HasProperty(Dictionary::Stretchy));
      CHECK(plus.LeadingSpace() == Length {.Type = LengthType::Px, .Value = 2});
      CHECK(plus.TrailingSpace() == Length {.Type = LengthType::Em, .Value = 1});
      CHECK(plus.MinSize() == Length {.Type = LengthType::Percentage, .Value = 50});
      CHECK(plus.MaxSize() == Length {.Type = LengthType::Ex, .Value = 3});

      auto &open = mo(u8"open");
      REQUIRE_FALSE(open.SetAttribute(u8"stretchy", u8"FALSE").HasException());
      CHECK_FALSE(open.HasProperty(Dictionary::Stretchy));
      // Other properties keep the dictionary's values.
      CHECK(open.HasProperty(Dictionary::Symmetric));

      open.RemoveAttribute(u8"stretchy");
      CHECK(open.HasProperty(Dictionary::Stretchy));
    }

    SECTION("An explicit form")
    {
      auto &open = mo(u8"open");
      REQUIRE_FALSE(open.SetAttribute(u8"form", u8"postfix").HasException());
      CHECK(open.Form() == Form::Postfix);
      // There's no postfix "(", so the defaults apply.
      CHECK_FALSE(open.HasProperty(Dictionary::Stretchy));
      CHECK(open.DefaultLeadingSpace().Value == 5);
    }

    SECTION("Changes to the row and the text")
    {
      // An operator's form follows its siblings.
      auto &minus = mo(u8"minus");
      REQUIRE(minus.Form() == Form::Prefix);
      REQUIRE(minus.DefaultLeadingSpace().Value == 0);
      auto &row = *minus.ParentNode();
      auto before =
        document->CreateElementNS(dom::DOMStringAtom(infra::Namespaces::MathML), dom::DOMStringAtom(u8"mi"));
      REQUIRE_FALSE(row.InsertBefore(*before.ReleaseValue(), &minus).HasException());
      CHECK(minus.Form() == Form::Infix);
      CHECK(minus.DefaultLeadingSpace().Value == 4);

      // A ")" between two siblings has no infix entry, so it keeps its postfix one.
      auto &close = mo(u8"close");
      auto after =
        document->CreateElementNS(dom::DOMStringAtom(infra::Namespaces::MathML), dom::DOMStringAtom(u8"mi"));
      REQUIRE_FALSE(close.ParentNode()->AppendChild(*after.ReleaseValue()).HasException());
      CHECK(close.Form() == Form::Postfix);

      // And its character follows its text.
      auto &plus = mo(u8"plus");
      REQUIRE(plus.HasProperty(Dictionary::Symmetric) == false);
      REQUIRE_FALSE(plus.TextContent(u8"∑").HasException());
      CHECK(plus.GetOperatorChar().Character == U'∑');
      CHECK(plus.HasProperty(Dictionary::LargeOp));
    }
  }
}
