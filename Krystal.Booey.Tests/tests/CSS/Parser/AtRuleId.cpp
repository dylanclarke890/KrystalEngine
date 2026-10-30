#include "Krystal.Booey/CSS/Parser/AtRuleId.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("AtRuleIdFromName", "[CSS][Parser][AtRuleId]")
  {
    SECTION("Every at-rule")
    {
      CHECK(AtRuleIdFromName(u8"annotation") == AtRuleId::Annotation);
      CHECK(AtRuleIdFromName(u8"character-variant") == AtRuleId::CharacterVariant);
      CHECK(AtRuleIdFromName(u8"charset") == AtRuleId::Charset);
      CHECK(AtRuleIdFromName(u8"container") == AtRuleId::Container);
      CHECK(AtRuleIdFromName(u8"counter-style") == AtRuleId::CounterStyle);
      CHECK(AtRuleIdFromName(u8"font-face") == AtRuleId::FontFace);
      CHECK(AtRuleIdFromName(u8"font-feature-values") == AtRuleId::FontFeatureValues);
      CHECK(AtRuleIdFromName(u8"font-palette-values") == AtRuleId::FontPaletteValues);
      CHECK(AtRuleIdFromName(u8"function") == AtRuleId::Function);
      CHECK(AtRuleIdFromName(u8"import") == AtRuleId::Import);
      CHECK(AtRuleIdFromName(u8"keyframes") == AtRuleId::Keyframes);
      CHECK(AtRuleIdFromName(u8"layer") == AtRuleId::Layer);
      CHECK(AtRuleIdFromName(u8"media") == AtRuleId::Media);
      CHECK(AtRuleIdFromName(u8"namespace") == AtRuleId::Namespace);
      CHECK(AtRuleIdFromName(u8"ornaments") == AtRuleId::Ornaments);
      CHECK(AtRuleIdFromName(u8"page") == AtRuleId::Page);
      CHECK(AtRuleIdFromName(u8"position-try") == AtRuleId::PositionTry);
      CHECK(AtRuleIdFromName(u8"property") == AtRuleId::Property);
      CHECK(AtRuleIdFromName(u8"scope") == AtRuleId::Scope);
      CHECK(AtRuleIdFromName(u8"starting-style") == AtRuleId::StartingStyle);
      CHECK(AtRuleIdFromName(u8"styleset") == AtRuleId::Styleset);
      CHECK(AtRuleIdFromName(u8"stylistic") == AtRuleId::Stylistic);
      CHECK(AtRuleIdFromName(u8"supports") == AtRuleId::Supports);
      CHECK(AtRuleIdFromName(u8"swash") == AtRuleId::Swash);
      CHECK(AtRuleIdFromName(u8"view-transition") == AtRuleId::ViewTransition);
    }

    SECTION("ASCII case-insensitive")
    {
      CHECK(AtRuleIdFromName(u8"MEDIA") == AtRuleId::Media);
      CHECK(AtRuleIdFromName(u8"Font-Face") == AtRuleId::FontFace);
      CHECK(AtRuleIdFromName(u8"sTaRtInG-sTyLe") == AtRuleId::StartingStyle);
    }

    SECTION("Unknown names")
    {
      CHECK(AtRuleIdFromName(u8"") == AtRuleId::Invalid);
      CHECK(AtRuleIdFromName(u8"medi") == AtRuleId::Invalid);
      CHECK(AtRuleIdFromName(u8"mediaa") == AtRuleId::Invalid);
      CHECK(AtRuleIdFromName(u8"viewport") == AtRuleId::Invalid);
      // Non-standard names aren't supported.
      CHECK(AtRuleIdFromName(u8"-webkit-keyframes") == AtRuleId::Invalid);
      CHECK(AtRuleIdFromName(u8"-internal-base-appearance") == AtRuleId::Invalid);
    }
  }
}
