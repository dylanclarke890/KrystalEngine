#include "Krystal.Booey/MathML/Internals/MathMLElementFactory.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/MathML/MathMLFractionElement.hpp"
#include "Krystal.Booey/MathML/MathMLMathElement.hpp"
#include "Krystal.Booey/MathML/MathMLMencloseElement.hpp"
#include "Krystal.Booey/MathML/MathMLOperatorElement.hpp"
#include "Krystal.Booey/MathML/MathMLPaddedElement.hpp"
#include "Krystal.Booey/MathML/MathMLRootElement.hpp"
#include "Krystal.Booey/MathML/MathMLScriptsElement.hpp"
#include "Krystal.Booey/MathML/MathMLSpaceElement.hpp"
#include "Krystal.Booey/MathML/MathMLTokenElement.hpp"
#include "Krystal.Booey/MathML/MathMLUnderOverElement.hpp"
#include "Krystal.Booey/MathML/MathMLUnknownElement.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::mathml::tests
{
  using html::TagName;
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD Ref<dom::Element> Create(dom::Document &document, dom::DOMStringView localName) noexcept
    {
      auto element = document.CreateElementNS(dom::DOMStringAtom(infra::Namespaces::MathML),
                                              dom::DOMStringAtom(localName));
      krys_assert(!element.HasException());
      krys_assert(element.Value()->LocalName() == localName);
      return element.ReleaseValue();
    }

    KRYS_NODISCARD MathMLElementInterface InterfaceOf(dom::Document &document,
                                                      dom::DOMStringView localName) noexcept
    {
      auto element = Create(document, localName);
      auto *mathMLElement = DynamicDowncast<MathMLElement>(*element);
      return mathMLElement ? mathMLElement->Interface() : MathMLElementInterface::None;
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "MathMLElementFactory: the elements",
                   "[MathML][MathMLElementFactory]")
  {
    auto document = ResolverFixture::Parse(u8"<!DOCTYPE html>");
    using enum MathMLElementInterface;

    CHECK(InterfaceOf(*document, u8"math") == Math);
    CHECK(InterfaceOf(*document, u8"mfrac") == Fraction);
    CHECK(InterfaceOf(*document, u8"menclose") == Menclose);
    CHECK(InterfaceOf(*document, u8"mo") == Operator);
    CHECK(InterfaceOf(*document, u8"mpadded") == Padded);
    CHECK(InterfaceOf(*document, u8"mspace") == Space);
    for (auto localName : {u8"mi", u8"mn", u8"ms", u8"mtext"})
    {
      CAPTURE(css::tests::ToNarrow(localName));
      CHECK(InterfaceOf(*document, localName) == Token);
    }
    for (auto localName : {u8"mrow", u8"merror", u8"mphantom", u8"mstyle", u8"semantics", u8"maction"})
    {
      CAPTURE(css::tests::ToNarrow(localName));
      CHECK(InterfaceOf(*document, localName) == Row);
    }
    for (auto localName :
         {u8"annotation", u8"annotation-xml", u8"mtable", u8"mtr", u8"mtd", u8"mprescripts", u8"none"})
    {
      CAPTURE(css::tests::ToNarrow(localName));
      CHECK(InterfaceOf(*document, localName) == Presentation);
    }

    SECTION("Roots and scripts know their kind")
    {
      CHECK(Downcast<MathMLRootElement>(*Create(*document, u8"msqrt")).GetRootType() == RootType::SquareRoot);
      CHECK(Downcast<MathMLRootElement>(*Create(*document, u8"mroot")).GetRootType()
            == RootType::RootWithIndex);

      using ScriptType = MathMLScriptsElement::ScriptType;
      auto scriptType = [&](dom::DOMStringView localName)
      {
        return Downcast<MathMLScriptsElement>(*Create(*document, localName)).GetScriptType();
      };
      CHECK(scriptType(u8"msub") == ScriptType::Sub);
      CHECK(scriptType(u8"msup") == ScriptType::Super);
      CHECK(scriptType(u8"msubsup") == ScriptType::SubSup);
      CHECK(scriptType(u8"mmultiscripts") == ScriptType::Multiscripts);
      CHECK(scriptType(u8"munder") == ScriptType::Under);
      CHECK(scriptType(u8"mover") == ScriptType::Over);
      CHECK(scriptType(u8"munderover") == ScriptType::UnderOver);
      CHECK(Is<MathMLUnderOverElement>(*Create(*document, u8"munder")));
      CHECK_FALSE(Is<MathMLUnderOverElement>(*Create(*document, u8"msub")));
    }

    SECTION("Elements outside MathML Core are unknown, and laid out as rows")
    {
      for (auto localName : {u8"mfenced", u8"mglyph", u8"malignmark", u8"maligngroup", u8"mlabeledtr",
                             u8"mlongdiv", u8"mstack", u8"msline", u8"div", u8"svg"})
      {
        CAPTURE(css::tests::ToNarrow(localName));
        auto element = Create(*document, localName);
        REQUIRE(Is<MathMLUnknownElement>(*element));
        CHECK_FALSE(Downcast<MathMLElement>(*element).IsPresentationMathML());
      }
    }

    SECTION("Tag names are case-sensitive")
    {
      CHECK(Is<MathMLUnknownElement>(*Create(*document, u8"MFRAC")));
      CHECK(Is<MathMLUnknownElement>(*Create(*document, u8"Math")));
    }

    CHECK(MathMLElementFactory::TryCreate(*document, TagName::div) == nullptr);
    CHECK(MathMLElementFactory::TryCreate(*document, TagName::Unknown) == nullptr);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "MathMLElementFactory: parsed elements",
                   "[MathML][MathMLElementFactory]")
  {
    auto document = ResolverFixture::Parse(u8"<!DOCTYPE html><math id=m><mfrac id=f><mi id=i>x</mi><mo "
                                           u8"id=o>+</mo></mfrac></math><mfrac id=h></mfrac>");

    CHECK(Is<MathMLFractionElement>(*document->GetElementById(u8"f")));
    CHECK(Is<MathMLTokenElement>(*document->GetElementById(u8"i")));
    CHECK(Is<MathMLTokenElement>(*document->GetElementById(u8"o")));
    CHECK(Is<MathMLOperatorElement>(*document->GetElementById(u8"o")));
    CHECK(Is<MathMLMathElement>(*document->GetElementById(u8"m")));

    // MathML's tag names in HTML are unknown HTML elements.
    CHECK_FALSE(Is<MathMLElement>(*document->GetElementById(u8"h")));
  }

  TEST_CASE("ParseTagName: MathML's tag names", "[HTML][TagName]")
  {
    CHECK(html::ParseTagName(u8"mfrac") == TagName::mfrac);
    CHECK(html::ParseTagName(u8"mmultiscripts") == TagName::mmultiscripts);
    CHECK(html::ParseTagName(u8"mprescripts") == TagName::mprescripts);
    CHECK(html::ParseTagName(u8"munderover") == TagName::munderover);
    CHECK(html::ParseTagName(u8"annotation") == TagName::annotation);
    CHECK(html::ParseTagName(u8"annotation-xml") == TagName::annotation_xml);
    CHECK(html::ParseTagName(u8"none") == TagName::none);
    CHECK(html::ParseTagName(u8"mtd") == TagName::mtd);
    CHECK(html::ParseTagName(u8"mfenced") == TagName::Unknown);
    CHECK(html::ParseTagName(u8"MFRAC") == TagName::Unknown);
  }
}
