#include "Krystal.Booey/CSS/Properties/PropertyShorthand.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyShorthandFunctions.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    List<PropertyId> Longhands(const PropertyShorthand &shorthand)
    {
      return List<PropertyId>(shorthand.begin(), shorthand.end());
    }
  }

  TEST_CASE("PropertyShorthand", "[CSS][Properties][PropertyShorthand]")
  {
    SECTION("Default is empty")
    {
      PropertyShorthand shorthand;
      CHECK(shorthand.Id() == PropertyId::Invalid);
      CHECK(shorthand.Size() == 0uz);
      CHECK(shorthand.begin() == shorthand.end());
    }

    SECTION("ShorthandForProperty")
    {
      auto margin = ShorthandForProperty(PropertyId::Margin);
      CHECK(margin.Id() == PropertyId::Margin);
      CHECK(margin.Size() == 4uz);
      CHECK(margin.Properties().size() == 4uz);
      CHECK(Longhands(margin)
            == List<PropertyId> {PropertyId::MarginTop, PropertyId::MarginRight, PropertyId::MarginBottom,
                                 PropertyId::MarginLeft});
    }

    SECTION("ShorthandForProperty on a longhand is empty")
    {
      auto shorthand = ShorthandForProperty(PropertyId::MarginTop);
      CHECK(shorthand.Id() == PropertyId::Invalid);
      CHECK(shorthand.Size() == 0uz);
    }

    SECTION("MatchingShorthandsForLonghand")
    {
      auto shorthands = MatchingShorthandsForLonghand(PropertyId::MarginTop);
      REQUIRE(shorthands.size() == 2uz);
      CHECK(shorthands[0].Id() == PropertyId::All);
      CHECK(shorthands[1].Id() == PropertyId::Margin);

      CHECK(MatchingShorthandsForLonghand(PropertyId::Margin).empty());
    }

    SECTION("IndexOfShorthandForLonghand")
    {
      auto shorthands = MatchingShorthandsForLonghand(PropertyId::AnimationRangeStart);
      REQUIRE(shorthands.size() == 3uz);
      CHECK(IndexOfShorthandForLonghand(PropertyId::All, shorthands) == 0u);
      CHECK(IndexOfShorthandForLonghand(PropertyId::Animation, shorthands) == 1u);
      CHECK(IndexOfShorthandForLonghand(PropertyId::AnimationRange, shorthands) == 2u);
    }

    SECTION("TransitionShorthandForParsing")
    {
      auto shorthand = TransitionShorthandForParsing();
      CHECK(shorthand.Id() == PropertyId::Transition);
      CHECK(Longhands(shorthand)
            == List<PropertyId> {PropertyId::TransitionDuration, PropertyId::TransitionTimingFunction,
                                 PropertyId::TransitionDelay, PropertyId::TransitionBehavior,
                                 PropertyId::TransitionProperty});

      // Same longhands as the generated shorthand, in a different order.
      auto generated = Longhands(ShorthandForProperty(PropertyId::Transition));
      auto parsing = Longhands(shorthand);
      std::ranges::sort(generated);
      std::ranges::sort(parsing);
      CHECK(generated == parsing);
    }
  }
}
