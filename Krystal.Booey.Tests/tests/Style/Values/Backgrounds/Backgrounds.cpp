#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/BackgroundLayers.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/BorderImage.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/FillLayers.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/LineWidth.hpp"
#include "Krystal.Booey/Style/Values/CSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::style::tests
{
  using css::PropertyId;
  using namespace css::Literals;

  namespace
  {
    /// @brief Converts the first layer of a parsed layered property.
    template <typename T>
    KRYS_NODISCARD T ConvertFirstLayer(BuilderFixture &fixture, PropertyId property,
                                       utf8_stringview value) noexcept
    {
      auto parsed = fixture.Parse(property, value);
      fixture.CurrentProperty.Id = property;
      const auto *list = DynamicDowncast<css::ValueList>(*parsed);
      const auto &layer = list ? *list->Item(0) : *parsed;
      return ConvertCSSValueToStyle<T>(*fixture.State, layer);
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style background layer values", "[Style][Values][Backgrounds]")
  {
    SECTION("background-size")
    {
      auto cover = ConvertFirstLayer<BackgroundSize>(*this, PropertyId::BackgroundSize, u8"cover");
      CHECK(cover.IsCover());
      CHECK(Serialised(cover) == "cover");

      auto size = ConvertFirstLayer<BackgroundSize>(*this, PropertyId::BackgroundSize, u8"10px auto");
      CHECK(size.IsLengthSize());
      // The second value is kept even when it's `auto`.
      CHECK(Serialised(size) == "10px auto");
      CHECK(Created(size) == "10px auto");

      auto both = ConvertFirstLayer<BackgroundSize>(*this, PropertyId::BackgroundSize, u8"50% 0");
      CHECK(Serialised(both) == "50% 0px");
      CHECK(both.IsEmpty());

      // `auto` only blends with `auto`.
      CHECK_FALSE(Blending<BackgroundSize> {}.CanBlend(size, both));
      CHECK(Blending<BackgroundSize> {}.CanBlend(
        ConvertFirstLayer<BackgroundSize>(*this, PropertyId::BackgroundSize, u8"10px 20px"), both));
      CHECK_FALSE(Blending<BackgroundSize> {}.CanBlend(size, cover));
    }

    SECTION("background-repeat")
    {
      auto repeatX = ConvertFirstLayer<RepeatStyle>(*this, PropertyId::BackgroundRepeat, u8"repeat-x");
      CHECK(repeatX.X() == FillRepeat::Repeat);
      CHECK(repeatX.Y() == FillRepeat::NoRepeat);
      CHECK(Serialised(repeatX) == "repeat-x");

      CHECK(Serialised(ConvertFirstLayer<RepeatStyle>(*this, PropertyId::BackgroundRepeat, u8"space round"))
            == "space round");
      CHECK(Serialised(ConvertFirstLayer<RepeatStyle>(*this, PropertyId::BackgroundRepeat, u8"round round"))
            == "round");
    }

    SECTION("layers")
    {
      BackgroundLayer layer;
      CHECK(layer.IsInitial());
      CHECK_FALSE(layer.HasImage());
      CHECK(layer.GetClip() == FillBox::BorderBox);
      CHECK(layer.GetOrigin() == FillBox::PaddingBox);
      CHECK(layer.GetAttachment() == FillAttachment::ScrollBackground);
      CHECK(layer.HasRepeatXY());
      CHECK(layer.IsSizeUnset());

      layer.SetClip(FillBox::ContentBox);
      CHECK(layer.IsClipSet());
      CHECK_FALSE((layer == BackgroundLayer {}));

      auto image = ConvertFirstLayer<ImageOrNone>(*this, PropertyId::BackgroundImage,
                                                  u8"url(https://example.com/a.png), none");
      BackgroundLayers layers {BackgroundLayer {krys::move(image)}, BackgroundLayer {css::keywords::None {}}};
      layers[0].SetClip(FillBox::PaddingBox);
      layers[0].SetAttachment(FillAttachment::FixedBackground);
      layers[1].SetClip(FillBox::BorderBox);
      layers.PrepareForUse();

      CHECK(HasImageInAnyLayer(layers));
      CHECK(HasImageWithAttachment(layers, FillAttachment::FixedBackground));
      CHECK_FALSE(HasAnyBackgroundClipText(layers));

      // Each layer's clip max is the widest clip from it to the bottom layer.
      ComputeClipMax(layers);
      CHECK_FALSE(layers[0].ClipOccludesNextLayers());
      CHECK(layers[1].ClipOccludesNextLayers());

      // The shorthand position is the two longhands.
      BackgroundLayer positioned;
      positioned.SetPosition(Position {PositionX {10_css_px}, PositionY {20_css_percentage}});
      CHECK(positioned.IsPositionSet());
      CHECK(positioned.GetPositionX() == PositionX {10_css_px});
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style border image", "[Style][Values][Backgrounds]")
  {
    BorderImage initial;
    CHECK(initial.Source.IsNone());
    CHECK(Serialised(initial.Slice) == "100%");
    CHECK(Serialised(initial.Width) == "1");
    CHECK(Serialised(initial.Outset) == "0");
    CHECK(Serialised(initial.Repeat) == "stretch");
    CHECK(Serialised(initial) == "none");

    auto source =
      Convert<BorderImageSource>(PropertyId::BorderImageSource, u8"url(https://example.com/a.png)");
    CHECK(source.IsImage());

    auto slice = Convert<BorderImageSlice>(PropertyId::BorderImageSlice, u8"10 20% fill");
    CHECK(slice.Fill);
    CHECK(Serialised(slice) == "10 20% fill");
    CHECK(Created(slice) == "10 20% fill");

    auto width = Convert<BorderImageWidth>(PropertyId::BorderImageWidth, u8"1px auto 2");
    // The left edge repeats the right one, so it's omitted.
    CHECK(Serialised(width) == "1px auto 2");

    auto outset = Convert<BorderImageOutset>(PropertyId::BorderImageOutset, u8"2px 3");
    CHECK(Serialised(outset) == "2px 3");
    CHECK_FALSE(outset.IsZero());
    CHECK(Convert<BorderImageOutset>(PropertyId::BorderImageOutset, u8"0").IsZero());

    auto repeat = Convert<BorderImageRepeat>(PropertyId::BorderImageRepeat, u8"round space");
    CHECK(Serialised(repeat) == "round space");

    BorderImage image {krys::move(source), krys::move(slice), krys::move(width), krys::move(outset),
                       krys::move(repeat)};
    CHECK(Serialised(image)
          == "url(\"https://example.com/a.png\") 10 20% fill / 1px auto 2 / 2px 3 round space");

    SECTION("blending")
    {
      auto from = Convert<BorderImageOutset>(PropertyId::BorderImageOutset, u8"0px 2");
      auto to = Convert<BorderImageOutset>(PropertyId::BorderImageOutset, u8"10px 4");
      CHECK(Blending<BorderImageOutset> {}.CanBlend(from, to));
      CHECK(Serialised(Blending<BorderImageOutset> {}.Blend(from, to, BlendingContext {0.5})) == "5px 3");
      // Lengths and numbers don't blend.
      CHECK_FALSE(Blending<BorderImageOutset> {}.CanBlend(
        from, Convert<BorderImageOutset>(PropertyId::BorderImageOutset, u8"1 2")));
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "Style LineWidth", "[Style][Values][Backgrounds]")
  {
    CHECK(Convert<LineWidth>(PropertyId::BorderTopWidth, u8"thin").Value == 1_css_px);
    CHECK(Convert<LineWidth>(PropertyId::BorderTopWidth, u8"medium").Value == 3_css_px);
    CHECK(Convert<LineWidth>(PropertyId::BorderTopWidth, u8"thick").Value == 5_css_px);
    CHECK(Convert<LineWidth>(PropertyId::BorderTopWidth, u8"2.7px").Value == 2_css_px);
    // Widths below a device pixel round up to one, so borders don't vanish.
    CHECK(Convert<LineWidth>(PropertyId::BorderTopWidth, u8"0.2px").Value == 1_css_px);
    CHECK(Convert<LineWidth>(PropertyId::BorderTopWidth, u8"0").IsZero());
    CHECK(Serialised(Convert<LineWidth>(PropertyId::BorderTopWidth, u8"thick")) == "5px");
  }
}
