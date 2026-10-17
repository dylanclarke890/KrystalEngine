#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineItemsBuilder.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineSoftLineBreakItem.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineTextItem.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextBreakingPositionCache.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::tests
{
  using layout::tests::RegisterAhem;
  using layout::tests::RootBoxOf;
  using style::tests::ResolverFixture;

  namespace
  {
    using Type = layout::InlineItem::Type;

    /// @brief A text item's content, or the type of a non-text item.
    KRYS_NODISCARD utf8_string Describe(const layout::InlineItem &item) noexcept
    {
      if (auto *textItem = DynamicDowncast<layout::InlineTextItem>(item))
      {
        return utf8_string {textItem->Content()};
      }
      switch (item.GetType())
      {
        case Type::HardLineBreak:        return u8"<br>";
        case Type::SoftLineBreak:        return u8"<\\n>";
        case Type::WordBreakOpportunity: return u8"<wbr>";
        case Type::AtomicInlineBox:      return u8"<atomic>";
        case Type::InlineBoxStart:       return u8"<start>";
        case Type::InlineBoxEnd:         return u8"<end>";
        case Type::Float:                return u8"<float>";
        case Type::Block:                return u8"<block>";
        case Type::Opaque:               return u8"<opaque>";
        case Type::Text:                 break;
      }
      return u8"?";
    }

    KRYS_NODISCARD List<utf8_string> Describe(const layout::InlineItemList &items) noexcept
    {
      List<utf8_string> descriptions;
      for (auto &item : items)
      {
        descriptions.push_back(Describe(item));
      }
      return descriptions;
    }

    KRYS_NODISCARD List<UBiDiLevel> BidiLevels(const layout::InlineItemList &items) noexcept
    {
      List<UBiDiLevel> levels;
      for (auto &item : items)
      {
        levels.push_back(item.BidiLevel());
      }
      return levels;
    }

    KRYS_NODISCARD const layout::InlineTextItem &TextItem(const layout::InlineItemList &items,
                                                          size_t index) noexcept
    {
      REQUIRE(index < items.size());
      REQUIRE(Is<layout::InlineTextItem>(items[index]));
      return Downcast<layout::InlineTextItem>(items[index]);
    }

    constexpr UBiDiLevel Opaque = layout::InlineItem::OpaqueBidiLevel;
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "InlineItemsBuilder: items",
                   "[Layout][Inline][InlineItemsBuilder]")
  {
    RegisterAhem();
    layout::TextBreakingPositionCache::Singleton().Clear();
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      div { font: 10px Ahem }
      #ib { display: inline-block }
      #f { float: left }
      #abs { position: absolute }
      #pre { white-space: pre }
      #breakSpaces { white-space: break-spaces }
      #manual { hyphens: manual }
      #none { hyphens: none }
    </style><body>
    <div id=root>ab cd<span id=s>ef</span><br>gh<wbr><span id=ib>x</span><span id=f>y</span><span id=abs>z</span><span id=empty></span></div>
    <div id=pre>a
b	c  d</div>
    <div id=breakSpaces>a  b</div>
    <div id=manual>ex&shy;ample</div>
    <div id=none>ex&shy;ample</div>
    <div id=block>a<span>b<div>c</div>d</span>e</div>
    </body>)html"};
    (void)fixture.Document->UpdateLayout();

    SECTION("Inline content becomes a flat list")
    {
      auto &root = RootBoxOf(fixture, u8"root");
      layout::InlineContentCache cache;
      layout::InlineItemsBuilder(cache, root).Build({});

      auto &inlineItems = cache.GetInlineItems();
      auto &items = inlineItems.Content();
      CHECK(Describe(items)
            == List<utf8_string> {u8"ab", u8" ", u8"cd", u8"<start>", u8"ef", u8"<end>", u8"<br>", u8"gh",
                                  u8"<wbr>", u8"<atomic>", u8"<float>", u8"<opaque>", u8"<start>",
                                  u8"<end>"});

      CHECK(TextItem(items, 0).Start() == 0);
      CHECK(TextItem(items, 0).Length() == 2);
      CHECK_FALSE(TextItem(items, 0).IsWhitespace());
      CHECK(TextItem(items, 1).IsWhitespace());
      CHECK(TextItem(items, 1).IsWordSeparator());
      CHECK(TextItem(items, 1).IsFullyTrimmable());
      CHECK(TextItem(items, 2).Start() == 3);
      CHECK(TextItem(items, 2).End() == 5);

      // Ahem's glyphs are 1em squares; a collapsible space is one space wide.
      CHECK(TextItem(items, 0).Width() == 20.f);
      CHECK(TextItem(items, 1).Width() == 10.f);
      CHECK(TextItem(items, 2).Width() == 20.f);
      CHECK(TextItem(items, 4).Width() == 20.f);

      CHECK_FALSE(inlineItems.RequiresVisualReordering());
      CHECK_FALSE(inlineItems.HasTextAndLineBreakOnlyContent());
      CHECK(inlineItems.InlineBoxCount() == 2);
      CHECK(inlineItems.HasInlineBoxes());
      CHECK_FALSE(inlineItems.IsPopulatedFromCache());

      for (auto &item : items)
      {
        CHECK(item.BidiLevel() == UBIDI_DEFAULT_LTR);
      }
    }

    SECTION("Text and line breaks only")
    {
      auto &root = RootBoxOf(fixture, u8"pre");
      layout::InlineContentCache cache;
      layout::InlineItemsBuilder(cache, root).Build({});
      CHECK(cache.GetInlineItems().HasTextAndLineBreakOnlyContent());
      CHECK_FALSE(cache.GetInlineItems().HasInlineBoxes());
    }

    SECTION("Preserved new lines are soft line breaks; preserved tabs aren't word separators")
    {
      auto &root = RootBoxOf(fixture, u8"pre");
      layout::InlineContentCache cache;
      layout::InlineItemsBuilder(cache, root).Build({});

      auto &items = cache.GetInlineItems().Content();
      CHECK(Describe(items) == List<utf8_string> {u8"a", u8"<\\n>", u8"b", u8"\t", u8"c", u8"  ", u8"d"});
      CHECK(Downcast<layout::InlineSoftLineBreakItem>(items[1]).Position() == 1);
      CHECK_FALSE(TextItem(items, 3).IsWordSeparator());
      CHECK_FALSE(TextItem(items, 3).IsFullyTrimmable());
      CHECK(TextItem(items, 5).IsWordSeparator());
      // Tabs depend on their position, so a box with tabs has no preserved white space measured up front.
      CHECK_FALSE(TextItem(items, 3).Width());
      CHECK_FALSE(TextItem(items, 5).Width());
      CHECK(TextItem(items, 4).Width() == 10.f);
    }

    SECTION("break-spaces makes an item of each space")
    {
      auto &root = RootBoxOf(fixture, u8"breakSpaces");
      layout::InlineContentCache cache;
      layout::InlineItemsBuilder(cache, root).Build({});
      CHECK(Describe(cache.GetInlineItems().Content()) == List<utf8_string> {u8"a", u8" ", u8" ", u8"b"});
    }

    SECTION("Soft hyphens")
    {
      layout::InlineContentCache manualCache;
      layout::InlineItemsBuilder(manualCache, RootBoxOf(fixture, u8"manual")).Build({});
      auto &manual = manualCache.GetInlineItems().Content();
      REQUIRE(manual.size() == 2);
      CHECK(TextItem(manual, 0).Content() == u8"ex\u00AD");
      CHECK(TextItem(manual, 0).HasTrailingSoftHyphen());
      CHECK_FALSE(TextItem(manual, 1).HasTrailingSoftHyphen());

      // Without hyphenation the pieces stay together.
      layout::InlineContentCache noneCache;
      layout::InlineItemsBuilder(noneCache, RootBoxOf(fixture, u8"none")).Build({});
      auto &none = noneCache.GetInlineItems().Content();
      REQUIRE(none.size() == 1);
      CHECK(TextItem(none, 0).Length() == 9);
      CHECK_FALSE(TextItem(none, 0).HasTrailingSoftHyphen());
    }

    SECTION("Blocks in inline content")
    {
      layout::InlineContentCache cache;
      layout::InlineItemsBuilder(cache, RootBoxOf(fixture, u8"block")).Build({});
      CHECK(Describe(cache.GetInlineItems().Content())
            == List<utf8_string> {u8"a", u8"<start>", u8"b", u8"<block>", u8"d", u8"<end>", u8"e"});
    }

    SECTION("Partial builds replace the items from the damage on")
    {
      auto &root = RootBoxOf(fixture, u8"root");
      layout::InlineContentCache cache;
      layout::InlineItemsBuilder(cache, root).Build({});
      auto expected = Describe(cache.GetInlineItems().Content());

      // From the start of an inline box's content, and from the middle of a text box.
      layout::InlineItemsBuilder(cache, root).Build({4, 0});
      CHECK(Describe(cache.GetInlineItems().Content()) == expected);
      CHECK(cache.GetInlineItems().InlineBoxCount() == 2);
      layout::InlineItemsBuilder(cache, root).Build({2, 0});
      CHECK(Describe(cache.GetInlineItems().Content()) == expected);
      CHECK(TextItem(cache.GetInlineItems().Content(), 2).Width() == 20.f);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "InlineItemsBuilder: bidi",
                   "[Layout][Inline][InlineItemsBuilder]")
  {
    RegisterAhem();
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      div { font: 10px Ahem }
    </style><body>
    <div id=mixed>abc שלום def</div>
    <div id=split>abcשלום</div>
    <div id=rtl dir=rtl>abc שלום</div>
    <div id=span>abc<span>שלום</span></div>
    <div id=embed>a<span dir=rtl style="unicode-bidi: embed">b</span>c</div>
    <div id=empty><span style="direction: rtl"></span></div>
    <div id=plaintext dir=rtl style="unicode-bidi: plaintext">abc def</div>
    </body>)html"};
    (void)fixture.Document->UpdateLayout();

    auto build = [&](dom::DOMStringView id, layout::InlineContentCache &cache)
    {
      layout::InlineItemsBuilder(cache, RootBoxOf(fixture, id)).Build({});
    };

    SECTION("Right-to-left runs get odd levels")
    {
      layout::InlineContentCache cache;
      build(u8"mixed", cache);
      auto &items = cache.GetInlineItems().Content();
      CHECK(Describe(items)
            == List<utf8_string> {u8"abc", u8" ", u8"\u05E9\u05DC\u05D5\u05DD", u8" ", u8"def"});
      CHECK(BidiLevels(items) == List<UBiDiLevel> {0, 0, 1, 0, 0});
      CHECK(TextItem(items, 2).Direction() == TextDirection::RTL);
      CHECK(TextItem(items, 2).Start() == 4);
      CHECK(TextItem(items, 2).Length() == 8);
      CHECK(cache.GetInlineItems().RequiresVisualReordering());
    }

    SECTION("Text items are split at bidi boundaries")
    {
      layout::InlineContentCache cache;
      build(u8"split", cache);
      auto &items = cache.GetInlineItems().Content();
      CHECK(Describe(items) == List<utf8_string> {u8"abc", u8"\u05E9\u05DC\u05D5\u05DD"});
      CHECK(BidiLevels(items) == List<UBiDiLevel> {0, 1});
      // The split items are measured on their own.
      CHECK(TextItem(items, 0).Width() == 30.f);
    }

    SECTION("A right-to-left block")
    {
      layout::InlineContentCache cache;
      build(u8"rtl", cache);
      CHECK(BidiLevels(cache.GetInlineItems().Content()) == List<UBiDiLevel> {2, 1, 1});
    }

    SECTION("Inline boxes with content are opaque")
    {
      layout::InlineContentCache cache;
      build(u8"span", cache);
      auto &items = cache.GetInlineItems().Content();
      CHECK(Describe(items)
            == List<utf8_string> {u8"abc", u8"<start>", u8"\u05E9\u05DC\u05D5\u05DD", u8"<end>"});
      CHECK(BidiLevels(items) == List<UBiDiLevel> {0, Opaque, 1, Opaque});
    }

    SECTION("Embeddings")
    {
      layout::InlineContentCache cache;
      build(u8"embed", cache);
      auto &items = cache.GetInlineItems().Content();
      CHECK(Describe(items) == List<utf8_string> {u8"a", u8"<start>", u8"b", u8"<end>", u8"c"});
      // The embedding's control characters give its start and end a position in the paragraph: the start
      // takes the level of the embedding's content (left-to-right text in a right-to-left embedding is at the
      // next even level), the end the level after it.
      CHECK(BidiLevels(items) == List<UBiDiLevel> {0, 2, 2, 0, 0});
    }

    SECTION("No content to reorder")
    {
      layout::InlineContentCache cache;
      build(u8"empty", cache);
      auto &items = cache.GetInlineItems().Content();
      CHECK(Describe(items) == List<utf8_string> {u8"<start>", u8"<end>"});
      CHECK(BidiLevels(items) == List<UBiDiLevel> {UBIDI_DEFAULT_LTR, UBIDI_DEFAULT_LTR});
    }

    SECTION("Plaintext left-to-right content isn't reordered")
    {
      layout::InlineContentCache cache;
      build(u8"plaintext", cache);
      CHECK(BidiLevels(cache.GetInlineItems().Content())
            == List<UBiDiLevel> {UBIDI_DEFAULT_LTR, UBIDI_DEFAULT_LTR, UBIDI_DEFAULT_LTR});
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "InlineItemsBuilder: text spacing",
                   "[Layout][Inline][InlineItemsBuilder]")
  {
    RegisterAhem();
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      div { font: 10px Ahem; text-autospace: normal }
      #off { text-autospace: no-autospace }
    </style><body>
    <div id=items>漢a</div>
    <div id=boundary><span>漢</span>a</div>
    <div id=off>漢a</div>
    </body>)html"};
    (void)fixture.Document->UpdateLayout();

    SECTION("Spacing between ideographs and letters in adjacent items is trimmable")
    {
      layout::InlineContentCache cache;
      auto &root = RootBoxOf(fixture, u8"items");
      layout::InlineItemsBuilder(cache, root).Build({});
      auto &items = cache.GetInlineItems().Content();
      CHECK(Describe(items) == List<utf8_string> {u8"\u6F22", u8"a"});
      CHECK(cache.GetInlineItems().HasTextAutospace());

      auto spacing = TextAutospace::TextAutospaceSize(*root.Style().GetFontCascade().PrimaryFont());
      CHECK(spacing > 0);
      auto &trimmable = cache.GetTrimmableTextSpacings();
      REQUIRE(trimmable.size() == 1);
      CHECK(trimmable.at(1) == spacing);
      CHECK(cache.GetInlineBoxBoundaryTextSpacings().empty());
    }

    SECTION("Spacing after an inline box boundary goes on the start of the box that ends")
    {
      layout::InlineContentCache cache;
      layout::InlineItemsBuilder(cache, RootBoxOf(fixture, u8"boundary")).Build({});
      auto &items = cache.GetInlineItems().Content();
      CHECK(Describe(items) == List<utf8_string> {u8"<start>", u8"\u6F22", u8"<end>", u8"a"});
      auto &boundary = cache.GetInlineBoxBoundaryTextSpacings();
      REQUIRE(boundary.size() == 1);
      CHECK(boundary.contains(0));
    }

    SECTION("No spacing without autospace")
    {
      layout::InlineContentCache cache;
      layout::InlineItemsBuilder(cache, RootBoxOf(fixture, u8"off")).Build({});
      CHECK_FALSE(cache.GetInlineItems().HasTextAutospace());
      CHECK(cache.GetTrimmableTextSpacings().empty());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "InlineItemsBuilder: breaking position cache",
                   "[Layout][Inline][InlineItemsBuilder]")
  {
    RegisterAhem();
    auto &breakingPositionCache = layout::TextBreakingPositionCache::Singleton();
    breakingPositionCache.Clear();
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      div { font: 10px Ahem }
      #pre, #lines { white-space: pre }
    </style><body>
    <div id=text>hello world foo</div>
    <div id=same>hello world foo</div>
    <div id=pre>hello world foo</div>
    <div id=short>a b</div>
    <div id=lines>hello
world foo</div>
    </body>)html"};
    (void)fixture.Document->UpdateLayout();
    // Releasing the line layouts layout made caches their breaking positions; start from an empty cache.
    layout::tests::DetachLineLayouts(fixture);
    breakingPositionCache.Clear();

    layout::InlineContentCache cache;
    auto &root = RootBoxOf(fixture, u8"text");
    layout::InlineItemsBuilder(cache, root).Build({});
    auto &items = cache.GetInlineItems().Content();
    REQUIRE(Describe(items) == List<utf8_string> {u8"hello", u8" ", u8"world", u8" ", u8"foo"});
    CHECK_FALSE(cache.GetInlineItems().IsPopulatedFromCache());

    layout::InlineItemsBuilder::PopulateBreakingPositionCache(items);
    auto &textBox = Downcast<layout::InlineTextBox>(*root.FirstChild());
    auto *positions =
      breakingPositionCache.Get({textBox.Content(), layout::TextBreakingPositionContext {textBox.Style()}});
    REQUIRE(positions);
    CHECK(List<size_t>(positions->begin(), positions->end()) == List<size_t> {5, 6, 11, 12, 15});

    SECTION("Text with the same content and breaking style comes from the cache")
    {
      layout::InlineContentCache sameCache;
      layout::InlineItemsBuilder(sameCache, RootBoxOf(fixture, u8"same")).Build({});
      CHECK(sameCache.GetInlineItems().IsPopulatedFromCache());
      CHECK(Describe(sameCache.GetInlineItems().Content()) == Describe(items));
      CHECK(TextItem(sameCache.GetInlineItems().Content(), 2).Width() == 50.f);
    }

    SECTION("Preserved white space breaks differently")
    {
      layout::InlineContentCache preCache;
      layout::InlineItemsBuilder(preCache, RootBoxOf(fixture, u8"pre")).Build({});
      CHECK_FALSE(preCache.GetInlineItems().IsPopulatedFromCache());
    }

    SECTION("Short text isn't cached")
    {
      layout::InlineContentCache shortCache;
      auto &shortRoot = RootBoxOf(fixture, u8"short");
      layout::InlineItemsBuilder(shortCache, shortRoot).Build({});
      layout::InlineItemsBuilder::PopulateBreakingPositionCache(shortCache.GetInlineItems().Content());
      auto &shortBox = Downcast<layout::InlineTextBox>(*shortRoot.FirstChild());
      CHECK_FALSE(breakingPositionCache.Get(
        {shortBox.Content(), layout::TextBreakingPositionContext {shortBox.Style()}}));
    }

    SECTION("Soft line breaks are cached after their new line")
    {
      layout::InlineContentCache linesCache;
      auto &linesRoot = RootBoxOf(fixture, u8"lines");
      layout::InlineItemsBuilder(linesCache, linesRoot).Build({});
      layout::InlineItemsBuilder::PopulateBreakingPositionCache(linesCache.GetInlineItems().Content());
      auto &linesBox = Downcast<layout::InlineTextBox>(*linesRoot.FirstChild());
      auto *linePositions = breakingPositionCache.Get(
        {linesBox.Content(), layout::TextBreakingPositionContext {linesBox.Style()}});
      REQUIRE(linePositions);
      CHECK(Describe(linesCache.GetInlineItems().Content())
            == List<utf8_string> {u8"hello", u8"<\\n>", u8"world", u8" ", u8"foo"});
      CHECK(List<size_t>(linePositions->begin(), linePositions->end()) == List<size_t> {5, 6, 11, 12, 15});
    }

    breakingPositionCache.Clear();
  }

  TEST_CASE("TextBreakingPositionCache", "[Layout][Inline][TextBreakingPositionCache]")
  {
    layout::TextBreakingPositionCache cache;
    auto key = layout::TextBreakingPositionCache::Key {utf8_string {u8"hello world"},
                                                       layout::TextBreakingPositionContext {}};
    CHECK_FALSE(cache.Get(key));

    cache.Set(key, {5, 6, 11});
    REQUIRE(cache.Get(key));
    CHECK(cache.Get(key)->size() == 3);

    auto otherContext = key;
    otherContext.Context.WordBreak = WordBreak::BreakAll;
    CHECK_FALSE(cache.Get(otherContext));
    auto otherLocale = key;
    otherLocale.Context.Locale = StringAtom {u8"ja"};
    CHECK_FALSE(cache.Get(otherLocale));

    cache.Clear();
    CHECK_FALSE(cache.Get(key));
  }
}
