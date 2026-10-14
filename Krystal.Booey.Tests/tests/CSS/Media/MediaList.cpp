#include "Krystal.Booey/CSS/Media/MediaList.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSImportRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSMediaRule.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("MediaList: a style sheet's list", "[CSS][CSSOM][MediaList]")
  {
    auto sheet = SheetFromText(u8"a { }");
    auto &media = *sheet->Media();

    REQUIRE(media.Length() == 0);
    REQUIRE(media.Item(0).empty());
    REQUIRE(media.MediaText().empty());
    REQUIRE_FALSE(media.IsSupportedPropertyIndex(0));
    REQUIRE(media.ParentRule() == nullptr);

    media.SetMediaText(u8"SCREEN and (min-width: 100px), print");
    REQUIRE(media.Length() == 2);
    REQUIRE(media.IsSupportedPropertyIndex(1));
    REQUIRE(ToNarrow(media.Item(0)) == "screen and (min-width: 100px)");
    REQUIRE(ToNarrow(media.Item(1)) == "print");
    REQUIRE(media.Item(2).empty());
    REQUIRE(ToNarrow(media.MediaText()) == "screen and (min-width: 100px), print");

    SECTION("appendMedium")
    {
      media.AppendMedium(u8"speech");
      REQUIRE(ToNarrow(media.MediaText()) == "screen and (min-width: 100px), print, speech");

      // Appending an empty string does nothing.
      media.AppendMedium(u8"");
      REQUIRE(media.Length() == 3);

      // A query that doesn't parse becomes `not all`.
      media.AppendMedium(u8"screen and");
      REQUIRE(ToNarrow(media.Item(3)) == "not all");
    }

    SECTION("deleteMedium")
    {
      // Matching is ASCII case-insensitive.
      REQUIRE_FALSE(media.DeleteMedium(u8"PRINT").HasException());
      REQUIRE(ToNarrow(media.MediaText()) == "screen and (min-width: 100px)");

      REQUIRE(ExceptionCodeOf(media.DeleteMedium(u8"print")) == dom::ExceptionCode::NotFoundError);
      REQUIRE(media.Length() == 1);
    }

    SECTION("an invalid media text")
    {
      media.SetMediaText(u8"screen, print and");
      REQUIRE(ToNarrow(media.MediaText()) == "screen, not all");
    }

    SECTION("detaching keeps a copy")
    {
      media.DetachFromParent();
      REQUIRE(media.ParentStyleSheet() == nullptr);
      REQUIRE(ToNarrow(media.MediaText()) == "screen and (min-width: 100px), print");
    }
  }

  TEST_CASE("MediaList: an @media rule's list", "[CSS][CSSOM][MediaList]")
  {
    auto sheet = SheetFromText(u8"@media screen { a { } }");
    auto &rule = RuleAt<CSSMediaRule>(*sheet, 0);
    auto *media = rule.Media();

    REQUIRE(media);
    REQUIRE(media == rule.Media());
    REQUIRE(media->ParentRule() == &rule);
    REQUIRE(media->ParentStyleSheet() == nullptr);
    REQUIRE(ToNarrow(media->MediaText()) == "screen");

    media->AppendMedium(u8"print");
    REQUIRE(RuleText(rule) == "@media screen, print {\n  a { }\n}");

    REQUIRE_FALSE(media->DeleteMedium(u8"screen").HasException());
    REQUIRE(RuleText(rule) == "@media print {\n  a { }\n}");
  }

  TEST_CASE("MediaList: an @import rule's list", "[CSS][CSSOM][MediaList]")
  {
    auto sheet = SheetFromText(u8"@import \"a.css\" screen;");
    auto &rule = RuleAt<CSSImportRule>(*sheet, 0);
    auto &media = rule.Media();

    REQUIRE(&media == &rule.Media());
    REQUIRE(media.ParentRule() == &rule);
    REQUIRE(ToNarrow(media.MediaText()) == "screen");

    media.SetMediaText(u8"print");
    REQUIRE(RuleText(rule) == "@import url(\"a.css\") print;");

    media.SetMediaText(u8"");
    REQUIRE(RuleText(rule) == "@import url(\"a.css\");");
  }

  TEST_CASE("MediaList: a rule's list outlives the rule", "[CSS][CSSOM][MediaList]")
  {
    RefPtr<MediaList> media;
    {
      auto sheet = SheetFromText(u8"@media screen { }");
      media = ShareRefPtr(RuleAt<CSSMediaRule>(*sheet, 0).Media());
      // The sheet is the only owner of the rule wrapper.
    }

    REQUIRE(media->ParentRule() == nullptr);
    REQUIRE(ToNarrow(media->MediaText()) == "screen");
  }
}
