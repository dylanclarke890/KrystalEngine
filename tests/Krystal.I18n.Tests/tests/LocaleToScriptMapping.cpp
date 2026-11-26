#include "Krystal.I18n/LocaleToScriptMapping.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Text/Script.hpp"

namespace krys::i18n::tests
{
  TEST_CASE("ScriptNameToCode", "[I18n][LocaleToScriptMapping]")
  {
    CHECK(ScriptNameToCode(u8"latn") == text::Script::Latin);
    CHECK(ScriptNameToCode(u8"Arab") == text::Script::Arabic);
    CHECK(ScriptNameToCode(u8"HANT") == text::Script::TraditionalHan);
    // Japanese scripts are treated as one.
    CHECK(ScriptNameToCode(u8"hira") == text::Script::KatakanaOrHiragana);
    CHECK(ScriptNameToCode(u8"zzzz") == text::Script::Unknown);
    CHECK(ScriptNameToCode(u8"xyzw") == text::Script::Invalid);
    CHECK(ScriptNameToCode(u8"latin") == text::Script::Invalid);
    CHECK(ScriptNameToCode(u8"la1n") == text::Script::Invalid);
    CHECK(ScriptNameToCode(u8"") == text::Script::Invalid);
  }

  TEST_CASE("LocaleToScriptCode", "[I18n][LocaleToScriptMapping]")
  {
    CHECK(LocaleToScriptCode(u8"en") == text::Script::Latin);
    CHECK(LocaleToScriptCode(u8"en-GB") == text::Script::Latin);
    CHECK(LocaleToScriptCode(u8"RU") == text::Script::Cyrillic);
    CHECK(LocaleToScriptCode(u8"ja") == text::Script::KatakanaOrHiragana);
    CHECK(LocaleToScriptCode(u8"zh") == text::Script::Han);
    // Regions, then script subtags, refine the language.
    CHECK(LocaleToScriptCode(u8"zh-HK") == text::Script::TraditionalHan);
    CHECK(LocaleToScriptCode(u8"zh_tw") == text::Script::TraditionalHan);
    CHECK(LocaleToScriptCode(u8"zh-Hant") == text::Script::TraditionalHan);
    CHECK(LocaleToScriptCode(u8"sr-Latn") == text::Script::Latin);
    CHECK(LocaleToScriptCode(u8"xx-Arab") == text::Script::Arabic);
    CHECK(LocaleToScriptCode(u8"xx-Arab-YY") == text::Script::Arabic);
    CHECK(LocaleToScriptCode(u8"xx-Zzzz") == text::Script::Common);
    CHECK(LocaleToScriptCode(u8"xx") == text::Script::Common);
    CHECK(LocaleToScriptCode(u8"") == text::Script::Common);
  }
}
