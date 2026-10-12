#include "Krystal.Booey/CSS/Values/Aggregates.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::css::tests
{
  enum class TestFlag : uint8
  {
    A,
    B,
    C,
    D
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::css::tests::TestFlag, 4uz);

// Types defined through the conformance macros, which other value headers use at namespace/global scope.
namespace krys::boo::css::tests
{
  struct MacroTupleParameters
  {
    int value;

    bool operator==(const MacroTupleParameters &) const noexcept = default;
  };

  DEFINE_TYPE_WRAPPER_GET(MacroTupleParameters, value);

  struct MacroSpacePair
  {
    int first;
    int second;

    bool operator==(const MacroSpacePair &) const noexcept = default;
  };

  template <size_t I>
  KRYS_NODISCARD const int &get(const MacroSpacePair &pair) noexcept
  {
    if constexpr (I == 0)
    {
      return pair.first;
    }
    else
    {
      return pair.second;
    }
  }

  DEFINE_TYPE_WRAPPER(MacroWrapper, CommaSeparatedPair<int>);
  DEFINE_TYPE_EXTENDER(MacroExtender, CommaSeparatedPair<int>);

  struct MacroRange : SpaceSeparatedList<int>
  {
    using SpaceSeparatedList<int>::SpaceSeparatedList;
  };

  struct MacroEmpty
  {
    bool operator==(const MacroEmpty &) const noexcept = default;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::tests::MacroTupleParameters, 1)
DEFINE_COALESCING_SLASH_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::css::tests::MacroSpacePair, 2)
DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::css::tests::MacroWrapper)
DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_EXTENDER(krys::boo::css::tests::MacroExtender)
DEFINE_SPACE_SEPARATED_RANGE_LIKE_CONFORMANCE(krys::boo::css::tests::MacroRange)
DEFINE_EMPTY_LIKE_CONFORMANCE(krys::boo::css::tests::MacroEmpty)

namespace krys::boo::css::tests
{
  namespace
  {
    struct SevenDefaulter
    {
      const int &operator()() const noexcept
      {
        static const int value = 7;
        return value;
      }

      bool operator==(const SevenDefaulter &) const noexcept = default;
    };

    struct TestEnumSetOrNone : EnumSetOrKeywordBase<TestEnumSetOrNone, EnumSet<TestFlag>, keywords::None>
    {
      using Base::Base;
    };

    struct NegativeIsEmptyTraits
    {
      static bool IsEmptyValue(int value) noexcept
      {
        return value < 0;
      }

      static int EmptyValue() noexcept
      {
        return -1;
      }
    };

    struct TestIntOrAuto : ValueOrKeyword<int, keywords::Auto, NegativeIsEmptyTraits>
    {
      using Base::Base;
    };

    template <typename Range>
    KRYS_NODISCARD List<int> ToList(const Range &range) noexcept
    {
      List<int> result;
      for (const auto &item : range)
      {
        result.push_back(static_cast<int>(item));
      }
      return result;
    }
  }

