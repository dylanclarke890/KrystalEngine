#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/SelectorTestUtils.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    using B = SelectorBuilder;

    // `div > p.a, :is(.x, .y), #z`
    KRYS_NODISCARD SelectorList Sample() noexcept
    {
      return B::List(
        B::Combine(B::Type(u8"div"), SelectorRelation::Child, B::Compound(B::Type(u8"p"), B::Class(u8"a"))),
        B::PseudoClass(SelectorPseudoClass::Is, B::MutableList(B::Class(u8"x"), B::Class(u8"y"))),
        B::Id(u8"z"));
    }
  }

  TEST_CASE("SelectorList(empty)", "[CSS][Selectors][SelectorList]")
  {
    SelectorList list;
    CHECK(list.IsEmpty());
    CHECK(list.ComponentCount() == 0);
    CHECK(list.SelectorCount() == 0);
    CHECK(list.begin() == list.end());
    CHECK(list.SelectorsText() == u8"");
    CHECK(!list.HasOnlyNestingSelector());
    CHECK(!list.HasExplicitNestingParent());
    CHECK(list == SelectorList {});
    CHECK(SelectorList(list).IsEmpty());
  }

  TEST_CASE("SelectorList(layout)", "[CSS][Selectors][SelectorList]")
  {
    auto list = Sample();
    CHECK(!list.IsEmpty());
    CHECK(list.ComponentCount() == 5);
    CHECK(list.SelectorCount() == 3);
    CHECK(list.SelectorsText() == u8"div > p.a, :is(.x, .y), #z");

    List<CSSOMString> complexSelectors;
    for (const auto &selector : list)
    {
      complexSelectors.push_back(selector.SelectorText());
    }
    CHECK(complexSelectors == List<CSSOMString> {u8"div > p.a", u8":is(.x, .y)", u8"#z"});

    CHECK(list.IndexOfNextSelectorAfter(0) == 3uz);
    CHECK(list.IndexOfNextSelectorAfter(1) == 3uz);
    CHECK(list.IndexOfNextSelectorAfter(3) == 4uz);
    CHECK(!list.IndexOfNextSelectorAfter(4));
    CHECK(list.SelectorComponentAt(4)->IsLastInSelectorList());
    CHECK(!list.SelectorComponentAt(2)->IsLastInSelectorList());
  }

  TEST_CASE("SelectorList(copy)", "[CSS][Selectors][SelectorList]")
  {
    auto original = CreateUnique<SelectorList>(Sample());
    SelectorList copy(*original);
    CHECK(copy == *original);
    CHECK(DefaultHash<SelectorList>::Hash(copy) == DefaultHash<SelectorList>::Hash(*original));

    // The copy owns its own rare data and selector lists.
    original.reset();
    CHECK(copy.SelectorsText() == u8"div > p.a, :is(.x, .y), #z");
    CHECK(copy.ComponentCount() == 5);

    SelectorList moved(krys::move(copy));
    CHECK(moved.SelectorCount() == 3);
    SelectorList assigned;
    assigned = krys::move(moved);
    CHECK(assigned.SelectorsText() == u8"div > p.a, :is(.x, .y), #z");
  }

  TEST_CASE("SelectorList(equality)", "[CSS][Selectors][SelectorList]")
  {
    CHECK(Sample() == Sample());
    CHECK(!(Sample() == B::List(B::Id(u8"z"))));
    CHECK(!(B::List(B::Id(u8"z")) == Sample()));
    CHECK(!(B::List(B::Id(u8"z")) == B::List(B::Class(u8"z"))));
    CHECK(B::List(B::Class(u8"a"), B::Class(u8"b")) != B::List(B::Class(u8"b"), B::Class(u8"a")));
  }

  TEST_CASE("SelectorList::Copy", "[CSS][Selectors][SelectorList]")
  {
    auto list = Sample();
    const auto *subject = list.FirstSelectorComponent();

    auto simple = SelectorList::CopySimple(*subject);
    CHECK(simple.ComponentCount() == 1);
    CHECK(simple.SelectorsText() == u8"p");

    auto complex = SelectorList::CopyComplex(*subject);
    CHECK(complex.ComponentCount() == 3);
    CHECK(complex.SelectorCount() == 1);
    CHECK(complex.SelectorsText() == u8"div > p.a");

    auto fromMiddle = SelectorList::CopyComplex(*list.SelectorComponentAt(1));
    CHECK(fromMiddle.SelectorsText() == u8"div > .a");
  }

  TEST_CASE("SelectorList::Join", "[CSS][Selectors][SelectorList]")
  {
    auto a = B::List(B::Class(u8"a"));
    auto b = B::List(B::Combine(B::Class(u8"b"), SelectorRelation::Child, B::Class(u8"c")), B::Id(u8"d"));

    auto joined = SelectorList::Join(a, b);
    CHECK(joined.SelectorsText() == u8".a, .b > .c, #d");
    CHECK(joined.SelectorCount() == 3);
    CHECK(SelectorList::Join(a, SelectorList {}).SelectorsText() == u8".a");
    CHECK(SelectorList::Join(SelectorList {}, b).SelectorsText() == u8".b > .c, #d");

    SelectorList empty;
    auto many = SelectorList::Join(List<const SelectorList *> {&a, &empty, &b, &a});
    CHECK(many.SelectorsText() == u8".a, .b > .c, #d, .a");
    CHECK(many.SelectorCount() == 4);
    CHECK(SelectorList::Join(List<const SelectorList *> {&empty}).IsEmpty());
    CHECK(SelectorList::Join(List<const SelectorList *> {}).IsEmpty());
  }

  TEST_CASE("SelectorList(nesting)", "[CSS][Selectors][SelectorList]")
  {
    CHECK(B::List(B::NestingParent()).HasOnlyNestingSelector());
    CHECK(!B::List(B::Compound(B::NestingParent(), B::Class(u8"a"))).HasOnlyNestingSelector());
    CHECK(!B::List(B::NestingParent(), B::Class(u8"a")).HasOnlyNestingSelector());
    CHECK(!B::List(B::Class(u8"a")).HasOnlyNestingSelector());

    CHECK(B::List(B::Class(u8"a"), B::Combine(B::NestingParent(), SelectorRelation::Child, B::Class(u8"b")))
            .HasExplicitNestingParent());
    CHECK(B::List(B::PseudoClass(SelectorPseudoClass::Is, B::MutableList(B::NestingParent())))
            .HasExplicitNestingParent());
    CHECK(!Sample().HasExplicitNestingParent());
  }
}
