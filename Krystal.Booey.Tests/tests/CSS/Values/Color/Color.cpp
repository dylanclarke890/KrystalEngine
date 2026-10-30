#include "Krystal.Booey/CSS/Values/Color/Color.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Calc/Value.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Values/Color/AbsoluteColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/ColorLayers.hpp"
#include "Krystal.Booey/CSS/Values/Color/ColorMix.hpp"
#include "Krystal.Booey/CSS/Values/Color/ContrastColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/DynamicRangeLimit.hpp"
#include "Krystal.Booey/CSS/Values/Color/DynamicRangeLimitMix.hpp"
#include "Krystal.Booey/CSS/Values/Color/LightDarkColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/PlatformColorResolutionState.hpp"
#include "Krystal.Booey/CSS/Values/Color/RelativeColor.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/UnitType.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Core/Types/Markable.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    template <typename T>
    CSSOMString SerialiseForTest(const T &value)
    {
      return Serialise(DefaultSerialisationContext(), value);
    }

    boo::Color Resolve(const Color &color, PlatformColorResolutionState state = {})
    {
      return CreateColor(color, state);
    }

    Color Keyword(ValueId id)
    {
      return Color {KeywordColor {id}};
    }

    Color Hex(uint8 r, uint8 g, uint8 b, uint8 a = 255)
    {
      return Color {HexColor {SRGBA<uint8> {r, g, b, a}}};
    }

    // A number calc() that needs conversion data (its leaf is an em length).
    Ref<calc::Value> CreateEmCalc()
    {
      return calc::Value::Create(Category::Number, All,
                                 calc::Tree {.Root = calc::CreateNumeric(1.0, UnitType::em),
                                             .Type = calc::Type {},
                                             .Stage = calc::Stage::Specified,
                                             .RequiresConversionData = true});
    }

    using LegacyRGBNumber = AbsoluteColor<RGBFunctionLegacy<Number<>>>;
    using ModernRGB = AbsoluteColor<RGBFunctionModernAbsolute>;
    using RelativeRGB = RelativeColor<RGBFunctionModernRelative>;

    struct TestDelegate : PlatformColorResolutionDelegate
    {
      mutable int Calls {0};

      boo::Color CurrentColor() const noexcept override
      {
        ++Calls;
        return boo::Color::green;
      }
    };
  }

  TEST_CASE("Keyword colours", "[CSS][Color][KeywordColor]")
  {
    SECTION("Classification")
    {
      CHECK(IsAbsoluteColorKeyword(ValueId::Red));
      CHECK(IsAbsoluteColorKeyword(ValueId::Aliceblue));
      CHECK(IsAbsoluteColorKeyword(ValueId::Transparent));
      CHECK_FALSE(IsAbsoluteColorKeyword(ValueId::Currentcolor));
      CHECK_FALSE(IsAbsoluteColorKeyword(ValueId::Canvas));

      CHECK(IsCurrentColorKeyword(ValueId::Currentcolor));

      CHECK(IsSystemColorKeyword(ValueId::Canvas));
      CHECK(IsSystemColorKeyword(ValueId::Text));
      CHECK(IsSystemColorKeyword(ValueId::Menu));
      CHECK(IsSystemColorKeyword(ValueId::Windowtext));
      CHECK_FALSE(IsSystemColorKeyword(ValueId::Red));

      CHECK(IsDeprecatedSystemColorKeyword(ValueId::Activeborder));
      CHECK(IsDeprecatedSystemColorKeyword(ValueId::Menu));
      CHECK_FALSE(IsDeprecatedSystemColorKeyword(ValueId::Canvas));

      CHECK(IsColorKeyword(ValueId::Red));
      CHECK(IsColorKeyword(ValueId::Currentcolor));
      CHECK(IsColorKeyword(ValueId::Canvas));
      CHECK_FALSE(IsColorKeyword(ValueId::Auto));
    }

    SECTION("Allowed colour types")
    {
      CHECK(IsColorKeyword(ValueId::Red, ColorType::Absolute));
      CHECK_FALSE(IsColorKeyword(ValueId::Red, ColorType::Current | ColorType::System));
      CHECK(IsColorKeyword(ValueId::Currentcolor, ColorType::Current));
      CHECK_FALSE(IsColorKeyword(ValueId::Currentcolor, ColorType::Absolute));
      CHECK(IsColorKeyword(ValueId::Canvas, ColorType::System));
      CHECK_FALSE(IsColorKeyword(ValueId::Canvas, ColorType::Absolute | ColorType::Current));
      CHECK_FALSE(IsColorKeyword(ValueId::Red, ColorType::None));
    }

    SECTION("Colours")
    {
      CHECK(ColorFromAbsoluteKeyword(ValueId::Red) == boo::Color::red);
      CHECK(ColorFromAbsoluteKeyword(ValueId::Aliceblue) == boo::Color {SRGBA<uint8> {240, 248, 255}});
      CHECK(ColorFromAbsoluteKeyword(ValueId::Transparent) == boo::Color::transparentBlack);
      // CSS grey is #808080, which is `Color::darkGray` (`Color::gray` is #a0a0a0).
      CHECK(ColorFromKeyword(ValueId::Grey, style::ColorOptions::None) == boo::Color::darkGray);
      // System colours are the theme's, which are semantic.
      CHECK(EqualIgnoringSemanticColor(ColorFromKeyword(ValueId::Canvas, style::ColorOptions::None),
                                       boo::Color::white));
    }

    SECTION("Resolution through the state")
    {
      CHECK(Resolve(Keyword(ValueId::Red)) == boo::Color::red);
      CHECK(Resolve(Keyword(ValueId::Buttonface)) == boo::Color::lightGray);

      // No delegate and no stored colour: the invalid colour.
      CHECK_FALSE(Resolve(Keyword(ValueId::Currentcolor)).IsValid());

      PlatformColorResolutionState state;
      state.ResolvedCurrentColor = boo::Color::blue;
      state.ResolvedWebkitLink = boo::Color::red;
      state.ResolvedWebkitLinkVisited = boo::Color::green;
      CHECK(Resolve(Keyword(ValueId::Currentcolor), state) == boo::Color::blue);
      CHECK(Resolve(Keyword(ValueId::WebkitLink), state) == boo::Color::red);
      state.ForVisitedLink = style::ForVisitedLink::Yes;
      CHECK(Resolve(Keyword(ValueId::WebkitLink), state) == boo::Color::green);

      state.KeywordOptions = style::ColorOptions::UseDarkAppearance;
      CHECK(Resolve(Keyword(ValueId::Linktext), state) == boo::Color {SRGBA<uint8> {158, 158, 255}});
    }

    SECTION("The delegate is asked once, then the result is cached")
    {
      TestDelegate delegate;
      PlatformColorResolutionState state {.Delegate = &delegate};
      auto color = Keyword(ValueId::Currentcolor);
      CHECK(CreateColor(color, state) == boo::Color::green);
      CHECK(CreateColor(color, state) == boo::Color::green);
      CHECK(delegate.Calls == 1);
      CHECK(state.ResolvedCurrentColor == boo::Color::green);
      CHECK_FALSE(state.ResolvedWebkitLink);

      // The base delegate gives the invalid colour.
      PlatformColorResolutionDelegate base;
      PlatformColorResolutionState baseState {.Delegate = &base};
      CHECK_FALSE(CreateColor(Keyword(ValueId::WebkitFocusRingColor), baseState).IsValid());
    }

    SECTION("Serialisation and queries")
    {
      CHECK(SerialiseForTest(Keyword(ValueId::Aliceblue)) == u8"aliceblue");
      CHECK(SerialiseForTest(Keyword(ValueId::Currentcolor)) == u8"currentcolor");
      CHECK(ContainsCurrentColor(Keyword(ValueId::Currentcolor)));
      CHECK_FALSE(ContainsCurrentColor(Keyword(ValueId::Red)));
      CHECK(ContainsColorSchemeDependentColor(Keyword(ValueId::Canvas)));
      CHECK_FALSE(ContainsColorSchemeDependentColor(Keyword(ValueId::Red)));
    }
  }

  TEST_CASE("Hex and resolved colours", "[CSS][Color]")
  {
    CHECK(Resolve(Hex(1, 2, 3)) == boo::Color {SRGBA<uint8> {1, 2, 3}});
    CHECK(SerialiseForTest(Hex(255, 0, 0)) == u8"rgb(255, 0, 0)");
    CHECK(SerialiseForTest(Hex(255, 0, 0, 0)) == u8"rgba(255, 0, 0, 0)");

    Color resolved {ResolvedColor {boo::Color::blue}};
    CHECK(Resolve(resolved) == boo::Color::blue);
    CHECK(SerialiseForTest(resolved) == boo::SerialisationForCSS(boo::Color::blue));

    CHECK_FALSE(ContainsCurrentColor(Hex(1, 2, 3)));
    CHECK_FALSE(ContainsColorSchemeDependentColor(resolved));
  }

  TEST_CASE("Color", "[CSS][Color]")
  {
    SECTION("Accessors")
    {
      auto keyword = Keyword(ValueId::Red);
      CHECK(keyword.IsKeyword());
      CHECK_FALSE(keyword.IsHex());
      CHECK_FALSE(keyword.IsResolved());
      REQUIRE(keyword.Keyword());
      CHECK(keyword.Keyword()->Value == ValueId::Red);
      CHECK_FALSE(keyword.Hex());

      auto hex = Hex(1, 2, 3);
      REQUIRE(hex.Hex());
      CHECK(hex.Hex()->Value == SRGBA<uint8> {1, 2, 3});

      Color resolved {ResolvedColor {boo::Color::red}};
      REQUIRE(resolved.Resolved());
      CHECK(resolved.Resolved()->Value == boo::Color::red);
    }

    SECTION("Absolute colour")
    {
      CHECK(Keyword(ValueId::Red).GetAbsoluteColor() == boo::Color::red);
      CHECK_FALSE(Keyword(ValueId::Currentcolor).GetAbsoluteColor().IsValid());
      CHECK_FALSE(Keyword(ValueId::Canvas).GetAbsoluteColor().IsValid());
      CHECK(Hex(1, 2, 3).GetAbsoluteColor() == boo::Color {SRGBA<uint8> {1, 2, 3}});
      CHECK_FALSE(Color {ContrastColor {Keyword(ValueId::Red)}}.GetAbsoluteColor().IsValid());
    }

    SECTION("Copies, moves and equality, including indirect alternatives")
    {
      Color mix {ColorMix {.ColourInterpolationMethod = DefaultInterpolationMethodForColorMix,
                           .MixComponents1 = {Keyword(ValueId::Red), null},
                           .MixComponents2 = {Hex(0, 0, 255), null}}};
      Color copy = mix;
      CHECK(copy == mix);
      CHECK(SerialiseForTest(copy) == SerialiseForTest(mix));

      Color other {ColorMix {.ColourInterpolationMethod = DefaultInterpolationMethodForColorMix,
                             .MixComponents1 = {Keyword(ValueId::Red), null},
                             .MixComponents2 = {Hex(0, 0, 254), null}}};
      CHECK_FALSE(copy == other);

      copy = other;
      CHECK(copy == other);

      Color moved = krys::move(copy);
      CHECK(moved == other);

      CHECK_FALSE(Keyword(ValueId::Red) == Hex(255, 0, 0));
      CHECK(Keyword(ValueId::Red) == Keyword(ValueId::Red));
    }

    SECTION("Markable")
    {
      Markable<Color> empty;
      CHECK_FALSE(empty);
      Markable<Color> red {Keyword(ValueId::Red)};
      REQUIRE(red);
      CHECK(*red == Keyword(ValueId::Red));
    }

    SECTION("SwitchOn sees through the indirection")
    {
      Color contrast {ContrastColor {Hex(0, 0, 0)}};
      auto isContrast =
        contrast.SwitchOn([](const ContrastColor &) { return true; }, [](const auto &) { return false; });
      CHECK(isContrast);
    }
  }

  TEST_CASE("Absolute colours", "[CSS][Color][AbsoluteColor]")
  {
    SECTION("Legacy rgb() is stored as 8-bit sRGB, clamped")
    {
      Color color {LegacyRGBNumber {{Number<> {255.0}, Number<> {300.0}, Number<> {-5.0}, null}}};
      CHECK(Resolve(color) == boo::Color {SRGBA<uint8> {255, 255, 0}});
      CHECK(SerialiseForTest(color) == u8"rgb(255 300 -5)");
    }

    SECTION("Alpha")
    {
      Color color {ModernRGB {{Number<> {0.0}, Number<> {0.0}, Number<> {255.0},
                               Maybe<ModernRGB::Descriptor::R::Result> {Percentage<> {50.0}}}}};
      CHECK(Resolve(color).AlphaByte() == 128);
      CHECK(SerialiseForTest(color) == u8"rgb(0 0 255 / 50%)");
    }

    SECTION("Modern rgb() with none keeps the float colour")
    {
      Color color {ModernRGB {{keywords::None {}, Number<> {0.0}, Number<> {0.0}, null}}};
      auto resolved = Resolve(color);
      REQUIRE(resolved.IsValid());
      CHECK_FALSE(resolved.TryGetAsSRGBABytes());
      CHECK(resolved.AnyComponentIsNone());
      CHECK(SerialiseForTest(color) == u8"rgb(none 0 0)");
    }

    SECTION("Nested colours keep full precision")
    {
      Color color {ModernRGB {{Number<> {255.0}, Number<> {0.0}, Number<> {0.0}, null}}};
      PlatformColorResolutionState state;
      CHECK(Resolve(color, state).TryGetAsSRGBABytes());
      state.NestingLevel = 1;
      CHECK_FALSE(Resolve(color, state).TryGetAsSRGBABytes());
      CHECK(state.NestingLevel == 1);
    }

    SECTION("lab() and color()")
    {
      Color lab {AbsoluteColor<LabFunction> {{Percentage<> {50.0}, Number<> {20.0}, Number<> {-30.0}, null}}};
      auto resolved = Resolve(lab);
      REQUIRE(resolved.IsValid());
      CHECK(resolved.Space() == ColourSpace::Lab);
      CHECK(SerialiseForTest(lab) == u8"lab(50% 20 -30)");

      Color p3 {AbsoluteColor<ColorRGBFunction<ExtendedDisplayP3<float>>> {
        {Number<> {1.0}, Number<> {0.5}, Percentage<> {0.0}, null}}};
      CHECK(Resolve(p3).Space() == ColourSpace::ExtendedDisplayP3);
      CHECK(SerialiseForTest(p3) == u8"color(display-p3 1 0.5 0%)");
    }

    SECTION("hsl() and hwb() normalise their hue")
    {
      Color hsl {
        AbsoluteColor<HSLFunctionModern> {{Angle<> {480_css_deg}, Number<> {100.0}, Number<> {50.0}, null}}};
      // 480deg is 120deg: green.
      CHECK(Resolve(hsl) == boo::Color {SRGBA<uint8> {0, 255, 0}});
    }

    SECTION("Components that need conversion data")
    {
      ModernRGB absolute {{Number<> {CreateEmCalc()}, Number<> {0.0}, Number<> {0.0}, null}};
      CHECK(ComponentsRequireConversionData<RGBFunctionModernAbsolute>(absolute.Components));
      CHECK(ComponentsContainsUnevaluatedCalc<RGBFunctionModernAbsolute>(absolute.Components));

      // Without conversion data, the colour can't be resolved.
      CHECK_FALSE(Resolve(Color {krys::move(absolute)}).IsValid());
    }
  }

  TEST_CASE("Relative colours", "[CSS][Color][RelativeColor]")
  {
    SECTION("Channel keywords take the origin's components")
    {
      Color color {
        RelativeRGB {.Origin = Keyword(ValueId::Red),
                     .Components = {Symbol {SymbolRaw {ValueId::B}}, Symbol {SymbolRaw {ValueId::G}},
                                    Symbol {SymbolRaw {ValueId::R}}, null}}};
      // Relative colours keep the extended (float) colour type.
      CHECK(Resolve(color).Space() == ColourSpace::ExtendedSRGB);
      CHECK(Resolve(color).ToColorTypeLossy<SRGBA<uint8>>() == SRGBA<uint8> {0, 0, 255});
      CHECK(SerialiseForTest(color) == u8"rgb(from red b g r)");
    }

    SECTION("Alpha defaults to the origin's")
    {
      Color color {
        RelativeRGB {.Origin = Hex(255, 0, 0, 0),
                     .Components = {Symbol {SymbolRaw {ValueId::R}}, Number<> {0.0}, Number<> {0.0}, null}}};
      CHECK(Resolve(color).AlphaByte() == 0);
    }

    SECTION("An invalid origin gives the invalid colour")
    {
      Color color {
        RelativeRGB {.Origin = Keyword(ValueId::Currentcolor),
                     .Components = {Symbol {SymbolRaw {ValueId::R}}, Symbol {SymbolRaw {ValueId::G}},
                                    Symbol {SymbolRaw {ValueId::B}}, null}}};
      CHECK_FALSE(Resolve(color).IsValid());

      PlatformColorResolutionState state;
      state.ResolvedCurrentColor = boo::Color::blue;
      CHECK(Resolve(color, state).ToColorTypeLossy<SRGBA<uint8>>() == SRGBA<uint8> {0, 0, 255});
    }

    SECTION("color() serialises its colour space")
    {
      Color color {RelativeColor<ColorRGBFunction<ExtendedSRGBA<float>>> {
        .Origin = Keyword(ValueId::Red),
        .Components = {Symbol {SymbolRaw {ValueId::R}}, Symbol {SymbolRaw {ValueId::G}}, Number<> {1.0},
                       null}}};
      CHECK(SerialiseForTest(color) == u8"color(from red srgb r g 1)");
      CHECK(Resolve(color).ToColorTypeLossy<SRGBA<uint8>>() == SRGBA<uint8> {255, 0, 255});
    }

    SECTION("Current colour checks the origin")
    {
      Color current {
        RelativeRGB {.Origin = Keyword(ValueId::Currentcolor),
                     .Components = {Symbol {SymbolRaw {ValueId::R}}, Symbol {SymbolRaw {ValueId::G}},
                                    Symbol {SymbolRaw {ValueId::B}}, null}}};
      CHECK(ContainsCurrentColor(current));
      CHECK_FALSE(ContainsColorSchemeDependentColor(current));

      Color system {
        RelativeRGB {.Origin = Keyword(ValueId::Canvas),
                     .Components = {Symbol {SymbolRaw {ValueId::R}}, Symbol {SymbolRaw {ValueId::G}},
                                    Symbol {SymbolRaw {ValueId::B}}, null}}};
      CHECK_FALSE(ContainsCurrentColor(system));
      CHECK(ContainsColorSchemeDependentColor(system));
    }
  }

  TEST_CASE("color-mix()", "[CSS][Color][ColorMix]")
  {
    using MixPercentage = ColorMix::Component::PercentageType;

    auto mix = [](Maybe<MixPercentage> p1, Maybe<MixPercentage> p2,
                  ColourInterpolationMethod method = DefaultInterpolationMethodForColorMix)
    {
      return Color {ColorMix {.ColourInterpolationMethod = method,
                              .MixComponents1 = {Keyword(ValueId::Red), p1},
                              .MixComponents2 = {Keyword(ValueId::Blue), p2}}};
    };

    SECTION("Serialisation omits the defaults")
    {
      CHECK(SerialiseForTest(mix(null, null)) == u8"color-mix(red, blue)");
      CHECK(SerialiseForTest(mix(MixPercentage {50_css_percentage}, MixPercentage {50_css_percentage}))
            == u8"color-mix(red, blue)");
      CHECK(SerialiseForTest(mix(MixPercentage {30_css_percentage}, null)) == u8"color-mix(red 30%, blue)");
      CHECK(SerialiseForTest(mix(null, MixPercentage {30_css_percentage})) == u8"color-mix(red 70%, blue)");
      CHECK(SerialiseForTest(mix(MixPercentage {30_css_percentage}, MixPercentage {70_css_percentage}))
            == u8"color-mix(red 30%, blue)");
      CHECK(SerialiseForTest(mix(MixPercentage {30_css_percentage}, MixPercentage {20_css_percentage}))
            == u8"color-mix(red 30%, blue 20%)");
      CHECK(SerialiseForTest(mix(null, null,
                                 ColourInterpolationMethod {ColourInterpolationMethod::SRGB {},
                                                            AlphaPremultiplication::Premultiplied}))
            == u8"color-mix(in srgb, red, blue)");
    }

    SECTION("Resolution")
    {
      auto srgb =
        ColourInterpolationMethod {ColourInterpolationMethod::SRGB {}, AlphaPremultiplication::Premultiplied};

      auto even = Resolve(mix(null, null, srgb)).ToColorTypeLossy<SRGBA<uint8>>();
      CHECK(even == SRGBA<uint8> {128, 0, 128});

      auto quarter =
        Resolve(mix(MixPercentage {25_css_percentage}, null, srgb)).ToColorTypeLossy<SRGBA<uint8>>();
      CHECK(quarter == SRGBA<uint8> {64, 0, 191});

      // Percentages summing to less than 100% become an alpha multiplier.
      auto faded = Resolve(mix(MixPercentage {25_css_percentage}, MixPercentage {25_css_percentage}, srgb));
      CHECK(faded.AlphaByte() == 128);

      // Percentages summing to zero make the function invalid.
      CHECK_FALSE(Resolve(mix(MixPercentage {0_css_percentage}, MixPercentage {0_css_percentage})).IsValid());

      // An invalid colour makes the mix invalid.
      Color withCurrent {ColorMix {.ColourInterpolationMethod = DefaultInterpolationMethodForColorMix,
                                   .MixComponents1 = {Keyword(ValueId::Currentcolor), null},
                                   .MixComponents2 = {Keyword(ValueId::Blue), null}}};
      CHECK_FALSE(Resolve(withCurrent).IsValid());
      CHECK(ContainsCurrentColor(withCurrent));
    }
  }

  TEST_CASE("contrast-color()", "[CSS][Color][ContrastColor]")
  {
    CHECK(Resolve(Color {ContrastColor {Hex(0, 0, 0)}}) == boo::Color::white);
    CHECK(Resolve(Color {ContrastColor {Hex(255, 255, 255)}}) == boo::Color::black);
    CHECK(Resolve(Color {ContrastColor {Keyword(ValueId::Yellow)}}) == boo::Color::black);
    CHECK(Resolve(Color {ContrastColor {Keyword(ValueId::Navy)}}) == boo::Color::white);
    CHECK(SerialiseForTest(Color {ContrastColor {Keyword(ValueId::Red)}}) == u8"contrast-color(red)");
    CHECK(ContainsCurrentColor(Color {ContrastColor {Keyword(ValueId::Currentcolor)}}));
  }

  TEST_CASE("light-dark()", "[CSS][Color][LightDarkColor]")
  {
    Color color {LightDarkColor {Keyword(ValueId::White), Keyword(ValueId::Black)}};

    // No appearance: the invalid colour.
    CHECK_FALSE(Resolve(color).IsValid());

    CHECK(Resolve(color, {.Appearance = LightDarkColorAppearance::Light}) == boo::Color::white);
    CHECK(Resolve(color, {.Appearance = LightDarkColorAppearance::Dark}) == boo::Color::black);
    CHECK(SerialiseForTest(color) == u8"light-dark(white, black)");
    CHECK(ContainsColorSchemeDependentColor(color));
    CHECK_FALSE(ContainsCurrentColor(color));
    CHECK(
      ContainsCurrentColor(Color {LightDarkColor {Keyword(ValueId::White), Keyword(ValueId::Currentcolor)}}));
  }

  TEST_CASE("color-layers()", "[CSS][Color][ColorLayers]")
  {
    SECTION("An opaque layer hides the ones under it")
    {
      Color color {ColorLayers {.BlendMode = BlendMode::Normal, .Colors = {Hex(255, 0, 0), Hex(0, 0, 255)}}};
      CHECK(Resolve(color) == boo::Color::red);
      CHECK(SerialiseForTest(color) == u8"color-layers(rgb(255, 0, 0), rgb(0, 0, 255))");
    }

    SECTION("Translucent layers composite source-over")
    {
      Color color {
        ColorLayers {.BlendMode = BlendMode::Normal, .Colors = {Hex(255, 0, 0, 0), Hex(0, 0, 255)}}};
      CHECK(Resolve(color) == boo::Color::blue);
    }

    SECTION("The blend mode is serialised unless it's normal")
    {
      Color color {ColorLayers {.BlendMode = BlendMode::Multiply, .Colors = {Keyword(ValueId::Red)}}};
      CHECK(SerialiseForTest(color) == u8"color-layers(multiply, red)");
      CHECK_FALSE(ContainsCurrentColor(color));
      CHECK(ContainsColorSchemeDependentColor(
        Color {ColorLayers {.BlendMode = BlendMode::Normal, .Colors = {Keyword(ValueId::Canvas)}}}));
    }
  }

  TEST_CASE("Computed style dependencies and child visitation", "[CSS][Color]")
  {
    Color color {ModernRGB {{Number<> {CreateEmCalc()}, Number<> {0.0}, Number<> {0.0}, null}}};
    auto dependencies =
      CollectComputedStyleDependencies(Color {LightDarkColor {color, Keyword(ValueId::Red)}});
    CHECK(std::ranges::find(dependencies.Properties, PropertyId::FontSize) != dependencies.Properties.end());

    CHECK(CollectComputedStyleDependencies(Keyword(ValueId::Red)).IsComputationallyIndependent());

    auto calls = 0;
    auto status = VisitValueChildren(
      [&](Value &)
      {
        ++calls;
        return IterationStatus::Continue;
      },
      color);
    CHECK(status == IterationStatus::Continue);
    CHECK(calls == 0);
  }

  TEST_CASE("DynamicRangeLimit", "[CSS][Color][DynamicRangeLimit]")
  {
    DynamicRangeLimit standard {keywords::Standard {}};
    DynamicRangeLimit noLimit {keywords::NoLimit {}};
    CHECK(standard == DynamicRangeLimit {keywords::Standard {}});
    CHECK_FALSE(standard == noLimit);

    auto mix = []
    {
      DynamicRangeLimitMixParameters parameters;
      parameters.Value.emplace_back(DynamicRangeLimitMixComponent {
        DynamicRangeLimit {keywords::Standard {}}, DynamicRangeLimitMixPercentage {40_css_percentage}});
      parameters.Value.emplace_back(DynamicRangeLimitMixComponent {
        DynamicRangeLimit {keywords::NoLimit {}}, DynamicRangeLimitMixPercentage {60_css_percentage}});
      return DynamicRangeLimit {
        DynamicRangeLimitMixFunction {DynamicRangeLimitMixFunctionValue {krys::move(parameters)}}};
    };

    auto a = mix();
    auto b = mix();
    CHECK(a == b);
    CHECK_FALSE(a == standard);

    auto isMix = a.SwitchOn([](const DynamicRangeLimitMixFunction &) { return true; },
                            [](const auto &) { return false; });
    CHECK(isMix);

    DynamicRangeLimit moved = krys::move(a);
    CHECK(moved == b);
  }
}