  TEST_CASE("Aggregates(serialization traits)", "[CSS][Aggregates]")
  {
    STATIC_CHECK(SerializationSeparator<SpaceSeparatedArray<int, 3>> == SerializationSeparatorType::Space);
    STATIC_CHECK(SerializationSeparator<CommaSeparatedArray<int, 3>> == SerializationSeparatorType::Comma);
    STATIC_CHECK(SerializationSeparator<SpaceSeparatedList<int>> == SerializationSeparatorType::Space);
    STATIC_CHECK(SerializationSeparator<CommaSeparatedList<int>> == SerializationSeparatorType::Comma);
    STATIC_CHECK(SerializationSeparator<SpaceSeparatedFixedList<int>> == SerializationSeparatorType::Space);
    STATIC_CHECK(SerializationSeparator<CommaSeparatedFixedList<int>> == SerializationSeparatorType::Comma);
    STATIC_CHECK(SerializationSeparator<SpaceSeparatedListHashSet<int>> == SerializationSeparatorType::Space);
    STATIC_CHECK(SerializationSeparator<CommaSeparatedListHashSet<int>> == SerializationSeparatorType::Comma);
    STATIC_CHECK(SerializationSeparator<SpaceSeparatedTuple<int, float>> == SerializationSeparatorType::Space);
    STATIC_CHECK(SerializationSeparator<CommaSeparatedTuple<int, float>> == SerializationSeparatorType::Comma);
    STATIC_CHECK(SerializationSeparator<SpaceSeparatedRectEdges<int>> == SerializationSeparatorType::Space);
    STATIC_CHECK(SerializationSeparator<CommaSeparatedRectEdges<int>> == SerializationSeparatorType::Comma);
    STATIC_CHECK(SerializationSeparator<SpaceSeparatedEnumSet<TestFlag>> == SerializationSeparatorType::Space);
    STATIC_CHECK(SerializationSeparator<CommaSeparatedEnumSet<TestFlag>> == SerializationSeparatorType::Comma);
    STATIC_CHECK(SerializationSeparator<ListOrDefault<CommaSeparatedList<int>, SevenDefaulter>>
                 == SerializationSeparatorType::Comma);
    STATIC_CHECK(SerializationSeparator<int> == SerializationSeparatorType::None);

    STATIC_CHECK(SerializationCoalescing<MinimallySerializingSpaceSeparatedPair<int>>
                 == SerializationCoalescingType::Minimal);
    STATIC_CHECK(SerializationCoalescing<MinimallySerializingSpaceSeparatedRectEdges<int>>
                 == SerializationCoalescingType::Minimal);
    STATIC_CHECK(SerializationCoalescing<SpaceSeparatedPair<int>> == SerializationCoalescingType::None);

    STATIC_CHECK(SerializationSeparatorString<SpaceSeparatedList<int>> == u8" ");
    STATIC_CHECK(SerializationSeparatorString<CommaSeparatedList<int>> == u8", ");
    STATIC_CHECK(SerializationSeparatorStringForType<SerializationSeparatorType::Slash> == u8" / ");
    STATIC_CHECK(SerializationSeparatorString<int> == u8"");
  }

  TEST_CASE("Aggregates(like-ness traits)", "[CSS][Aggregates]")
  {
    STATIC_CHECK(TreatAsTupleLike<SpaceSeparatedArray<int, 2>>);
    STATIC_CHECK(TreatAsTupleLike<CommaSeparatedTuple<int, float>>);
    STATIC_CHECK(TreatAsTupleLike<SpaceSeparatedPoint<int>>);
    STATIC_CHECK(TreatAsTupleLike<MinimallySerializingSpaceSeparatedRectCorners<int>>);
    STATIC_CHECK(TreatAsTupleLike<FunctionNotation<ValueId::Calc, int>>);
    STATIC_CHECK(TreatAsRangeLike<SpaceSeparatedList<int>>);
    STATIC_CHECK(TreatAsRangeLike<CommaSeparatedEnumSet<TestFlag>>);
    STATIC_CHECK(TreatAsVariantLike<ListOrNone<SpaceSeparatedList<int>>>);
    STATIC_CHECK(TreatAsOptionalLike<ListOrNull<SpaceSeparatedList<int>>>);
    STATIC_CHECK(TreatAsOptionalLike<Maybe<int>>);
    STATIC_CHECK_FALSE(TreatAsTupleLike<int>);
    STATIC_CHECK_FALSE(TreatAsRangeLike<int>);
  }

  TEST_CASE("Aggregates(tuple sizes)", "[CSS][Aggregates]")
  {
    STATIC_CHECK(std::tuple_size_v<SpaceSeparatedArray<int, 3>> == 3);
    STATIC_CHECK(std::tuple_size_v<CommaSeparatedTuple<int, float, double>> == 3);
    STATIC_CHECK(std::tuple_size_v<SpaceSeparatedPoint<int>> == 2);
    STATIC_CHECK(std::tuple_size_v<MinimallySerializingSpaceSeparatedSize<int>> == 2);
    STATIC_CHECK(std::tuple_size_v<SpaceSeparatedRectEdges<int>> == 4);
    STATIC_CHECK(std::tuple_size_v<MinimallySerializingSpaceSeparatedRectCorners<int>> == 4);
    STATIC_CHECK(std::tuple_size_v<FunctionNotation<ValueId::Calc, int>> == 1);
  }

