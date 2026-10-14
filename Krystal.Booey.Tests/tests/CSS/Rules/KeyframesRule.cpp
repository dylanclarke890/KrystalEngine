#include "Krystal.Booey/CSS/Rules/KeyframesRule.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/RuleTestUtils.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<KeyframeRule> CreateKeyframe(const CSSOMString &keyText, utf8_stringview declarations)
    {
      auto keyframe = KeyframeRule::Create(ImmutableDeclarations(declarations));
      REQUIRE(keyframe->SetKeyText(keyText));
      return keyframe;
    }
  }

  TEST_CASE_METHOD(ValueFixture, "KeyframeRule", "[CSS][Rules][KeyframesRule]")
  {
    SECTION("Type")
    {
      auto keyframe = KeyframeRule::Create(ImmutableDeclarations(u8"color: red"));
      CHECK(keyframe->Type() == RuleType::Keyframe);
      CHECK(keyframe->IsKeyframeRule());
      CHECK(Is<KeyframeRule>(static_cast<Rule &>(*keyframe)));
      CHECK(keyframe->Keys().empty());
    }

    SECTION("Keys")
    {
      Pair<ValueId, double> keys[] = {
        {ValueId::Invalid, 0.0}, {ValueId::Cover, 0.5}, {ValueId::ExitCrossing, 1.0}};
      auto keyframe = KeyframeRule::Create(keys, ImmutableDeclarations(u8"color: red"));
      REQUIRE(keyframe->Keys().size() == 3);
      CHECK(keyframe->Keys()[1] == KeyframeRule::Key {ValueId::Cover, 0.5});
      CHECK(ToNarrow(keyframe->KeyText()) == "0%,cover 50%,exit-crossing 100%");
    }

    SECTION("SetKey")
    {
      auto keyframe = KeyframeRule::Create(ImmutableDeclarations(u8"color: red"));
      keyframe->SetKey({ValueId::Invalid, 0.125});
      CHECK(ToNarrow(keyframe->KeyText()) == "12.5%");
    }

    SECTION("SetKeyText")
    {
      auto keyframe = KeyframeRule::Create(ImmutableDeclarations(u8"color: red"));
      CHECK(keyframe->SetKeyText(u8"from, entry 25%, to"));
      CHECK(ToNarrow(keyframe->KeyText()) == "0%,entry 25%,100%");
      CHECK(keyframe->SetKeyText(u8"contain 10%, entry-crossing 20%, exit 30%"));
      CHECK(ToNarrow(keyframe->KeyText()) == "contain 10%,entry-crossing 20%,exit 30%");

      // An invalid key list leaves the keys unchanged.
      CHECK_FALSE(keyframe->SetKeyText(u8"50"));
      CHECK_FALSE(keyframe->SetKeyText(u8"10%,"));
      CHECK_FALSE(keyframe->SetKeyText(u8""));
      CHECK(ToNarrow(keyframe->KeyText()) == "contain 10%,entry-crossing 20%,exit 30%");
    }

    SECTION("CssText")
    {
      CHECK(ToNarrow(CreateKeyframe(u8"50%", u8"color: red; width: 1px")->CssText())
            == "50% { color: red; width: 1px; }");
      CHECK(
        ToNarrow(KeyframeRule::Create(MutableStyleProperties::Create(ParserMode::HTMLStandard))->CssText())
        == " { }");
    }

    SECTION("MutableProperties")
    {
      auto keyframe = CreateKeyframe(u8"0%", u8"color: red");
      CHECK_FALSE(keyframe->Properties().IsMutable());
      AddDeclaration(keyframe->MutableProperties(), u8"width: 1px", DeclarationTestContext());
      CHECK(AsText(keyframe->Properties()) == "color: red; width: 1px;");
    }
  }

  TEST_CASE_METHOD(ValueFixture, "KeyframesRule", "[CSS][Rules][KeyframesRule]")
  {
    auto rule = KeyframesRule::Create(CSSOMStringAtom(u8"spin"));
    CHECK(rule->Type() == RuleType::Keyframes);
    CHECK(rule->IsKeyframesRule());
    CHECK(rule->Name() == CSSOMStringAtom(u8"spin"));
    rule->SetName(CSSOMStringAtom(u8"turn"));
    CHECK(rule->Name() == CSSOMStringAtom(u8"turn"));

    rule->ParserAppendKeyframe(CreateKeyframe(u8"from", u8"color: red"));
    rule->ParserAppendKeyframe(nullptr);
    rule->ParserAppendKeyframe(CreateKeyframe(u8"50%, cover 10%", u8"color: green"));
    rule->WrapperAppendKeyframe(CreateKeyframe(u8"from", u8"color: blue"));
    REQUIRE(rule->Keyframes().size() == 3);

    SECTION("FindKeyframeIndex finds the last match")
    {
      CHECK(rule->FindKeyframeIndex(u8"0%") == 2u);
      CHECK(rule->FindKeyframeIndex(u8"50%, cover 10%") == 1u);
      // The keys must match in order.
      CHECK_FALSE(rule->FindKeyframeIndex(u8"cover 10%, 50%"));
      CHECK_FALSE(rule->FindKeyframeIndex(u8"50%"));
      CHECK_FALSE(rule->FindKeyframeIndex(u8"nonsense"));
    }

    SECTION("WrapperRemoveKeyframe")
    {
      rule->WrapperRemoveKeyframe(2);
      CHECK(rule->FindKeyframeIndex(u8"from") == 0u);
      rule->ShrinkToFit();
      CHECK(rule->Keyframes().size() == 2);
    }

    SECTION("Copies share the keyframes")
    {
      Ref<Rule> base = rule;
      auto copy = base->Copy();
      auto &keyframes = Downcast<KeyframesRule>(*copy);
      REQUIRE(keyframes.Keyframes().size() == 3);
      CHECK(keyframes.Keyframes()[0].get() == rule->Keyframes()[0].get());
      CHECK(keyframes.Name() == CSSOMStringAtom(u8"turn"));
    }
  }
}
