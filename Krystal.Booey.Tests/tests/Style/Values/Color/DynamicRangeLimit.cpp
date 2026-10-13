#include "Krystal.Booey/Style/Values/Color/DynamicRangeLimit.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_floating_point.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Color.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Values/Color/DynamicRangeLimit.hpp"
#include "Krystal.Booey/CSS/Values/Color/DynamicRangeLimitMix.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/PlatformDynamicRangeLimit.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Values/Color/Opacity.hpp"
#include <string>

namespace krys::boo::style::tests
{
  namespace
  {
    using Catch::Matchers::WithinAbs;

    /// @brief Parses and converts a `<'dynamic-range-limit'>`; fails the test unless every token was
    /// consumed.
    DynamicRangeLimit Convert(utf8_stringview text)
    {
      css::InputStream input {css::CSSOMString(text)};
      css::Tokenizer tokenizer {input};
      tokenizer.PumpTokenizer();
      auto tokens = tokenizer.Tokens();

      css::ParserContext context {css::ParserMode::HTMLStandard};
      context.cssDynamicRangeLimitMixEnabled = true;
      context.cssConstrainedDynamicRangeLimitEnabled = true;
      css::PropertyParserState state {.Context = context};
      auto result = css::PropertyParserHelpers::ConsumeUnresolvedDynamicRangeLimit(tokens, state);
      REQUIRE(result);
      REQUIRE(tokens.IsAtEnd());
      return ConvertToStyle(*result, css::ToLengthConversionData {});
    }

    /// @brief The computed value, serialised as a std::string, which Catch2 can print.
    std::string Serialised(const DynamicRangeLimit &limit)
    {
      RenderStyle style;
      auto serialised = Serialise(css::DefaultSerialisationContext(), style, limit);
      return std::string(serialised.begin(), serialised.end());
    }

    std::string Serialised(utf8_stringview text)
    {
      return Serialised(Convert(text));
    }

    float PlatformValue(const DynamicRangeLimit &limit)
    {
      return limit.ToPlatformDynamicRangeLimit().Value();
    }

    const DynamicRangeLimit Standard {css::keywords::Standard {}};
    const DynamicRangeLimit Constrained {css::keywords::Constrained {}};
    const DynamicRangeLimit NoLimit {css::keywords::NoLimit {}};
  }

  TEST_CASE("Style dynamic-range-limit(keywords)", "[Style][Values][Color]")
  {
    CHECK(Convert(u8"standard") == Standard);
    CHECK(Convert(u8"constrained") == Constrained);
    CHECK(Convert(u8"no-limit") == NoLimit);
    CHECK(Standard != NoLimit);

    CHECK(Serialised(u8"standard") == "standard");
    CHECK(Serialised(u8"constrained") == "constrained");
    CHECK(Serialised(u8"no-limit") == "no-limit");
  }

  TEST_CASE("Style dynamic-range-limit(mixes)", "[Style][Values][Color]")
  {
    SECTION("weighted sums of the keywords")
    {
      CHECK(Serialised(u8"dynamic-range-limit-mix(standard 25%, no-limit 75%)")
            == "dynamic-range-limit-mix(standard 25%, no-limit 75%)");

      // Repeated keywords add up.
      CHECK(Serialised(u8"dynamic-range-limit-mix(standard 25%, no-limit 50%, standard 25%)")
            == "dynamic-range-limit-mix(standard 50%, no-limit 50%)");
    }

    SECTION("percentages are normalised to sum to 100%")
    {
      CHECK(Serialised(u8"dynamic-range-limit-mix(standard 10%, constrained 30%)")
            == "dynamic-range-limit-mix(standard 25%, constrained 75%)");
      CHECK(Serialised(u8"dynamic-range-limit-mix(standard 100%, no-limit 100%)")
            == "dynamic-range-limit-mix(standard 50%, no-limit 50%)");
    }

    SECTION("a keyword with all the weight is the keyword")
    {
      CHECK(Convert(u8"dynamic-range-limit-mix(standard 50%, standard 50%)") == Standard);
      CHECK(Convert(u8"dynamic-range-limit-mix(no-limit 10%, no-limit 90%)") == NoLimit);
      CHECK(Convert(u8"dynamic-range-limit-mix(constrained 30%, standard 0%)") == Constrained);
    }

    SECTION("nested mixes are flattened")
    {
      CHECK(
        Serialised(u8"dynamic-range-limit-mix(dynamic-range-limit-mix(standard 50%, constrained 50%) 40%, "
                   u8"no-limit 60%)")
        == "dynamic-range-limit-mix(standard 20%, constrained 20%, no-limit 60%)");
    }

    SECTION("copies")
    {
      auto mix = Convert(u8"dynamic-range-limit-mix(standard 25%, no-limit 75%)");
      auto copy = mix;
      CHECK(copy == mix);
      CHECK(Serialised(copy) == Serialised(mix));

      DynamicRangeLimit assigned = Standard;
      assigned = mix;
      CHECK(assigned == mix);
      CHECK(assigned != Standard);
    }
  }

