#include "Krystal.Booey/CSS/Selectors/MutableSelector.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/SelectorTestUtils.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParserContext.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    using B = SelectorBuilder;
  }

  TEST_CASE("MutableSelector::ParsePseudoClassSelector", "[CSS][Selectors][MutableSelector]")
  {
    SelectorParserContext context;

    auto hover = MutableSelector::ParsePseudoClassSelector(u8"HoVeR", context);
    REQUIRE(hover);
    CHECK(hover->Match() == SelectorMatch::PseudoClass);
    CHECK(hover->PseudoClass() == SelectorPseudoClass::Hover);

    // Single-colon pseudo-elements keep their lowercased name as the value.
    auto before = MutableSelector::ParsePseudoClassSelector(u8"Before", context);
    REQUIRE(before);
    CHECK(before->Match() == SelectorMatch::PseudoElement);
    CHECK(before->PseudoElement() == SelectorPseudoElement::Before);
    CHECK(before->Value() == CSSOMStringAtom(u8"before"));
    CHECK(B::List(krys::move(before)).SelectorsText() == u8"::before");

    CHECK(!MutableSelector::ParsePseudoClassSelector(u8"nope", context));
    CHECK(MutableSelector::ParsePseudoClassSelector(u8"popover-open", context));
  }

  TEST_CASE("MutableSelector::ParsePseudoElementSelector", "[CSS][Selectors][MutableSelector]")
  {
    SelectorParserContext context;

    auto marker = MutableSelector::ParsePseudoElementSelector(u8"MARKER", context);
    REQUIRE(marker);
    CHECK(marker->Match() == SelectorMatch::PseudoElement);
    CHECK(marker->PseudoElement() == SelectorPseudoElement::Marker);
    CHECK(marker->Value() == CSSOMStringAtom(u8"marker"));

    auto part = MutableSelector::ParsePseudoElementSelector(u8"Details-Content", context);
    REQUIRE(part);
    CHECK(part->PseudoElement() == SelectorPseudoElement::UserAgentPart);
    CHECK(B::List(krys::move(part)).SelectorsText() == u8"::details-content");

    CHECK(!MutableSelector::ParsePseudoElementSelector(u8"hover", context));
    CHECK(MutableSelector::ParsePseudoElementSelector(u8"target-text", context));
  }

  TEST_CASE("MutableSelector(complex selector building)", "[CSS][Selectors][MutableSelector]")
  {
    auto selector = B::Class(u8"a");
    CHECK(selector->LeftmostSimpleSelector() == selector.get());
    CHECK(!selector->StartsWithExplicitCombinator());

    // `.b .a`, then relative `.c > .b .a` keeps the explicit combinator.
    selector->PrependComplexSelectorComponentAsRelative(B::Class(u8"b"));
    CHECK(selector->GetSelector()->Relation() == SelectorRelation::Descendant);
    selector->LeftmostSimpleSelector()->SetRelation(SelectorRelation::Child);
    CHECK(selector->StartsWithExplicitCombinator());
    selector->PrependComplexSelectorComponentAsRelative(B::Class(u8"c"));
    CHECK(selector->LeftmostSimpleSelector()->Value() == CSSOMStringAtom(u8"c"));
    CHECK(B::List(krys::move(selector)).SelectorsText() == u8".c > .b .a");

    // A tag is put at the start of the compound.
    auto compound = B::Compound(B::Class(u8"x"), B::Id(u8"y"));
    compound->AppendTagInComplexSelector(
      dom::QualifiedName(dom::StarAtom(), CSSOMStringAtom::Null(), CSSOMStringAtom(u8"div")));
    CHECK(compound->Match() == SelectorMatch::Type);
    CHECK(B::List(krys::move(compound)).SelectorsText() == u8"div.x#y");

    // Releasing the preceding components leaves the subject.
    auto complex = B::Combine(B::Type(u8"div"), SelectorRelation::Child, B::Class(u8"a"));
    auto preceding = complex->ReleaseFromComplexSelector();
    CHECK(complex->GetSelector()->Relation() == SelectorRelation::Compounding);
    CHECK(!complex->PrecedingComplexSelectorComponent());
    CHECK(B::List(krys::move(complex)).SelectorsText() == u8".a");
    CHECK(B::List(krys::move(preceding)).SelectorsText() == u8"div");
  }

  TEST_CASE("MutableSelector(copy from Selector)", "[CSS][Selectors][MutableSelector]")
  {
    auto list = B::List(
      B::Combine(B::Type(u8"div"), SelectorRelation::Child, B::Compound(B::Type(u8"p"), B::Class(u8"a"))),
      B::Id(u8"z"));

    auto chain = CreateUnique<MutableSelector>(*list.FirstSelectorComponent());
    auto simple =
      CreateUnique<MutableSelector>(*list.FirstSelectorComponent(), MutableSelector::SimpleSelectorTag {});
    CHECK(!simple->PrecedingComplexSelectorComponent());

    // The selector list bits are reset, so the copies can go into a new list.
    auto copied = B::List(krys::move(chain), krys::move(simple));
    CHECK(copied.SelectorsText() == u8"div > p.a, p");
    CHECK(copied.SelectorCount() == 2);
  }

  TEST_CASE("MutableSelector(queries)", "[CSS][Selectors][MutableSelector]")
  {
    CHECK(B::PseudoElement(SelectorPseudoElement::Before, u8"before")->MatchesPseudoElement());
    CHECK(B::PseudoClass(SelectorPseudoClass::Is,
                         B::MutableList(B::PseudoElement(SelectorPseudoElement::Before, u8"before")))
            ->MatchesPseudoElement());
    CHECK(!B::PseudoClass(SelectorPseudoClass::Is, B::MutableList(B::Class(u8"a")))->MatchesPseudoElement());

    CHECK(
      B::PseudoElement(SelectorPseudoElement::Part, u8"part")->NeedsImplicitShadowCombinatorForMatching());
    CHECK(B::PseudoElement(SelectorPseudoElement::UserAgentPart, u8"details-content")
            ->NeedsImplicitShadowCombinatorForMatching());
    CHECK(!B::PseudoElement(SelectorPseudoElement::Before, u8"before")
             ->NeedsImplicitShadowCombinatorForMatching());
    CHECK(!B::Class(u8"a")->NeedsImplicitShadowCombinatorForMatching());

    CHECK(B::PseudoClass(SelectorPseudoClass::Host)->IsHostPseudoClass());
    CHECK(
      B::Combine(B::NestingParent(), SelectorRelation::Child, B::Class(u8"a"))->HasExplicitNestingParent());
    CHECK(!B::Class(u8"a")->HasExplicitNestingParent());
    CHECK(B::Combine(B::PseudoClass(SelectorPseudoClass::Scope), SelectorRelation::Child, B::Class(u8"a"))
            ->HasExplicitPseudoClassScope());
    CHECK(!B::Class(u8"a")->HasExplicitPseudoClassScope());
  }

  TEST_CASE("MutableSelector(long chain)", "[CSS][Selectors][MutableSelector]")
  {
    // Destroying a long chain must not recurse once per component.
    auto selector = B::Class(u8"a");
    for (int32 i = 0; i < 100'000; ++i)
    {
      auto next = B::Class(u8"a");
      next->SetPrecedingComplexSelectorComponent(krys::move(selector));
      selector = krys::move(next);
    }
    selector.reset();
    SUCCEED();
  }
}
