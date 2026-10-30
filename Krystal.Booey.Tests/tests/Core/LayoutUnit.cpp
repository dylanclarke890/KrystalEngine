#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"

namespace krys::boo::tests
{
  using Catch::Matchers::WithinAbs;
  using Catch::Matchers::WithinULP;

  namespace
  {
    constexpr float Tolerance = 1.0f / FixedPointDenominator;
  }

  TEST_CASE("LayoutUnit: int", "[Core][LayoutUnit]")
  {
    STATIC_CHECK(LayoutUnit(Limits<int32>::min()).ToInt() == IntMinForLayoutUnit);
    STATIC_CHECK(LayoutUnit(Limits<int32>::min() / 2).ToInt() == IntMinForLayoutUnit);
    STATIC_CHECK(LayoutUnit(IntMinForLayoutUnit - 1).ToInt() == IntMinForLayoutUnit);
    STATIC_CHECK(LayoutUnit(IntMinForLayoutUnit).ToInt() == IntMinForLayoutUnit);
    STATIC_CHECK(LayoutUnit(IntMinForLayoutUnit + 1).ToInt() == IntMinForLayoutUnit + 1);
    STATIC_CHECK(LayoutUnit(IntMinForLayoutUnit / 2).ToInt() == IntMinForLayoutUnit / 2);
    STATIC_CHECK(LayoutUnit(-10'000).ToInt() == -10'000);
    STATIC_CHECK(LayoutUnit(-1'000).ToInt() == -1'000);
    STATIC_CHECK(LayoutUnit(-100).ToInt() == -100);
    STATIC_CHECK(LayoutUnit(-10).ToInt() == -10);
    STATIC_CHECK(LayoutUnit(-1).ToInt() == -1);
    STATIC_CHECK(LayoutUnit(0).ToInt() == 0);
    STATIC_CHECK(LayoutUnit(1).ToInt() == 1);
    STATIC_CHECK(LayoutUnit(100).ToInt() == 100);
    STATIC_CHECK(LayoutUnit(1'000).ToInt() == 1'000);
    STATIC_CHECK(LayoutUnit(10'000).ToInt() == 10'000);
    STATIC_CHECK(LayoutUnit(IntMaxForLayoutUnit / 2).ToInt() == IntMaxForLayoutUnit / 2);
    STATIC_CHECK(LayoutUnit(IntMaxForLayoutUnit - 1).ToInt() == IntMaxForLayoutUnit - 1);
    STATIC_CHECK(LayoutUnit(IntMaxForLayoutUnit).ToInt() == IntMaxForLayoutUnit);
    STATIC_CHECK(LayoutUnit(IntMaxForLayoutUnit + 1).ToInt() == IntMaxForLayoutUnit);
    STATIC_CHECK(LayoutUnit(Limits<int32>::max() / 2).ToInt() == IntMaxForLayoutUnit);
    STATIC_CHECK(LayoutUnit(Limits<int32>::max()).ToInt() == IntMaxForLayoutUnit);
  }

  TEST_CASE("LayoutUnit: unsigned", "[Core][LayoutUnit]")
  {
    STATIC_CHECK(LayoutUnit(uint16 {7}).ToInt() == 7);
    STATIC_CHECK(LayoutUnit(7u).ToInt() == 7);
    STATIC_CHECK(LayoutUnit(static_cast<uint32>(IntMaxForLayoutUnit) - 1u).ToInt()
                 == IntMaxForLayoutUnit - 1);
    STATIC_CHECK(LayoutUnit(Limits<uint32>::max()) == LayoutUnit::Max());
    STATIC_CHECK(LayoutUnit(uint64 {7}).ToInt() == 7);
    STATIC_CHECK(LayoutUnit(static_cast<uint64>(IntMaxForLayoutUnit)).ToInt() == IntMaxForLayoutUnit);
    STATIC_CHECK(LayoutUnit(static_cast<uint64>(IntMaxForLayoutUnit) + 1) == LayoutUnit::Max());
    // Large enough that multiplying by the denominator would wrap.
    STATIC_CHECK(LayoutUnit(Limits<uint64>::max() / 4) == LayoutUnit::Max());
    STATIC_CHECK((7_lu).ToInt() == 7);
    STATIC_CHECK(LayoutUnit(-5).ToUnsigned() == static_cast<uint32>(-5));
  }

  TEST_CASE("LayoutUnit: float", "[Core][LayoutUnit]")
  {
    CHECK_THAT(LayoutUnit(1.0f).ToFloat(), WithinULP(1.0f, 4));
    CHECK_THAT(LayoutUnit(1.25f).ToFloat(), WithinULP(1.25f, 4));
    CHECK(LayoutUnit(1.25f) == LayoutUnit(1.25f + Tolerance / 2));
    CHECK(LayoutUnit(-2.0f) == LayoutUnit(-2.0f - Tolerance / 2));
    CHECK_THAT(LayoutUnit(1.1f).ToFloat(), WithinAbs(1.1f, Tolerance));
    CHECK_THAT(LayoutUnit(1.33f).ToFloat(), WithinAbs(1.33f, Tolerance));
    CHECK_THAT(LayoutUnit(1.3333f).ToFloat(), WithinAbs(1.3333f, Tolerance));
    CHECK_THAT(LayoutUnit(1.53434f).ToFloat(), WithinAbs(1.53434f, Tolerance));
    CHECK_THAT(LayoutUnit(345'634).ToFloat(), WithinAbs(345634.0f, Tolerance));
    CHECK_THAT(LayoutUnit(345634.12335f).ToFloat(), WithinAbs(345634.12335f, Tolerance));
    CHECK_THAT(LayoutUnit(-345634.12335f).ToFloat(), WithinAbs(-345634.12335f, Tolerance));
    CHECK_THAT(LayoutUnit(-345'634).ToFloat(), WithinAbs(-345634.0f, Tolerance));
    CHECK_THAT(LayoutUnit(33554432.f).ToFloat(), WithinAbs(33554432.f, Tolerance));
    CHECK_THAT(LayoutUnit(-33554432.f).ToFloat(), WithinAbs(-33554432.f, Tolerance));
    CHECK_THAT(LayoutUnit(33554432.f).ToDouble(), WithinAbs(33554432.f, Tolerance));
    CHECK_THAT(LayoutUnit(-33554432.f).ToDouble(), WithinAbs(-33554432.f, Tolerance));

    LayoutUnit assigned;
    assigned = 2.5f;
    CHECK(assigned.RawValue() == 160);
  }

  TEST_CASE("LayoutUnit: rounding", "[Core][LayoutUnit]")
  {
    CHECK(LayoutUnit(-1.9f).Round() == -2);
    CHECK(LayoutUnit(-1.6f).Round() == -2);
    CHECK(LayoutUnit::FromFloatRound(-1.51f).Round() == -2);
    CHECK(LayoutUnit::FromFloatRound(-1.5f).Round() == -1);
    CHECK(LayoutUnit::FromFloatRound(-1.49f).Round() == -1);
    CHECK(LayoutUnit(-1.0f).Round() == -1);
    CHECK(LayoutUnit::FromFloatRound(-0.99f).Round() == -1);
    CHECK(LayoutUnit::FromFloatRound(-0.51f).Round() == -1);
    CHECK(LayoutUnit::FromFloatRound(-0.50f).Round() == 0);
    CHECK(LayoutUnit::FromFloatRound(-0.49f).Round() == 0);
    CHECK(LayoutUnit(-0.1f).Round() == 0);
    CHECK(LayoutUnit(0.0f).Round() == 0);
    CHECK(LayoutUnit(0.1f).Round() == 0);
    CHECK(LayoutUnit::FromFloatRound(0.49f).Round() == 0);
    CHECK(LayoutUnit::FromFloatRound(0.50f).Round() == 1);
    CHECK(LayoutUnit::FromFloatRound(0.51f).Round() == 1);
    CHECK(LayoutUnit(0.99f).Round() == 1);
    CHECK(LayoutUnit(1.0f).Round() == 1);
    CHECK(LayoutUnit::FromFloatRound(1.49f).Round() == 1);
    CHECK(LayoutUnit::FromFloatRound(1.5f).Round() == 2);
    CHECK(LayoutUnit::FromFloatRound(1.51f).Round() == 2);
    // The fractional part of LayoutUnit::Max() is 0x3f, so it should round up.
    STATIC_CHECK(((Limits<int32>::max() / FixedPointDenominator) + 1) == LayoutUnit::Max().Round());
    // The fractional part of LayoutUnit::Min() is 0, so the next bigger possible
    // value should round down.
    LayoutUnit epsilon;
    epsilon.SetRawValue(1);
    CHECK(((Limits<int32>::min() / FixedPointDenominator)) == (LayoutUnit::Min() + epsilon).Round());

    CHECK(RoundToInt(LayoutUnit(1.5f)) == 2);
    CHECK(RoundToInt(1.5f) == 2);
    CHECK(FloorToInt(LayoutUnit(-1.5f)) == -2);
  }

  TEST_CASE("LayoutUnit: multiplication", "[Core][LayoutUnit]")
  {
    STATIC_CHECK((LayoutUnit(1) * LayoutUnit(1)).ToInt() == 1);
    STATIC_CHECK((LayoutUnit(1) * LayoutUnit(2)).ToInt() == 2);
    STATIC_CHECK((LayoutUnit(2) * LayoutUnit(1)).ToInt() == 2);
    STATIC_CHECK((LayoutUnit(2) * LayoutUnit(0.5)).ToInt() == 1);
    STATIC_CHECK((LayoutUnit(0.5) * LayoutUnit(2)).ToInt() == 1);
    STATIC_CHECK((LayoutUnit(100) * LayoutUnit(1)).ToInt() == 100);

    STATIC_CHECK((LayoutUnit(-1) * LayoutUnit(1)).ToInt() == -1);
    STATIC_CHECK((LayoutUnit(-1) * LayoutUnit(2)).ToInt() == -2);
    STATIC_CHECK((LayoutUnit(-2) * LayoutUnit(1)).ToInt() == -2);
    STATIC_CHECK((LayoutUnit(-2) * LayoutUnit(0.5)).ToInt() == -1);
    STATIC_CHECK((LayoutUnit(-0.5) * LayoutUnit(2)).ToInt() == -1);
    STATIC_CHECK((LayoutUnit(-100) * LayoutUnit(1)).ToInt() == -100);

    STATIC_CHECK((LayoutUnit(-1) * LayoutUnit(-1)).ToInt() == 1);
    STATIC_CHECK((LayoutUnit(-1) * LayoutUnit(-2)).ToInt() == 2);
    STATIC_CHECK((LayoutUnit(-2) * LayoutUnit(-1)).ToInt() == 2);
    STATIC_CHECK((LayoutUnit(-2) * LayoutUnit(-0.5)).ToInt() == 1);
    STATIC_CHECK((LayoutUnit(-0.5) * LayoutUnit(-2)).ToInt() == 1);
    STATIC_CHECK((LayoutUnit(-100) * LayoutUnit(-1)).ToInt() == 100);

    STATIC_CHECK((LayoutUnit(100) * LayoutUnit(3.33)).Round() == 333);
    STATIC_CHECK((LayoutUnit(-100) * LayoutUnit(3.33)).Round() == -333);
    STATIC_CHECK((LayoutUnit(-100) * LayoutUnit(-3.33)).Round() == 333);

    size_t aHundredSizeT = 100;
    CHECK((LayoutUnit(aHundredSizeT) * LayoutUnit(1)).ToInt() == 100);
    CHECK((aHundredSizeT * LayoutUnit(4)).ToInt() == 400);
    CHECK((LayoutUnit(4) * aHundredSizeT).ToInt() == 400);

    constexpr int32 quarterMax = IntMaxForLayoutUnit / 4;
    STATIC_CHECK((LayoutUnit(quarterMax) * LayoutUnit(2)).ToInt() == quarterMax * 2);
    STATIC_CHECK((LayoutUnit(quarterMax) * LayoutUnit(3)).ToInt() == quarterMax * 3);
    STATIC_CHECK((LayoutUnit(quarterMax) * LayoutUnit(4)).ToInt() == quarterMax * 4);
    STATIC_CHECK((LayoutUnit(quarterMax) * LayoutUnit(5)).ToInt() == IntMaxForLayoutUnit);

    size_t overflowIntSizeT = static_cast<size_t>(IntMaxForLayoutUnit) * 4;
    CHECK((LayoutUnit(overflowIntSizeT) * LayoutUnit(2)).ToInt() == IntMaxForLayoutUnit);
    CHECK((overflowIntSizeT * LayoutUnit(4)).ToInt() == IntMaxForLayoutUnit);
    CHECK((LayoutUnit(4) * overflowIntSizeT).ToInt() == IntMaxForLayoutUnit);

    // Saturates towards the minimum when the signs differ.
    STATIC_CHECK(LayoutUnit(quarterMax) * LayoutUnit(-5) == LayoutUnit::Min());

    // Mixed with floating point, the result is floating point.
    STATIC_CHECK(LayoutUnit(2) * 0.25f == 0.5f);
    STATIC_CHECK(0.25 * LayoutUnit(2) == 0.5);
  }

  TEST_CASE("LayoutUnit: division", "[Core][LayoutUnit]")
  {
    STATIC_CHECK((LayoutUnit(1) / LayoutUnit(1)).ToInt() == 1);
    STATIC_CHECK((LayoutUnit(1) / LayoutUnit(2)).ToInt() == 0);
    STATIC_CHECK((LayoutUnit(2) / LayoutUnit(1)).ToInt() == 2);
    STATIC_CHECK((LayoutUnit(2) / LayoutUnit(0.5)).ToInt() == 4);
    STATIC_CHECK((LayoutUnit(0.5) / LayoutUnit(2)).ToInt() == 0);
    STATIC_CHECK((LayoutUnit(100) / LayoutUnit(10)).ToInt() == 10);
    STATIC_CHECK((LayoutUnit(1) / LayoutUnit(2)).ToFloat() == 0.5f);
    STATIC_CHECK((LayoutUnit(0.5) / LayoutUnit(2)).ToFloat() == 0.25f);

    STATIC_CHECK((LayoutUnit(-1) / LayoutUnit(1)).ToInt() == -1);
    STATIC_CHECK((LayoutUnit(-1) / LayoutUnit(2)).ToInt() == 0);
    STATIC_CHECK((LayoutUnit(-2) / LayoutUnit(1)).ToInt() == -2);
    STATIC_CHECK((LayoutUnit(-2) / LayoutUnit(0.5)).ToInt() == -4);
    STATIC_CHECK((LayoutUnit(-0.5) / LayoutUnit(2)).ToInt() == 0);
    STATIC_CHECK((LayoutUnit(-100) / LayoutUnit(10)).ToInt() == -10);
    STATIC_CHECK((LayoutUnit(-1) / LayoutUnit(2)).ToFloat() == -0.5f);
    STATIC_CHECK((LayoutUnit(-0.5) / LayoutUnit(2)).ToFloat() == -0.25f);

    STATIC_CHECK((LayoutUnit(-1) / LayoutUnit(-1)).ToInt() == 1);
    STATIC_CHECK((LayoutUnit(-1) / LayoutUnit(-2)).ToInt() == 0);
    STATIC_CHECK((LayoutUnit(-2) / LayoutUnit(-1)).ToInt() == 2);
    STATIC_CHECK((LayoutUnit(-2) / LayoutUnit(-0.5)).ToInt() == 4);
    STATIC_CHECK((LayoutUnit(-0.5) / LayoutUnit(-2)).ToInt() == 0);
    STATIC_CHECK((LayoutUnit(-100) / LayoutUnit(-10)).ToInt() == 10);
    STATIC_CHECK((LayoutUnit(-1) / LayoutUnit(-2)).ToFloat() == 0.5f);
    STATIC_CHECK((LayoutUnit(-0.5) / LayoutUnit(-2)).ToFloat() == 0.25f);

    size_t aHundredSizeT = 100;
    CHECK((LayoutUnit(aHundredSizeT) / LayoutUnit(2)).ToInt() == 50);
    CHECK((aHundredSizeT / LayoutUnit(4)).ToInt() == 25);
    CHECK((LayoutUnit(400) / aHundredSizeT).ToInt() == 4);

    STATIC_CHECK((LayoutUnit(IntMaxForLayoutUnit) / LayoutUnit(2)).ToInt() == IntMaxForLayoutUnit / 2);
    STATIC_CHECK((LayoutUnit(IntMaxForLayoutUnit) / LayoutUnit(0.5)).ToInt() == IntMaxForLayoutUnit);
  }

  TEST_CASE("LayoutUnit: addition, subtraction and modulo", "[Core][LayoutUnit]")
  {
    STATIC_CHECK(LayoutUnit(1) + LayoutUnit(2) == LayoutUnit(3));
    STATIC_CHECK(LayoutUnit::Max() + LayoutUnit(1) == LayoutUnit::Max());
    STATIC_CHECK(LayoutUnit::Min() - LayoutUnit(1) == LayoutUnit::Min());
    STATIC_CHECK(LayoutUnit(1) - LayoutUnit(3) == LayoutUnit(-2));
    STATIC_CHECK(LayoutUnit(1) + 2 == LayoutUnit(3));
    STATIC_CHECK(2 + LayoutUnit(1) == LayoutUnit(3));
    STATIC_CHECK(LayoutUnit(1) + 0.5f == 1.5f);
    STATIC_CHECK(LayoutUnit(1) - 0.5f == 0.5f);

    LayoutUnit value(1);
    value += LayoutUnit(2);
    CHECK(value == LayoutUnit(3));
    value -= 1;
    CHECK(value == LayoutUnit(2));
    value *= LayoutUnit(3);
    CHECK(value == LayoutUnit(6));
    value /= LayoutUnit(4);
    CHECK(value == LayoutUnit(1.5));
    value = LayoutUnit::Max();
    value += LayoutUnit(1);
    CHECK(value == LayoutUnit::Max());

    float f = 1.0f;
    f += LayoutUnit(1);
    CHECK(f == 2.0f);

    // `%` is the remainder of the fixed-point division (a = (a / b) * b + a % b), not an integer modulo.
    STATIC_CHECK(LayoutUnit(5.5) % LayoutUnit(2) == LayoutUnit());
    STATIC_CHECK(LayoutUnit(1) % LayoutUnit::FromRawValue(100) == LayoutUnit::FromRawValue(1));
    STATIC_CHECK(LayoutUnit(7) % 4 == LayoutUnit());
    STATIC_CHECK(7 % LayoutUnit(4) == LayoutUnit());
    STATIC_CHECK(IntMod(LayoutUnit(5.5), LayoutUnit(2)) == LayoutUnit(1.5));
    STATIC_CHECK(IntMod(LayoutUnit(-5.5), LayoutUnit(2)) == LayoutUnit(-1.5));
  }

  TEST_CASE("LayoutUnit: comparison", "[Core][LayoutUnit]")
  {
    STATIC_CHECK(LayoutUnit(1) < LayoutUnit(2));
    STATIC_CHECK(LayoutUnit(1) == 1);
    STATIC_CHECK(1 == LayoutUnit(1));
    STATIC_CHECK(LayoutUnit(1) < 2);
    STATIC_CHECK(LayoutUnit(1.5) == 1.5f);
    STATIC_CHECK(LayoutUnit(1.5) > 1.25f);
    STATIC_CHECK(LayoutUnit(1.5) < 1.75);
    STATIC_CHECK(static_cast<bool>(LayoutUnit(0.5)));
    STATIC_CHECK_FALSE(static_cast<bool>(LayoutUnit()));
    STATIC_CHECK(IsIntegerValue(LayoutUnit(3)));
    STATIC_CHECK_FALSE(IsIntegerValue(LayoutUnit(3.5)));
  }

  TEST_CASE("LayoutUnit: ceil", "[Core][LayoutUnit]")
  {
    STATIC_CHECK(LayoutUnit(0).Ceil() == 0);
    STATIC_CHECK(LayoutUnit(0.1).Ceil() == 1);
    STATIC_CHECK(LayoutUnit(0.5).Ceil() == 1);
    STATIC_CHECK(LayoutUnit(0.9).Ceil() == 1);
    STATIC_CHECK(LayoutUnit(1.0).Ceil() == 1);
    STATIC_CHECK(LayoutUnit(1.1).Ceil() == 2);

    STATIC_CHECK(LayoutUnit(-0.1).Ceil() == 0);
    STATIC_CHECK(LayoutUnit(-0.5).Ceil() == 0);
    STATIC_CHECK(LayoutUnit(-0.9).Ceil() == 0);
    STATIC_CHECK(LayoutUnit(-1.0).Ceil() == -1);

    STATIC_CHECK(LayoutUnit(IntMaxForLayoutUnit).Ceil() == IntMaxForLayoutUnit);
    STATIC_CHECK((LayoutUnit(IntMaxForLayoutUnit) - LayoutUnit(0.5)).Ceil() == IntMaxForLayoutUnit);
    STATIC_CHECK((LayoutUnit(IntMaxForLayoutUnit) - LayoutUnit(1)).Ceil() == IntMaxForLayoutUnit - 1);

    STATIC_CHECK(LayoutUnit(IntMinForLayoutUnit).Ceil() == IntMinForLayoutUnit);

    CHECK(LayoutUnit(1.5).CeilToFloat() == 1.5f);
    CHECK(LayoutUnit(1.5).CeilToFloat() == LayoutUnit(1.5).ToFloat());
  }

  TEST_CASE("LayoutUnit: floor", "[Core][LayoutUnit]")
  {
    STATIC_CHECK(LayoutUnit(0).Floor() == 0);
    STATIC_CHECK(LayoutUnit(0.1).Floor() == 0);
    STATIC_CHECK(LayoutUnit(0.5).Floor() == 0);
    STATIC_CHECK(LayoutUnit(0.9).Floor() == 0);
    STATIC_CHECK(LayoutUnit(1.0).Floor() == 1);
    STATIC_CHECK(LayoutUnit(1.1).Floor() == 1);

    STATIC_CHECK(LayoutUnit(-0.1).Floor() == -1);
    STATIC_CHECK(LayoutUnit(-0.5).Floor() == -1);
    STATIC_CHECK(LayoutUnit(-0.9).Floor() == -1);
    STATIC_CHECK(LayoutUnit(-1.0).Floor() == -1);

    STATIC_CHECK(LayoutUnit(IntMaxForLayoutUnit).Floor() == IntMaxForLayoutUnit);

    STATIC_CHECK(LayoutUnit(IntMinForLayoutUnit).Floor() == IntMinForLayoutUnit);
    STATIC_CHECK((LayoutUnit(IntMinForLayoutUnit) + LayoutUnit(0.5)).Floor() == IntMinForLayoutUnit);
    STATIC_CHECK((LayoutUnit(IntMinForLayoutUnit) + LayoutUnit(1)).Floor() == IntMinForLayoutUnit + 1);
  }

  TEST_CASE("LayoutUnit: pixel snapping", "[Core][LayoutUnit]")
  {
    for (int32 i = -100'000; i <= 100'000; ++i)
    {
      REQUIRE(RoundToDevicePixel(LayoutUnit(i), 1) == static_cast<float>(i));
      REQUIRE(RoundToDevicePixel(LayoutUnit(i), 2) == static_cast<float>(i));
      REQUIRE(RoundToDevicePixel(LayoutUnit(i), 3) == static_cast<float>(i));
    }

    for (float i = -10'000; i < 0; i = i + 0.5f)
    {
      REQUIRE_THAT(RoundToDevicePixel(LayoutUnit(i), 2), WithinULP(i, 4));
    }

    for (float i = -10000.25f; i < 0; i = i + 0.5f)
    {
      REQUIRE_THAT(RoundToDevicePixel(LayoutUnit(i), 2), WithinULP(i + 0.25f, 4));
    }

    CHECK(FloorToDevicePixel(LayoutUnit(1.75f), 2) == 1.5f);
    CHECK(CeilToDevicePixel(LayoutUnit(1.25f), 2) == 1.5f);
    CHECK(FloorToDevicePixel(1.75f, 2) == 1.5f);
    CHECK(CeilToDevicePixel(1.25f, 2) == 1.5f);
    CHECK(RoundToDevicePixel(1.25f, 2) == 1.5f);
  }

  TEST_CASE("LayoutUnit: unary minus", "[Core][LayoutUnit]")
  {
    STATIC_CHECK(LayoutUnit() == -LayoutUnit());
    STATIC_CHECK(LayoutUnit(999) == -LayoutUnit(-999));
    STATIC_CHECK(LayoutUnit(-999) == -LayoutUnit(999));

    LayoutUnit negativeMax;
    negativeMax.SetRawValue(LayoutUnit::Min().RawValue() + 1);
    CHECK(negativeMax == -LayoutUnit::Max());
    CHECK(LayoutUnit::Max() == -negativeMax);
    // -LayoutUnit::Min() is saturated to LayoutUnit::Max()
    STATIC_CHECK(LayoutUnit::Max() == -LayoutUnit::Min());

    STATIC_CHECK(LayoutUnit(-3).Abs() == LayoutUnit(3));
    STATIC_CHECK(AbsoluteValue(LayoutUnit(3)) == LayoutUnit(3));
    STATIC_CHECK(LayoutUnit::Min().Abs() == LayoutUnit::Max());
  }

  TEST_CASE("LayoutUnit: FromFloatCeil", "[Core][LayoutUnit]")
  {
    CHECK(LayoutUnit(1.25f) == LayoutUnit::FromFloatCeil(1.25f));
    CHECK(LayoutUnit(1.25f + Tolerance) == LayoutUnit::FromFloatCeil(1.25f + Tolerance / 2));
    CHECK(LayoutUnit() == LayoutUnit::FromFloatCeil(-Tolerance / 2));

    // Larger than Max()
    CHECK(LayoutUnit::Max() == LayoutUnit::FromFloatCeil(Limits<float>::max()));
    CHECK(LayoutUnit::Max() == LayoutUnit::FromFloatCeil(Limits<float>::infinity()));
    // Smaller than Min()
    CHECK(LayoutUnit::Min() == LayoutUnit::FromFloatCeil(Limits<float>::lowest()));
    CHECK(LayoutUnit::Min() == LayoutUnit::FromFloatCeil(-Limits<float>::infinity()));
  }

  TEST_CASE("LayoutUnit: FromFloatFloor", "[Core][LayoutUnit]")
  {
    CHECK(LayoutUnit(1.25f) == LayoutUnit::FromFloatFloor(1.25f));
    CHECK(LayoutUnit(1.25f) == LayoutUnit::FromFloatFloor(1.25f + Tolerance / 2));
    CHECK(LayoutUnit(-Tolerance) == LayoutUnit::FromFloatFloor(-Tolerance / 2));

    // Larger than Max()
    CHECK(LayoutUnit::Max() == LayoutUnit::FromFloatFloor(Limits<float>::max()));
    CHECK(LayoutUnit::Max() == LayoutUnit::FromFloatFloor(Limits<float>::infinity()));
    // Smaller than Min()
    CHECK(LayoutUnit::Min() == LayoutUnit::FromFloatFloor(Limits<float>::lowest()));
    CHECK(LayoutUnit::Min() == LayoutUnit::FromFloatFloor(-Limits<float>::infinity()));
  }

  TEST_CASE("LayoutUnit: FromFloatRound", "[Core][LayoutUnit]")
  {
    CHECK(LayoutUnit(1.25f) == LayoutUnit::FromFloatRound(1.25f));
    CHECK(LayoutUnit(1.25f) == LayoutUnit::FromFloatRound(1.25f + Tolerance / 4));
    CHECK(LayoutUnit(1.25f + Tolerance) == LayoutUnit::FromFloatRound(1.25f + Tolerance * 3 / 4));
    CHECK(LayoutUnit(-Tolerance) == LayoutUnit::FromFloatRound(-Tolerance * 3 / 4));

    // Larger than Max()
    CHECK(LayoutUnit::Max() == LayoutUnit::FromFloatRound(Limits<float>::max()));
    CHECK(LayoutUnit::Max() == LayoutUnit::FromFloatRound(Limits<float>::infinity()));
    // Smaller than Min()
    CHECK(LayoutUnit::Min() == LayoutUnit::FromFloatRound(Limits<float>::lowest()));
    CHECK(LayoutUnit::Min() == LayoutUnit::FromFloatRound(-Limits<float>::infinity()));
  }

  TEST_CASE("LayoutUnit: increment", "[Core][LayoutUnit]")
  {
    STATIC_CHECK(LayoutUnit(-1) == ++LayoutUnit(-2));
    STATIC_CHECK(LayoutUnit(0) == ++LayoutUnit(-1));
    STATIC_CHECK(LayoutUnit(1) == ++LayoutUnit(0));
    STATIC_CHECK(LayoutUnit(2) == ++LayoutUnit(1));
    STATIC_CHECK(LayoutUnit::Max() == ++LayoutUnit(LayoutUnit::Max()));
  }

  TEST_CASE("LayoutUnit: limits and fractions", "[Core][LayoutUnit]")
  {
    STATIC_CHECK(LayoutUnit::Epsilon() == 1.0f / 64);
    STATIC_CHECK(LayoutUnit::NearlyMax().RawValue() == Limits<int32>::max() - 32);
    STATIC_CHECK(LayoutUnit::NearlyMin().RawValue() == Limits<int32>::min() + 32);
    STATIC_CHECK(LayoutUnit::Max().MightBeSaturated());
    STATIC_CHECK(LayoutUnit::Min().MightBeSaturated());
    STATIC_CHECK_FALSE(LayoutUnit(1).MightBeSaturated());
    STATIC_CHECK(LayoutUnit(1.5).Fraction() == LayoutUnit(0.5));
    STATIC_CHECK(LayoutUnit(-1.5).Fraction() == LayoutUnit(-0.5));
    STATIC_CHECK(LayoutUnit::Clamp(1e20) == LayoutUnit::Max());
    STATIC_CHECK(LayoutUnit::Clamp(-1e20) == LayoutUnit::Min());
    STATIC_CHECK(LayoutUnit::Clamp(2.5) == LayoutUnit(2.5));
    STATIC_CHECK(LayoutUnit::FromRawValue(96) == LayoutUnit(1.5));

    LayoutUnit wide;
    wide.SetRawValue(int64 {96});
    CHECK(wide == LayoutUnit(1.5));
  }

  TEST_CASE("LayoutUnit: hash and markable traits", "[Core][LayoutUnit]")
  {
    CHECK(DefaultHash<LayoutUnit>::Hash(LayoutUnit(2)) == DefaultHash<int32>::Hash(128));
    CHECK(DefaultHash<LayoutUnit>::Equal(LayoutUnit(2), LayoutUnit(2)));
    STATIC_CHECK(HashTraits<LayoutUnit>::EmptyValue() == LayoutUnit::Min());
    LayoutUnit slot;
    HashTraits<LayoutUnit>::ConstructDeletedValue(slot);
    CHECK(HashTraits<LayoutUnit>::IsDeletedValue(slot));
    STATIC_CHECK(MarkableTraits<LayoutUnit>::IsEmptyValue(MarkableTraits<LayoutUnit>::EmptyValue()));
    STATIC_CHECK(MarkableTraits<LayoutUnit>::EmptyValue() == LayoutUnit(-1));
  }
}
