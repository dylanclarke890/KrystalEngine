#include "Krystal.Booey/Core/LocaleToScriptMapping.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::tests
{
  TEST_CASE("ScriptNameToCode", "[Core][LocaleToScriptMapping]")
  {
    CHECK(ScriptNameToCode(u8"latn") == USCRIPT_LATIN);
    CHECK(ScriptNameToCode(u8"Arab") == USCRIPT_ARABIC);
    CHECK(ScriptNameToCode(u8"HANT") == USCRIPT_TRADITIONAL_HAN);
    // Japanese scripts are treated as one.
    CHECK(ScriptNameToCode(u8"hira") == USCRIPT_KATAKANA_OR_HIRAGANA);
    CHECK(ScriptNameToCode(u8"zzzz") == USCRIPT_UNKNOWN);
    CHECK(ScriptNameToCode(u8"xyzw") == USCRIPT_INVALID_CODE);
    CHECK(ScriptNameToCode(u8"latin") == USCRIPT_INVALID_CODE);
    CHECK(ScriptNameToCode(u8"la1n") == USCRIPT_INVALID_CODE);
    CHECK(ScriptNameToCode(u8"") == USCRIPT_INVALID_CODE);
  }

  TEST_CASE("LocaleToScriptCode", "[Core][LocaleToScriptMapping]")
  {
    CHECK(LocaleToScriptCode(u8"en") == USCRIPT_LATIN);
    CHECK(LocaleToScriptCode(u8"en-GB") == USCRIPT_LATIN);
    CHECK(LocaleToScriptCode(u8"RU") == USCRIPT_CYRILLIC);
    CHECK(LocaleToScriptCode(u8"ja") == USCRIPT_KATAKANA_OR_HIRAGANA);
    CHECK(LocaleToScriptCode(u8"zh") == USCRIPT_HAN);
    // Regions, then script subtags, refine the language.
    CHECK(LocaleToScriptCode(u8"zh-HK") == USCRIPT_TRADITIONAL_HAN);
    CHECK(LocaleToScriptCode(u8"zh_tw") == USCRIPT_TRADITIONAL_HAN);
    CHECK(LocaleToScriptCode(u8"zh-Hant") == USCRIPT_TRADITIONAL_HAN);
    CHECK(LocaleToScriptCode(u8"sr-Latn") == USCRIPT_LATIN);
    CHECK(LocaleToScriptCode(u8"xx-Arab") == USCRIPT_ARABIC);
    CHECK(LocaleToScriptCode(u8"xx-Arab-YY") == USCRIPT_ARABIC);
    CHECK(LocaleToScriptCode(u8"xx-Zzzz") == USCRIPT_COMMON);
    CHECK(LocaleToScriptCode(u8"xx") == USCRIPT_COMMON);
    CHECK(LocaleToScriptCode(u8"") == USCRIPT_COMMON);
  }
}
