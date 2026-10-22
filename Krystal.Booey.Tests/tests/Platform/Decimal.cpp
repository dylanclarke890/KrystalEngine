#include "Krystal.Booey/Platform/Decimal.hpp"
#include "catch2/catch_test_macros.hpp"
#include <cmath>
#include <string>

namespace krys::boo::tests
{
  namespace
  {
    KRYS_NODISCARD Decimal FromString(const char *string) noexcept
    {
      return Decimal::FromString(utf8_stringview(reinterpret_cast<const char8 *>(string)));
    }

    KRYS_NODISCARD std::string ToString(const Decimal &decimal) noexcept
    {
      auto string = decimal.ToString();
      return std::string(reinterpret_cast<const char *>(string.data()), string.size());
    }

    KRYS_NODISCARD Decimal Encode(uint64 coefficient, int exponent, Decimal::Sign sign) noexcept
    {
      return Decimal(sign, exponent, coefficient);
    }
  }

  TEST_CASE("Decimal: arithmetic is exact in base ten", "[Platform][Decimal]")
  {
    CHECK(ToString(FromString("0.1") + FromString("0.2")) == "0.3");
    CHECK(ToString(Decimal(1) - FromString("0.9")) == "0.1");
    CHECK(ToString(Decimal(3) * FromString("1.1")) == "3.3");
    CHECK(ToString(Decimal(1) / Decimal(4)) == "0.25");
    CHECK(ToString(Decimal(2) / Decimal(3)) == "0.666666666666667");
    CHECK(ToString(Decimal(-5) + Decimal(5)) == "0");
    CHECK(ToString(-Decimal(7)) == "-7");

    // A product that overflows 64 bits is scaled down.
    auto big = Encode(Decimal::MaxCoefficient, 0, Decimal::Positive);
    auto product = big * big;
    CHECK(product.IsFinite());
    CHECK(product.Exponent() > 0);
  }

  TEST_CASE("Decimal: special values", "[Platform][Decimal]")
  {
    auto infinity = Decimal::Infinity(Decimal::Positive);
    auto negativeInfinity = Decimal::Infinity(Decimal::Negative);
    auto nan = Decimal::NaN();

    CHECK((infinity + negativeInfinity).IsNaN());
    CHECK((infinity - negativeInfinity) == infinity);
    CHECK((Decimal(0) * infinity).IsNaN());
    CHECK((Decimal(1) / Decimal(0)) == infinity);
    CHECK((Decimal(0) / Decimal(0)).IsNaN());
    CHECK((Decimal(1) / infinity).IsZero());
    CHECK((nan + Decimal(1)).IsNaN());

    // NaN equals only NaN (their encodings match), and is unequal to nothing.
    CHECK(nan == nan);
    CHECK_FALSE(nan == Decimal(1));
    CHECK_FALSE(nan != nan);
    CHECK_FALSE(nan != Decimal(1));
    CHECK_FALSE(nan < Decimal(1));
    CHECK_FALSE(nan > Decimal(1));

    CHECK(ToString(infinity) == "Infinity");
    CHECK(ToString(negativeInfinity) == "-Infinity");
    CHECK(ToString(nan) == "NaN");
  }

