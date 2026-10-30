#include "Krystal.Booey/Style/Values/Types.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo::style::tests
{
  using namespace css::Literals;

  namespace
  {
    // Has only `IsZero`, so `IsValueKnownZero` falls back to it.
    struct OnlyIsZero
    {
      bool Zero {false};

      KRYS_NODISCARD bool IsZero() const noexcept
      {
        return Zero;
      }
    };

    struct HasEmpty
    {
      bool Empty {false};

      KRYS_NODISCARD bool IsEmpty() const noexcept
      {
        return Empty;
      }
    };

    constexpr css::NoConversionDataRequiredTag Token {};
  }

  TEST_CASE("Style Types: conversions through aggregates", "[Style][Types]")
  {
    using CSSSize = css::SpaceSeparatedSize<css::Number<>>;
    using StyleSize = css::SpaceSeparatedSize<Number<>>;
    STATIC_CHECK((SameType<StyleType<CSSSize>, StyleSize>));

    auto size = ConvertToStyle(CSSSize {css::Number<> {1}, css::Number<> {2}}, Token);
    CHECK(size.Width() == Number<> {1});
    CHECK(size.Height() == Number<> {2});

    auto present = ConvertToStyle(Maybe<css::Number<>> {css::Number<> {3}}, Token);
    REQUIRE(present);
    CHECK(*present == Number<> {3});
    CHECK_FALSE(ConvertToStyle(Maybe<css::Number<>> {}, Token));

    auto variant =
      ConvertToStyle(Variant<css::Number<>, css::keywords::Auto> {css::keywords::Auto {}}, Token);
    CHECK(std::holds_alternative<css::keywords::Auto>(variant));

    auto list =
      ConvertToStyle(css::CommaSeparatedList<css::Number<>> {css::Number<> {1}, css::Number<> {2}}, Token);
    REQUIRE(list.size() == 2);
    CHECK(list[1] == Number<> {2});

    // Non-converting types pass through.
    CHECK(ConvertToStyle(css::keywords::Auto {}, Token) == css::keywords::Auto {});
  }

  TEST_CASE("Style Types: blending through aggregates", "[Style][Types]")
  {
    using Size = css::SpaceSeparatedSize<Number<>>;
    const Size from {Number<> {0}, Number<> {10}};
    const Size to {Number<> {4}, Number<> {20}};

    CHECK(EqualsForBlending(from, from));
    CHECK_FALSE(EqualsForBlending(from, to));
    CHECK(CanBlend(from, to));
    CHECK_FALSE(RequiresInterpolationForAccumulativeIteration(from, to));

    auto blended = Blend(from, to, BlendingContext(0.5));
    CHECK(blended.Width() == Number<> {2});
    CHECK(blended.Height() == Number<> {15});

    const Maybe<Number<>> none;
    const Maybe<Number<>> some {Number<> {1}};
    CHECK(EqualsForBlending(none, none));
    CHECK_FALSE(CanBlend(none, some));
    CHECK(Blend(some, Maybe<Number<>> {Number<> {3}}, BlendingContext(0.5)) == Number<> {2});

    using Choice = Variant<Number<>, Percentage<>>;
    CHECK(CanBlend(Choice {Number<> {0}}, Choice {Number<> {2}}));
    CHECK_FALSE(CanBlend(Choice {Number<> {0}}, Choice {Percentage<> {2}}));
    CHECK(Blend(Choice {Number<> {0}}, Choice {Number<> {2}}, BlendingContext(0.5)) == Choice {Number<> {1}});

    const css::CommaSeparatedList<Number<>> listFrom {Number<> {0}, Number<> {2}};
    const css::CommaSeparatedList<Number<>> listTo {Number<> {2}, Number<> {6}};
    auto blendedList = Blend(listFrom, listTo, BlendingContext(0.5));
    REQUIRE(blendedList.size() == 2);
    CHECK(blendedList[0] == Number<> {1});
    CHECK(blendedList[1] == Number<> {4});
    CHECK_FALSE(CanBlend(listFrom, css::CommaSeparatedList<Number<>> {Number<> {1}}));

    CHECK(Blend(css::keywords::Auto {}, css::keywords::Auto {}, BlendingContext(0.5))
          == css::keywords::Auto {});
  }

  TEST_CASE("Style Types: zero and empty invokers", "[Style][Types]")
  {
    CHECK(IsValueZero(Number<> {0}));
    CHECK_FALSE(IsValueZero(Number<> {1}));
    CHECK(IsValueKnownZero(Number<> {0}));
    CHECK(IsValueZero(css::SpaceSeparatedSize<Number<>> {Number<> {0}, Number<> {0}}));
    CHECK_FALSE(IsValueZero(css::SpaceSeparatedSize<Number<>> {Number<> {0}, Number<> {1}}));
    CHECK(IsValueZero(Variant<Number<>, Percentage<>> {Percentage<> {0}}));

    // Types with only `IsZero` report it as known zero.
    CHECK(IsValueKnownZero(OnlyIsZero {true}));
    CHECK_FALSE(IsValueKnownZero(OnlyIsZero {false}));

    CHECK(IsValueEmpty(css::SpaceSeparatedSize<Number<>> {Number<> {0}, Number<> {1}}));
    CHECK_FALSE(IsValueEmpty(css::SpaceSeparatedSize<Number<>> {Number<> {2}, Number<> {1}}));
    CHECK(IsValueKnownEmpty(css::SpaceSeparatedSize<Number<>> {Number<> {2}, Number<> {0}}));
    CHECK(IsValueKnownEmpty(HasEmpty {true}));
    CHECK_FALSE(IsValueKnownEmpty(HasEmpty {false}));
  }

  TEST_CASE("Style Types: evaluation through wrappers", "[Style][Types]")
  {
    CHECK(Evaluate<double>(Variant<Number<>, Percentage<>> {Number<> {2}}) == 2.0);
    CHECK(Evaluate<double>(Variant<Number<>, Percentage<>> {Percentage<> {50}}, 10.0) == 5.0);
  }
}
