#include "Krystal.Booey/Style/Difference.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Style/InlineTextBoxStyle.hpp"

namespace krys::boo::style::tests
{
  TEST_CASE_METHOD(css::tests::ValueFixture, "Difference: what a style change needs", "[Style][Difference]")
  {
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      div { width: 10px; height: 10px }
    </style>
    <div id=plain></div><div id=plain2></div>
    <div id=width style="width: 20px"></div>
    <div id=background style="background-color: red"></div>
    <div id=background2 style="background-color: blue"></div>
    <div id=translucent style="opacity: 0.5"></div><div id=translucent2 style="opacity: 0.6"></div>
    <div id=rotated style="transform: rotate(10deg)"></div><div id=rotated2 style="transform: rotate(20deg)"></div>
    <div id=abs style="position: absolute; left: 10px"></div><div id=abs2 style="position: absolute; left: 20px"></div>
    <div id=rel style="position: relative; left: 10px"></div><div id=rel2 style="position: relative; left: 20px"></div>
    <div id=nopointer style="pointer-events: none"></div>
    <div id=z style="position: relative; z-index: 1"></div><div id=z2 style="position: relative; z-index: 2"></div>
    <div id=hidden style="visibility: hidden; background-color: red"></div>
    <div id=hidden2 style="visibility: hidden; background-color: blue"></div>
    <div id=decoration style="text-decoration-color: red"></div>
    <div id=border style="border: 1px solid red"></div><div id=border2 style="border: 1px solid blue"></div>
    <div id=border3 style="border: 1px solid red"></div>
    <div id=underline style="text-decoration-line: underline"></div>
    <div id=wavy style="text-decoration-line: underline; text-decoration-style: wavy"></div>
    <div id=under style="text-decoration-line: underline; text-underline-position: under"></div>)html"};
    fixture.Document->UpdateStyleIfNeeded();

    auto styleOf = [&](utf8_stringview id) noexcept -> const RenderStyle &
    {
      return *fixture.ById(id).GetRenderStyle();
    };
    auto difference = [&](utf8_stringview a, utf8_stringview b) noexcept
    {
      return GetDifference(styleOf(a), styleOf(b));
    };

    SECTION("Equal styles")
    {
      CHECK(difference(u8"plain", u8"plain2") == DifferenceResult::Equal);
      CHECK(difference(u8"plain", u8"plain2").ContextSensitiveProperties.IsEmpty());
      CHECK(difference(u8"plain", u8"plain") == Difference {});
    }

    SECTION("Box size changes need layout")
    {
      CHECK(difference(u8"plain", u8"width") == DifferenceResult::Layout);
    }

    SECTION("Opacity")
    {
      CHECK(difference(u8"plain", u8"translucent") == DifferenceResult::Layout);
      auto diff = difference(u8"translucent", u8"translucent2");
      CHECK(diff == DifferenceResult::Equal);
      CHECK(diff.ContextSensitiveProperties == EnumSet {DifferenceContextSensitiveProperty::Opacity});
    }

    SECTION("Transforms are context sensitive")
    {
      CHECK(difference(u8"plain", u8"rotated") == DifferenceResult::Layout);
      auto diff = difference(u8"rotated", u8"rotated2");
      CHECK(diff == DifferenceResult::Equal);
      CHECK(diff.ContextSensitiveProperties.Contains(DifferenceContextSensitiveProperty::Transform));
    }

    SECTION("Moving an absolutely positioned box only needs out-of-flow movement")
    {
      CHECK(difference(u8"abs", u8"abs2") == DifferenceResult::LayoutOutOfFlowMovementOnly);
      CHECK(difference(u8"rel", u8"rel2") == DifferenceResult::Layout);
    }

    SECTION("Layer, paint and compositing changes")
    {
      CHECK(difference(u8"z", u8"z2") == DifferenceResult::RepaintLayer);
      CHECK(difference(u8"background", u8"background2") == DifferenceResult::Repaint);
      CHECK(difference(u8"plain", u8"decoration") == DifferenceResult::Repaint);
      CHECK(difference(u8"plain", u8"nopointer") == DifferenceResult::RecompositeLayer);
    }

    SECTION("Invisible boxes don't need repainting")
    {
      CHECK(difference(u8"hidden", u8"hidden2") == DifferenceResult::Equal);
    }

    SECTION("Results are ordered")
    {
      CHECK(Difference {DifferenceResult::Repaint} < Difference {DifferenceResult::Layout});
      CHECK(Difference {DifferenceResult::Layout} > DifferenceResult::RepaintLayer);
    }

    SECTION("Borders and layer repaints")
    {
      CHECK(BorderIsEquivalentForPainting(styleOf(u8"border"), styleOf(u8"border3")));
      CHECK_FALSE(BorderIsEquivalentForPainting(styleOf(u8"border"), styleOf(u8"border2")));
      CHECK(DifferenceRequiresLayerRepaint(styleOf(u8"background"), styleOf(u8"background2"), false));
      CHECK_FALSE(DifferenceRequiresLayerRepaint(styleOf(u8"plain"), styleOf(u8"plain2"), true));
    }

    SECTION("Text decorations")
    {
      CHECK_FALSE(IsAlignedForUnder(styleOf(u8"underline")));
      CHECK(IsAlignedForUnder(styleOf(u8"under")));
      CHECK(InkOverflowForDecorations(styleOf(u8"plain")).IsEmpty());
      // A plain underline at 16px stays within the line box; a wavy one doesn't.
      CHECK(InkOverflowForDecorations(styleOf(u8"underline")).IsEmpty());
      auto wavy = InkOverflowForDecorations(styleOf(u8"wavy"));
      CHECK(wavy.Left == LayoutUnit(1));
      CHECK(wavy.Bottom == LayoutUnit(2));
      CHECK(difference(u8"plain", u8"under") == DifferenceResult::Layout);
    }

    SECTION("Renderers have initialized styles")
    {
      auto *renderer = fixture.ById(u8"plain").Renderer();
      REQUIRE(renderer);
      CHECK(renderer->HasInitializedStyle());
    }
  }
}
