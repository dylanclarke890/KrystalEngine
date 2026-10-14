#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/SelectorTestUtils.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParserContext.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    using B = SelectorBuilder;

    KRYS_NODISCARD CSSOMString Text(UniquePtr<MutableSelector> selector) noexcept
    {
      return B::List(krys::move(selector)).SelectorsText();
    }

    KRYS_NODISCARD uint32 Specificity(UniquePtr<MutableSelector> selector) noexcept
    {
      return B::List(krys::move(selector)).FirstSelectorComponent()->ComputeSpecificity();
    }

    KRYS_NODISCARD UniquePtr<MutableSelector> Nth(SelectorPseudoClass pseudoClass, int32 a, int32 b) noexcept
    {
      auto selector = B::PseudoClass(pseudoClass);
      selector->SetNth(a, b);
      selector->SetArgument(CSSOMStringAtom(u8"an+b"));
      return selector;
    }

    KRYS_NODISCARD UniquePtr<MutableSelector> WithArguments(UniquePtr<MutableSelector> selector,
                                                            FixedVector<CSSOMStringAtom> arguments) noexcept
    {
      selector->SetArgumentList(krys::move(arguments));
      return selector;
    }
  }

  TEST_CASE("Selector(enum traits)", "[CSS][Selectors][Selector]")
  {
    STATIC_CHECK(EnumTraits<SelectorMatch>::DistinctValues == 18uz);
    STATIC_CHECK(EnumTraits<SelectorRelation>::DistinctValues == 8uz);
    STATIC_CHECK(EnumTraits<SelectorPagePseudoClass>::DistinctValues == 3uz);
    STATIC_CHECK(EnumTraits<PseudoElementType>::DistinctValues == 17uz);
    STATIC_CHECK(ToUnderlying(SelectorMatch::ForgivingUnknownNestContaining) == 17u);
    STATIC_CHECK(ToUnderlying(SelectorRelation::ShadowSlotted) == 7u);
  }

  TEST_CASE("Selector::SelectorText(type, id, class, nesting)", "[CSS][Selectors][Selector]")
  {
    CHECK(Text(B::Type(u8"div")) == u8"div");
    CHECK(Text(B::Type(u8"*")) == u8"*");
    CHECK(Text(B::Type(u8"rect", CSSOMStringAtom(u8"svg"))) == u8"svg|rect");
    CHECK(Text(B::Type(u8"*", dom::StarAtom())) == u8"*|*");
    CHECK(Text(B::Type(u8"p", CSSOMStringAtom::Empty())) == u8"|p");
    CHECK(Text(B::Compound(B::Type(u8"div"), B::Id(u8"a"), B::Class(u8"b"))) == u8"div#a.b");
    CHECK(Text(B::Class(u8"a b")) == u8".a\\ b");
    CHECK(Text(B::Id(u8"1x")) == u8"#\\31 x");
    CHECK(Text(B::NestingParent()) == u8"&");
    CHECK(Text(B::Compound(B::NestingParent(), B::Class(u8"a"))) == u8"&.a");
    CHECK(Text(B::Simple(SelectorMatch::ForgivingUnknown, u8"!!x")) == u8"!!x");
  }

  TEST_CASE("Selector::SelectorText(combinators)", "[CSS][Selectors][Selector]")
  {
    CHECK(Text(B::Combine(B::Type(u8"div"), SelectorRelation::Child, B::Type(u8"p"))) == u8"div > p");
    CHECK(
      Text(B::Combine(B::Combine(B::Combine(B::Type(u8"a"), SelectorRelation::NextSibling, B::Type(u8"b")),
                                 SelectorRelation::SubsequentSibling, B::Type(u8"c")),
                      SelectorRelation::Descendant, B::Type(u8"d")))
      == u8"a + b ~ c d");
    CHECK(Text(B::Combine(B::Compound(B::Type(u8"ul"), B::Class(u8"x")), SelectorRelation::Child,
                          B::Compound(B::Type(u8"li"), B::Class(u8"y"), B::Class(u8"z"))))
          == u8"ul.x > li.y.z");
  }

  TEST_CASE("Selector::SelectorText(implicit and relative)", "[CSS][Selectors][Selector]")
  {
    auto implicitParent = []
    {
      auto selector = B::NestingParent();
      selector->SetImplicit();
      return selector;
    };

    CHECK(Text(B::Combine(implicitParent(), SelectorRelation::Child, B::Class(u8"a"))) == u8"> .a");
    CHECK(Text(B::Combine(implicitParent(), SelectorRelation::Descendant, B::Class(u8"a"))) == u8".a");
    CHECK(Text(B::Combine(implicitParent(), SelectorRelation::NextSibling, B::Class(u8"a"))) == u8"+ .a");

    auto hasScope = CreateUnique<MutableSelector>();
    hasScope->SetMatch(SelectorMatch::HasScope);
    auto relative = B::Combine(krys::move(hasScope), SelectorRelation::Child, B::Class(u8"a"));
    CHECK(Text(B::PseudoClass(SelectorPseudoClass::Has, B::MutableList(krys::move(relative))))
          == u8":has(> .a)");

    auto hasScope2 = CreateUnique<MutableSelector>();
    hasScope2->SetMatch(SelectorMatch::HasScope);
    auto descendant = B::Combine(krys::move(hasScope2), SelectorRelation::Descendant, B::Class(u8"b"));
    CHECK(Text(B::PseudoClass(SelectorPseudoClass::Has, B::MutableList(krys::move(descendant))))
          == u8":has(.b)");
  }

  TEST_CASE("Selector::SelectorText(attributes)", "[CSS][Selectors][Selector]")
  {
    CHECK(Text(B::Attribute(SelectorMatch::AttributeExists, u8"href")) == u8"[href]");
    CHECK(Text(B::Attribute(SelectorMatch::AttributeEquals, u8"a", u8"b")) == u8"[a=\"b\"]");
    CHECK(Text(B::Attribute(SelectorMatch::AttributeIncludes, u8"a", u8"b")) == u8"[a~=\"b\"]");
    CHECK(Text(B::Attribute(SelectorMatch::AttributeDash, u8"lang", u8"en")) == u8"[lang|=\"en\"]");
    CHECK(Text(B::Attribute(SelectorMatch::AttributePrefix, u8"a", u8"b")) == u8"[a^=\"b\"]");
    CHECK(Text(B::Attribute(SelectorMatch::AttributeSuffix, u8"a", u8"b")) == u8"[a$=\"b\"]");
    CHECK(Text(B::Attribute(SelectorMatch::AttributeSubstring, u8"a", u8"b")) == u8"[a*=\"b\"]");
    CHECK(Text(B::Attribute(SelectorMatch::AttributeEquals, u8"a", u8"B", IsCaseSensitive(false)))
          == u8"[a=\"B\" i]");
    CHECK(Text(B::Attribute(SelectorMatch::AttributeEquals, u8"href", u8"x", IsCaseSensitive(true),
                            CSSOMStringAtom(u8"xlink")))
          == u8"[xlink|href=\"x\"]");
    CHECK(
      Text(B::Attribute(SelectorMatch::AttributeExists, u8"a", {}, IsCaseSensitive(true), dom::StarAtom()))
      == u8"[*|a]");
    // An empty prefix (`[|a]`, no namespace) serialises without it.
    CHECK(Text(B::Attribute(SelectorMatch::AttributeExists, u8"a", {}, IsCaseSensitive(true),
                            CSSOMStringAtom::Empty()))
          == u8"[a]");
    CHECK(Text(B::Attribute(SelectorMatch::AttributeEquals, u8"a", u8"q\"t")) == u8"[a=\"q\\\"t\"]");
  }

  TEST_CASE("Selector::SelectorText(pseudo-classes)", "[CSS][Selectors][Selector]")
  {
    CHECK(Text(B::PseudoClass(SelectorPseudoClass::Hover)) == u8":hover");
    CHECK(Text(B::Compound(B::Type(u8"a"), B::PseudoClass(SelectorPseudoClass::AnyLink))) == u8"a:any-link");
    CHECK(Text(B::PseudoClass(SelectorPseudoClass::Host)) == u8":host");
    CHECK(Text(B::PseudoClass(SelectorPseudoClass::Host, B::MutableList(B::Class(u8"a")))) == u8":host(.a)");
    CHECK(Text(B::PseudoClass(SelectorPseudoClass::Not, B::MutableList(B::Class(u8"a"), B::Class(u8"b"))))
          == u8":not(.a, .b)");
    CHECK(Text(B::PseudoClass(
            SelectorPseudoClass::Is,
            B::MutableList(B::Combine(B::Type(u8"a"), SelectorRelation::Child, B::Type(u8"b")))))
          == u8":is(a > b)");
    CHECK(Text(B::PseudoClass(SelectorPseudoClass::Where, B::MutableList(B::Id(u8"x")))) == u8":where(#x)");

    auto dir = B::PseudoClass(SelectorPseudoClass::Dir);
    dir->SetArgument(CSSOMStringAtom(u8"ltr"));
    CHECK(Text(krys::move(dir)) == u8":dir(ltr)");

    auto state = B::PseudoClass(SelectorPseudoClass::State);
    state->SetArgument(CSSOMStringAtom(u8"my state"));
    CHECK(Text(krys::move(state)) == u8":state(my\\ state)");

    auto lang = B::PseudoClass(SelectorPseudoClass::Lang);
    lang->SetLangList({{CSSOMStringAtom(u8"en"), true}, {CSSOMStringAtom(u8"fr"), false}});
    CHECK(Text(krys::move(lang)) == u8":lang(\"en\", fr)");

    CHECK(Text(WithArguments(B::PseudoClass(SelectorPseudoClass::ActiveViewTransitionType),
                             {CSSOMStringAtom(u8"a"), CSSOMStringAtom(u8"b")}))
          == u8":active-view-transition-type(a, b)");
  }

  TEST_CASE("Selector::SelectorText(An+B)", "[CSS][Selectors][Selector]")
  {
    CHECK(Text(Nth(SelectorPseudoClass::NthChild, 2, 1)) == u8":nth-child(2n+1)");
    CHECK(Text(Nth(SelectorPseudoClass::NthChild, 0, 0)) == u8":nth-child(0)");
    CHECK(Text(Nth(SelectorPseudoClass::NthChild, 0, 5)) == u8":nth-child(5)");
    CHECK(Text(Nth(SelectorPseudoClass::NthChild, 0, -5)) == u8":nth-child(-5)");
    CHECK(Text(Nth(SelectorPseudoClass::NthChild, 1, 0)) == u8":nth-child(n)");
    CHECK(Text(Nth(SelectorPseudoClass::NthChild, -1, 0)) == u8":nth-child(-n)");
    CHECK(Text(Nth(SelectorPseudoClass::NthLastChild, -1, 3)) == u8":nth-last-child(-n+3)");
    CHECK(Text(Nth(SelectorPseudoClass::NthChild, 3, -2)) == u8":nth-child(3n-2)");
    CHECK(Text(Nth(SelectorPseudoClass::NthChild, 10, 0)) == u8":nth-child(10n)");

    auto of = Nth(SelectorPseudoClass::NthChild, 2, 0);
    of->AdoptSelectorVector(B::MutableList(B::Class(u8"a"), B::Type(u8"p")));
    CHECK(Text(krys::move(of)) == u8":nth-child(2n of .a, p)");

    // :nth-of-type keeps its argument as written.
    auto ofType = B::PseudoClass(SelectorPseudoClass::NthOfType);
    ofType->SetNth(2, 1);
    ofType->SetArgument(CSSOMStringAtom(u8"odd"));
    CHECK(Text(krys::move(ofType)) == u8":nth-of-type(odd)");
  }

  TEST_CASE("Selector::SelectorText(pseudo-elements)", "[CSS][Selectors][Selector]")
  {
    CHECK(Text(B::PseudoElement(SelectorPseudoElement::Before, u8"before")) == u8"::before");
    CHECK(
      Text(B::Compound(B::Type(u8"p"), B::PseudoElement(SelectorPseudoElement::FirstLine, u8"first-line")))
      == u8"p::first-line");
    CHECK(Text(B::PseudoElement(SelectorPseudoElement::UserAgentPart, u8"details-content"))
          == u8"::details-content");
    CHECK(Text(WithArguments(B::PseudoElement(SelectorPseudoElement::Part, u8"part"),
                             {CSSOMStringAtom(u8"a"), CSSOMStringAtom(u8"b")}))
          == u8"::part(a b)");
    CHECK(Text(WithArguments(B::PseudoElement(SelectorPseudoElement::Highlight, u8"highlight"),
                             {CSSOMStringAtom(u8"x")}))
          == u8"::highlight(x)");
    CHECK(Text(WithArguments(
            B::PseudoElement(SelectorPseudoElement::ViewTransitionGroup, u8"view-transition-group"),
            {dom::StarAtom(), CSSOMStringAtom(u8"a"), CSSOMStringAtom(u8"b")}))
          == u8"::view-transition-group(*.a.b)");

    auto slotted = B::PseudoElement(SelectorPseudoElement::Slotted, u8"slotted");
    slotted->AdoptSelectorVector(B::MutableList(B::Class(u8"a")));
    CHECK(Text(krys::move(slotted)) == u8"::slotted(.a)");

    CHECK(Text(B::PseudoElement(SelectorPseudoElement::Cue, u8"cue")) == u8"::cue");
    auto cue = B::PseudoElement(SelectorPseudoElement::Cue, u8"cue");
    cue->AdoptSelectorVector(B::MutableList(B::Type(u8"b")));
    CHECK(Text(krys::move(cue)) == u8"::cue(b)");
  }

  TEST_CASE("Selector::SelectorText(page)", "[CSS][Selectors][Selector]")
  {
    auto first = MutableSelector::ParsePagePseudoSelector(u8"first");
    REQUIRE(first);
    CHECK(Text(krys::move(first)) == u8":first");
    CHECK(Text(MutableSelector::ParsePagePseudoSelector(u8"LEFT")) == u8":left");
    CHECK(Text(MutableSelector::ParsePagePseudoSelector(u8"right")) == u8":right");
    CHECK(!MutableSelector::ParsePagePseudoSelector(u8"blank"));
  }

  TEST_CASE("Selector::SetValue(match lowercase)", "[CSS][Selectors][Selector]")
  {
    auto quirky = CreateUnique<MutableSelector>();
    quirky->SetMatch(SelectorMatch::Class);
    quirky->SetValue(CSSOMStringAtom(u8"FooBar"), MatchLowercase(true));
    auto list = B::List(krys::move(quirky));
    const auto &selector = *list.FirstSelectorComponent();
    CHECK(selector.Value() == CSSOMStringAtom(u8"foobar"));
    CHECK(selector.SerialisingValue() == CSSOMStringAtom(u8"FooBar"));
    CHECK(list.SelectorsText() == u8".FooBar");

    // Already lowercase: no rare data needed, both values are the same atom.
    auto lower = CreateUnique<MutableSelector>();
    lower->SetMatch(SelectorMatch::Id);
    lower->SetValue(CSSOMStringAtom(u8"foo"), MatchLowercase(true));
    CHECK(lower->GetSelector()->Value() == lower->GetSelector()->SerialisingValue());
    CHECK(lower->GetSelector()->Argument() == CSSOMStringAtom::Null());
    CHECK(!lower->GetSelector()->ArgumentList());
  }

  TEST_CASE("Selector::ComputeSpecificity", "[CSS][Selectors][Selector]")
  {
    constexpr uint32 A = 0x10000;
    constexpr uint32 B_ = 0x100;
    constexpr uint32 C = 1;

    CHECK(Specificity(B::Type(u8"*")) == 0);
    CHECK(Specificity(B::Type(u8"div")) == C);
    CHECK(Specificity(B::Compound(B::Type(u8"div"), B::Id(u8"a"), B::Class(u8"b"))) == A + B_ + C);
    CHECK(Specificity(B::Attribute(SelectorMatch::AttributeExists, u8"href")) == B_);
    CHECK(Specificity(B::PseudoClass(SelectorPseudoClass::Hover)) == B_);
    CHECK(Specificity(B::Combine(
            B::Type(u8"div"), SelectorRelation::Child,
            B::Compound(B::Type(u8"p"), B::PseudoElement(SelectorPseudoElement::Before, u8"before"))))
          == 3 * C);

    CHECK(Specificity(B::PseudoClass(SelectorPseudoClass::Is, B::MutableList(B::Id(u8"a"), B::Class(u8"b"))))
          == A);
    CHECK(
      Specificity(B::PseudoClass(SelectorPseudoClass::Not, B::MutableList(B::Type(u8"a"), B::Class(u8"b"))))
      == B_);
    CHECK(Specificity(B::PseudoClass(SelectorPseudoClass::Has, B::MutableList(B::Id(u8"a")))) == A);
    CHECK(Specificity(B::PseudoClass(SelectorPseudoClass::Where, B::MutableList(B::Id(u8"a")))) == 0);
    // :is() ignores pseudo-elements in its argument.
    CHECK(Specificity(
            B::PseudoClass(SelectorPseudoClass::Is,
                           B::MutableList(B::Compound(
                             B::Class(u8"a"), B::PseudoElement(SelectorPseudoElement::Before, u8"before")))))
          == B_);

    auto nth = Nth(SelectorPseudoClass::NthChild, 2, 0);
    nth->AdoptSelectorVector(B::MutableList(B::Id(u8"a"), B::Class(u8"b")));
    CHECK(Specificity(krys::move(nth)) == A + B_);
    CHECK(Specificity(Nth(SelectorPseudoClass::NthOfType, 2, 0)) == B_);
    CHECK(Specificity(B::PseudoClass(SelectorPseudoClass::Host, B::MutableList(B::Class(u8"a")))) == 2 * B_);

    auto slotted = B::PseudoElement(SelectorPseudoElement::Slotted, u8"slotted");
    slotted->AdoptSelectorVector(B::MutableList(B::Class(u8"a")));
    CHECK(Specificity(krys::move(slotted)) == B_);

    CHECK(Specificity(
            WithArguments(B::PseudoElement(SelectorPseudoElement::ViewTransitionNew, u8"view-transition-new"),
                          {dom::StarAtom()}))
          == 0);
    CHECK(Specificity(
            WithArguments(B::PseudoElement(SelectorPseudoElement::ViewTransitionNew, u8"view-transition-new"),
                          {dom::StarAtom(), CSSOMStringAtom(u8"a")}))
          == C);
    CHECK(Specificity(
            WithArguments(B::PseudoElement(SelectorPseudoElement::ViewTransitionNew, u8"view-transition-new"),
                          {CSSOMStringAtom(u8"x")}))
          == C);

    auto implicitParent = B::NestingParent();
    implicitParent->SetImplicit();
    CHECK(Specificity(B::Combine(krys::move(implicitParent), SelectorRelation::Child, B::Id(u8"a"))) == A);
  }

  TEST_CASE("Selector::ComputeSpecificityTuple(saturates)", "[CSS][Selectors][Selector]")
  {
    auto compound = B::Class(u8"c");
    for (int32 i = 0; i < 300; ++i)
    {
      compound->PrependComplexSelectorComponent(SelectorRelation::Compounding, B::Class(u8"c"));
    }
    compound->PrependComplexSelectorComponent(SelectorRelation::Compounding, B::Id(u8"i"));
    compound->PrependComplexSelectorComponent(SelectorRelation::Compounding, B::Type(u8"t"));
    auto list = B::List(krys::move(compound));
    auto tuple = list.FirstSelectorComponent()->ComputeSpecificityTuple();
    CHECK(tuple[0] == 1);
    CHECK(tuple[1] == 0xFF);
    CHECK(tuple[2] == 1);
  }

  TEST_CASE("Selector::SpecificityForPage", "[CSS][Selectors][Selector]")
  {
    auto pageSelector = [](UniquePtr<MutableSelector> selector)
    {
      for (auto *current = selector.get(); current; current = current->PrecedingComplexSelectorComponent())
      {
        current->SetForPage();
      }
      return B::List(krys::move(selector));
    };

    auto named =
      pageSelector(B::Compound(B::Type(u8"toc"), MutableSelector::ParsePagePseudoSelector(u8"first")));
    CHECK(named.FirstSelectorComponent()->IsForPage());
    CHECK(named.FirstSelectorComponent()->SpecificityForPage() == 6);
    CHECK(named.SelectorsText() == u8"toc:first");

    auto left = pageSelector(MutableSelector::ParsePagePseudoSelector(u8"left"));
    CHECK(left.FirstSelectorComponent()->SpecificityForPage() == 1);
  }

  TEST_CASE("Selector::MatchNth", "[CSS][Selectors][Selector]")
  {
    auto matches = [](int32 a, int32 b)
    {
      auto list = B::List(Nth(SelectorPseudoClass::NthChild, a, b));
      List<int32> result;
      for (int32 count = 1; count <= 10; ++count)
      {
        if (list.FirstSelectorComponent()->MatchNth(count))
        {
          result.push_back(count);
        }
      }
      return result;
    };

    CHECK(matches(2, 1) == List<int32> {1, 3, 5, 7, 9});
    CHECK(matches(3, 0) == List<int32> {3, 6, 9});
    CHECK(matches(-1, 3) == List<int32> {1, 2, 3});
    CHECK(matches(0, 5) == List<int32> {5});
    CHECK(matches(-2, 7) == List<int32> {1, 3, 5, 7});

    auto list = B::List(Nth(SelectorPseudoClass::NthChild, 4, -1));
    CHECK(list.FirstSelectorComponent()->NthA() == 4);
    CHECK(list.FirstSelectorComponent()->NthB() == -1);
  }

  TEST_CASE("Selector(compound traversal)", "[CSS][Selectors][Selector]")
  {
    // Stored as [p, .a, .b, div].
    auto list = B::List(B::Combine(B::Type(u8"div"), SelectorRelation::Child,
                                   B::Compound(B::Type(u8"p"), B::Class(u8"a"), B::Class(u8"b"))));
    REQUIRE(list.ComponentCount() == 4);
    const auto *p = list.SelectorComponentAt(0);
    const auto *a = list.SelectorComponentAt(1);
    const auto *b = list.SelectorComponentAt(2);
    const auto *div = list.SelectorComponentAt(3);

    CHECK(p->IsLastInComplexSelector());
    CHECK(!p->IsFirstInComplexSelector());
    CHECK(div->IsFirstInComplexSelector());
    CHECK(div->IsLastInSelectorList());
    CHECK(p->PrecedingComplexSelectorComponent() == a);
    CHECK(div->PrecedingComplexSelectorComponent() == nullptr);

    CHECK(p->FirstInCompoundSelector() == b);
    CHECK(a->FirstInCompoundSelector() == b);
    CHECK(b->LastInCompoundSelector() == p);
    CHECK(div->FirstInCompoundSelector() == div);
    CHECK(div->LastInCompoundSelector() == div);
    CHECK(p->PrecedingCompoundSelectorComponent() == a);
    CHECK(b->PrecedingCompoundSelectorComponent() == nullptr);

    CHECK(b->Relation() == SelectorRelation::Child);
    CHECK(b->HasDescendantOrChildRelation());
    CHECK(!b->HasDescendantRelation());
    CHECK(p->TagQualifiedName().LocalName() == CSSOMStringAtom(u8"p"));
    CHECK(p->TagLocalNameLower() == CSSOMStringAtom(u8"p"));
    CHECK(a->Value() == CSSOMStringAtom(u8"a"));
  }

  TEST_CASE("Selector(queries)", "[CSS][Selectors][Selector]")
  {
    auto sibling = B::List(B::Combine(B::Type(u8"a"), SelectorRelation::NextSibling, B::Type(u8"b")));
    CHECK(sibling.FirstSelectorComponent()->IsSiblingSelector());
    CHECK(
      B::List(B::PseudoClass(SelectorPseudoClass::FirstChild)).FirstSelectorComponent()->IsSiblingSelector());
    CHECK(!B::List(B::PseudoClass(SelectorPseudoClass::Root)).FirstSelectorComponent()->IsSiblingSelector());
    CHECK(IsTreeStructuralPseudoClass(SelectorPseudoClass::Root));
    CHECK(!IsTreeStructuralPseudoClass(SelectorPseudoClass::Hover));
    CHECK(IsLogicalCombinationPseudoClass(SelectorPseudoClass::Where));
    CHECK(!IsLogicalCombinationPseudoClass(SelectorPseudoClass::Host));

    auto attribute =
      B::List(B::Attribute(SelectorMatch::AttributeDash, u8"lang", u8"en", IsCaseSensitive(false)));
    CHECK(attribute.FirstSelectorComponent()->IsAttributeSelector());
    CHECK(attribute.FirstSelectorComponent()->IsAttributeValueMatchingCaseInsensitive());
    CHECK(attribute.FirstSelectorComponent()->Attribute().LocalName() == CSSOMStringAtom(u8"lang"));
    CHECK(!B::List(B::Class(u8"x")).FirstSelectorComponent()->IsAttributeSelector());

    auto host = B::List(B::PseudoClass(SelectorPseudoClass::Host));
    CHECK(host.FirstSelectorComponent()->IsHostPseudoClass());
    CHECK(!host.FirstSelectorComponent()->IsScopePseudoClass());

    auto scope = B::List(
      B::Combine(B::PseudoClass(SelectorPseudoClass::Scope), SelectorRelation::Child, B::Class(u8"a")));
    CHECK(scope.FirstSelectorComponent()->HasScope());
    CHECK(scope.FirstSelectorComponent()->HasExplicitPseudoClassScope());

    auto nestedScope = B::List(
      B::PseudoClass(SelectorPseudoClass::Is, B::MutableList(B::PseudoClass(SelectorPseudoClass::Scope))));
    CHECK(!nestedScope.FirstSelectorComponent()->HasScope());
    CHECK(nestedScope.FirstSelectorComponent()->HasExplicitPseudoClassScope());

    auto nested = B::List(B::PseudoClass(SelectorPseudoClass::Not, B::MutableList(B::NestingParent())));
    CHECK(nested.FirstSelectorComponent()->HasExplicitNestingParent());
    CHECK(!host.FirstSelectorComponent()->HasExplicitNestingParent());
    CHECK(B::List(B::Simple(SelectorMatch::ForgivingUnknownNestContaining, u8"&!"))
            .FirstSelectorComponent()
            ->HasExplicitNestingParent());

    // Visiting only the subject stops at the first combinator.
    auto complex = B::List(
      B::Combine(B::Class(u8"a"), SelectorRelation::Child, B::Compound(B::Class(u8"b"), B::Class(u8"c"))));
    List<CSSOMString> visited;
    complex.FirstSelectorComponent()->VisitSimpleSelectors(
      [&](const Selector &selector)
      {
        visited.push_back(CSSOMString(selector.Value().View()));
        return false;
      },
      Selector::VisitFunctionalPseudoClasses::No, Selector::VisitOnlySubject::Yes);
    CHECK(visited == List<CSSOMString> {u8"b", u8"c"});
  }

  TEST_CASE("Selector(pseudo-element queries)", "[CSS][Selectors][Selector]")
  {
    auto before =
      B::List(B::Compound(B::Class(u8"a"), B::PseudoElement(SelectorPseudoElement::Before, u8"before")));
    CHECK(ComplexSelectorCanMatchPseudoElement(*before.FirstSelectorComponent()));
    CHECK(!ComplexSelectorMatchesElementBackedPseudoElement(*before.FirstSelectorComponent()));

    auto part = B::List(
      B::Compound(B::Class(u8"a"), WithArguments(B::PseudoElement(SelectorPseudoElement::Part, u8"part"),
                                                 {CSSOMStringAtom(u8"p")})));
    CHECK(ComplexSelectorMatchesElementBackedPseudoElement(*part.FirstSelectorComponent()));

    auto plain = B::List(B::Class(u8"a"));
    CHECK(!ComplexSelectorCanMatchPseudoElement(*plain.FirstSelectorComponent()));
    CHECK(!ComplexSelectorMatchesElementBackedPseudoElement(*plain.FirstSelectorComponent()));

    CHECK(IsElementBackedPseudoElement(SelectorPseudoElement::Part));
    CHECK(IsElementBackedPseudoElement(SelectorPseudoElement::Slotted));
    CHECK(IsElementBackedPseudoElement(SelectorPseudoElement::UserAgentPart));
    CHECK(IsElementBackedPseudoElement(SelectorPseudoElement::Cue));
    CHECK(!IsElementBackedPseudoElement(SelectorPseudoElement::Marker));
  }

  TEST_CASE("Selector::StylePseudoElementTypeFor", "[CSS][Selectors][Selector]")
  {
    CHECK(Selector::StylePseudoElementTypeFor(SelectorPseudoElement::Before) == PseudoElementType::Before);
    CHECK(Selector::StylePseudoElementTypeFor(SelectorPseudoElement::FirstLetter)
          == PseudoElementType::FirstLetter);
    CHECK(Selector::StylePseudoElementTypeFor(SelectorPseudoElement::ViewTransitionNew)
          == PseudoElementType::ViewTransitionNew);
    CHECK(Selector::StylePseudoElementTypeFor(SelectorPseudoElement::Checkmark)
          == PseudoElementType::Checkmark);
    CHECK(!Selector::StylePseudoElementTypeFor(SelectorPseudoElement::Part));
    CHECK(!Selector::StylePseudoElementTypeFor(SelectorPseudoElement::UserAgentPart));
    CHECK(!Selector::StylePseudoElementTypeFor(SelectorPseudoElement::Cue));
  }

  TEST_CASE("Selector::ParsePseudoElementName", "[CSS][Selectors][Selector]")
  {
    SelectorParserContext context;
    CHECK(Selector::ParsePseudoElementName(u8"BEFORE", context) == SelectorPseudoElement::Before);
    CHECK(Selector::ParsePseudoElementName(u8"details-content", context)
          == SelectorPseudoElement::UserAgentPart);
    CHECK(!Selector::ParsePseudoElementName(u8"", context));
    CHECK(!Selector::ParsePseudoElementName(u8"nope", context));
    CHECK(!Selector::ParsePseudoElementName(u8"-webkit-nope", context));
    CHECK(!Selector::ParsePseudoElementName(u8"view-transition", context));
    CHECK(!Selector::ParsePseudoElementName(u8"thumb", context));

    context.viewTransitionsEnabled = true;
    context.thumbAndTrackPseudoElementsEnabled = true;
    CHECK(Selector::ParsePseudoElementName(u8"view-transition", context)
          == SelectorPseudoElement::ViewTransition);
    CHECK(Selector::ParsePseudoElementName(u8"thumb", context) == SelectorPseudoElement::UserAgentPart);
  }

  TEST_CASE("Selector(equality and hashing)", "[CSS][Selectors][Selector]")
  {
    auto a =
      B::List(B::Compound(B::Class(u8"a"), B::PseudoElement(SelectorPseudoElement::Before, u8"before")));
    auto b = B::List(B::Class(u8"a"));
    auto c =
      B::List(B::Compound(B::Class(u8"a"), B::PseudoElement(SelectorPseudoElement::Before, u8"before")));

    const auto &first = *a.FirstSelectorComponent();
    CHECK(!ComplexSelectorsEqual(first, *b.FirstSelectorComponent()));
    CHECK(ComplexSelectorsEqual(first, *b.FirstSelectorComponent(),
                                ComplexSelectorsEqualMode::IgnoreNonElementBackedPseudoElements));
    CHECK(HashComplexSelector(first, ComplexSelectorsEqualMode::IgnoreNonElementBackedPseudoElements)
          == HashComplexSelector(*b.FirstSelectorComponent(),
                                 ComplexSelectorsEqualMode::IgnoreNonElementBackedPseudoElements));
    CHECK(ComplexSelectorsEqual(first, *c.FirstSelectorComponent()));
    CHECK(HashComplexSelector(first) == HashComplexSelector(*c.FirstSelectorComponent()));

    // Relation is ignored by SimpleSelectorEqual, but not by ComplexSelectorsEqual.
    auto child = B::List(B::Combine(B::Type(u8"x"), SelectorRelation::Child, B::Class(u8"y")));
    auto descendant = B::List(B::Combine(B::Type(u8"x"), SelectorRelation::Descendant, B::Class(u8"y")));
    CHECK(child.FirstSelectorComponent()->SimpleSelectorEqual(*descendant.FirstSelectorComponent()));
    CHECK(!ComplexSelectorsEqual(*child.FirstSelectorComponent(), *descendant.FirstSelectorComponent()));

    auto withList = [](CSSOMStringView name)
    {
      return B::List(B::PseudoClass(SelectorPseudoClass::Is, B::MutableList(B::Class(name))));
    };
    CHECK(withList(u8"p") == withList(u8"p"));
    CHECK(!(withList(u8"p") == withList(u8"q")));
    CHECK(DefaultHash<SelectorList>::Hash(withList(u8"p"))
          == DefaultHash<SelectorList>::Hash(withList(u8"p")));
  }
}
