#include "Krystal.Web/Css/Values/Color/Color.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Types/Markable.hpp"
#include "Krystal.Web.Tests/Css/ColorTestUtils.hpp"
#include "Krystal.Web.Tests/Css/ValueTestUtils.hpp"
#include "Krystal.Web/Css/Calc/Value.hpp"
#include "Krystal.Web/Css/Values/Color/AbsoluteColor.hpp"
#include "Krystal.Web/Css/Values/Color/ColorLayers.hpp"
#include "Krystal.Web/Css/Values/Color/ColorLayersResolver.hpp"
#include "Krystal.Web/Css/Values/Color/ColorMix.hpp"
#include "Krystal.Web/Css/Values/Color/ContrastColor.hpp"
#include "Krystal.Web/Css/Values/Color/DynamicRangeLimit.hpp"
#include "Krystal.Web/Css/Values/Color/DynamicRangeLimitMix.hpp"
#include "Krystal.Web/Css/Values/Color/LightDarkColor.hpp"
#include "Krystal.Web/Css/Values/Color/PlatformColorResolutionState.hpp"
#include "Krystal.Web/Css/Values/Color/RelativeColor.hpp"
#include "Krystal.Web/Css/Values/SerialisationContext.hpp"
#include "Krystal.Web/Css/Values/ToLengthConversionData.hpp"
#include "Krystal.Web/Css/Values/Units.hpp"
#include "Krystal.Web/Graphics/Color/ColorSerialisation.hpp"

namespace krys::web::css::tests
{
  namespace
  {
    web::Color Resolve(const Color &color, PlatformColorResolutionState state = {})
    {
      return CreateColor(color, state);
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

      web::Color CurrentColor() const noexcept override
      {
        ++Calls;
        return web::Color::green;
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
      CHECK_FALSE(IsColorKeyword(ValueId::Red, {ColorType::Current, ColorType::System}));
      CHECK(IsColorKeyword(ValueId::Currentcolor, ColorType::Current));
      CHECK_FALSE(IsColorKeyword(ValueId::Currentcolor, ColorType::Absolute));
      CHECK(IsColorKeyword(ValueId::Canvas, ColorType::System));
      CHECK_FALSE(IsColorKeyword(ValueId::Canvas, {ColorType::Absolute, ColorType::Current}));
      CHECK_FALSE(IsColorKeyword(ValueId::Red, {}));
    }

    SECTION("Colours")
    {
      CHECK(ColorFromAbsoluteKeyword(ValueId::Red) == web::Color::red);
      CHECK(ColorFromAbsoluteKeyword(ValueId::Aliceblue) == web::Color {SRGBA<uint8> {240, 248, 255}});
      CHECK(ColorFromAbsoluteKeyword(ValueId::Transparent) == web::Color::transparentBlack);
      // CSS grey is #808080, which is `Color::darkGray` (`Color::gray` is #a0a0a0).
      CHECK(ColorFromKeyword(ValueId::Grey, {}) == web::Color::darkGray);
      // System colours are the theme's, which are semantic.
      CHECK(EqualIgnoringSemanticColor(ColorFromKeyword(ValueId::Canvas, {}), web::Color::white));
    }

    SECTION("Resolution through the state")
    {
      CHECK(Resolve(ColorKeyword(ValueId::Red)) == web::Color::red);
      CHECK(Resolve(ColorKeyword(ValueId::Buttonface)) == web::Color::lightGray);

      // No delegate and no stored colour: the invalid colour.
      CHECK_FALSE(Resolve(ColorKeyword(ValueId::Currentcolor)).IsValid());

      PlatformColorResolutionState state;
      state.ResolvedCurrentColor = web::Color::blue;
      state.ResolvedWebkitLink = web::Color::red;
      CHECK(Resolve(ColorKeyword(ValueId::Currentcolor), state) == web::Color::blue);
      CHECK(Resolve(ColorKeyword(ValueId::WebkitLink), state) == web::Color::red);

      state.KeywordOptions = style::ColorOption::UseDarkAppearance;
      CHECK(Resolve(ColorKeyword(ValueId::Linktext), state) == web::Color {SRGBA<uint8> {158, 158, 255}});
    }

    SECTION("The delegate is asked once, then the result is cached")
    {
      TestDelegate delegate;
      PlatformColorResolutionState state {.Delegate = &delegate};
      auto color = ColorKeyword(ValueId::Currentcolor);
      CHECK(CreateColor(color, state) == web::Color::green);
      CHECK(CreateColor(color, state) == web::Color::green);
      CHECK(delegate.Calls == 1);
      CHECK(state.ResolvedCurrentColor == web::Color::green);
      CHECK_FALSE(state.ResolvedWebkitLink);

      // The base delegate gives the invalid colour.
      PlatformColorResolutionDelegate base;
      PlatformColorResolutionState baseState {.Delegate = &base};
      CHECK_FALSE(CreateColor(ColorKeyword(ValueId::WebkitFocusRingColor), baseState).IsValid());
    }

    SECTION("Serialisation and queries")
    {
      CHECK(SerialiseForTest(ColorKeyword(ValueId::Aliceblue)) == u8"aliceblue");
      CHECK(SerialiseForTest(ColorKeyword(ValueId::Currentcolor)) == u8"currentcolor");
      CHECK(ContainsCurrentColor(ColorKeyword(ValueId::Currentcolor)));
      CHECK_FALSE(ContainsCurrentColor(ColorKeyword(ValueId::Red)));
      CHECK(ContainsColorSchemeDependentColor(ColorKeyword(ValueId::Canvas)));
      CHECK_FALSE(ContainsColorSchemeDependentColor(ColorKeyword(ValueId::Red)));
    }
  }