  TEST_CASE("Decimal: comparison", "[Platform][Decimal]")
  {
    CHECK(Decimal(1) < Decimal(2));
    CHECK(FromString("1.5") > Decimal(1));
    CHECK(FromString("1.50") == FromString("1.5"));
    CHECK(Decimal(-1) < Decimal(0));
    CHECK(Decimal::Infinity(Decimal::Negative) < Decimal(-1'000));
    CHECK(Decimal::Zero(Decimal::Negative) == Decimal::Zero(Decimal::Positive));
  }

  TEST_CASE("Decimal: rounding and remainders", "[Platform][Decimal]")
  {
    CHECK(ToString(FromString("1.5").Ceil()) == "2");
    CHECK(ToString(FromString("-1.5").Ceil()) == "-1");
    CHECK(ToString(FromString("1.5").Floor()) == "1");
    CHECK(ToString(FromString("-1.5").Floor()) == "-2");
    CHECK(ToString(FromString("1.5").Round()) == "2");
    CHECK(ToString(FromString("1.4").Round()) == "1");
    CHECK(ToString(FromString("-1.5").Round()) == "-2");
    CHECK(ToString(FromString("0.4").Ceil()) == "1");
    CHECK(ToString(FromString("-0.4").Floor()) == "-1");
    CHECK(ToString(Encode(10, -1, Decimal::Positive).Ceil()) == "1");
    CHECK(ToString(Encode(10, -1, Decimal::Negative).Floor()) == "-1");

    CHECK(ToString(Decimal(10).Remainder(Decimal(3))) == "1");
    CHECK(ToString(Decimal(-10).Remainder(Decimal(3))) == "-1");
    CHECK(ToString(FromString("5.5").Remainder(Decimal(2))) == "1.5");
    CHECK(FromString("0.3").Remainder(FromString("0.1")).IsZero());
    CHECK(ToString(FromString("-3.5").Abs()) == "3.5");
  }

  TEST_CASE("Decimal: parsing strings", "[Platform][Decimal]")
  {
    CHECK(ToString(FromString("0")) == "0");
    CHECK(ToString(FromString("-0")) == "-0");
    CHECK(ToString(FromString("+1")) == "1");
    CHECK(ToString(FromString("123.456")) == "123.456");
    CHECK(ToString(FromString(".5")) == "0.5");
    CHECK(ToString(FromString("5.")) == "5");
    CHECK(ToString(FromString("1e3")) == "1e+3");
    CHECK(ToString(FromString("1.5E-2")) == "0.015");
    CHECK(ToString(FromString("1e+2")) == "1e+2");
    CHECK(FromString("1e+2") == Decimal(100));
    CHECK(ToString(FromString("0001")) == "1");

    CHECK(FromString("").IsNaN());
    CHECK(FromString("abc").IsNaN());
    CHECK(FromString("1e").IsNaN());
    CHECK(FromString("--1").IsNaN());
    CHECK(FromString("1.2.3").IsNaN());
    CHECK(FromString(" 1").IsNaN());

    // Exponents beyond the range become infinities or zeros.
    CHECK(FromString("1e2000").IsInfinity());
    CHECK(FromString("1e-2000").IsZero());
  }

  TEST_CASE("Decimal: serialising", "[Platform][Decimal]")
  {
    CHECK(ToString(Encode(1, 21, Decimal::Positive)) == "1e+21");
    CHECK(ToString(Encode(15, 20, Decimal::Positive)) == "1.5e+21");
    CHECK(ToString(Encode(1, -7, Decimal::Positive)) == "1e-7");
    CHECK(ToString(Encode(1, -6, Decimal::Positive)) == "0.000001");
    CHECK(ToString(Encode(12'345, -2, Decimal::Negative)) == "-123.45");
    CHECK(ToString(Encode(100, 0, Decimal::Positive)) == "100");
  }

  TEST_CASE("Decimal: doubles", "[Platform][Decimal]")
  {
    CHECK(ToString(Decimal::FromDouble(0.1)) == "0.1");
    CHECK(ToString(Decimal::FromDouble(-2.5)) == "-2.5");
    CHECK(ToString(Decimal::FromDouble(1e21)) == "1e+21");
    CHECK(Decimal::FromDouble(INFINITY).IsInfinity());
    CHECK(Decimal::FromDouble(NAN).IsNaN());

    CHECK(FromString("0.1").ToDouble() == 0.1);
    CHECK(FromString("-1234.5").ToDouble() == -1234.5);
    CHECK(Decimal::DoubleMax().ToDouble() == 1.7976931348623157e308);
    CHECK(std::isinf(Decimal::Infinity(Decimal::Negative).ToDouble()));
    CHECK(std::isnan(Decimal::NaN().ToDouble()));
  }
}