  TEST_CASE("Aggregates(identifiers)", "[CSS][Aggregates]")
  {
    CustomIdentifier a {CSSOMStringAtom(u8"foo")};
    CHECK(a == CustomIdentifier {CSSOMStringAtom(u8"foo")});
    CHECK(a == CSSOMStringAtom(u8"foo"));
    CHECK_FALSE(a == CSSOMStringAtom(u8"bar"));

    CHECK(PropertyIdentifier {PropertyId::Color} == PropertyIdentifier {PropertyId::Color});
    CHECK_FALSE(PropertyIdentifier {PropertyId::Color} == PropertyIdentifier {PropertyId::MarginTop});

    CHECK(MarkableTraits<CustomIdentifier>::IsEmptyValue(MarkableTraits<CustomIdentifier>::EmptyValue()));
    CHECK_FALSE(MarkableTraits<CustomIdentifier>::IsEmptyValue(a));
  }

  TEST_CASE("Aggregates(FunctionNotation)", "[CSS][Aggregates]")
  {
    FunctionNotation<ValueId::Calc, int> function {5};
    STATIC_CHECK(FunctionNotation<ValueId::Calc, int>::name == ValueId::Calc);
    CHECK(*function == 5);
    CHECK(get<0>(function) == 5);
    const int &asParameters = function;
    CHECK(asParameters == 5);
    CHECK(function == FunctionNotation<ValueId::Calc, int> {5});
    CHECK_FALSE(function == FunctionNotation<ValueId::Calc, int> {6});
  }

  TEST_CASE("Aggregates(arrays and tuples)", "[CSS][Aggregates]")
  {
    SpaceSeparatedArray<int, 3> space {1, 2, 3};
    CHECK(get<0>(space) == 1);
    CHECK(get<2>(space) == 3);
    CHECK(space == SpaceSeparatedArray<int, 3> {1, 2, 3});
    CHECK_FALSE(space == SpaceSeparatedArray<int, 3> {1, 2, 4});

    SpaceSeparatedArray deduced {1, 2};
    STATIC_CHECK(std::is_same_v<decltype(deduced), SpaceSeparatedArray<int, 2>>);

    CommaSeparatedPair<int> comma {4, 5};
    CHECK(get<1>(comma) == 5);

    MinimallySerializingSpaceSeparatedPair<int> pair {1, 2};
    CHECK(pair.first() == 1);
    CHECK(pair.second() == 2);
    CHECK(get<1>(pair) == 2);

    SpaceSeparatedTuple<int, float> spaceTuple {1, 2.5f};
    CHECK(get<0>(spaceTuple) == 1);
    CHECK(get<1>(spaceTuple) == 2.5f);
    CHECK(spaceTuple == SpaceSeparatedTuple<int, float> {1, 2.5f});

    CommaSeparatedTuple<int, float> commaTuple {3, 4.5f};
    CHECK(get<0>(commaTuple) == 3);
  }

  TEST_CASE("Aggregates(points and sizes)", "[CSS][Aggregates]")
  {
    SpaceSeparatedPoint<int> point {1, 2};
    CHECK(point.x() == 1);
    CHECK(point.y() == 2);
    CHECK(get<1>(point) == 2);

    SpaceSeparatedSize<int> size {3, 4};
    CHECK(size.Width() == 3);
    CHECK(size.Height() == 4);

    MinimallySerializingSpaceSeparatedPoint<int> repeatedPoint {5};
    CHECK(repeatedPoint.x() == 5);
    CHECK(repeatedPoint.y() == 5);

    MinimallySerializingSpaceSeparatedSize<int> minimalSize {6, 7};
    CHECK(minimalSize.Width() == 6);
    CHECK(minimalSize.Height() == 7);
    CHECK(minimalSize == MinimallySerializingSpaceSeparatedSize<int> {6, 7});
  }

