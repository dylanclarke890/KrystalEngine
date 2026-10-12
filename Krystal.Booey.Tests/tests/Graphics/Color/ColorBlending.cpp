#include "Krystal.Booey/Graphics/Color/ColorBlending.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"

namespace krys::boo::tests
{
  using Catch::Approx;

  TEST_CASE("ColorBlending: BlendSourceOver", "[Graphics][Color][Blending]")
  {
    // Opaque sources and invisible backdrops return the source.
    CHECK(BlendSourceOver(Color::white, Color::red) == Color {Color::red});
    CHECK(BlendSourceOver(Color {}, SRGBA<uint8> {255, 0, 0, 128}) == Color {SRGBA<uint8> {255, 0, 0, 128}});

    // Invisible sources return the backdrop.
    CHECK(BlendSourceOver(Color::white, SRGBA<uint8> {255, 0, 0, 0}) == Color {Color::white});

    CHECK(SerialisationForCSS(BlendSourceOver(Color::white, SRGBA<uint8> {255, 0, 0, 128}))
          == u8"rgb(255, 127, 127)");
    CHECK(SerialisationForCSS(BlendSourceOver(SRGBA<uint8> {0, 0, 255, 128}, SRGBA<uint8> {255, 0, 0, 128}))
          == u8"rgba(170, 0, 84, 0.75)");
  }

  TEST_CASE("ColorBlending: BlendWithWhite", "[Graphics][Color][Blending]")
  {
    CHECK(SerialisationForCSS(BlendWithWhite(Color::red)) == u8"rgba(255, 0, 0, 0.8)");
    // White needs the least transparency (60%) to stay non-negative.
    CHECK(BlendWithWhite(Color::white).AlphaByte() == 153);

    // Non-opaque colors are left alone.
    CHECK(BlendWithWhite(SRGBA<uint8> {1, 2, 3, 4}) == Color {SRGBA<uint8> {1, 2, 3, 4}});

    // The semantic flag is kept.
    CHECK(BlendWithWhite(Color {Color::red, ColorFlags::Semantic}).IsSemantic());
  }

  TEST_CASE("ColorBlending: Blend", "[Graphics][Color][Blending]")
  {
    // Legacy colors blend in sRGB and come back as bytes.
    CHECK(Blend(Color::red, Color::blue, BlendingContext {0.5}) == Color {SRGBA<uint8> {128, 0, 128, 255}});
    CHECK(Blend(Color::red, Color::blue, BlendingContext {0}) == Color {Color::red});
    CHECK(Blend(Color::red, Color::blue, BlendingContext {1}) == Color {Color::blue});

    // Premultiplied: a transparent endpoint doesn't tint the result.
    CHECK(Blend(Color::red, SRGBA<uint8> {0, 0, 255, 0}, BlendingContext {0.5})
          == Color {SRGBA<uint8> {255, 0, 0, 128}});

    // Other colors blend in OKLab.
    auto modern = Blend(Lab<float> {0, 0, 0, 1}, Lab<float> {100, 0, 0, 1}, BlendingContext {0.5});
    CHECK(modern.Space() == ColorSpace::OKLab);
    CHECK(modern.ToColorTypeLossy<OKLab<float>>().Resolved().Lightness == Approx(0.5f).margin(0.001));

    // Additive composition sums the colors.
    BlendingContext add {1, false, CompositeOperation::Add};
    CHECK(Blend(SRGBA<uint8> {100, 0, 0, 255}, SRGBA<uint8> {100, 0, 0, 255}, add)
            .ToColorTypeLossy<SRGBA<uint8>>()
            .Resolved()
            .Red
          == 200);

    // The end of an animation to an invalid color stays invalid.
    CHECK_FALSE(Blend(Color::red, Color {}, BlendingContext {1}).IsValid());
  }

  TEST_CASE("ColorBlending: BlendWithoutPremultiply", "[Graphics][Color][Blending]")
  {
    CHECK(BlendWithoutPremultiply(Color::red, Color::blue, BlendingContext {0.5})
          == Color {SRGBA<uint8> {128, 0, 128, 255}});
    CHECK(BlendWithoutPremultiply(Color::red, SRGBA<uint8> {0, 0, 255, 0}, BlendingContext {0.5})
          == Color {SRGBA<uint8> {128, 0, 128, 128}});
    CHECK_FALSE(BlendWithoutPremultiply(Color::red, Color {}, BlendingContext {1}).IsValid());
  }

  TEST_CASE("AnimationUtilities: BlendingContext", "[Graphics][Animation]")
  {
    BlendingContext context;
    CHECK(context.Progress == 0);
    CHECK(context.IsReplace());

    BlendingContext accumulate {0.5, false, CompositeOperation::Replace,
                                IterationCompositeOperation::Accumulate};
    CHECK_FALSE(accumulate.IsReplace());

    BlendingContext discrete {0.4, true, CompositeOperation::Add};
    discrete.NormalizeProgress();
    CHECK(discrete.Progress == 0);
    CHECK(discrete.CompositeOperation == CompositeOperation::Replace);

    BlendingContext discreteLate {0.5, true};
    discreteLate.NormalizeProgress();
    CHECK(discreteLate.Progress == 1);

    BlendingContext continuous {0.4, false, CompositeOperation::Add};
    continuous.NormalizeProgress();
    CHECK(continuous.Progress == 0.4);
    CHECK(continuous.CompositeOperation == CompositeOperation::Add);
  }

  TEST_CASE("AnimationUtilities: scalar blends", "[Graphics][Animation]")
  {
    BlendingContext half {0.5};
    CHECK(Blend(10.0, 20.0, half) == 15.0);
    CHECK(Blend(10.0f, 20.0f, half) == 15.0f);
    CHECK(Blend(int32 {10}, int32 {15}, half) == 13);
    CHECK(Blend(int32 {-10}, int32 {-15}, half) == -12);
    CHECK(Blend(uint32 {10}, uint32 {15}, half) == 13u);

    // Add and accumulate add the from value again.
    BlendingContext add {0.5, false, CompositeOperation::Add};
    CHECK(Blend(10.0, 20.0, add) == 25.0);
    CHECK(Blend(int32 {10}, int32 {20}, add) == 25);

    // Iterations accumulate the to value.
    BlendingContext iteration {0.5, false, CompositeOperation::Replace,
                               IterationCompositeOperation::Accumulate, 2};
    CHECK(Blend(10.0, 20.0, iteration) == 55.0);
    CHECK(Blend(10.0f, 20.0f, iteration) == 55.0f);
    CHECK(Blend(int32 {10}, int32 {20}, iteration) == 55);
    CHECK(Blend(uint32 {10}, uint32 {20}, iteration) == 55u);
  }
}
