#include "Krystal.Text/LocaleToScriptMapping.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Text/Script.hpp"

namespace krys::text::tests
{
  TEST_CASE("ScriptNameToCode", "[Core][LocaleToScriptMapping]")
  {
    CHECK(ScriptNameToCode(u8"latn") == Script::Latin);
    CHECK(ScriptNameToCode(u8"Arab") == Script::Arabic);
    CHECK(ScriptNameToCode(u8"HANT") == Script::TraditionalHan);
    // Japanese scripts are treated as one.
    CHECK(ScriptNameToCode(u8"hira") == Script::KatakanaOrHiragana);
    CHECK(ScriptNameToCode(u8"zzzz") == Script::Unknown);
    CHECK(ScriptNameToCode(u8"xyzw") == Script::Invalid);
    CHECK(ScriptNameToCode(u8"latin") == Script::Invalid);
    CHECK(ScriptNameToCode(u8"la1n") == Script::Invalid);
    CHECK(ScriptNameToCode(u8"") == Script::Invalid);
  }

  TEST_CASE("LocaleToScriptCode", "[Core][LocaleToScriptMapping]")
  {
    CHECK(LocaleToScriptCode(u8"en") == Script::Latin);
    CHECK(LocaleToScriptCode(u8"en-GB") == Script::Latin);
    CHECK(LocaleToScriptCode(u8"RU") == Script::Cyrillic);
    CHECK(LocaleToScriptCode(u8"ja") == Script::KatakanaOrHiragana);
    CHECK(LocaleToScriptCode(u8"zh") == Script::Han);
    // Regions, then script subtags, refine the language.
    CHECK(LocaleToScriptCode(u8"zh-HK") == Script::TraditionalHan);
    CHECK(LocaleToScriptCode(u8"zh_tw") == Script::TraditionalHan);
    CHECK(LocaleToScriptCode(u8"zh-Hant") == Script::TraditionalHan);
    CHECK(LocaleToScriptCode(u8"sr-Latn") == Script::Latin);
    CHECK(LocaleToScriptCode(u8"xx-Arab") == Script::Arabic);
    CHECK(LocaleToScriptCode(u8"xx-Arab-YY") == Script::Arabic);
    CHECK(LocaleToScriptCode(u8"xx-Zzzz") == Script::Common);
    CHECK(LocaleToScriptCode(u8"xx") == Script::Common);
    CHECK(LocaleToScriptCode(u8"") == Script::Common);
  }
}