  TEST_CASE("Aggregates(rect edges and corners)", "[CSS][Aggregates]")
  {
    SpaceSeparatedRectEdges<int> edges {1, 2, 3, 4};
    CHECK(get<0>(edges) == 1);
    CHECK(get<1>(edges) == 2);
    CHECK(get<2>(edges) == 3);
    CHECK(get<3>(edges) == 4);
    CHECK(edges == SpaceSeparatedRectEdges<int> {1, 2, 3, 4});

    CommaSeparatedRectEdges<int> repeated {9};
    CHECK(get<0>(repeated) == 9);
    CHECK(get<3>(repeated) == 9);

    MinimallySerializingSpaceSeparatedRectEdges<int> minimal {1, 2, 1, 2};
    CHECK(get<2>(minimal) == 1);

    MinimallySerializingSpaceSeparatedRectCorners<int> corners {1, 2, 3, 4};
    CHECK(get<0>(corners) == 1);
    CHECK(get<1>(corners) == 2);
    CHECK(get<2>(corners) == 3);
    CHECK(get<3>(corners) == 4);
  }

  TEST_CASE("Aggregates(SpaceSeparatedList and CommaSeparatedList)", "[CSS][Aggregates]")
  {
    SpaceSeparatedList<int> list {1, 2, 3};
    CHECK(list.size() == 3);
    CHECK_FALSE(list.isEmpty());
    CHECK(list[1] == 2);
    CHECK(list.contains(3));
    CHECK_FALSE(list.contains(4));
    CHECK(list.containsIf([](int x) { return x > 2; }));
    CHECK(ToList(list) == List<int> {1, 2, 3});
    CHECK(List<int>(list.rbegin(), list.rend()) == List<int> {3, 2, 1});
    CHECK(list == SpaceSeparatedList<int> {1, 2, 3});
    CHECK(SpaceSeparatedList<int> {}.isEmpty());

    auto doubled = list.map([](int x) { return x * 2; });
    CHECK(ToList(doubled) == List<int> {2, 4, 6});

    auto mapped = CommaSeparatedList<int>::map(List<int> {4, 5}, [](int x) { return x + 1; });
    CHECK(ToList(mapped) == List<int> {5, 6});
  }

  TEST_CASE("Aggregates(fixed lists)", "[CSS][Aggregates]")
  {
    SpaceSeparatedFixedList<int> list {1, 2, 3};
    CHECK(list.size() == 3);
    CHECK_FALSE(list.isEmpty());
    CHECK(list.first() == 1);
    CHECK(list.last() == 3);
    CHECK(list[1] == 2);
    CHECK(list.contains(2));
    CHECK(list.containsIf([](int x) { return x == 3; }));
    CHECK(ToList(list) == List<int> {1, 2, 3});

    auto mapped = CommaSeparatedFixedList<int>::map(List<int> {1, 2}, [](int x) { return x * 10; });
    CHECK(ToList(mapped) == List<int> {10, 20});

    auto generated =
      SpaceSeparatedFixedList<int>::createWithSizeFromGenerator(3, [](size_t i) { return static_cast<int>(i); });
    CHECK(ToList(generated) == List<int> {0, 1, 2});
  }

  TEST_CASE("Aggregates(list hash sets)", "[CSS][Aggregates]")
  {
    SpaceSeparatedListHashSet<int> set {3, 1, 3, 2};
    CHECK(set.size() == 3);
    CHECK(set.contains(1));
    CHECK_FALSE(set.contains(4));
    // Insertion ordered, duplicates dropped.
    CHECK(ToList(set) == List<int> {3, 1, 2});

    auto mapped = CommaSeparatedListHashSet<int>::map(List<int> {1, 2, 1}, [](int x) { return x * 2; });
    CHECK(ToList(mapped) == List<int> {2, 4});
    CHECK(CommaSeparatedListHashSet<int> {}.isEmpty());
  }

  TEST_CASE("Aggregates(ListOrNone)", "[CSS][Aggregates]")
  {
    ListOrNone<SpaceSeparatedList<int>> none {keywords::None {}};
    CHECK(none.IsNone());
    CHECK_FALSE(none.IsList());
    CHECK(none.size() == 0);

    ListOrNone<SpaceSeparatedList<int>> list {SpaceSeparatedList<int> {1, 2}};
    CHECK(list.IsList());
    CHECK(list.size() == 2);
    CHECK(list[0] == 1);
    CHECK(ToList(list) == List<int> {1, 2});

    CHECK(none.SwitchOn([](keywords::None) { return 0; }, [](const SpaceSeparatedList<int> &) { return 1; }) == 0);
    CHECK(list.SwitchOn([](keywords::None) { return 0; },
                        [](const SpaceSeparatedList<int> &l) { return static_cast<int>(l.size()); })
          == 2);

    ListOrNone<CommaSeparatedFixedList<int>> fixed {CommaSeparatedFixedList<int> {4, 5}};
    CHECK(fixed.first() == 4);
    CHECK(fixed.last() == 5);
  }

