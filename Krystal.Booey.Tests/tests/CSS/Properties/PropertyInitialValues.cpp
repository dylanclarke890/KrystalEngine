#include "Krystal.Booey/CSS/Properties/PropertyInitialValues.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/StylePropertiesTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    /// @brief Whether the parsed value of `longhand` in `css` is its initial value.
    KRYS_NODISCARD bool IsInitial(utf8_stringview css, PropertyId longhand)
    {
      auto properties = ParseDeclarations(css);
      auto value = properties->GetPropertyCSSValue(longhand);
      REQUIRE(value);
      return IsInitialValueForLonghand(longhand, *value);
    }
  }

  TEST_CASE("InitialValueTextForLonghand", "[CSS][Properties][PropertyInitialValues]")
  {
    CHECK(ToNarrow(InitialValueTextForLonghand(PropertyId::Display)) == "inline");
    CHECK(ToNarrow(InitialValueTextForLonghand(PropertyId::BackgroundColor)) == "transparent");
    CHECK(ToNarrow(InitialValueTextForLonghand(PropertyId::Opacity)) == "1");
    CHECK(ToNarrow(InitialValueTextForLonghand(PropertyId::BorderImageSlice)) == "100%");
    CHECK(ToNarrow(InitialValueTextForLonghand(PropertyId::TransformOriginX)) == "50%");
    CHECK(ToNarrow(InitialValueTextForLonghand(PropertyId::StrokeWidth)) == "1px");
    CHECK(ToNarrow(InitialValueTextForLonghand(PropertyId::TransitionDuration)) == "0s");
    CHECK(ToNarrow(InitialValueTextForLonghand(PropertyId::TabSize)) == "8");
    CHECK(ToNarrow(InitialValueTextForLonghand(PropertyId::GlyphOrientationHorizontal)) == "0deg");
  }

  TEST_CASE("InitialValueIdForLonghand", "[CSS][Properties][PropertyInitialValues]")
  {
    CHECK(InitialValueIdForLonghand(PropertyId::Display) == ValueId::Inline);
    CHECK(InitialValueIdForLonghand(PropertyId::BorderTopStyle) == ValueId::None);
    CHECK(InitialValueIdForLonghand(PropertyId::Opacity) == ValueId::Invalid);
  }

  TEST_CASE_METHOD(ValueFixture, "IsInitialValueForLonghand", "[CSS][Properties][PropertyInitialValues]")
  {
    SECTION("Keywords and numbers")
    {
      CHECK(IsInitial(u8"display: inline", PropertyId::Display));
      CHECK_FALSE(IsInitial(u8"display: block", PropertyId::Display));
      CHECK(IsInitial(u8"opacity: 1", PropertyId::Opacity));
      CHECK_FALSE(IsInitial(u8"opacity: 0.5", PropertyId::Opacity));
      CHECK_FALSE(IsInitial(u8"opacity: calc(1)", PropertyId::Opacity));
      CHECK(IsInitial(u8"transition-duration: 0s", PropertyId::TransitionDuration));
      CHECK_FALSE(IsInitial(u8"transition-duration: 0ms", PropertyId::TransitionDuration));
    }

    SECTION("Implicit initial values")
    {
      CHECK(IsInitial(u8"background: red", PropertyId::BackgroundImage));
      CHECK_FALSE(IsInitial(u8"background: red", PropertyId::BackgroundColor));
    }

    SECTION("Multi-part initial values")
    {
      CHECK(IsInitial(u8"background-size: auto", PropertyId::BackgroundSize));
      CHECK(IsInitial(u8"background-size: auto auto", PropertyId::BackgroundSize));
      CHECK_FALSE(IsInitial(u8"background-size: 10px auto", PropertyId::BackgroundSize));
      CHECK(IsInitial(u8"border-image-outset: 0", PropertyId::BorderImageOutset));
      CHECK_FALSE(IsInitial(u8"border-image-outset: 0 1", PropertyId::BorderImageOutset));
      CHECK(IsInitial(u8"border-image-repeat: stretch", PropertyId::BorderImageRepeat));
      CHECK_FALSE(IsInitial(u8"border-image-repeat: stretch round", PropertyId::BorderImageRepeat));
      CHECK(IsInitial(u8"border-image-slice: 100%", PropertyId::BorderImageSlice));
      CHECK_FALSE(IsInitial(u8"border-image-slice: 100% fill", PropertyId::BorderImageSlice));
      CHECK(IsInitial(u8"border-image-width: 1", PropertyId::BorderImageWidth));
      CHECK_FALSE(IsInitial(u8"border-image-width: 1px", PropertyId::BorderImageWidth));
      CHECK(IsInitial(u8"mask-border-slice: 0", PropertyId::MaskBorderSlice));
      CHECK_FALSE(IsInitial(u8"mask-border-slice: 0 fill", PropertyId::MaskBorderSlice));
      CHECK(IsInitial(u8"mask-border-width: auto", PropertyId::MaskBorderWidth));
      CHECK_FALSE(IsInitial(u8"mask-border-width: 1", PropertyId::MaskBorderWidth));
      CHECK(IsInitial(u8"offset-rotate: auto", PropertyId::OffsetRotate));
      CHECK_FALSE(IsInitial(u8"offset-rotate: 10deg", PropertyId::OffsetRotate));
    }
  }
}
