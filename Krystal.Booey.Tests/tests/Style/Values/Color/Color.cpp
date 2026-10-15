#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Color.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Values/Color/KeywordColor.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Values/Color/ColorLayers.hpp"
#include "Krystal.Booey/Style/Values/Color/ColorMix.hpp"
#include "Krystal.Booey/Style/Values/Color/ColorResolutionState.hpp"
#include "Krystal.Booey/Style/Values/Color/ContrastColor.hpp"
#include "Krystal.Booey/Style/Values/Color/RelativeColor.hpp"
#include "Krystal.Booey/Style/Values/Color/SystemColor.hpp"
#include "Krystal.Core/Types/Markable.hpp"
#include <string>

namespace krys::boo::style::tests
{
  namespace
  {
    const auto ColorFunctionsEnabled = []
    {
      css::ParserContext context {css::ParserMode::HTMLStandard};
      context.colorLayersEnabled = true;
      context.contrastColorEnabled = true;
      return context;
    }();

    /// @brief Tokenizes `text` and consumes a colour from it; fails the test unless every token was consumed.
    css::Color ParseCSS(utf8_stringview text, const css::ParserContext &context = ColorFunctionsEnabled)
    {
      css::InputStream input {css::CSSOMString(text)};
      css::Tokenizer tokenizer {input};
      tokenizer.PumpTokenizer();
      auto tokens = tokenizer.Tokens();

      css::PropertyParserState state {.Context = context};
      auto result = css::PropertyParserHelpers::ConsumeUnresolvedColor(tokens, state);
      REQUIRE(result);
      REQUIRE(tokens.IsAtEnd());
      return krys::move(*result);
    }

    /// @brief A document and style to resolve colours against.
    struct Resolution
    {
      Ref<dom::Document> Document {CreateRef<dom::Document>()};
      RenderStyle Style {RenderStyle::Create()};

      Color Convert(const css::Color &value, ForVisitedLink forVisitedLink = ForVisitedLink::No)
      {
        return ToStyleColor(value, Document, Style, css::ToLengthConversionData {}, forVisitedLink);
      }

      Color Convert(utf8_stringview text, ForVisitedLink forVisitedLink = ForVisitedLink::No)
      {
        return Convert(ParseCSS(text), forVisitedLink);
      }
    };

    /// @brief The colour as 8-bit sRGB, converting out-of-line colours.
    SRGBA<uint8> Bytes(const boo::Color &color)
    {
      return color.ToColorTypeLossy<SRGBA<uint8>>();
    }

    /// @brief Serialises for tokenization as a std::string, which Catch2 can print.
    std::string Tokenised(const Color &color)
    {
      auto serialised = SerialisationForCSSTokenization(css::DefaultSerialisationContext(), color);
      return std::string(serialised.begin(), serialised.end());
    }

    std::string ToString(const css::CSSOMString &value)
    {
      return std::string(value.begin(), value.end());
    }

    /// @brief The platform colour serialised as a std::string, which Catch2 can print.
    std::string Serialised(const boo::Color &color)
    {
      return ToString(boo::SerialisationForCSS(color));
    }

    constexpr auto Red = SRGBA<uint8> {255, 0, 0};
    constexpr auto Green = SRGBA<uint8> {0, 128, 0};
    constexpr auto Blue = SRGBA<uint8> {0, 0, 255};
    constexpr auto White = SRGBA<uint8> {255, 255, 255};
    constexpr auto Black = SRGBA<uint8> {0, 0, 0};
  }