  TEST_CASE("Hex and resolved colours", "[CSS][Color]")
  {
    CHECK(Resolve(Hex(1, 2, 3)) == web::Color {SRGBA<uint8> {1, 2, 3}});
    CHECK(SerialiseForTest(Hex(255, 0, 0)) == u8"rgb(255, 0, 0)");
    CHECK(SerialiseForTest(Hex(255, 0, 0, 0)) == u8"rgba(255, 0, 0, 0)");

    Color resolved {ResolvedColor {web::Color::blue}};
    CHECK(Resolve(resolved) == web::Color::blue);
    CHECK(SerialiseForTest(resolved) == web::SerialisationForCSS(web::Color::blue));

    CHECK_FALSE(ContainsCurrentColor(Hex(1, 2, 3)));
    CHECK_FALSE(ContainsColorSchemeDependentColor(resolved));
  }

  TEST_CASE("Color", "[CSS][Color]")
  {
    SECTION("Accessors")
    {
      auto keyword = ColorKeyword(ValueId::Red);
      CHECK(keyword.IsKeyword());
      CHECK_FALSE(keyword.IsHex());
      CHECK_FALSE(keyword.IsResolved());
      REQUIRE(keyword.Keyword());
      CHECK(keyword.Keyword()->Value == ValueId::Red);
      CHECK_FALSE(keyword.Hex());

      auto hex = Hex(1, 2, 3);
      REQUIRE(hex.Hex());
      CHECK(hex.Hex()->Value == SRGBA<uint8> {1, 2, 3});

      Color resolved {ResolvedColor {web::Color::red}};
      REQUIRE(resolved.Resolved());
      CHECK(resolved.Resolved()->Value == web::Color::red);
    }

    SECTION("Absolute colour")
    {
      CHECK(ColorKeyword(ValueId::Red).GetAbsoluteColor() == web::Color::red);
      CHECK_FALSE(ColorKeyword(ValueId::Currentcolor).GetAbsoluteColor().IsValid());
      CHECK_FALSE(ColorKeyword(ValueId::Canvas).GetAbsoluteColor().IsValid());
      CHECK(Hex(1, 2, 3).GetAbsoluteColor() == web::Color {SRGBA<uint8> {1, 2, 3}});
      CHECK_FALSE(Color {ContrastColor {ColorKeyword(ValueId::Red)}}.GetAbsoluteColor().IsValid());
    }

    SECTION("Copies, moves and equality, including indirect alternatives")
    {
      Color mix {ColorMix {.ColourInterpolationMethod = DefaultInterpolationMethodForColorMix,
                           .MixComponents1 = {ColorKeyword(ValueId::Red), null},
                           .MixComponents2 = {Hex(0, 0, 255), null}}};
      Color copy = mix;
      CHECK(copy == mix);
      CHECK(SerialiseForTest(copy) == SerialiseForTest(mix));

      Color other {ColorMix {.ColourInterpolationMethod = DefaultInterpolationMethodForColorMix,
                             .MixComponents1 = {ColorKeyword(ValueId::Red), null},
                             .MixComponents2 = {Hex(0, 0, 254), null}}};
      CHECK_FALSE(copy == other);

      copy = other;
      CHECK(copy == other);

      Color moved = krys::move(copy);
      CHECK(moved == other);

      CHECK_FALSE(ColorKeyword(ValueId::Red) == Hex(255, 0, 0));
      CHECK(ColorKeyword(ValueId::Red) == ColorKeyword(ValueId::Red));
    }

    SECTION("Markable")
    {
      Markable<Color> empty;
      CHECK_FALSE(empty);
      Markable<Color> red {ColorKeyword(ValueId::Red)};
      REQUIRE(red);
      CHECK(*red == ColorKeyword(ValueId::Red));
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
      CHECK(Resolve(color) == web::Color {SRGBA<uint8> {255, 255, 0}});
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
      CHECK(Resolve(hsl) == web::Color {SRGBA<uint8> {0, 255, 0}});
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
        RelativeRGB {.Origin = ColorKeyword(ValueId::Red),
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
        RelativeRGB {.Origin = ColorKeyword(ValueId::Currentcolor),
                     .Components = {Symbol {SymbolRaw {ValueId::R}}, Symbol {SymbolRaw {ValueId::G}},
                                    Symbol {SymbolRaw {ValueId::B}}, null}}};
      CHECK_FALSE(Resolve(color).IsValid());

      PlatformColorResolutionState state;
      state.ResolvedCurrentColor = web::Color::blue;
      CHECK(Resolve(color, state).ToColorTypeLossy<SRGBA<uint8>>() == SRGBA<uint8> {0, 0, 255});
    }

    SECTION("color() serialises its colour space")
    {
      Color color {RelativeColor<ColorRGBFunction<ExtendedSRGBA<float>>> {
        .Origin = ColorKeyword(ValueId::Red),
        .Components = {Symbol {SymbolRaw {ValueId::R}}, Symbol {SymbolRaw {ValueId::G}}, Number<> {1.0},
                       null}}};
      CHECK(SerialiseForTest(color) == u8"color(from red srgb r g 1)");
      CHECK(Resolve(color).ToColorTypeLossy<SRGBA<uint8>>() == SRGBA<uint8> {255, 0, 255});
    }

    SECTION("Current colour checks the origin")
    {
      Color current {
        RelativeRGB {.Origin = ColorKeyword(ValueId::Currentcolor),
                     .Components = {Symbol {SymbolRaw {ValueId::R}}, Symbol {SymbolRaw {ValueId::G}},
                                    Symbol {SymbolRaw {ValueId::B}}, null}}};
      CHECK(ContainsCurrentColor(current));
      CHECK_FALSE(ContainsColorSchemeDependentColor(current));

      Color system {
        RelativeRGB {.Origin = ColorKeyword(ValueId::Canvas),
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
                              .MixComponents1 = {ColorKeyword(ValueId::Red), p1},
                              .MixComponents2 = {ColorKeyword(ValueId::Blue), p2}}};
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
                                   .MixComponents1 = {ColorKeyword(ValueId::Currentcolor), null},
                                   .MixComponents2 = {ColorKeyword(ValueId::Blue), null}}};
      CHECK_FALSE(Resolve(withCurrent).IsValid());
      CHECK(ContainsCurrentColor(withCurrent));
    }
  }

  TEST_CASE("contrast-color()", "[CSS][Color][ContrastColor]")
  {
    CHECK(Resolve(Color {ContrastColor {Hex(0, 0, 0)}}) == web::Color::white);
    CHECK(Resolve(Color {ContrastColor {Hex(255, 255, 255)}}) == web::Color::black);
    CHECK(Resolve(Color {ContrastColor {ColorKeyword(ValueId::Yellow)}}) == web::Color::black);
    CHECK(Resolve(Color {ContrastColor {ColorKeyword(ValueId::Navy)}}) == web::Color::white);
    CHECK(SerialiseForTest(Color {ContrastColor {ColorKeyword(ValueId::Red)}}) == u8"contrast-color(red)");
    CHECK(ContainsCurrentColor(Color {ContrastColor {ColorKeyword(ValueId::Currentcolor)}}));
  }

  TEST_CASE("light-dark()", "[CSS][Color][LightDarkColor]")
  {
    Color color {LightDarkColor {ColorKeyword(ValueId::White), ColorKeyword(ValueId::Black)}};

    // No appearance: the invalid colour.
    CHECK_FALSE(Resolve(color).IsValid());

    CHECK(Resolve(color, {.Appearance = LightDarkColorAppearance::Light}) == web::Color::white);
    CHECK(Resolve(color, {.Appearance = LightDarkColorAppearance::Dark}) == web::Color::black);
    CHECK(SerialiseForTest(color) == u8"light-dark(white, black)");
    CHECK(ContainsColorSchemeDependentColor(color));
    CHECK_FALSE(ContainsCurrentColor(color));
    CHECK(ContainsCurrentColor(
      Color {LightDarkColor {ColorKeyword(ValueId::White), ColorKeyword(ValueId::Currentcolor)}}));
  }

  TEST_CASE("color-layers()", "[CSS][Color][ColorLayers]")
  {
    SECTION("An opaque layer hides the ones under it")
    {
      Color color {ColorLayers {.BlendMode = BlendMode::Normal, .Colors = {Hex(255, 0, 0), Hex(0, 0, 255)}}};
      CHECK(Resolve(color) == web::Color::red);
      CHECK(SerialiseForTest(color) == u8"color-layers(rgb(255, 0, 0), rgb(0, 0, 255))");
    }

    SECTION("Translucent layers composite source-over")
    {
      Color color {
        ColorLayers {.BlendMode = BlendMode::Normal, .Colors = {Hex(255, 0, 0, 0), Hex(0, 0, 255)}}};
      CHECK(Resolve(color) == web::Color::blue);
    }

    SECTION("The blend mode is serialised unless it's normal")
    {
      Color color {ColorLayers {.BlendMode = BlendMode::Multiply, .Colors = {ColorKeyword(ValueId::Red)}}};
      CHECK(SerialiseForTest(color) == u8"color-layers(multiply, red)");
      CHECK_FALSE(ContainsCurrentColor(color));
      CHECK(ContainsColorSchemeDependentColor(
        Color {ColorLayers {.BlendMode = BlendMode::Normal, .Colors = {ColorKeyword(ValueId::Canvas)}}}));
    }
  }

  TEST_CASE("Computed style dependencies and child visitation", "[CSS][Color]")
  {
    Color color {ModernRGB {{Number<> {CreateEmCalc()}, Number<> {0.0}, Number<> {0.0}, null}}};
    auto dependencies =
      CollectComputedStyleDependencies(Color {LightDarkColor {color, ColorKeyword(ValueId::Red)}});
    CHECK(std::ranges::find(dependencies.Properties, PropertyId::FontSize) != dependencies.Properties.end());

    CHECK(CollectComputedStyleDependencies(ColorKeyword(ValueId::Red)).IsComputationallyIndependent());

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

  TEST_CASE("color-layers(): blend modes", "[CSS][Color][ColorLayers]")
  {
    auto blend = [](BlendMode mode, web::Color source, web::Color backdrop)
    {
      List<web::Color> colors {krys::move(source), krys::move(backdrop)};
      auto color = BlendSourceOver(ColorLayersResolver {.BlendMode = mode, .Colors = colors});
      return color.ToColorTypeLossy<SRGBA<uint8>>();
    };
    auto rgba = [](uint8 r, uint8 g, uint8 b, uint8 a = 255)
    {
      return SRGBA<uint8> {r, g, b, a};
    };
    auto color = [&](uint8 r, uint8 g, uint8 b, uint8 a = 255)
    {
      return web::Color(rgba(r, g, b, a));
    };

    SECTION("separable")
    {
      CHECK(blend(BlendMode::Multiply, color(255, 0, 0), color(0, 255, 0)) == rgba(0, 0, 0));
      CHECK(blend(BlendMode::Screen, color(255, 0, 0), color(0, 255, 0)) == rgba(255, 255, 0));
      CHECK(blend(BlendMode::Darken, color(200, 10, 90), color(100, 50, 90)) == rgba(100, 10, 90));
      CHECK(blend(BlendMode::Lighten, color(200, 10, 90), color(100, 50, 90)) == rgba(200, 50, 90));
      CHECK(blend(BlendMode::Difference, color(255, 255, 255), color(100, 0, 255)) == rgba(155, 255, 0));
      CHECK(blend(BlendMode::Exclusion, color(255, 0, 255), color(255, 255, 0)) == rgba(0, 255, 255));
      CHECK(blend(BlendMode::Overlay, color(255, 255, 255), color(0, 255, 0)) == rgba(0, 255, 0));
      CHECK(blend(BlendMode::HardLight, color(255, 0, 255), color(0, 255, 0)) == rgba(255, 0, 255));
      CHECK(blend(BlendMode::ColorDodge, color(0, 0, 255), color(0, 128, 128)) == rgba(0, 128, 255));
      CHECK(blend(BlendMode::ColorBurn, color(255, 255, 0), color(255, 128, 128)) == rgba(255, 128, 0));
      CHECK(blend(BlendMode::SoftLight, color(128, 128, 128), color(100, 200, 50)) == rgba(100, 200, 50));
    }

    SECTION("non-separable")
    {
      // Red takes the luminosity of 128 grey, about 0.502, by adding 0.202 and clipping the result to 1,
      // which moves green and blue from 0.202 to 0.502 - 0.3 x 0.7115 = 0.2885.
      CHECK(blend(BlendMode::Color, color(255, 0, 0), color(128, 128, 128)) == rgba(255, 74, 74));
      CHECK(blend(BlendMode::Luminosity, color(255, 255, 255), color(0, 0, 255)) == rgba(255, 255, 255));
      // A grey has no hue or saturation to give.
      auto saturation = blend(BlendMode::Saturation, color(128, 128, 128), color(200, 100, 100)).Resolved();
      CHECK(saturation.Red == saturation.Green);
      CHECK(saturation.Blue == saturation.Green);
      auto hue = blend(BlendMode::Hue, color(0, 0, 255), color(128, 128, 128)).Resolved();
      CHECK(hue.Red == hue.Green);
      CHECK(hue.Blue == hue.Green);
    }

    SECTION("with alpha")
    {
      // The blend shows as far as the source is opaque, the source as far as the backdrop isn't.
      CHECK(blend(BlendMode::Multiply, color(255, 0, 0, 128), color(255, 255, 255)) == rgba(255, 127, 127));
      CHECK(blend(BlendMode::Multiply, color(255, 0, 0, 128), color(0, 0, 0, 0)) == rgba(255, 0, 0, 128));
      CHECK(blend(BlendMode::Multiply, color(0, 0, 0, 0), color(0, 255, 0)) == rgba(0, 255, 0));
      CHECK(blend(BlendMode::Multiply, color(0, 0, 0, 0), color(0, 0, 0, 0)) == rgba(0, 0, 0, 0));
    }

    SECTION("an opaque colour hides what's beneath it only in normal blending")
    {
      auto layers = [&](BlendMode mode)
      {
        List<web::Color> colors {color(255, 255, 255), color(255, 0, 0), color(0, 255, 0)};
        return BlendSourceOver(ColorLayersResolver {.BlendMode = mode, .Colors = colors})
          .ToColorTypeLossy<SRGBA<uint8>>();
      };
      CHECK(layers(BlendMode::Normal) == rgba(255, 255, 255));
      CHECK(layers(BlendMode::Multiply) == rgba(0, 0, 0));
    }
  }
}
