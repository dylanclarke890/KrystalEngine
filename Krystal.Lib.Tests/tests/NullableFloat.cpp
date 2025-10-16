/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Krystal.Lib/NullableFloat.hpp"
#include "Krystal.Lib/ComparisonHelpers.hpp"
#include <catch_all.hpp>
#include <cmath>

namespace Krys::Tests
{
  constexpr auto empty = NullableFloat {};
  constexpr auto zero = NullableFloat {0.0f};
  constexpr auto one = NullableFloat {1.0f};
  constexpr auto positive = NullableFloat {1234.5f};
  constexpr auto negative = NullableFloat {-9876.5f};

  TEST_CASE("NullableFloat::value", "[LayoutEngine]")
  {
    REQUIRE(Krys::IsUndefined(empty.Value()));
    REQUIRE(zero.Value() == 0.0f);
    REQUIRE(one.Value() == 1.0f);
    REQUIRE(positive.Value() == 1234.5f);
    REQUIRE(negative.Value() == -9876.5f);

    REQUIRE(!empty.HasValue());
    REQUIRE(zero.HasValue());
    REQUIRE(one.HasValue());
    REQUIRE(positive.HasValue());
    REQUIRE(negative.HasValue());
  }

  TEST_CASE("NullableFloat::equality", "[LayoutEngine]")
  {
    REQUIRE(empty == empty);
    REQUIRE(!(empty == zero));
    REQUIRE(!(empty == negative));
    REQUIRE(!(empty == 12.3f));

    REQUIRE(zero == zero);
    REQUIRE(zero == 0.0f);
    REQUIRE_FALSE(zero == positive);
    REQUIRE_FALSE(zero == -5555.5f);

    REQUIRE(one == one);
    REQUIRE(one == 1.0f);
    REQUIRE_FALSE(one == positive);

    REQUIRE(positive == positive);
    REQUIRE(positive == positive.Value());
    REQUIRE_FALSE(positive == one);

    REQUIRE(negative == negative);
    REQUIRE(negative == negative.Value());
    REQUIRE_FALSE(negative == zero);
  }

  TEST_CASE("NullableFloat::inequality", "[LayoutEngine]")
  {
    REQUIRE_FALSE(empty != empty);
    REQUIRE(empty != zero);
    REQUIRE(empty != negative);
    REQUIRE(empty != 12.3f);

    REQUIRE_FALSE(zero != zero);
    REQUIRE_FALSE(zero != 0.0f);
    REQUIRE(zero != positive);
    REQUIRE(zero != -5555.5f);

    REQUIRE_FALSE(one != one);
    REQUIRE_FALSE(one != 1.0f);
    REQUIRE(one != positive);

    REQUIRE_FALSE(positive != positive);
    REQUIRE_FALSE(positive != positive.Value());
    REQUIRE(positive != one);

    REQUIRE_FALSE(negative != negative);
    REQUIRE_FALSE(negative != negative.Value());
    REQUIRE(negative != zero);
  }

  TEST_CASE("NullableFloat::greater_than_with_undefined", "[LayoutEngine]")
  {
    REQUIRE_FALSE(empty > empty);
    REQUIRE_FALSE(empty > zero);
    REQUIRE_FALSE(empty > one);
    REQUIRE_FALSE(empty > positive);
    REQUIRE_FALSE(empty > negative);
    REQUIRE_FALSE(zero > empty);
    REQUIRE_FALSE(one > empty);
    REQUIRE_FALSE(positive > empty);
    REQUIRE_FALSE(negative > empty);
  }

  TEST_CASE("NullableFloat::greater_than", "[LayoutEngine]")
  {
    REQUIRE(zero > negative);
    REQUIRE_FALSE(zero > zero);
    REQUIRE_FALSE(zero > positive);
    REQUIRE_FALSE(zero > one);

    REQUIRE(one > negative);
    REQUIRE(one > zero);
    REQUIRE_FALSE(one > positive);

    REQUIRE(negative > NullableFloat {-INFINITY});
  }

