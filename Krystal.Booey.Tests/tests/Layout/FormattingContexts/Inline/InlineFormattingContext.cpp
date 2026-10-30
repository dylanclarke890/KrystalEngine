#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingContext.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Block/BlockFormattingState.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/FormattingContext.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineContentCache.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextBreakingPositionCache.hpp"
#include "Krystal.Booey/Layout/LayoutState.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxInlines.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInitialContainingBlock.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::tests
{
  using layout::tests::RegisterAhem;
  using layout::tests::RootBoxOf;
  using style::tests::ResolverFixture;

  namespace
  {
    /// @brief Zero edges for every inline-level box under @p box (inline boxes, line breaks, atomic boxes),
    /// as the box geometry updater sets up before inline layout.
    void InitializeInlineLevelGeometries(layout::LayoutState &layoutState,
                                         const layout::ElementBox &box) noexcept
    {
      for (auto *child = box.FirstChild(); child; child = child->NextSibling())
      {
        if (child->IsInlineTextBox())
        {
          continue;
        }
        auto &geometry = layoutState.EnsureGeometryForBox(*child);
        geometry.SetHorizontalMargin({});
        geometry.SetVerticalMargin({});
        geometry.SetBorder({});
        geometry.SetPadding({});
        geometry.SetContentBoxSize({});
        if (auto *elementBox = DynamicDowncast<layout::ElementBox>(*child))
        {
          InitializeInlineLevelGeometries(layoutState, *elementBox);
        }
      }
    }

    /// @brief A layout state whose formatting context callback leaves the boxes as they are: the tests give
    /// atomic inline boxes their geometry up front, as laying them out would.
    KRYS_NODISCARD UniquePtr<layout::LayoutState> MakeLayoutState(ResolverFixture &fixture) noexcept
    {
      auto &renderView = *fixture.Document->GetRenderView();
      return CreateUnique<layout::LayoutState>(
        *fixture.Document, renderView.GetInitialContainingBlock(), layout::LayoutState::Type::Secondary,
        [](const layout::ElementBox &, Maybe<LayoutUnit>, Maybe<LayoutUnit>, layout::LayoutState &) {},
        nullptr, nullptr, nullptr);
    }

    /// @brief Lays out the inline content of a block with id @p id in a @p width wide content box.
    class InlineLayout
    {
    private:
      UniquePtr<layout::LayoutState> _ownedLayoutState;
      layout::LayoutState &_layoutState;
      const layout::ElementBox &_root;
      layout::BlockLayoutState _blockLayoutState;
      layout::InlineFormattingContext _formattingContext;
      LayoutUnit _width;

    public:
      InlineLayout(ResolverFixture &fixture, dom::DOMStringView id, LayoutUnit width,
                   Maybe<layout::BlockLayoutState::LineClamp> lineClamp = null) noexcept
          : _ownedLayoutState(MakeLayoutState(fixture)), _layoutState(*_ownedLayoutState),
            _root(RootBoxOf(fixture, id)),
            _blockLayoutState(_layoutState.EnsureBlockFormattingState(_root).GetPlacedFloats(), {},
                              lineClamp),
            _formattingContext(_root, _layoutState, _blockLayoutState), _width(width)
      {
        auto &initialContainingBlock = layout::FormattingContext::GetInitialContainingBlock(_root);
        auto &initialContainingBlockGeometry = _layoutState.EnsureGeometryForBox(initialContainingBlock);
        initialContainingBlockGeometry.SetContentBoxSize({800_lu, 600_lu});

        auto &rootGeometry = _layoutState.EnsureGeometryForBox(_root);
        rootGeometry.SetTopLeft({});
        rootGeometry.SetContentBoxSize({width, 0_lu});
        rootGeometry.SetHorizontalMargin({});
        rootGeometry.SetVerticalMargin({});
        rootGeometry.SetBorder({});
        rootGeometry.SetPadding({});
        InitializeInlineLevelGeometries(_layoutState, _root);
      }

      KRYS_NODISCARD layout::InlineFormattingContext &FormattingContext() noexcept
      {
        return _formattingContext;
      }

      KRYS_NODISCARD layout::BoxGeometry &GeometryOf(ResolverFixture &fixture, dom::DOMStringView id) noexcept
      {
        auto *box = fixture.ById(id).Renderer()->LayoutBox();
        REQUIRE(box);
        return _layoutState.EnsureGeometryForBox(*box);
      }

      KRYS_NODISCARD layout::InlineLayoutResult Layout() noexcept
      {
        auto constraints = layout::ConstraintsForInlineContent {
          layout::ConstraintsForInFlowContent {{0_lu, _width}, 0_lu}, 0_lu, {_width, 0_lu}};
        return _formattingContext.Layout(constraints);
      }
    };

    /// @brief A display box: its text (or kind) and visual rect.
    struct DisplayBox
    {
      utf8_string Text;
      FloatRect Rect;

      KRYS_NODISCARD friend bool operator==(const DisplayBox &, const DisplayBox &) noexcept = default;
    };

    KRYS_NODISCARD List<DisplayBox> Describe(const inline_display::Boxes &boxes) noexcept
    {
      List<DisplayBox> descriptions;
      for (auto &box : boxes)
      {
        auto text = [&]() -> utf8_string
        {
          if (box.IsTextOrSoftLineBreak())
          {
            return utf8_string {box.GetText().RenderedContent()};
          }
          if (box.IsRootInlineBox())
          {
            return u8"<root>";
          }
          if (box.IsNonRootInlineBox())
          {
            return u8"<inline>";
          }
          if (box.IsLineBreakBox())
          {
            return u8"<br>";
          }
          if (box.IsAtomicInlineBox())
          {
            return u8"<atomic>";
          }
          return u8"?";
        }();
        descriptions.push_back({krys::move(text), box.VisualRectIgnoringBlockDirection()});
      }
      return descriptions;
    }

    KRYS_NODISCARD List<FloatRect> LineBoxRects(const inline_display::Lines &lines) noexcept
    {
      List<FloatRect> rects;
      for (auto &line : lines)
      {
        rects.push_back(line.LineBoxRect());
      }
      return rects;
    }

    KRYS_NODISCARD List<utf8_string> LineTexts(const inline_display::Content &content) noexcept
    {
      // The lines' boxes follow each other; the first box indexes are only set when the content is handed to
      // the renderers.
      List<utf8_string> texts;
      auto firstBoxIndex = size_t {0};
      for (auto &line : content.LineList)
      {
        auto text = utf8_string {};
        auto lineEnd = firstBoxIndex + line.BoxCount();
        for (auto index = std::exchange(firstBoxIndex, lineEnd); index < lineEnd; ++index)
        {
          auto &box = content.BoxList[index];
          if (box.IsText())
          {
            text += box.GetText().RenderedContent();
          }
        }
        texts.push_back(krys::move(text));
      }
      return texts;
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "InlineFormattingContext: line layout",
                   "[Layout][Inline][InlineFormattingContext]")
  {
    RegisterAhem();
    layout::TextBreakingPositionCache::Singleton().Clear();
    // Ahem's glyphs are 1em squares, 0.8em above the baseline and 0.2em below, with no line gap: at 10px,
    // each character is 10px wide and a line is 10px tall.
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      div { font: 10px Ahem }
      #center { text-align: center }
      #big { font-size: 20px }
      #ib { display: inline-block }
      #ellipsis { white-space: nowrap; overflow: hidden; text-overflow: ellipsis }
    </style><body>
    <div id=simple>ab cd</div>
    <div id=wrap>ab cd ef</div>
    <div id=br>ab<br>cd</div>
    <div id=center>ab</div>
    <div id=span>ab<span id=s>cd</span>ef</div>
    <div id=mixed>ab<span id=big>cd</span></div>
    <div id=atomic>ab<span id=ib></span></div>
    <div id=rtl dir=rtl>ab cd</div>
    <div id=ellipsis>abcdef</div>
    <div id=clamp>ab cd ef</div>
    </body>)html"};
    (void)fixture.Document->UpdateLayout();

    SECTION("Text that fits makes one line")
    {
      auto layout = InlineLayout {fixture, u8"simple", 300_lu};
      auto result = layout.Layout();

      CHECK(result.ContentRange == layout::InlineLayoutResult::Range::Full);
      CHECK_FALSE(result.DidDiscardContent);
      REQUIRE(result.DisplayContent.LineList.size() == 1);
      auto &line = result.DisplayContent.LineList[0];
      CHECK(line.LineBoxRect() == FloatRect {0, 0, 300, 10});
      CHECK(line.Baseline() == 8.f);
      CHECK(line.ContentLogicalWidth() == 50.f);
      CHECK(line.HasContentfulInFlowBox());
      CHECK_FALSE(line.HasEllipsis());
      CHECK(line.BoxCount() == 2);
      CHECK(Describe(result.DisplayContent.BoxList)
            == List<DisplayBox> {{u8"<root>", {0, 0, 50, 10}}, {u8"ab cd", {0, 0, 50, 10}}});
    }

    SECTION("Text wraps at the available width, and trailing spaces are trimmed")
    {
      auto layout = InlineLayout {fixture, u8"wrap", 35_lu};
      auto result = layout.Layout();

      CHECK(LineTexts(result.DisplayContent) == List<utf8_string> {u8"ab", u8"cd", u8"ef"});
      CHECK(LineBoxRects(result.DisplayContent.LineList)
            == List<FloatRect> {{0, 0, 35, 10}, {0, 10, 35, 10}, {0, 20, 35, 10}});
      CHECK(layout.FormattingContext().GetLayoutState().LineCount() == 3);
    }

    SECTION("A forced line break ends the line")
    {
      auto layout = InlineLayout {fixture, u8"br", 300_lu};
      auto result = layout.Layout();

      CHECK(LineTexts(result.DisplayContent) == List<utf8_string> {u8"ab", u8"cd"});
      CHECK(Describe(result.DisplayContent.BoxList)
            == List<DisplayBox> {{u8"<root>", {0, 0, 20, 10}},
                                 {u8"ab", {0, 0, 20, 10}},
                                 {u8"<br>", {20, 0, 0, 10}},
                                 {u8"<root>", {0, 10, 20, 10}},
                                 {u8"cd", {0, 10, 20, 10}}});
    }

    SECTION("text-align offsets the content in the line")
    {
      auto layout = InlineLayout {fixture, u8"center", 100_lu};
      auto result = layout.Layout();

      REQUIRE(result.DisplayContent.LineList.size() == 1);
      CHECK(result.DisplayContent.LineList[0].ContentLogicalLeft() == 40.f);
      CHECK(Describe(result.DisplayContent.BoxList)
            == List<DisplayBox> {{u8"<root>", {40, 0, 20, 10}}, {u8"ab", {40, 0, 20, 10}}});
    }

    SECTION("Inline boxes get display boxes, and their padding offsets the content")
    {
      auto layout = InlineLayout {fixture, u8"span", 300_lu};
      layout.GeometryOf(fixture, u8"s").SetPadding({{5_lu, 5_lu}, {0_lu, 0_lu}});
      auto result = layout.Layout();

      // The span's border box is 30px wide (20px of text and 5px of padding on each side) and as tall as its
      // content.
      CHECK(Describe(result.DisplayContent.BoxList)
            == List<DisplayBox> {{u8"<root>", {0, 0, 70, 10}},
                                 {u8"ab", {0, 0, 20, 10}},
                                 {u8"<inline>", {20, 0, 30, 10}},
                                 {u8"cd", {25, 0, 20, 10}},
                                 {u8"ef", {50, 0, 20, 10}}});
      auto &spanGeometry = layout.GeometryOf(fixture, u8"s");
      CHECK(layout::BoxGeometry::BorderBoxTopLeft(spanGeometry) == LayoutPoint {20_lu, 0_lu});
      CHECK(spanGeometry.ContentBoxWidth() == 20_lu);
    }

    SECTION("A larger font stretches the line around the shared baseline")
    {
      auto layout = InlineLayout {fixture, u8"mixed", 300_lu};
      auto result = layout.Layout();

      // The 20px span has a 16px ascent and a 4px descent: the line is 20px tall, with the baseline 16px
      // down.
      REQUIRE(result.DisplayContent.LineList.size() == 1);
      auto &line = result.DisplayContent.LineList[0];
      CHECK(line.LineBoxRect() == FloatRect {0, 0, 300, 20});
      CHECK(line.Baseline() == 16.f);
      CHECK(Describe(result.DisplayContent.BoxList)
            == List<DisplayBox> {{u8"<root>", {0, 8, 60, 10}},
                                 {u8"ab", {0, 8, 20, 10}},
                                 {u8"<inline>", {20, 0, 40, 20}},
                                 {u8"cd", {20, 0, 40, 20}}});
    }

    SECTION("Atomic inline boxes sit on the baseline with their margin box bottom")
    {
      auto layout = InlineLayout {fixture, u8"atomic", 300_lu};
      layout.GeometryOf(fixture, u8"ib").SetContentBoxSize({30_lu, 30_lu});
      auto result = layout.Layout();

      // The inline-block's baseline is its bottom margin edge: 30px above the baseline and the root inline
      // box's 2px descent below it.
      REQUIRE(result.DisplayContent.LineList.size() == 1);
      CHECK(result.DisplayContent.LineList[0].LineBoxRect() == FloatRect {0, 0, 300, 32});
      CHECK(Describe(result.DisplayContent.BoxList)
            == List<DisplayBox> {
              {u8"<root>", {0, 22, 50, 10}}, {u8"ab", {0, 22, 20, 10}}, {u8"<atomic>", {20, 0, 30, 30}}});
      auto &atomicGeometry = layout.GeometryOf(fixture, u8"ib");
      CHECK(layout::BoxGeometry::BorderBoxTopLeft(atomicGeometry) == LayoutPoint {20_lu, 0_lu});
    }

    SECTION("Right-to-left content starts at the right edge")
    {
      auto layout = InlineLayout {fixture, u8"rtl", 100_lu};
      auto result = layout.Layout();

      REQUIRE(result.DisplayContent.LineList.size() == 1);
      CHECK_FALSE(result.DisplayContent.LineList[0].IsLeftToRightInlineDirection());
      CHECK(Describe(result.DisplayContent.BoxList)
            == List<DisplayBox> {{u8"<root>", {50, 0, 50, 10}}, {u8"ab cd", {50, 0, 50, 10}}});
    }

    SECTION("text-overflow: ellipsis truncates overflowing content")
    {
      auto layout = InlineLayout {fixture, u8"ellipsis", 35_lu};
      auto result = layout.Layout();

      REQUIRE(result.DisplayContent.LineList.size() == 1);
      auto &line = result.DisplayContent.LineList[0];
      CHECK(line.ContentLogicalWidth() == 60.f);
      REQUIRE(line.HasEllipsis());
      CHECK(line.GetEllipsis()->EllipsisType == inline_display::Line::Ellipsis::Type::Inline);
      REQUIRE(result.DisplayContent.BoxList.size() == 2);
      auto &text = result.DisplayContent.BoxList[1].GetText();
      REQUIRE(text.PartiallyVisibleContentLength());
      CHECK(*text.PartiallyVisibleContentLength() < 4);
    }

    SECTION("Legacy line-clamp truncates the lines after the clamp")
    {
      auto layout =
        InlineLayout {fixture, u8"clamp", 35_lu, layout::BlockLayoutState::LineClamp {2, false, true}};
      auto result = layout.Layout();

      // Legacy clamping lays out every line, but the ones after the clamp are truncated, and the clamped line
      // gets a block ellipsis.
      auto &lines = result.DisplayContent.LineList;
      REQUIRE(lines.size() == 3);
      CHECK_FALSE(lines[0].IsFullyTruncatedInBlockDirection());
      CHECK_FALSE(lines[1].IsFullyTruncatedInBlockDirection());
      CHECK(lines[2].IsFullyTruncatedInBlockDirection());
      CHECK_FALSE(lines[0].HasEllipsis());
      REQUIRE(lines[1].HasEllipsis());
      CHECK(lines[1].GetEllipsis()->EllipsisType == inline_display::Line::Ellipsis::Type::Block);
      CHECK(layout.FormattingContext().GetLayoutState().LegacyClampedLineIndex() == 1u);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "InlineFormattingContext: intrinsic widths",
                   "[Layout][Inline][InlineFormattingContext]")
  {
    RegisterAhem();
    layout::TextBreakingPositionCache::Singleton().Clear();
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      div { font: 10px Ahem }
      #nowrap { white-space: nowrap }
    </style><body>
    <div id=words>ab cdef gh</div>
    <div id=nowrap>ab cd</div>
    <div id=br>abc<br>de</div>
    </body>)html"};
    (void)fixture.Document->UpdateLayout();

    SECTION("The minimum is the longest word, the maximum the whole line")
    {
      auto layout = InlineLayout {fixture, u8"words", 300_lu};
      CHECK(layout.FormattingContext().MinimumContentSize() == 40_lu);
      CHECK(layout.FormattingContext().MaximumContentSize() == 100_lu);
      CHECK(layout.FormattingContext().MinimumMaximumContentSize() == std::pair {40_lu, 100_lu});
    }

    SECTION("Content that can't wrap has the same minimum and maximum")
    {
      auto layout = InlineLayout {fixture, u8"nowrap", 300_lu};
      CHECK(layout.FormattingContext().MinimumMaximumContentSize() == std::pair {50_lu, 50_lu});
    }

    SECTION("A forced line break splits the maximum")
    {
      auto layout = InlineLayout {fixture, u8"br", 300_lu};
      CHECK(layout.FormattingContext().MinimumMaximumContentSize() == std::pair {30_lu, 30_lu});
    }

    SECTION("The cached single line is reused when it fits")
    {
      auto layout = InlineLayout {fixture, u8"words", 300_lu};
      (void)layout.FormattingContext().MaximumContentSize();
      auto result = layout.Layout();
      CHECK(LineTexts(result.DisplayContent) == List<utf8_string> {u8"ab cdef gh"});
    }
  }
}