  TEST_CASE("Style colour(construction)", "[Style][Values][Color]")
  {
    SECTION("the default is currentcolor")
    {
      Color color;
      CHECK(color.IsCurrentColor());
      CHECK(color.ContainsCurrentColor());
      CHECK_FALSE(color.IsResolvedColor());
      CHECK(color == Color {css::keywords::Currentcolor {}});
      CHECK(color == Color::GetCurrentColor());
    }

    SECTION("convenience constructors make resolved colours")
    {
      CHECK(Color {boo::Color {Red}}.GetResolvedColor() == boo::Color {Red});
      CHECK(Color {Blue}.GetResolvedColor() == boo::Color {Blue});
      CHECK(Color {css::keywords::Transparent {}}.GetResolvedColor() == boo::Color::transparentBlack);
      CHECK(Color {css::keywords::Black {}}.GetResolvedColor() == boo::Color::black);
      CHECK(Color {css::keywords::White {}}.GetResolvedColor() == boo::Color::white);

      Color resolved {Red};
      CHECK(resolved.IsResolvedColor());
      CHECK_FALSE(resolved.IsCurrentColor());
      CHECK_FALSE(resolved.IsColorMix());
      CHECK_FALSE(resolved.IsContrastColor());
      CHECK_FALSE(resolved.IsRelativeColor());
      CHECK_FALSE(resolved.ContainsCurrentColor());
      CHECK(resolved.ResolveColor(boo::Color {Blue}) == boo::Color {Red});
    }

    SECTION("currentcolor resolves to the current colour")
    {
      CHECK(Color {}.ResolveColor(boo::Color {Green}) == boo::Color {Green});
      CHECK(ResolveColor(Color {}, boo::Color {Blue}) == boo::Color {Blue});
    }

    SECTION("Markable")
    {
      Markable<Color> empty;
      CHECK_FALSE(empty);

      Markable<Color> value {Color {Red}};
      REQUIRE(value);
      CHECK(*value == Color {Red});
    }
  }

  TEST_CASE("Style colour(conversion of resolved colours)", "[Style][Values][Color]")
  {
    Resolution resolution;

    SECTION("absolute colours resolve during conversion")
    {
      for (auto text : {u8"red", u8"#ff0000", u8"rgb(255 0 0)", u8"rgb(calc(200 + 55), 0, 0)",
                        u8"hsl(0 100% 50%)", u8"color(srgb 1 0 0)"})
      {
        auto color = resolution.Convert(text);
        INFO(ToString(css::CSSOMString(text)));
        REQUIRE(color.IsResolvedColor());
        CHECK(Bytes(color.GetResolvedColor()) == Red);
      }
    }

    SECTION("transparent")
    {
      CHECK(resolution.Convert(u8"transparent").GetResolvedColor() == boo::Color::transparentBlack);
    }

    SECTION("composites of resolved colours resolve during conversion")
    {
      auto mix = resolution.Convert(u8"color-mix(in srgb, red, blue)");
      REQUIRE(mix.IsResolvedColor());
      CHECK(Bytes(mix.GetResolvedColor()) == SRGBA<uint8> {128, 0, 128});

      auto contrast = resolution.Convert(u8"contrast-color(white)");
      REQUIRE(contrast.IsResolvedColor());
      CHECK(Bytes(contrast.GetResolvedColor()) == Black);

      auto relative = resolution.Convert(u8"rgb(from red b g r)");
      REQUIRE(relative.IsResolvedColor());
      CHECK(Bytes(relative.GetResolvedColor()) == Blue);

      // Every layer is resolved, so the layers are blended now.
      auto layers = resolution.Convert(u8"color-layers(rgb(0 0 255 / 0.5), red)");
      REQUIRE(layers.IsResolvedColor());
      CHECK(Serialised(layers.GetResolvedColor()) == "rgb(127, 0, 128)");
    }

    SECTION("the nesting level is restored")
    {
      auto state = ColorResolutionState {.Document = resolution.Document,
                                         .Style = resolution.Style,
                                         .ConversionData = {},
                                         .ForVisitedLink = ForVisitedLink::No};
      auto color =
        ToStyleColor(ParseCSS(u8"color-mix(in srgb, rgb(from red r g b), contrast-color(red))"), state);
      CHECK(color.IsResolvedColor());
      CHECK(state.NestingLevel == 0);
    }
  }

