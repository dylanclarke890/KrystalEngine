#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/ComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<PrimitiveValue> Px(double value) noexcept
    {
      return PrimitiveValue::Create(value, UnitType::px);
    }
  }

  TEST_CASE_METHOD(ValueFixture, "ValueList::Create", "[CSS][Values][ValueList]")
  {
    SECTION("Separator characters")
    {
      auto comma = ValueList::Create(U',', ValueListBuilder {Px(1.0), Px(2.0)});
      auto slash = ValueList::Create(U'/', ValueListBuilder {Px(1.0), Px(2.0)});
      auto space = ValueList::Create(U' ', ValueListBuilder {Px(1.0), Px(2.0)});

      CHECK(comma->Separator() == ValueSeparator::CommaSeparator);
      CHECK(slash->Separator() == ValueSeparator::SlashSeparator);
      CHECK(space->Separator() == ValueSeparator::SpaceSeparator);
      CHECK(CssText(*comma) == "1px, 2px");
      CHECK(CssText(*slash) == "1px / 2px");
      CHECK(CssText(*space) == "1px 2px");
    }

    SECTION("Space-separated overloads")
    {
      auto empty = ValueList::CreateSpaceSeparated();
      CHECK(empty->IsValueListValue());
      CHECK(empty->size() == 0u);
      CHECK(CssText(*empty) == "");

      CHECK(CssText(*ValueList::CreateSpaceSeparated(Px(1.0))) == "1px");
      CHECK(CssText(*ValueList::CreateSpaceSeparated(Px(1.0), Px(2.0))) == "1px 2px");
      CHECK(CssText(*ValueList::CreateSpaceSeparated(Px(1.0), Px(2.0), Px(3.0))) == "1px 2px 3px");
      CHECK(CssText(*ValueList::CreateSpaceSeparated(Px(1.0), Px(2.0), Px(3.0), Px(4.0)))
            == "1px 2px 3px 4px");
      CHECK(CssText(*ValueList::CreateSpaceSeparated(
              ValueListBuilder {Px(1.0), Px(2.0), Px(3.0), Px(4.0), Px(5.0)}))
            == "1px 2px 3px 4px 5px");
    }

    SECTION("Comma-separated overloads")
    {
      auto single = ValueList::CreateCommaSeparated(Px(1.0));
      CHECK(single->Separator() == ValueSeparator::CommaSeparator);
      CHECK(CssText(*single) == "1px");
      CHECK(CssText(*ValueList::CreateCommaSeparated(ValueListBuilder {Px(1.0), Px(2.0), Px(3.0)}))
            == "1px, 2px, 3px");
    }

    SECTION("Slash-separated overloads")
    {
      auto single = ValueList::CreateSlashSeparated(Px(1.0));
      CHECK(single->Separator() == ValueSeparator::SlashSeparator);
      CHECK(CssText(*single) == "1px");
      CHECK(CssText(*ValueList::CreateSlashSeparated(Px(1.0), Px(2.0))) == "1px / 2px");
      CHECK(CssText(*ValueList::CreateSlashSeparated(ValueListBuilder {Px(1.0), Px(2.0), Px(3.0)}))
            == "1px / 2px / 3px");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "ValueList container protocol", "[CSS][Values][ValueList]")
  {
    auto list = ValueList::CreateSpaceSeparated(Px(1.0), PrimitiveValue::Create(ValueId::Auto), Px(3.0));

    CHECK(list->size() == 3u);
    CHECK(list->Length() == 3u);
    CHECK(CssText((*list)[0u]) == "1px");
    CHECK(CssText((*list)[1u]) == "auto");
    CHECK(CssText((*list)[2u]) == "3px");

    CHECK(list->Item(1u) == &(*list)[1u]);
    CHECK(list->Item(3u) == nullptr);
    CHECK(list->ItemWithoutBoundsCheck(2u) == &(*list)[2u]);
    CHECK(list->ProtectedItem(0u).get() == &(*list)[0u]);
    CHECK(list->ProtectedItem(3u).get() == nullptr);

    SECTION("Iteration")
    {
      List<std::string> items;
      for (auto &item : *list)
      {
        items.push_back(CssText(item));
      }
      CHECK(items == List<std::string> {"1px", "auto", "3px"});
    }

    SECTION("HasValue")
    {
      CHECK(list->HasValue(*Px(3.0)));
      CHECK_FALSE(list->HasValue(*Px(4.0)));
      CHECK(list->HasValue(ValueId::Auto));
      CHECK_FALSE(list->HasValue(ValueId::None));
    }

    SECTION("CopyValues shares the items")
    {
      auto values = list->CopyValues();
      REQUIRE(values.size() == 3uz);
      CHECK(values[0uz].get() == &(*list)[0u]);
      CHECK(values[2uz].get() == &(*list)[2u]);

      auto copy = ValueList::CreateCommaSeparated(krys::move(values));
      CHECK(CssText(*copy) == "1px, auto, 3px");
      CHECK(CssText(*list) == "1px auto 3px");
    }

    SECTION("SerialiseItems")
    {
      CHECK(ToNarrow(list->SerialiseItems(DefaultSerialisationContext())) == "1px auto 3px");

      StringBuilder builder;
      builder.Append(u8"x: ");
      list->SerialiseItems(builder, DefaultSerialisationContext());
      CHECK(ToNarrow(builder) == "x: 1px auto 3px");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "ValueList equality and hashing", "[CSS][Values][ValueList]")
  {
    auto a = ValueList::CreateSpaceSeparated(Px(1.0), Px(2.0));
    auto b = ValueList::CreateSpaceSeparated(Px(1.0), Px(2.0));
    auto differentItem = ValueList::CreateSpaceSeparated(Px(1.0), Px(3.0));
    auto differentSeparator = ValueList::CreateCommaSeparated(ValueListBuilder {Px(1.0), Px(2.0)});
    auto differentLength = ValueList::CreateSpaceSeparated(Px(1.0), Px(2.0), Px(3.0));

    CHECK(*a == *b);
    CHECK(a->ItemsEqual(*b));
    CHECK_FALSE(*a == *differentItem);
    CHECK_FALSE(a->ItemsEqual(*differentItem));
    CHECK_FALSE(*a == *differentSeparator);
    CHECK(a->ItemsEqual(*differentSeparator));
    CHECK_FALSE(*a == *differentLength);
    CHECK_FALSE(a->ItemsEqual(*differentLength));

    SECTION("A single-item list equals its item")
    {
      auto single = ValueList::CreateCommaSeparated(Px(1.0));
      auto item = Px(1.0);
      CHECK(single->ContainsSingleEqualItem(*item));
      CHECK(*single == *item);
      CHECK(*item == *single);
      CHECK_FALSE(*single == *Px(2.0));
      CHECK_FALSE(a->ContainsSingleEqualItem(*item));
    }

    SECTION("Hashing")
    {
      size_t hashA = 0uz, hashB = 0uz, hashItem = 0uz, hashSeparator = 0uz;
      CHECK(a->AddHash(hashA));
      CHECK(b->AddHash(hashB));
      CHECK(differentItem->AddHash(hashItem));
      CHECK(differentSeparator->AddHash(hashSeparator));
      CHECK(hashA == hashB);
      CHECK(hashA != hashItem);
      CHECK(hashA != hashSeparator);
    }
  }

  TEST_CASE_METHOD(ValueFixture, "ValueList visits and collects from its items", "[CSS][Values][ValueList]")
  {
    auto list = ValueList::CreateSpaceSeparated(Px(1.0), PrimitiveValue::Create(2.0, UnitType::em));

    auto children = Children(*list);
    REQUIRE(children.size() == 2uz);
    CHECK(children[0uz] == &(*list)[0u]);
    CHECK(children[1uz] == &(*list)[1u]);

    SECTION("Visitation stops when the visitor is done")
    {
      size_t visited = 0uz;
      auto status = list->VisitChildren(
        [&](Value &)
        {
          ++visited;
          return IterationStatus::Done;
        });
      CHECK(status == IterationStatus::Done);
      CHECK(visited == 1uz);
    }

    SECTION("Computed style dependencies")
    {
      CHECK(list->ComputedStyleDependencies().Properties == List<PropertyId> {PropertyId::FontSize});
      CHECK(ValueList::CreateSpaceSeparated(Px(1.0))->ComputedStyleDependencies().Properties.empty());
    }
  }
}