  TEST_CASE("Aggregates(ListOrDefault)", "[CSS][Aggregates]")
  {
    using Type = ListOrDefault<CommaSeparatedList<int>, SevenDefaulter>;

    Type defaulted {Type::DefaultValue};
    CHECK(defaulted.IsDefault());
    CHECK(defaulted.size() == 1);
    CHECK(defaulted[0] == 7);
    CHECK(defaulted.contains(7));
    CHECK_FALSE(defaulted.contains(1));
    CHECK(defaulted.containsIf([](int x) { return x == 7; }));
    CHECK(ToList(defaulted) == List<int> {7});

    Type list {CommaSeparatedList<int> {1, 2}};
    CHECK_FALSE(list.IsDefault());
    CHECK(list.size() == 2);
    CHECK(list.contains(2));
    CHECK_FALSE(list.contains(7));
    CHECK(ToList(list) == List<int> {1, 2});
  }

  TEST_CASE("Aggregates(ListOrNull)", "[CSS][Aggregates]")
  {
    ListOrNull<SpaceSeparatedFixedList<int>> empty {std::nullopt};
    CHECK(empty.IsNull());
    CHECK_FALSE(empty.IsList());
    CHECK_FALSE(static_cast<bool>(empty));
    CHECK(empty.TryList() == nullptr);
    CHECK(empty.GetList().isEmpty());

    ListOrNull<SpaceSeparatedFixedList<int>> list {SpaceSeparatedFixedList<int> {1, 2, 3}};
    CHECK(list.IsList());
    CHECK(static_cast<bool>(list));
    REQUIRE(list.TryList() != nullptr);
    CHECK(list.size() == 3);
    CHECK(list.first() == 1);
    CHECK(list.last() == 3);
    CHECK(list[1] == 2);
    CHECK(list.contains(3));
    CHECK(list.containsIf([](int x) { return x == 1; }));
    CHECK(list->size() == 3);
    CHECK(ToList(list) == List<int> {1, 2, 3});
    CHECK(list == ListOrNull<SpaceSeparatedFixedList<int>> {SpaceSeparatedFixedList<int> {1, 2, 3}});
  }

  TEST_CASE("Aggregates(EnumSetOrKeyword)", "[CSS][Aggregates]")
  {
    TestEnumSetOrNone none {keywords::None {}};
    CHECK(none.IsKeyword());
    CHECK_FALSE(none.IsEnumSet());
    CHECK(none.TryEnumSet() == nullptr);

    TestEnumSetOrNone set {TestFlag::A, TestFlag::C};
    CHECK(set.IsEnumSet());
    CHECK(set.Size() == 2);
    CHECK(set.Contains(TestFlag::A));
    CHECK_FALSE(set.Contains(TestFlag::B));
    CHECK(set.ContainsAny({TestFlag::B, TestFlag::C}));
    CHECK(set.ContainsAll({TestFlag::A, TestFlag::C}));
    CHECK_FALSE(set.ContainsOnly({TestFlag::A}));

    auto roundTrip = TestEnumSetOrNone::FromRaw(set.ToRaw());
    CHECK(roundTrip == set);
    CHECK(TestEnumSetOrNone::FromRaw(0).IsKeyword());

    CHECK(set.SwitchOn([](keywords::None) { return 0uz; }, [](const EnumSet<TestFlag> &s) { return s.Size(); })
          == 2uz);
  }

  TEST_CASE("Aggregates(ValueOrKeyword)", "[CSS][Aggregates]")
  {
    TestIntOrAuto keyword {keywords::Auto {}};
    CHECK(keyword.IsKeyword());
    CHECK_FALSE(keyword.IsValue());
    CHECK_FALSE(keyword.TryValue().has_value());
    CHECK(keyword.HoldsAlternative<keywords::Auto>());

    TestIntOrAuto value {5};
    CHECK(value.IsValue());
    CHECK(value.TryValue() == 5);
    CHECK(value.HoldsAlternative<int>());
    CHECK(value.SwitchOn([](keywords::Auto) { return -1; }, [](int x) { return x; }) == 5);
    CHECK(keyword.SwitchOn([](keywords::Auto) { return -1; }, [](int x) { return x; }) == -1);
  }