  TEST_CASE("Style colour(conversion of keywords)", "[Style][Values][Color]")
  {
    Resolution resolution;
    auto keyword = [&](css::ValueId id, ForVisitedLink forVisitedLink = ForVisitedLink::No)
    {
      return resolution.Convert(css::Color {css::KeywordColor {id}}, forVisitedLink);
    };

    SECTION("currentcolor stays unresolved")
    {
      CHECK(keyword(css::ValueId::Currentcolor).IsCurrentColor());
    }

    SECTION("document colours")
    {
      CHECK(keyword(css::ValueId::InternalDocumentTextColor).GetResolvedColor() == boo::Color::black);

      auto link = css::ColorFromKeyword(css::ValueId::WebkitLink, ColorOptions::None);
      auto visitedLink = css::ColorFromKeyword(css::ValueId::WebkitLink, ColorOptions::ForVisitedLink);
      REQUIRE(link != visitedLink);
      CHECK(keyword(css::ValueId::WebkitLink).GetResolvedColor() == link);
      CHECK(keyword(css::ValueId::WebkitLink, ForVisitedLink::Yes).GetResolvedColor() == visitedLink);

      CHECK(keyword(css::ValueId::WebkitActivelink).GetResolvedColor()
            == css::ColorFromKeyword(css::ValueId::WebkitActivelink, ColorOptions::None));
      CHECK(keyword(css::ValueId::WebkitFocusRingColor).GetResolvedColor()
            == FocusRingColor(ColorOptions::None));
    }

    SECTION("system colours follow the style's colour scheme")
    {
      auto light = css::ColorFromKeyword(css::ValueId::Linktext, ColorOptions::None);
      auto dark = css::ColorFromKeyword(css::ValueId::Linktext, ColorOptions::UseDarkAppearance);
      REQUIRE(light != dark);

      CHECK(keyword(css::ValueId::Linktext).GetResolvedColor() == light);

      resolution.Style.SetColorScheme(ColorScheme {css::SpaceSeparatedList<css::CustomIdentifier> {
                                                     css::CustomIdentifier {css::CSSOMStringAtom {u8"dark"}}},
                                                   null});
      CHECK(keyword(css::ValueId::Linktext).GetResolvedColor() == dark);
      CHECK(keyword(css::ValueId::WebkitLink).GetResolvedColor()
            == css::ColorFromKeyword(css::ValueId::WebkitLink, ColorOptions::UseDarkAppearance));
    }

    SECTION("light-dark() picks by the style's colour scheme")
    {
      CHECK(Bytes(resolution.Convert(u8"light-dark(red, blue)").GetResolvedColor()) == Red);

      resolution.Style.SetColorScheme(ColorScheme {css::SpaceSeparatedList<css::CustomIdentifier> {
                                                     css::CustomIdentifier {css::CSSOMStringAtom {u8"dark"}}},
                                                   null});
      CHECK(Bytes(resolution.Convert(u8"light-dark(red, blue)").GetResolvedColor()) == Blue);

      CHECK(resolution.Convert(u8"light-dark(currentcolor, blue)").IsResolvedColor());
      resolution.Style.SetColorScheme(ColorScheme {css::keywords::Normal {}});
      CHECK(resolution.Convert(u8"light-dark(currentcolor, blue)").IsCurrentColor());
    }
  }