  TEST_CASE("NullableFloat::less_than_with_undefined", "[LayoutEngine]")
  {
    REQUIRE_FALSE(empty < empty);
    REQUIRE_FALSE(zero < empty);
    REQUIRE_FALSE(one < empty);
    REQUIRE_FALSE(positive < empty);
    REQUIRE_FALSE(negative < empty);
    REQUIRE_FALSE(empty < zero);
    REQUIRE_FALSE(empty < one);
    REQUIRE_FALSE(empty < positive);
    REQUIRE_FALSE(empty < negative);
  }

  TEST_CASE("NullableFloat::less_than", "[LayoutEngine]")
  {
    REQUIRE(negative < zero);
    REQUIRE_FALSE(zero < zero);
    REQUIRE_FALSE(positive < zero);
    REQUIRE_FALSE(one < zero);

    REQUIRE(negative < one);
    REQUIRE(zero < one);
    REQUIRE_FALSE(positive < one);

    REQUIRE(NullableFloat {-INFINITY} < negative);
  }

  TEST_CASE("NullableFloat::greater_than_equals_with_undefined", "[LayoutEngine]")
  {
    REQUIRE(empty >= empty);
    REQUIRE_FALSE(empty >= zero);
    REQUIRE_FALSE(empty >= one);
    REQUIRE_FALSE(empty >= positive);
    REQUIRE_FALSE(empty >= negative);
    REQUIRE_FALSE(zero >= empty);
    REQUIRE_FALSE(one >= empty);
    REQUIRE_FALSE(positive >= empty);
    REQUIRE_FALSE(negative >= empty);
  }

  TEST_CASE("NullableFloat::greater_than_equals", "[LayoutEngine]")
  {
    REQUIRE(zero >= negative);
    REQUIRE(zero >= zero);
    REQUIRE_FALSE(zero >= positive);
    REQUIRE_FALSE(zero >= one);

    REQUIRE(one >= negative);
    REQUIRE(one >= zero);
    REQUIRE_FALSE(one >= positive);

    REQUIRE(negative >= NullableFloat {-INFINITY});
  }

  TEST_CASE("NullableFloat::less_than_equals_with_undefined", "[LayoutEngine]")
  {
    REQUIRE(empty <= empty);
    REQUIRE_FALSE(zero <= empty);
    REQUIRE_FALSE(one <= empty);
    REQUIRE_FALSE(positive <= empty);
    REQUIRE_FALSE(negative <= empty);
    REQUIRE_FALSE(empty <= zero);
    REQUIRE_FALSE(empty <= one);
    REQUIRE_FALSE(empty <= positive);
    REQUIRE_FALSE(empty <= negative);
  }

  TEST_CASE("NullableFloat::less_than_equals", "[LayoutEngine]")
  {
    REQUIRE(negative <= zero);
    REQUIRE(zero <= zero);
    REQUIRE_FALSE(positive <= zero);
    REQUIRE_FALSE(one <= zero);

    REQUIRE(negative <= one);
    REQUIRE(zero <= one);
    REQUIRE_FALSE(positive <= one);

    REQUIRE(NullableFloat {-INFINITY} <= negative);
  }

  TEST_CASE("NullableFloat::addition", "[LayoutEngine]")
  {
    auto n = negative.Value();
    auto p = positive.Value();

    REQUIRE(zero + one == one);
    REQUIRE(negative + positive == NullableFloat {n + p});
    REQUIRE(empty + zero == empty);
    REQUIRE(empty + empty == empty);
    REQUIRE(negative + empty == empty);
  }

  TEST_CASE("NullableFloat::maxOrDefined", "[LayoutEngine]")
  {
    REQUIRE(Krys::MaxOrDefined(empty, empty) == empty);
    REQUIRE(Krys::MaxOrDefined(empty, positive) == positive);
    REQUIRE(Krys::MaxOrDefined(negative, empty) == negative);
    REQUIRE(Krys::MaxOrDefined(negative, NullableFloat {-INFINITY}) == negative);
    REQUIRE(Krys::MaxOrDefined(NullableFloat {1.0f}, NullableFloat {1.125f}) == NullableFloat {1.125f});
  }

  TEST_CASE("NullableFloat::Value", "[LayoutEngine]")
  {
    REQUIRE(Krys::IsUndefined(empty.Value()));
    REQUIRE(zero.Value() == 0.0f);
    REQUIRE(NullableFloat {123456.78f}.Value() == 123456.78f);
  }
}