  TEST_CASE("Aggregates(enum sets)", "[CSS][Aggregates]")
  {
    SpaceSeparatedEnumSet<TestFlag> set {TestFlag::A, TestFlag::D};
    CHECK(set.Size() == 2);
    CHECK_FALSE(set.IsEmpty());
    CHECK(set.Contains(TestFlag::D));
    CHECK(set.ContainsAny({TestFlag::A}));
    CHECK(set.ContainsAll({TestFlag::A, TestFlag::D}));
    CHECK(set.ContainsOnly({TestFlag::A, TestFlag::B, TestFlag::D}));
    CHECK(ToList(set) == List<int> {0, 3});
    CHECK(SpaceSeparatedEnumSet<TestFlag>::FromRaw(set.ToRaw()) == set);

    SpaceSeparatedEnumSet<TestFlag> other {TestFlag::D, TestFlag::B};
    CHECK((set & other) == SpaceSeparatedEnumSet<TestFlag> {TestFlag::D});
    CHECK((set - other) == SpaceSeparatedEnumSet<TestFlag> {TestFlag::A});
    CHECK((set ^ other) == SpaceSeparatedEnumSet<TestFlag> {TestFlag::A, TestFlag::B});

    auto mapped = CommaSeparatedEnumSet<TestFlag>::Map(List<int> {1, 2}, [](int x) { return static_cast<TestFlag>(x); });
    CHECK(mapped == CommaSeparatedEnumSet<TestFlag> {TestFlag::B, TestFlag::C});
  }

  TEST_CASE("Aggregates(NonEmptyRangeLikeMarkableTraits)", "[CSS][Aggregates]")
  {
    using Traits = NonEmptyRangeLikeMarkableTraits<SpaceSeparatedList<int>>;
    CHECK(Traits::IsEmptyValue(Traits::EmptyValue()));
    CHECK_FALSE(Traits::IsEmptyValue(SpaceSeparatedList<int> {1}));
  }

  TEST_CASE("Aggregates(conformance macros)", "[CSS][Aggregates]")
  {
    STATIC_CHECK(TreatAsTupleLike<MacroTupleParameters>);
    STATIC_CHECK(std::tuple_size_v<MacroTupleParameters> == 1);
    CHECK(get<0>(MacroTupleParameters {4}) == 4);

    STATIC_CHECK(TreatAsTupleLike<MacroSpacePair>);
    STATIC_CHECK(std::tuple_size_v<MacroSpacePair> == 2);
    STATIC_CHECK(SerializationSeparator<MacroSpacePair> == SerializationSeparatorType::Slash);
    STATIC_CHECK(SerializationCoalescing<MacroSpacePair> == SerializationCoalescingType::Minimal);

    MacroWrapper wrapper {1, 2};
    STATIC_CHECK(TreatAsTupleLike<MacroWrapper>);
    STATIC_CHECK(std::tuple_size_v<MacroWrapper> == 1);
    CHECK(get<1>(*wrapper) == 2);
    CHECK(get<0>(wrapper) == CommaSeparatedPair<int> {1, 2});
    CHECK(wrapper == MacroWrapper {1, 2});

    MacroExtender extender {3, 4};
    STATIC_CHECK(TreatAsTupleLike<MacroExtender>);
    STATIC_CHECK(std::tuple_size_v<MacroExtender> == 2);
    STATIC_CHECK(SerializationSeparator<MacroExtender> == SerializationSeparatorType::Comma);
    CHECK(get<0>(extender) == 3);
    CHECK(get<1>(extender) == 4);

    STATIC_CHECK(TreatAsRangeLike<MacroRange>);
    STATIC_CHECK(SerializationSeparator<MacroRange> == SerializationSeparatorType::Space);

    STATIC_CHECK(TreatAsEmptyLike<MacroEmpty>);
  }
}