  TEST_CASE("Style dynamic-range-limit(blending)", "[Style][Values][Color]")
  {
    auto blend = [](const DynamicRangeLimit &from, const DynamicRangeLimit &to, double progress)
    {
      return Blending<DynamicRangeLimit> {}.Blend(from, to, BlendingContext {progress});
    };

    CHECK(blend(Standard, NoLimit, 0) == Standard);
    CHECK(blend(Standard, NoLimit, 1) == NoLimit);
    CHECK(blend(Standard, Standard, 0.5) == Standard);
    CHECK(Serialised(blend(Standard, NoLimit, 0.25))
          == "dynamic-range-limit-mix(standard 75%, no-limit 25%)");

    auto mix = Convert(u8"dynamic-range-limit-mix(standard 50%, constrained 50%)");
    CHECK(Serialised(blend(mix, NoLimit, 0.5))
          == "dynamic-range-limit-mix(standard 25%, constrained 25%, no-limit 50%)");
  }

  TEST_CASE("Style dynamic-range-limit(platform)", "[Style][Values][Color][Graphics]")
  {
    CHECK(PlatformDynamicRangeLimit::Standard().Value() == 0.0f);
    CHECK(PlatformDynamicRangeLimit::Constrained().Value() == 0.5f);
    CHECK(PlatformDynamicRangeLimit::NoLimit().Value() == 1.0f);
    CHECK(PlatformDynamicRangeLimit::InitialValue() == PlatformDynamicRangeLimit::NoLimit());
    CHECK(PlatformDynamicRangeLimit::InitialValueForVideos() == PlatformDynamicRangeLimit::NoLimit());
    CHECK(PlatformDynamicRangeLimit::DefaultWhenSuppressingHDR() == PlatformDynamicRangeLimit::Standard());
    CHECK(PlatformDynamicRangeLimit::DefaultWhenSuppressingHDRInVideos()
          == PlatformDynamicRangeLimit::Constrained());
    CHECK(PlatformDynamicRangeLimit::Standard() < PlatformDynamicRangeLimit::Constrained());

    CHECK(PlatformValue(Standard) == 0.0f);
    CHECK(PlatformValue(Constrained) == 0.5f);
    CHECK(PlatformValue(NoLimit) == 1.0f);

    // The weighted average of the keyword values.
    CHECK_THAT(PlatformValue(Convert(u8"dynamic-range-limit-mix(standard 25%, no-limit 75%)")),
               WithinAbs(0.75, 1e-6));
    CHECK_THAT(PlatformValue(Convert(u8"dynamic-range-limit-mix(standard 50%, constrained 50%)")),
               WithinAbs(0.25, 1e-6));
    CHECK_THAT(PlatformValue(Convert(u8"dynamic-range-limit-mix(constrained 40%, no-limit 60%)")),
               WithinAbs(0.8, 1e-6));
  }

  TEST_CASE("Style opacity", "[Style][Values][Color]")
  {
    using namespace css::Literals;

    constexpr Opacity transparent {0.0f};
    constexpr Opacity half {0.5_css_number};
    constexpr Opacity opaque {Opacity::Number {1.0f}};

    STATIC_CHECK(transparent.IsZero());
    STATIC_CHECK(transparent.IsTransparent());
    STATIC_CHECK_FALSE(transparent.IsOpaque());
    STATIC_CHECK_FALSE(half.IsZero());
    STATIC_CHECK_FALSE(half.IsOpaque());
    STATIC_CHECK(opaque.IsOpaque());
    STATIC_CHECK(transparent < half);
    STATIC_CHECK(half < opaque);
    STATIC_CHECK(half == Opacity {0.5f});
    CHECK(get<0>(half).Value == 0.5f);
  }
}
