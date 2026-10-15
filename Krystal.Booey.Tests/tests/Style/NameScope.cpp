#include "Krystal.Booey/Style/NameScope.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"

namespace krys::boo::style::tests
{
  namespace
  {
    css::CommaSeparatedListHashSet<css::CustomIdentifier> Names(std::initializer_list<const char8_t *> names)
    {
      css::CommaSeparatedListHashSet<css::CustomIdentifier> result;
      for (const auto *name : names)
      {
        result.Value.add({css::CSSOMStringAtom {name}});
      }
      return result;
    }

    // Which alternative `SwitchOn` visits: 0 for `none`, 1 for `all`, 2 for names.
    int Alternative(const NameScope &scope)
    {
      return scope.SwitchOn([](css::keywords::None) { return 0; }, [](css::keywords::All) { return 1; },
                            [](const css::CommaSeparatedListHashSet<css::CustomIdentifier> &) { return 2; });
    }
  }

  TEST_CASE("NameScope", "[Style][NameScope]")
  {
    auto none = NameScope {css::keywords::None {}};
    auto all = NameScope {css::keywords::All {}, ScopeOrdinal::Element};
    auto names = NameScope {Names({u8"--a", u8"--b"}), ScopeOrdinal::Element};

    CHECK(Alternative(none) == 0);
    CHECK(Alternative(all) == 1);
    CHECK(Alternative(names) == 2);
    // An empty name list behaves as `none`.
    CHECK(Alternative(NameScope {Names({}), ScopeOrdinal::Element}) == 0);

    CHECK(names == NameScope {Names({u8"--a", u8"--b"}), ScopeOrdinal::Element});
    // Order matters.
    CHECK_FALSE(names == NameScope {Names({u8"--b", u8"--a"}), ScopeOrdinal::Element});
    CHECK_FALSE(names == NameScope {Names({u8"--a", u8"--b"}), ScopeOrdinal::Shadow});
    CHECK_FALSE(none == all);
  }

  TEST_CASE_METHOD(BuilderFixture, "NameScope: conversion", "[Style][NameScope]")
  {
    using css::PrimitiveValue;
    using css::ValueId;
    auto convert = [&](const css::Value &value)
    {
      return CSSValueConversion<NameScope> {}(*State, value);
    };

    CHECK(convert(*PrimitiveValue::Create(ValueId::None)).Type == NameScope::Type::None);
    CHECK(convert(*PrimitiveValue::Create(ValueId::All))
          == NameScope {css::keywords::All {}, ScopeOrdinal::Element});
    CHECK(convert(*PrimitiveValue::CreateCustomIdent(css::CSSOMString {u8"--a"}))
          == NameScope {Names({u8"--a"}), ScopeOrdinal::Element});
    CHECK(convert(*css::ValueList::CreateCommaSeparated(
            css::ValueListBuilder {PrimitiveValue::CreateCustomIdent(css::CSSOMString {u8"--a"}),
                                   PrimitiveValue::CreateCustomIdent(css::CSSOMString {u8"--b"})}))
          == NameScope {Names({u8"--a", u8"--b"}), ScopeOrdinal::Element});
    CHECK_FALSE(IsInvalid());

    CHECK(convert(*PrimitiveValue::Create(ValueId::Auto)).Type == NameScope::Type::None);
    CHECK(IsInvalid());
  }
}