  TEST_CASE("Style colour(currentcolor-dependent colours)", "[Style][Values][Color]")
  {
    Resolution resolution;

    SECTION("color-mix()")
    {
      auto color = resolution.Convert(u8"color-mix(in srgb, currentcolor 25%, blue)");
      REQUIRE(color.IsColorMix());
      CHECK(color.ContainsCurrentColor());
      CHECK(Bytes(color.ResolveColor(boo::Color {Red})) == SRGBA<uint8> {64, 0, 191});
      CHECK(Tokenised(color) == "color-mix(in srgb, currentcolor 25%, rgb(0, 0, 255))");

      CHECK(Tokenised(resolution.Convert(u8"color-mix(currentcolor, blue)"))
            == "color-mix(currentcolor, rgb(0, 0, 255))");
      CHECK(Tokenised(resolution.Convert(u8"color-mix(in srgb, currentcolor 50%, blue 50%)"))
            == "color-mix(in srgb, currentcolor, rgb(0, 0, 255))");
      CHECK(Tokenised(resolution.Convert(u8"color-mix(in srgb, currentcolor 30%, blue 70%)"))
            == "color-mix(in srgb, currentcolor 30%, rgb(0, 0, 255))");
      CHECK(Tokenised(resolution.Convert(u8"color-mix(in srgb, currentcolor 30%, blue 20%)"))
            == "color-mix(in srgb, currentcolor 30%, rgb(0, 0, 255) 20%)");
      CHECK(Tokenised(resolution.Convert(u8"color-mix(in srgb, currentcolor, blue 20%)"))
            == "color-mix(in srgb, currentcolor 80%, rgb(0, 0, 255))");
      CHECK(Tokenised(resolution.Convert(u8"color-mix(in srgb, currentcolor, blue calc(10% + 10%))"))
            == "color-mix(in srgb, currentcolor 80%, rgb(0, 0, 255))");
    }

    SECTION("contrast-color()")
    {
      auto color = resolution.Convert(u8"contrast-color(currentcolor)");
      REQUIRE(color.IsContrastColor());
      CHECK(color.ContainsCurrentColor());
      CHECK(color.ResolveColor(boo::Color {White}) == boo::Color::black);
      CHECK(color.ResolveColor(boo::Color {Black}) == boo::Color::white);
      CHECK(Tokenised(color) == "contrast-color(currentcolor)");
    }

    SECTION("relative colours")
    {
      auto color = resolution.Convert(u8"rgb(from currentcolor b g r / calc(alpha / 2))");
      REQUIRE(color.IsRelativeColor());
      CHECK(color.ContainsCurrentColor());
      CHECK(Bytes(color.ResolveColor(boo::Color {Red})) == SRGBA<uint8> {0, 0, 255, 128});
      // The calc is simplified during conversion.
      CHECK(Tokenised(color) == "rgb(from currentcolor b g r / calc(0.5 * alpha))");

      auto lab = resolution.Convert(u8"lab(from currentcolor l a b)");
      REQUIRE(lab.IsRelativeColor());
      CHECK(Tokenised(lab) == "lab(from currentcolor l a b)");
      CHECK(Bytes(lab.ResolveColor(boo::Color {Green})) == Green);

      auto colorFunction = resolution.Convert(u8"color(from currentcolor display-p3 r g b)");
      REQUIRE(colorFunction.IsRelativeColor());
      CHECK(Tokenised(colorFunction) == "color(from currentcolor display-p3 r g b)");
    }

    SECTION("color-layers()")
    {
      // A layer depends on currentcolor, so the blend waits until use time.
      auto layers = resolution.Convert(u8"color-layers(rgb(0 0 255 / 0.5), red)").GetResolvedColor();
      auto color = resolution.Convert(u8"color-layers(rgb(0 0 255 / 0.5), currentcolor)");
      CHECK_FALSE(color.IsResolvedColor());
      CHECK(color.ContainsCurrentColor());
      CHECK(Serialised(color.ResolveColor(boo::Color {Red})) == Serialised(layers));
      CHECK(Tokenised(color) == "color-layers(rgba(0, 0, 255, 0.5), currentcolor)");

      // The parser doesn't take a blend mode yet.
      auto multiply =
        Color {ColorLayers {.BlendMode = BlendMode::Multiply, .Colors = {Color {}, Color {Red}}}};
      CHECK(Tokenised(multiply) == "color-layers(multiply, currentcolor, rgb(255, 0, 0))");
    }

    SECTION("nested")
    {
      auto color = resolution.Convert(u8"color-mix(in srgb, contrast-color(currentcolor), red)");
      REQUIRE(color.IsColorMix());
      CHECK(Bytes(color.ResolveColor(boo::Color {White})) == SRGBA<uint8> {128, 0, 0});
      CHECK(Tokenised(color) == "color-mix(in srgb, contrast-color(currentcolor), rgb(255, 0, 0))");
    }
  }

  TEST_CASE("Style colour(copies and equality)", "[Style][Values][Color]")
  {
    Resolution resolution;

    for (auto text : {u8"color-mix(in srgb, currentcolor 25%, blue)", u8"contrast-color(currentcolor)",
                      u8"rgb(from currentcolor r g b)", u8"color-layers(currentcolor, red)"})
    {
      INFO(ToString(css::CSSOMString(text)));
      auto original = resolution.Convert(text);
      auto copy = original;
      CHECK(copy == original);
      CHECK(Tokenised(copy) == Tokenised(original));

      Color assigned {Red};
      assigned = original;
      CHECK(assigned == original);

      auto moved = krys::move(copy);
      CHECK(moved == original);
    }

    CHECK(resolution.Convert(u8"contrast-color(currentcolor)")
          == resolution.Convert(u8"contrast-color(currentcolor)"));
    CHECK(resolution.Convert(u8"rgb(from currentcolor r g b)")
          != resolution.Convert(u8"rgb(from currentcolor g g b)"));
    CHECK(resolution.Convert(u8"rgb(from currentcolor r g b)")
          != resolution.Convert(u8"hsl(from currentcolor h s l)"));
    CHECK(Color {Red} != Color {});
    CHECK(Color {Red} != Color {Blue});
  }

