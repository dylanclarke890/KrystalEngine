#include "Krystal.Booey/Style/Adjuster.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/RuleTestUtils.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/BuilderTestUtils.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/Builder.hpp"
#include "Krystal.Booey/Style/Change.hpp"
#include "Krystal.Booey/Style/MatchResult.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Values/PointerEvents/TouchAction.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"

namespace krys::boo::style::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<dom::HTMLDocument> Parse(utf8_string &&input) noexcept
    {
      auto document = CreateRef<dom::HTMLDocument>();
      auto parser = CreateRef<html::HTMLDocumentParser>(*document);
      parser->InputStream().Append(krys::move(input), IsEOF(true));
      (void)parser->PumpTokenizer();
      return document;
    }

    struct AdjusterFixture : css::tests::ValueFixture
    {
      Ref<dom::HTMLDocument> Document {Parse(u8R"(<!DOCTYPE html><body><div id=parent>
        <div id=div></div><span id=span></span><img id=img><table id=table></table><ruby id=ruby></ruby>
        <textarea id=textarea></textarea><input id=password type=PassWord><marquee id=marquee></marquee>
        <legend id=legend></legend><rt id=rt></rt>
        <svg id=svg><circle id=circle /><svg id=inner /><foreignObject id=foreignObject><svg id=nested /></foreignObject></svg>
      </div></body>)")};

      /// @brief The mode declarations are parsed in.
      css::ParserMode Mode {css::ParserMode::HTMLStandard};

      KRYS_NODISCARD dom::Element &ById(dom::DOMStringView id) noexcept
      {
        auto element = Document->GetElementById(id);
        krys_assert(element);
        return *element;
      }

      /// @brief The style the builder computes from the author's `css` for `element`, inheriting from
      /// `parent` (the document's default style if null) as the resolver does, before adjusting.
      KRYS_NODISCARD UniquePtr<RenderStyle> Build(utf8_stringview css, const dom::Element *element = nullptr,
                                                  const RenderStyle *parent = nullptr) noexcept
      {
        auto defaultStyle = Resolver::DefaultStyleForElement(*Document, nullptr);
        auto style = Resolver::DefaultStyleForElement(*Document, nullptr);
        if (parent)
        {
          style->InheritFrom(*parent);
        }
        auto matched = MatchResult::Create();
        matched->AuthorDeclarations.push_back(MatchedProperties {
          .Properties = css::tests::ParseDeclarations(css, Mode)->ImmutableCopyIfNeeded()});
        Builder builder {*style,
                         BuilderContext {.Document = Document,
                                         .ParentStyle = parent ? parent : defaultStyle.get(),
                                         .Element = ShareRefPtr(element)},
                         *matched};
        builder.ApplyAllProperties();
        return style;
      }

      /// @brief The style of `element` from `css`, adjusted for its parent's style (and parent box style).
      KRYS_NODISCARD UniquePtr<RenderStyle> Adjusted(utf8_stringview css, dom::Element *element = nullptr,
                                                     const RenderStyle *parent = nullptr,
                                                     const RenderStyle *parentBox = nullptr) noexcept
      {
        auto defaultStyle = Resolver::DefaultStyleForElement(*Document, nullptr);
        auto &parentStyle = parent ? *parent : *defaultStyle;
        auto style = Build(css, element, &parentStyle);
        Adjuster {*Document, parentStyle, parentBox, element}.Adjust(*style);
        return style;
      }
    };
  }

  TEST_CASE_METHOD(AdjusterFixture, "Adjuster: display", "[Style][Adjuster]")
  {
    auto &div = ById(u8"div");

    SECTION("Blockification")
    {
      CHECK(Adjusted(u8"display: inline; float: left", &div)->Display() == DisplayType::Block);
      CHECK(Adjusted(u8"display: inline-flex; position: absolute", &div)->Display() == DisplayType::Flex);
      CHECK(Adjusted(u8"display: inline-table; position: fixed", &div)->Display() == DisplayType::Table);
      CHECK(Adjusted(u8"display: table-cell; float: right", &div)->Display() == DisplayType::Block);
      CHECK(Adjusted(u8"display: inline", &div)->Display() == DisplayType::Inline);
      // The document element is always block-level.
      CHECK(Adjusted(u8"display: inline-grid", Document->DocumentElement().get())->Display()
            == DisplayType::Grid);
      // <legend> is blockified.
      CHECK(Adjusted(u8"display: inline", &ById(u8"legend"))->Display() == DisplayType::Block);

      // A flex or grid container blockifies its children and stops them floating.
      auto flex = Adjusted(u8"display: flex");
      auto child = Adjusted(u8"display: inline; float: left", &div, flex.get());
      CHECK(child->Display() == DisplayType::Block);
      CHECK(child->Floating() == Float::None);

      // The parent box style is the one that blockifies, not the parent style.
      auto block = Adjusted(u8"display: block");
      CHECK(Adjusted(u8"display: inline", &div, block.get(), flex.get())->Display() == DisplayType::Block);
      CHECK(Adjusted(u8"display: inline", &div, flex.get(), block.get())->Display() == DisplayType::Inline);
    }

    SECTION("Ruby")
    {
      // Ruby display types are only parsed in user agent sheets.
      Mode = css::ParserMode::UASheet;

      // Only ruby elements may have ruby display types.
      CHECK(Adjusted(u8"display: ruby", &div)->Display() == DisplayType::Inline);
      CHECK(Adjusted(u8"display: block ruby", &div)->Display() == DisplayType::Block);
      CHECK(Adjusted(u8"display: ruby", &ById(u8"ruby"))->Display() == DisplayType::Ruby);
      CHECK(Adjusted(u8"display: ruby-text", &div)->Display() == DisplayType::Inline);
      CHECK(Adjusted(u8"display: ruby-text", &ById(u8"rt"))->Display() == DisplayType::RubyAnnotation);

      // Children of ruby boxes are inlinified, unless they're out of flow.
      auto ruby = Adjusted(u8"display: ruby", &ById(u8"ruby"));
      CHECK(Adjusted(u8"display: block", &div, ruby.get())->Display() == DisplayType::InlineBlock);
      CHECK(Adjusted(u8"display: flex", &div, ruby.get())->Display() == DisplayType::InlineFlex);
      CHECK(Adjusted(u8"display: block; position: absolute", &div, ruby.get())->Display()
            == DisplayType::Block);

      // Ruby boxes are isolated.
      CHECK(ruby->GetUnicodeBidi() == UnicodeBidi::Isolate);
      CHECK(Adjusted(u8"display: ruby; unicode-bidi: bidi-override", &ById(u8"ruby"))->GetUnicodeBidi()
            == UnicodeBidi::IsolateOverride);

      // Ruby text doesn't float or position.
      auto rt = Adjusted(u8"float: left; position: relative", &ById(u8"rt"));
      CHECK(rt->Floating() == Float::None);
      CHECK(rt->GetPosition() == PositionType::Static);
    }

    SECTION("display: contents")
    {
      CHECK(Adjusted(u8"display: contents", &div)->Display() == DisplayType::Contents);
      // Elements that can't be unboxed don't generate boxes.
      CHECK(Adjusted(u8"display: contents", &ById(u8"img"))->Display() == DisplayType::None);
      CHECK(Adjusted(u8"display: contents", &ById(u8"textarea"))->Display() == DisplayType::None);
      CHECK(Adjusted(u8"display: contents", &ById(u8"svg"))->Display() == DisplayType::None);
      // The document element is a block.
      CHECK(Adjusted(u8"display: contents", Document->DocumentElement().get())->Display()
            == DisplayType::Block);
      // Without an element, only ::before and ::after may have display: contents.
      CHECK(Adjusted(u8"display: contents")->Display() == DisplayType::None);
    }

    SECTION("Table parts")
    {
      auto row = Adjusted(u8"display: table-row; position: relative; writing-mode: vertical-rl", &div);
      CHECK(row->GetPosition() == PositionType::Static);
      // writing-mode doesn't apply to rows.
      CHECK(row->GetWritingMode().ComputedWritingMode() == StyleWritingMode::HorizontalTb);
      CHECK(Adjusted(u8"display: table-cell; position: relative", &div)->GetPosition()
            == PositionType::Relative);

      // Tables reset the -webkit- text-align values.
      CHECK(Adjusted(u8"text-align: -webkit-center", &ById(u8"table"))->GetTextAlign() == TextAlign::Start);
      CHECK(Adjusted(u8"text-align: -webkit-center", &div)->GetTextAlign() == TextAlign::WebKitCenter);
    }

    SECTION("Writing modes")
    {
      // An inline box in a different writing mode from its parent becomes an inline block.
      CHECK(Adjusted(u8"display: inline; writing-mode: vertical-lr", &div)->Display()
            == DisplayType::InlineBlock);
      CHECK(Adjusted(u8"display: inline", &div)->Display() == DisplayType::Inline);
    }

    SECTION("Grid auto flow")
    {
      CHECK(Adjusted(u8"display: grid", &div)->GetGridAutoFlow().GetDirection()
            == GridAutoFlow::Direction::Row);
      CHECK(Adjusted(u8"display: grid; grid-auto-flow: column", &div)->GetGridAutoFlow().GetDirection()
            == GridAutoFlow::Direction::Column);
    }
  }

  TEST_CASE_METHOD(AdjusterFixture, "Adjuster: z-index, overflow and stacking", "[Style][Adjuster]")
  {
    auto &div = ById(u8"div");

    SECTION("Used z-index")
    {
      // z-index only applies to positioned boxes...
      CHECK(Adjusted(u8"z-index: 5", &div)->UsedZIndex().IsAuto());
      CHECK(Adjusted(u8"z-index: 5; position: relative", &div)->UsedZIndex() == ZIndex {5});
      // ... and flex and grid items.
      auto flex = Adjusted(u8"display: flex");
      CHECK(Adjusted(u8"z-index: 5", &div, flex.get())->UsedZIndex() == ZIndex {5});

      // Auto becomes 0 for stacking contexts and the document element.
      CHECK(Adjusted(u8"opacity: 0.5", &div)->UsedZIndex() == ZIndex {0});
      CHECK(Adjusted(u8"transform: scale(2)", &div)->UsedZIndex() == ZIndex {0});
      CHECK(Adjusted(u8"position: fixed", &div)->UsedZIndex() == ZIndex {0});
      CHECK(Adjusted(u8"", Document->DocumentElement().get())->UsedZIndex() == ZIndex {0});
      CHECK(Adjusted(u8"position: relative", &div)->UsedZIndex().IsAuto());
    }

    SECTION("The builder's adjustments")
    {
      // The builder already sets the used z-index that doesn't depend on the element or parent.
      CHECK(Build(u8"opacity: 0.5")->UsedZIndex() == ZIndex {0});
      CHECK(Build(u8"z-index: 3; position: absolute")->UsedZIndex() == ZIndex {3});
      CHECK(Build(u8"z-index: 3")->UsedZIndex().IsAuto());
    }

    SECTION("Overflow")
    {
      auto style = Adjusted(u8"overflow-x: hidden", &div);
      CHECK(style->OverflowY() == Overflow::Auto);
      style = Adjusted(u8"overflow-y: scroll; overflow-x: clip", &div);
      CHECK(style->OverflowX() == Overflow::Hidden);
      style = Adjusted(u8"overflow-x: clip", &div);
      CHECK(style->OverflowX() == Overflow::Clip);
      CHECK(style->OverflowY() == Overflow::Visible);

      // Tables only support hidden and visible.
      style = Adjusted(u8"display: table; overflow: scroll", &div);
      CHECK(style->OverflowX() == Overflow::Visible);
      CHECK(style->OverflowY() == Overflow::Visible);
      // Conflicting values become visible.
      style = Adjusted(u8"display: table; overflow-x: hidden; overflow-y: auto", &div);
      CHECK(style->OverflowX() == Overflow::Visible);
      CHECK(style->OverflowY() == Overflow::Visible);
      style = Adjusted(u8"display: table; overflow: hidden", &div);
      CHECK(style->OverflowX() == Overflow::Hidden);
      CHECK(style->OverflowY() == Overflow::Hidden);

      // Text areas treat visible as auto.
      style = Adjusted(u8"", &ById(u8"textarea"));
      CHECK(style->OverflowX() == Overflow::Auto);
      CHECK(style->OverflowY() == Overflow::Auto);
    }

    SECTION("Flattening 3D transforms")
    {
      CHECK_FALSE(Adjusted(u8"transform-style: preserve-3d", &div)->TransformStyleForcedToFlat());
      CHECK(Adjusted(u8"transform-style: preserve-3d; opacity: 0.5", &div)->TransformStyleForcedToFlat());
      CHECK(Adjusted(u8"transform-style: preserve-3d; overflow: hidden", &div)->TransformStyleForcedToFlat());
    }

    SECTION("Effective transparency")
    {
      auto transparent = Adjusted(u8"opacity: 0", &div);
      CHECK(transparent->IsEffectivelyTransparent());
      CHECK(Adjusted(u8"opacity: 1", &div, transparent.get())->IsEffectivelyTransparent());
      CHECK_FALSE(Adjusted(u8"opacity: 0.1", &div)->IsEffectivelyTransparent());
    }
  }

  TEST_CASE_METHOD(AdjusterFixture, "Adjuster: inherited used values", "[Style][Adjuster]")
  {
    auto &div = ById(u8"div");

    SECTION("Text decorations in effect")
    {
      auto parent = Adjusted(u8"text-decoration-line: underline", &div);
      CHECK(parent->TextDecorationLineInEffect() == TextDecorationLine {TextDecorationLine::Flag::Underline});

      auto child = Adjusted(u8"text-decoration-line: overline", &div, parent.get());
      CHECK(
        child->TextDecorationLineInEffect()
        == TextDecorationLine {{TextDecorationLine::Flag::Underline, TextDecorationLine::Flag::Overline}});

      // Floats, out-of-flow boxes, inline blocks and outermost <svg> elements don't inherit them.
      CHECK(Adjusted(u8"text-decoration-line: overline; float: left", &div, parent.get())
              ->TextDecorationLineInEffect()
            == TextDecorationLine {TextDecorationLine::Flag::Overline});
      CHECK(Adjusted(u8"display: inline-block", &div, parent.get())->TextDecorationLineInEffect().IsNone());
      CHECK(Adjusted(u8"", &ById(u8"svg"), parent.get())->TextDecorationLineInEffect().IsNone());
      CHECK(Adjusted(u8"", &ById(u8"circle"), parent.get())->TextDecorationLineInEffect()
            == TextDecorationLine {TextDecorationLine::Flag::Underline});
    }

    SECTION("Touch action")
    {
      using TouchActions = EnumSet<boo::TouchAction>;
      auto parent = Adjusted(u8"touch-action: pan-x pinch-zoom", &div);
      CHECK(ConvertToPlatform(parent->UsedTouchAction())
            == TouchActions {boo::TouchAction::PanX, boo::TouchAction::PinchZoom});

      // The used touch action is the intersection with the ancestors'...
      auto child = Adjusted(u8"touch-action: pan-x pan-y", &div, parent.get());
      CHECK(ConvertToPlatform(child->UsedTouchAction()) == TouchActions {boo::TouchAction::PanX});
      CHECK(Adjusted(u8"touch-action: pan-y", &div, parent.get())->UsedTouchAction().IsNone());
      CHECK(Adjusted(u8"", &div, parent.get())->UsedTouchAction() == parent->UsedTouchAction());
      // ... up to the nearest scroll container.
      CHECK(Adjusted(u8"overflow: auto", &div, parent.get())->UsedTouchAction().IsAuto());
    }

    SECTION("Legacy justify-items")
    {
      auto parent = Adjusted(u8"justify-items: legacy left", &div);
      auto child = Adjusted(u8"", &div, parent.get());
      CHECK(child->GetJustifyItems() == parent->GetJustifyItems());
      CHECK_FALSE(Adjusted(u8"justify-items: center", &div, parent.get())->GetJustifyItems()
                  == parent->GetJustifyItems());
    }
  }

  TEST_CASE_METHOD(AdjusterFixture, "Adjuster: elements", "[Style][Adjuster]")
  {
    auto &div = ById(u8"div");

    SECTION("Password fields")
    {
      CHECK(Adjusted(u8"", &ById(u8"password"))->GetTextSecurity() == TextSecurity::Disc);
      CHECK(Adjusted(u8"", &div)->GetTextSecurity() == TextSecurity::None);
    }

    SECTION("Marquees")
    {
      auto style = Adjusted(u8"white-space: pre-wrap; text-align: center", &ById(u8"marquee"));
      CHECK(style->GetWhiteSpaceCollapse() == WhiteSpaceCollapse::Collapse);
      CHECK(style->GetTextWrapMode() == TextWrapMode::NoWrap);
      CHECK(style->GetTextAlign() == TextAlign::Start);
    }

    SECTION("hidden=until-found")
    {
      CHECK_FALSE(Adjusted(u8"", &div)->AutoRevealsWhenFound());
      REQUIRE_FALSE(div.SetAttribute(u8"hidden", u8"Until-Found").HasException());
      CHECK(Adjusted(u8"", &div)->AutoRevealsWhenFound());
      REQUIRE_FALSE(div.SetAttribute(u8"hidden", u8"").HasException());
      CHECK_FALSE(Adjusted(u8"", &div)->AutoRevealsWhenFound());
    }

    SECTION("inert")
    {
      CHECK_FALSE(Adjusted(u8"", &div)->EffectiveInert());
      REQUIRE_FALSE(div.SetAttribute(u8"inert", u8"").HasException());
      CHECK(Adjusted(u8"", &div)->EffectiveInert());
      // The inert attribute only applies to HTML elements.
      REQUIRE_FALSE(ById(u8"circle").SetAttribute(u8"inert", u8"").HasException());
      CHECK_FALSE(Adjusted(u8"", &ById(u8"circle"))->EffectiveInert());
    }

    SECTION("SVG")
    {
      auto &svg = ById(u8"svg");
      auto &inner = ById(u8"inner");
      auto &nested = ById(u8"nested");
      CHECK(DynamicDowncast<svg::SVGElement>(svg)->IsOutermostSVGSVGElement());
      CHECK_FALSE(DynamicDowncast<svg::SVGElement>(svg)->IsInnerSVGSVGElement());
      CHECK_FALSE(DynamicDowncast<svg::SVGElement>(inner)->IsOutermostSVGSVGElement());
      CHECK(DynamicDowncast<svg::SVGElement>(inner)->IsInnerSVGSVGElement());
      // An <svg> in a <foreignObject> is an outermost one (and an inner one).
      CHECK(DynamicDowncast<svg::SVGElement>(nested)->IsOutermostSVGSVGElement());
      CHECK_FALSE(DynamicDowncast<svg::SVGElement>(ById(u8"circle"))->IsOutermostSVGSVGElement());

      // Only outermost <svg> elements are positioned.
      CHECK(Adjusted(u8"position: absolute", &svg)->GetPosition() == PositionType::Absolute);
      CHECK(Adjusted(u8"position: absolute", &inner)->GetPosition() == PositionType::Static);
      CHECK(Adjusted(u8"position: absolute", &ById(u8"circle"))->GetPosition() == PositionType::Static);

      // <foreignObject> isn't zoomed again, and is a block.
      auto foreignObject = Adjusted(u8"display: inline; zoom: 2", &ById(u8"foreignObject"));
      CHECK(foreignObject->Display() == DisplayType::Block);
      CHECK(foreignObject->UsedZoom() == 1.0f);
    }
  }

  TEST_CASE_METHOD(AdjusterFixture, "Adjuster: content-visibility", "[Style][Adjuster]")
  {
    // Elements are inline without the user agent sheet.
    auto &div = ById(u8"div");

    auto hidden = Adjusted(u8"display: block; content-visibility: hidden", &div);
    CHECK(hidden->UsedContentVisibility() == ContentVisibility::Hidden);
    CHECK(IsSkippedContentRoot(*hidden, div));
    // Its descendants aren't skipped content roots themselves.
    CHECK(
      Adjusted(u8"display: block; content-visibility: hidden", &div, hidden.get())->UsedContentVisibility()
      == ContentVisibility::Hidden);

    // Size containment, and so skipping, doesn't apply to non-atomic inline boxes...
    auto &span = ById(u8"span");
    auto inlineHidden = Adjusted(u8"display: inline; content-visibility: hidden", &span);
    CHECK_FALSE(IsSkippedContentRoot(*inlineHidden, span));
    CHECK(inlineHidden->UsedContentVisibility() == ContentVisibility::Visible);
    // ... but does to replaced ones.
    auto &img = ById(u8"img");
    CHECK(img.IsReplaced());
    CHECK(IsSkippedContentRoot(*Adjusted(u8"display: inline; content-visibility: hidden", &img), img));
    CHECK_FALSE(div.IsReplaced());
    CHECK(DynamicDowncast<svg::SVGElement>(ById(u8"svg"))->IsReplaced());
    CHECK_FALSE(ById(u8"inner").IsReplaced());

    // No element is relevant to the user yet, so `auto` skips its contents.
    auto autoVisibility = Adjusted(u8"display: block; content-visibility: auto", &div);
    CHECK(autoVisibility->UsedContentVisibility() == ContentVisibility::Auto);
    CHECK(autoVisibility->ContainIntrinsicWidth().HasAuto());
    CHECK(autoVisibility->ContainIntrinsicHeight().HasAuto());

    CHECK(ShouldApplyLayoutContainment(*Adjusted(u8"display: block; contain: layout", &div), div));
    CHECK_FALSE(ShouldApplyLayoutContainment(*Adjusted(u8"contain: layout; display: table-row", &div), div));
    CHECK(ShouldApplyPaintContainment(*Adjusted(u8"contain: paint; display: table-cell", &div), div));
    CHECK(ShouldApplyStyleContainment(*Adjusted(u8"display: block; content-visibility: auto", &div), div));
    CHECK(ShouldApplyInlineSizeContainment(*Adjusted(u8"display: block; contain: inline-size", &div), div));
    CHECK_FALSE(ShouldApplySizeContainment(*Adjusted(u8"contain: size; display: table", &div), div));
  }

  TEST_CASE_METHOD(AdjusterFixture, "Adjuster: first-letter, first-line and the document element",
                   "[Style][Adjuster]")
  {
    auto firstLetter = Build(u8"display: block; float: left");
    firstLetter->SetPseudoElementIdentifier(PseudoElementIdentifier {PseudoElementType::FirstLetter});
    Adjuster::AdjustFirstLetterStyle(*firstLetter);
    CHECK(firstLetter->Display() == DisplayType::Block);
    firstLetter = Build(u8"display: block");
    firstLetter->SetPseudoElementIdentifier(PseudoElementIdentifier {PseudoElementType::FirstLetter});
    Adjuster::AdjustFirstLetterStyle(*firstLetter);
    CHECK(firstLetter->Display() == DisplayType::Inline);

    auto firstLine = Build(u8"display: block");
    Adjuster::AdjustFirstLineStyle(*firstLine);
    CHECK(firstLine->Display() == DisplayType::Block);
    firstLine->SetPseudoElementIdentifier(PseudoElementIdentifier {PseudoElementType::FirstLine});
    Adjuster::AdjustFirstLineStyle(*firstLine);
    CHECK(firstLine->Display() == DisplayType::Inline);

    // The document element's used writing mode and direction (propagated from <body>) are restored to its
    // computed ones.
    CHECK_FALSE(Adjuster::RestoreUsedDocumentElementStyleToComputed(*Build(u8"")));
    auto used = Build(u8"");
    std::ignore = used->SetWritingMode(StyleWritingMode::VerticalRl);
    auto restored = Adjuster::RestoreUsedDocumentElementStyleToComputed(*used);
    REQUIRE(restored);
    CHECK(restored->GetWritingMode().ComputedWritingMode() == StyleWritingMode::HorizontalTb);
    auto explicitlySet = Build(u8"writing-mode: vertical-rl");
    CHECK(Adjuster::RestoreUsedDocumentElementStyleToComputed(*explicitlySet)
            ->GetWritingMode()
            .ComputedWritingMode()
          == StyleWritingMode::VerticalRl);

    auto root = Build(u8"");
    CHECK_FALSE(Adjuster::AdjustEventListenerRegionTypesForRootStyle(*root, *Document));
    CHECK(root->EventListenerRegionTypes().IsEmpty());
  }

  TEST_CASE_METHOD(AdjusterFixture, "Change: determining changes", "[Style][Change]")
  {
    auto base = Build(u8"display: block; color: red");
    CHECK(DetermineChanges(*base, *Build(u8"display: block; color: red")).IsEmpty());

    // Colour is inherited through the fast path.
    CHECK(DetermineChanges(*base, *Build(u8"display: block; color: blue"))
          == EnumSet<Change> {Change::FastPathInherited});
    CHECK(DetermineChanges(*base, *Build(u8"display: block; color: red; font-size: 30px"))
            .Contains(Change::Inherited));
    CHECK(DetermineChanges(*base, *Build(u8"display: block; color: red; margin-top: 3px"))
          == EnumSet<Change> {Change::NonInherited});

    // Changing display needs a new renderer.
    CHECK(DetermineChanges(*base, *Build(u8"display: inline; color: red"))
          == EnumSet<Change> {Change::NonInherited, Change::Renderer});
    CHECK(DetermineChanges(*base, *Build(u8"display: block; color: red; column-span: all"))
            .Contains(Change::Renderer));

    // Query container changes affect descendants.
    CHECK(DetermineChanges(*base, *Build(u8"display: block; color: red; container-type: size"))
            .Contains(Change::Container));

    CHECK(InheritedChanges() == EnumSet<Change> {Change::Inherited, Change::FastPathInherited});
  }
}
