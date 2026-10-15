#include "Krystal.Booey/Style/BuilderState.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/BuilderStateInlines.hpp"
#include "Krystal.Booey/Style/Seam.hpp"

namespace krys::boo::style::tests
{
  using css::PropertyId;

  namespace
  {
    KRYS_NODISCARD Ref<dom::HTMLDocument> ParseDocument(utf8_string &&input) noexcept
    {
      auto document = CreateRef<dom::HTMLDocument>();
      auto parser = CreateRef<html::HTMLDocumentParser>(*document);
      parser->InputStream().Append(krys::move(input), IsEOF(true));
      (void)parser->PumpTokenizer();
      return document;
    }

    KRYS_NODISCARD dom::Element &ById(dom::Document &document, dom::DOMStringView id)
    {
      auto element = document.GetElementById(id);
      REQUIRE(element);
      return *element;
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "BuilderState", "[Style][BuilderState]")
  {
    SECTION("Styles, document and element")
    {
      CHECK(&State->Style() == &Style);
      CHECK(&State->ParentStyle() == &ParentStyle);
      CHECK_FALSE(State->RootElementStyle());
      CHECK(&State->GetDocument() == Document.get());
      CHECK_FALSE(State->GetElement());
      CHECK(State->CssToLengthConversionData().Style() == &Style);
      CHECK(State->CssToLengthConversionData().ParentStyle() == &ParentStyle);
      CHECK(State->ZoomWithTextZoomFactor() == 1.f);
    }

    SECTION("The current property")
    {
      CHECK(State->CssPropertyId() == PropertyId::Invalid);

      CurrentProperty.Id = PropertyId::Color;
      CurrentProperty.StyleScopeOrdinal = ScopeOrdinal::Shadow;
      CHECK(State->CssPropertyId() == PropertyId::Color);
      CHECK(State->StyleScopeOrdinal() == ScopeOrdinal::Shadow);
      CHECK_FALSE(State->IsAuthorOrigin());

      CurrentProperty.Origin = PropertyCascade::Origin::Author;
      CHECK(State->IsAuthorOrigin());

      State->SetCurrentProperty(nullptr);
      CHECK(State->CssPropertyId() == PropertyId::Invalid);
      CHECK_FALSE(State->IsAuthorOrigin());
    }

    SECTION("Invalid at computed-value time is per property")
    {
      CurrentProperty.Id = PropertyId::Color;
      State->SetCurrentPropertyInvalidAtComputedValueTime();
      CHECK(State->IsCurrentPropertyInvalidAtComputedValueTime());

      CurrentProperty.Id = PropertyId::Width;
      CHECK_FALSE(State->IsCurrentPropertyInvalidAtComputedValueTime());

      CurrentProperty.Id = PropertyId::Color;
      CHECK(State->IsCurrentPropertyInvalidAtComputedValueTime());
    }

    SECTION("Content attributes are registered for ::before and ::after")
    {
      State->RegisterContentAttribute(css::CSSOMStringAtom {u8"title"});
      CHECK(State->RegisteredContentAttributes().empty());

      Style.SetPseudoElementIdentifier(PseudoElementIdentifier {PseudoElementType::Before});
      State->RegisterContentAttribute(css::CSSOMStringAtom {u8"title"});
      Style.SetPseudoElementIdentifier(PseudoElementIdentifier {PseudoElementType::Marker});
      State->RegisterContentAttribute(css::CSSOMStringAtom {u8"alt"});
      Style.SetPseudoElementIdentifier(PseudoElementIdentifier {PseudoElementType::After});
      State->RegisterContentAttribute(css::CSSOMStringAtom {u8"lang"});

      REQUIRE(State->RegisteredContentAttributes().size() == 2);
      CHECK(State->RegisteredContentAttributes()[0] == css::CSSOMStringAtom {u8"title"});
      CHECK(State->RegisteredContentAttributes()[1] == css::CSSOMStringAtom {u8"lang"});
    }

    SECTION("Unit usage")
    {
      State->SetUsesViewportUnits();
      State->SetUsesContainerUnits();
      CHECK(Style.UsesViewportUnits());
      CHECK(Style.UsesContainerUnits());
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "BuilderState: elements", "[Style][BuilderState]")
  {
    auto document = ParseDocument(u8"<!DOCTYPE html><body><p id=a></p><p id=b></p>text<p id=c></p>"
                                  u8"<svg id=outer><svg id=inner></svg><circle id=circle /></svg></body>");

    SECTION("Sibling count and index")
    {
      Reset(&ById(*document, u8"b"));
      CHECK(State->SiblingCount() == 4);
      CHECK(State->SiblingIndex() == 2);
      CHECK(Style.UseTreeCountingFunctions());

      Reset(&ById(*document, u8"a"));
      CHECK(State->SiblingIndex() == 1);
    }

    SECTION("The root element has no siblings to count")
    {
      Reset(document->DocumentElement().get());
      CHECK(State->SiblingCount() == 1);
      CHECK(State->SiblingIndex() == 1);
      CHECK_FALSE(Style.UseTreeCountingFunctions());
    }

    SECTION("SVG zoom rules")
    {
      Reset(&ById(*document, u8"a"));
      CHECK_FALSE(State->UseSVGZoomRules());
      CHECK_FALSE(State->UseSVGZoomRulesForLength());

      Reset(&ById(*document, u8"circle"));
      CHECK(State->UseSVGZoomRules());
      CHECK(State->UseSVGZoomRulesForLength());

      // An `<svg>` with a parent is laid out like CSS boxes.
      Reset(&ById(*document, u8"inner"));
      CHECK(State->UseSVGZoomRules());
      CHECK_FALSE(State->UseSVGZoomRulesForLength());
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "BuilderState: font description", "[Style][BuilderState]")
  {
    CHECK_FALSE(State->FontDirty());
    CHECK(State->GetFontDescription().ComputedSize() == 16);
    CHECK(State->ParentFontDescription().ComputedSize() == 16);

    SECTION("Setting a value that's already set leaves the font clean")
    {
      State->SetFontDescriptionKeywordSizeFromIdentifier(css::ValueId::Medium);
      State->SetFontDescriptionIsAbsoluteSize(false);
      State->SetFontDescriptionFontSize(16);
      CHECK_FALSE(State->FontDirty());
    }

    SECTION("The font size sets the specified and computed sizes")
    {
      State->SetFontDescriptionFontSize(20);
      CHECK(State->FontDirty());
      CHECK(Style.GetFontDescription().SpecifiedSize() == 20);
      CHECK(Style.GetFontDescription().ComputedSize() == 20);
    }

    SECTION("The computed size follows the zoom")
    {
      State->SetUsedZoom(2);
      CHECK(State->FontDirty());
      State->SetFontDescriptionFontSize(10);
      CHECK(Style.GetFontDescription().SpecifiedSize() == 10);
      CHECK(Style.GetFontDescription().ComputedSize() == 20);
      CHECK(Style.GetFontDescription().UsedZoomFactor() == 2);

      FontCascadeDescription description;
      State->SetFontSize(description, 5);
      CHECK(description.SpecifiedSize() == 5);
      CHECK(description.ComputedSize() == 10);
    }

    SECTION("Keyword sizes")
    {
      State->SetFontDescriptionKeywordSizeFromIdentifier(css::ValueId::Large);
      CHECK(State->FontDirty());
      CHECK(Style.GetFontDescription().KeywordSizeAsIdentifier() == css::ValueId::Large);
    }

    SECTION("Variant groups set each of their parts")
    {
      State->SetFontDescriptionVariantLigatures(FontVariantLigatures {FontVariantLigaturesValues {
        .Common = boo::FontVariantLigatures::No, .Contextual = boo::FontVariantLigatures::Yes}});
      CHECK(State->FontDirty());
      CHECK(Style.GetFontDescription().VariantCommonLigatures() == boo::FontVariantLigatures::No);
      CHECK(Style.GetFontDescription().VariantDiscretionaryLigatures() == boo::FontVariantLigatures::Normal);
      CHECK(Style.GetFontDescription().VariantContextualAlternates() == boo::FontVariantLigatures::Yes);
    }

    SECTION("The writing mode dirties the font")
    {
      State->SetWritingMode(StyleWritingMode::VerticalRl);
      CHECK(State->FontDirty());
    }
  }

  TEST_CASE_METHOD(BuilderFixture, "BuilderState: native appearance", "[Style][BuilderState]")
  {
    auto document = ParseDocument(u8"<!DOCTYPE html><body><button id=button></button><p id=p></p></body>");

    // Link matching is off, so properties apply to both the regular and the visited link style.
    CHECK(State->ApplyPropertyToRegularStyle());
    CHECK(State->ApplyPropertyToVisitedLinkStyle());

    Reset(&ById(*document, u8"p"));
    State->DisableNativeAppearanceIfNeeded(PropertyId::BackgroundColor, PropertyCascade::Origin::Author);
    CHECK_FALSE(Style.NativeAppearanceDisabled());

    Reset(&ById(*document, u8"button"));
    State->DisableNativeAppearanceIfNeeded(PropertyId::BackgroundColor, PropertyCascade::Origin::UserAgent);
    CHECK_FALSE(Style.NativeAppearanceDisabled());
    State->DisableNativeAppearanceIfNeeded(PropertyId::Color, PropertyCascade::Origin::Author);
    CHECK_FALSE(Style.NativeAppearanceDisabled());
    State->DisableNativeAppearanceIfNeeded(PropertyId::BackgroundColor, PropertyCascade::Origin::Author);
    CHECK(Style.NativeAppearanceDisabled());
  }

  TEST_CASE("RenderStyle: writing mode", "[Style][RenderStyle]")
  {
    RenderStyle style {RenderStyle::Create()};
    CHECK(style.GetWritingMode().IsHorizontal());
    CHECK(style.GetWritingMode().IsBidiLTR());

    CHECK(style.SetWritingMode(StyleWritingMode::VerticalRl));
    CHECK_FALSE(style.SetWritingMode(StyleWritingMode::VerticalRl));
    CHECK(style.SetTextOrientation(TextOrientation::Upright));
    CHECK(style.SetDirection(TextDirection::RTL));

    auto mode = style.GetWritingMode();
    CHECK(mode.ComputedWritingMode() == StyleWritingMode::VerticalRl);
    CHECK(mode.ComputedTextOrientation() == TextOrientation::Upright);
    CHECK(mode.ComputedTextDirection() == TextDirection::RTL);

    // Changing the orientation keeps the writing mode.
    CHECK(style.SetTextOrientation(TextOrientation::Sideways));
    CHECK(style.GetWritingMode().ComputedWritingMode() == StyleWritingMode::VerticalRl);
    CHECK(style.GetWritingMode().ComputedTextOrientation() == TextOrientation::Sideways);
  }
}