  TEST_CASE("Style colour(serialisation)", "[Style][Values][Color]")
  {
    CHECK(Tokenised(Color {}) == "currentcolor");
    CHECK(Tokenised(Color {Red}) == "rgb(255, 0, 0)");
    CHECK(ToString(SerialisationForCSSTokenization(css::DefaultSerialisationContext(), CurrentColor {}))
          == "currentcolor");
    CHECK(ToString(SerialisationForCSSTokenization(css::DefaultSerialisationContext(),
                                                   ResolvedColor {boo::Color {Blue}}))
          == "rgb(0, 0, 255)");

    // Computed values serialise the used colour.
    RenderStyle style {RenderStyle::Create()};
    style.SetColor(boo::Color {Green});
    CHECK(ToString(Serialise(css::DefaultSerialisationContext(), style, Color {})) == "rgb(0, 128, 0)");
    CHECK(ToString(Serialise(css::DefaultSerialisationContext(), style, Color {Red})) == "rgb(255, 0, 0)");

    auto cssColor = ConvertToCSS(Color {}, style);
    REQUIRE(cssColor.Resolved());
    CHECK(cssColor.Resolved()->Value == boo::Color {Green});
  }

  TEST_CASE("Style colour(RenderStyle)", "[Style][Values][Color]")
  {
    RenderStyle style {RenderStyle::Create()};
    CHECK(style.GetColor() == boo::Color::black);
    CHECK(style.VisitedLinkColor() == boo::Color::black);

    style.SetColor(boo::Color {Red});
    style.SetVisitedLinkColor(boo::Color {Blue});
    CHECK(style.ColorResolvingCurrentColor(Color {}) == boo::Color {Red});
    CHECK(style.ColorResolvingCurrentColor(Color {}, true) == boo::Color {Blue});
    CHECK(style.ColorResolvingCurrentColor(Color {Green}, true) == boo::Color {Green});
  }

  TEST_CASE("Style colour(blending)", "[Style][Values][Color]")
  {
    RenderStyle aStyle {RenderStyle::Create()};
    aStyle.SetColor(boo::Color {Red});
    RenderStyle bStyle {RenderStyle::Create()};
    bStyle.SetColor(boo::Color {Blue});

    SECTION("equality")
    {
      CHECK(EqualsForBlending(Color {}, Color {}, aStyle, bStyle));
      CHECK(EqualsForBlending(Color {Red}, Color {Red}, aStyle, bStyle));
      CHECK_FALSE(EqualsForBlending(Color {Red}, Color {Blue}, aStyle, bStyle));
      CHECK(EqualsForBlending(Color {}, Color {Red}, aStyle, bStyle));
      CHECK_FALSE(EqualsForBlending(Color {Red}, Color {}, aStyle, bStyle));
    }

    SECTION("can blend unless both are currentcolor")
    {
      CHECK_FALSE(CanBlend(Color {}, Color {}));
      CHECK(CanBlend(Color {}, Color {Red}));
      CHECK(CanBlend(Color {Red}, Color {Blue}));
      CHECK(Blending<Color> {}.RequiresInterpolationForAccumulativeIteration(Color {}, Color {}));
    }

    SECTION("blends the used colours")
    {
      auto context = BlendingContext {0.5};
      auto blended = Blending<Color> {}.Blend(Color {}, Color {}, aStyle, bStyle, context);
      REQUIRE(blended.IsResolvedColor());
      CHECK(Bytes(blended.GetResolvedColor()) == SRGBA<uint8> {128, 0, 128});
    }
  }
}
