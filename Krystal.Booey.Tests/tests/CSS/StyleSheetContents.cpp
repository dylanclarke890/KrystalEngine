#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/RuleTestUtils.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/CSS/Rules/ImportRule.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<LayerRule> LayerStatement(utf8_stringview name)
    {
      return LayerRule::CreateStatement({{CSSOMStringAtom(name)}});
    }

    KRYS_NODISCARD Ref<ImportRule> Import(utf8_stringview href)
    {
      return ImportRule::Create(CSSOMString(href), {}, {}, {});
    }

    KRYS_NODISCARD Ref<NamespaceRule> Namespace(utf8_stringview prefix, utf8_stringview uri)
    {
      return NamespaceRule::Create(prefix.empty() ? CSSOMStringAtom::Null() : CSSOMStringAtom(prefix),
                                   CSSOMStringAtom(uri));
    }

    /// @brief The rule types of `sheet`, in `RuleAt` order.
    KRYS_NODISCARD List<RuleType> RuleTypes(const StyleSheetContents &sheet)
    {
      List<RuleType> types;
      for (uint32 i = 0; i < sheet.RuleCount(); ++i)
      {
        types.push_back(sheet.RuleAt(i)->Type());
      }
      return types;
    }

    /// @brief A selector list of `count` type selectors, `a, a, ...`.
    KRYS_NODISCARD CSSOMString ManySelectors(uint32 count)
    {
      CSSOMString text = u8"a";
      for (uint32 i = 1; i < count; ++i)
      {
        text += u8",a";
      }
      return text;
    }
  }

  TEST_CASE_METHOD(ValueFixture, "StyleSheetContents::Create", "[CSS][StyleSheetContents]")
  {
    auto sheet = StyleSheetContents::Create();
    CHECK(sheet->IsEmpty());
    CHECK(sheet->RuleCount() == 0);
    CHECK(sheet->GetParserContext().Mode == ParserMode::HTMLStandard);
    CHECK(sheet->DefaultNamespace() == dom::StarAtom());
    CHECK(sheet->OriginalURL().empty());
    CHECK(sheet->OwnerRule() == nullptr);
    CHECK(sheet->ParentStyleSheet() == nullptr);
    CHECK(sheet->RootStyleSheet() == sheet.get());
    CHECK_FALSE(sheet->IsUserStyleSheet());
    CHECK_FALSE(sheet->LoadCompleted());
    CHECK_FALSE(sheet->IsLoading());
    CHECK(sheet->LoadedFromOpaqueSource() == LoadedFromOpaqueSource::No);
    sheet->SetAsLoadedFromOpaqueSource();
    CHECK(sheet->LoadedFromOpaqueSource() == LoadedFromOpaqueSource::Yes);

    auto withURL =
      StyleSheetContents::Create(u8"https://example.com/a.css", ParserContext(ParserMode::HTMLQuirks));
    CHECK(withURL->OriginalURL() == u8"https://example.com/a.css");
    CHECK(withURL->GetParserContext().Mode == ParserMode::HTMLQuirks);

    // A sheet imported by a user sheet is a user sheet.
    sheet->SetIsUserStyleSheet(true);
    auto import = Import(u8"b.css");
    import->SetParentStyleSheet(sheet.get());
    auto imported = StyleSheetContents::Create(import.get(), u8"b.css", sheet->GetParserContext());
    CHECK(imported->IsUserStyleSheet());
    CHECK(imported->OwnerRule() == import.get());
    CHECK(imported->ParentStyleSheet() == sheet.get());
    CHECK(imported->RootStyleSheet() == sheet.get());
    imported->ClearOwnerRule();
    CHECK(imported->ParentStyleSheet() == nullptr);
  }

  TEST_CASE_METHOD(ValueFixture, "StyleSheetContents::ParserAppendRule", "[CSS][StyleSheetContents]")
  {
    auto sheet = StyleSheetContents::Create();

    SECTION("Rules are kept in their lists")
    {
      sheet->ParserAppendRule(LayerStatement(u8"a"));
      sheet->ParserAppendRule(Import(u8"a.css"));
      sheet->ParserAppendRule(Namespace(u8"svg", u8"http://www.w3.org/2000/svg"));
      sheet->ParserAppendRule(Namespace(u8"", u8"http://www.w3.org/1999/xhtml"));
      sheet->ParserAppendRule(CreateStyleRule(u8".a", u8"color: red"));
      sheet->ParserAppendRule(LayerStatement(u8"b"));

      CHECK(sheet->LayerRulesBeforeImportRules().size() == 1);
      CHECK(sheet->ImportRules().size() == 1);
      CHECK(sheet->NamespaceRules().size() == 2);
      CHECK(sheet->ChildRules().size() == 2);
      CHECK(RuleTypes(*sheet)
            == List<RuleType> {RuleType::LayerStatement, RuleType::Import, RuleType::Namespace,
                               RuleType::Namespace, RuleType::Style, RuleType::LayerStatement});

      CHECK(sheet->ImportRules()[0]->ParentStyleSheet() == sheet.get());
      CHECK(sheet->NamespaceURIFromPrefix(CSSOMStringAtom(u8"svg"))
            == CSSOMStringAtom(u8"http://www.w3.org/2000/svg"));
      CHECK(sheet->DefaultNamespace() == CSSOMStringAtom(u8"http://www.w3.org/1999/xhtml"));
      CHECK_FALSE(sheet->IsLoading());
    }

    SECTION("A layer statement after another rule is a child rule")
    {
      sheet->ParserAppendRule(Namespace(u8"svg", u8"http://www.w3.org/2000/svg"));
      sheet->ParserAppendRule(LayerStatement(u8"a"));
      CHECK(sheet->LayerRulesBeforeImportRules().empty());
      CHECK(sheet->ChildRules().size() == 1);

      auto block = StyleSheetContents::Create();
      block->ParserAppendRule(LayerRule::CreateBlock({CSSOMStringAtom(u8"a")}, {}));
      CHECK(block->LayerRulesBeforeImportRules().empty());
      CHECK(block->ChildRules().size() == 1);
    }

    SECTION("Rules with too many selector components are split")
    {
      sheet->ParserAppendRule(CreateStyleRule(ManySelectors(8'193), u8"color: red"));
      REQUIRE(sheet->ChildRules().size() == 2);
      auto &first = Downcast<StyleRule>(*sheet->ChildRules()[0]);
      auto &second = Downcast<StyleRule>(*sheet->ChildRules()[1]);
      CHECK(first.GetSelectorList().ComponentCount() == 8'192);
      CHECK(second.GetSelectorList().ComponentCount() == 1);
      CHECK(first.IsSplitRule());
      CHECK(second.IsLastRuleInSplitRule());

      // Nested rules with too many components are dropped.
      sheet->ParserAppendRule(StyleRuleWithNesting::Create(ImmutableDeclarations(u8"color: red"), false,
                                                           ParseSelectors(ManySelectors(8'193)), {}));
      CHECK(sheet->ChildRules().size() == 2);
      sheet->ParserAppendRule(StyleRuleWithNesting::Create(ImmutableDeclarations(u8"color: red"), false,
                                                           ParseSelectors(ManySelectors(8'192)), {}));
      CHECK(sheet->ChildRules().size() == 3);
    }

    SECTION("ClearRules")
    {
      auto import = Import(u8"a.css");
      sheet->ParserAppendRule(Ref<ImportRule>(import));
      sheet->ParserAppendRule(CreateStyleRule(u8".a", u8"color: red"));
      sheet->ParserSetEncodingFromCharsetRule(u8"utf-8");
      CHECK(sheet->EncodingFromCharsetRule() == u8"utf-8");
      sheet->ClearRules();
      CHECK(sheet->IsEmpty());
      CHECK(sheet->EncodingFromCharsetRule().empty());
      CHECK(import->ParentStyleSheet() == nullptr);
    }
  }

  TEST_CASE_METHOD(ValueFixture, "StyleSheetContents::WrapperInsertRule", "[CSS][StyleSheetContents]")
  {
    auto sheet = StyleSheetContents::Create();
    sheet->SetMutable();

    SECTION("Import rules")
    {
      CHECK(sheet->WrapperInsertRule(CreateStyleRule(u8".a", u8"color: red"), 0));
      // Not after a style rule.
      CHECK_FALSE(sheet->WrapperInsertRule(Import(u8"a.css"), 1));
      CHECK(sheet->WrapperInsertRule(Import(u8"a.css"), 0));
      CHECK(sheet->ImportRules()[0]->ParentStyleSheet() == sheet.get());
      // A style rule can't go before an import.
      CHECK_FALSE(sheet->WrapperInsertRule(CreateStyleRule(u8".b", u8"color: red"), 0));
      CHECK(RuleTypes(*sheet) == List<RuleType> {RuleType::Import, RuleType::Style});
    }

    SECTION("Namespace rules")
    {
      CHECK(sheet->WrapperInsertRule(Namespace(u8"a", u8"urn:a"), 0));
      CHECK(sheet->NamespaceURIFromPrefix(CSSOMStringAtom(u8"a")) == CSSOMStringAtom(u8"urn:a"));
      // A later namespace rule with the same prefix replaces the URI.
      CHECK(sheet->WrapperInsertRule(Namespace(u8"a", u8"urn:b"), 0));
      CHECK(sheet->NamespaceURIFromPrefix(CSSOMStringAtom(u8"a")) == CSSOMStringAtom(u8"urn:b"));
      CHECK(sheet->WrapperInsertRule(CreateStyleRule(u8".a", u8"color: red"), 2));
      // Not once there are other rules.
      CHECK_FALSE(sheet->WrapperInsertRule(Namespace(u8"c", u8"urn:c"), 2));
      CHECK_FALSE(sheet->WrapperInsertRule(Namespace(u8"c", u8"urn:c"), 3));
      CHECK(RuleTypes(*sheet) == List<RuleType> {RuleType::Namespace, RuleType::Namespace, RuleType::Style});
    }

    SECTION("A namespace rule after an import rule")
    {
      CHECK(sheet->WrapperInsertRule(Import(u8"a.css"), 0));
      CHECK(sheet->WrapperInsertRule(Namespace(u8"a", u8"urn:a"), 1));
      CHECK(sheet->WrapperInsertRule(Namespace(u8"b", u8"urn:b"), 2));
      CHECK(sheet->WrapperInsertRule(Namespace(u8"c", u8"urn:c"), 1));
      REQUIRE(sheet->NamespaceRules().size() == 3);
      CHECK(sheet->NamespaceRules()[0]->Prefix() == CSSOMStringAtom(u8"c"));
      CHECK(sheet->NamespaceRules()[1]->Prefix() == CSSOMStringAtom(u8"a"));
      CHECK(sheet->NamespaceRules()[2]->Prefix() == CSSOMStringAtom(u8"b"));
      CHECK_FALSE(sheet->WrapperInsertRule(Namespace(u8"d", u8"urn:d"), 0));
    }

    SECTION("Layer statements")
    {
      CHECK(sheet->WrapperInsertRule(LayerStatement(u8"a"), 0));
      CHECK(sheet->WrapperInsertRule(Import(u8"a.css"), 1));
      CHECK(sheet->WrapperInsertRule(LayerStatement(u8"b"), 0));
      CHECK(sheet->LayerRulesBeforeImportRules().size() == 2);
      // A layer block can't go before an import.
      CHECK_FALSE(sheet->WrapperInsertRule(LayerRule::CreateBlock({CSSOMStringAtom(u8"c")}, {}), 1));
      CHECK(RuleTypes(*sheet)
            == List<RuleType> {RuleType::LayerStatement, RuleType::LayerStatement, RuleType::Import});
    }

    SECTION("Early layer statements become child rules when needed")
    {
      CHECK(sheet->WrapperInsertRule(LayerStatement(u8"a"), 0));
      CHECK(sheet->WrapperInsertRule(CreateStyleRule(u8".a", u8"color: red"), 0));
      CHECK(sheet->LayerRulesBeforeImportRules().empty());
      CHECK(RuleTypes(*sheet) == List<RuleType> {RuleType::Style, RuleType::LayerStatement});
      // So an import can only go first.
      CHECK_FALSE(sheet->WrapperInsertRule(Import(u8"a.css"), 1));
      CHECK(sheet->WrapperInsertRule(Import(u8"a.css"), 0));
    }

    SECTION("Rules with too many selector components are refused")
    {
      CHECK_FALSE(sheet->WrapperInsertRule(CreateStyleRule(ManySelectors(8'193), u8"color: red"), 0));
      CHECK_FALSE(
        sheet->WrapperInsertRule(StyleRuleWithNesting::Create(ImmutableDeclarations(u8"color: red"), false,
                                                              ParseSelectors(ManySelectors(8'193)), {}),
                                 0));
      CHECK(sheet->IsEmpty());
    }
  }

  TEST_CASE_METHOD(ValueFixture, "StyleSheetContents::WrapperDeleteRule", "[CSS][StyleSheetContents]")
  {
    auto sheet = StyleSheetContents::Create();
    sheet->SetMutable();
    sheet->ParserAppendRule(LayerStatement(u8"a"));
    auto import = Import(u8"a.css");
    sheet->ParserAppendRule(Ref<ImportRule>(import));
    sheet->ParserAppendRule(Namespace(u8"a", u8"urn:a"));
    sheet->ParserAppendRule(CreateStyleRule(u8".a", u8"color: red"));

    // A namespace rule can't be deleted while there are other rules.
    CHECK_FALSE(sheet->WrapperDeleteRule(2));
    CHECK(sheet->WrapperDeleteRule(3));
    CHECK(sheet->WrapperDeleteRule(2));
    CHECK(sheet->WrapperDeleteRule(1));
    CHECK(import->ParentStyleSheet() == nullptr);
    CHECK(sheet->WrapperDeleteRule(0));
    CHECK(sheet->IsEmpty());
  }

  TEST_CASE_METHOD(ValueFixture, "StyleSheetContents traversal", "[CSS][StyleSheetContents]")
  {
    auto sheet = StyleSheetContents::Create();
    auto nested =
      StyleRuleWithNesting::Create(ImmutableDeclarations(u8"color: red"), false, ParseSelectors(u8".a"),
                                   {NestedDeclarationsRule::Create(ImmutableDeclarations(u8"width: 1px"))});
    auto media = MediaRule::Create(mq::MediaQueryParser::Parse(u8"print", StrictParserContext()),
                                   {CreateStyleRule(u8".b", u8"background-image: url(a.png)")});
    sheet->ParserAppendRule(Import(u8"a.css"));
    sheet->ParserAppendRule(Ref<Rule>(media));
    sheet->ParserAppendRule(Ref<Rule>(nested));

    SECTION("TraverseRules visits nested and group children")
    {
      List<RuleType> visited;
      CHECK_FALSE(sheet->TraverseRules(
        [&](const Rule &rule)
        {
          visited.push_back(rule.Type());
          return false;
        }));
      CHECK(visited
            == List<RuleType> {RuleType::Import, RuleType::Media, RuleType::Style, RuleType::StyleWithNesting,
                               RuleType::NestedDeclarations});

      uint32 count = 0;
      CHECK(sheet->TraverseRules(
        [&](const Rule &rule)
        {
          ++count;
          return rule.IsMediaRule();
        }));
      CHECK(count == 2);
    }

    SECTION("MayDependOnBaseURL")
    {
      CHECK(sheet->MayDependOnBaseURL());
      auto plain = StyleSheetContents::Create();
      plain->ParserAppendRule(CreateStyleRule(u8".a", u8"color: red"));
      CHECK_FALSE(plain->MayDependOnBaseURL());
    }

    SECTION("TraverseSubresources")
    {
      // Nothing is loaded yet.
      CHECK_FALSE(sheet->TraverseSubresources([](const CachedResource &) { return true; }));
    }

    SECTION("HasNestingRules and caching")
    {
      CHECK(sheet->HasNestingRules());
      CHECK_FALSE(sheet->IsCacheable());

      auto plain = StyleSheetContents::Create();
      plain->ParserAppendRule(CreateStyleRule(u8".a", u8"color: red"));
      CHECK_FALSE(plain->HasNestingRules());
      // Not loaded yet.
      CHECK_FALSE(plain->IsCacheable());
      plain->CheckLoaded();
      CHECK(plain->LoadCompleted());
      CHECK(plain->IsCacheable());
      CHECK(plain->IsCacheableWithNoBaseURLDependency());

      plain->AddedToMemoryCache();
      CHECK(plain->IsInMemoryCache());
      plain->RemovedFromMemoryCache();
      CHECK_FALSE(plain->IsInMemoryCache());

      plain->ParserAppendRule(CreateStyleRule(u8".b", u8"background-image: url(a.png)"));
      CHECK(plain->IsCacheable());
      CHECK_FALSE(plain->IsCacheableWithNoBaseURLDependency());

      plain->SetMutable();
      CHECK_FALSE(plain->IsCacheable());
    }

    SECTION("Namespace rules and load errors aren't cacheable")
    {
      auto namespaced = StyleSheetContents::Create();
      namespaced->ParserAppendRule(Namespace(u8"a", u8"urn:a"));
      namespaced->CheckLoaded();
      CHECK_FALSE(namespaced->IsCacheable());

      auto failed = StyleSheetContents::Create();
      failed->CheckLoaded();
      CHECK(failed->IsCacheable());
      failed->SetLoadErrorOccured();
      CHECK_FALSE(failed->IsCacheable());

      auto invalidHeader = StyleSheetContents::Create();
      invalidHeader->CheckLoaded();
      invalidHeader->SetHasSyntacticallyValidCSSHeader(false);
      CHECK_FALSE(invalidHeader->IsCacheable());
    }

    CHECK(sheet->EstimatedSizeInBytes() >= sizeof(StyleSheetContents) + 3 * StyleRule::AverageSizeInBytes());
  }

  TEST_CASE_METHOD(ValueFixture, "StyleSheetContents::Copy", "[CSS][StyleSheetContents]")
  {
    auto sheet = StyleSheetContents::Create();
    sheet->ParserAppendRule(LayerStatement(u8"a"));
    auto rule = CreateStyleRule(u8".a", u8"color: red");
    sheet->ParserAppendRule(Ref<Rule>(rule));
    sheet->ParserSetEncodingFromCharsetRule(u8"utf-8");
    sheet->ParserSetUsesStyleBasedEditability();
    sheet->CheckLoaded();

    auto copy = sheet->Copy();
    CHECK(copy->RuleCount() == 2);
    CHECK(copy->LoadCompleted());
    CHECK(copy->UsesStyleBasedEditability());
    CHECK(copy->EncodingFromCharsetRule() == u8"utf-8");
    CHECK(copy->ChildRules()[0].get() != rule.get());
    CHECK(copy->LayerRulesBeforeImportRules()[0].get() != sheet->LayerRulesBeforeImportRules()[0].get());
    CHECK(AsText(Downcast<StyleRule>(*copy->ChildRules()[0]).Properties()) == "color: red;");

    sheet->ShrinkToFit();
    CHECK(sheet->RuleCount() == 2);
  }

  TEST_CASE_METHOD(ValueFixture, "StyleSheetContents clients", "[CSS][StyleSheetContents]")
  {
    auto sheet = StyleSheetContents::Create();
    auto *client = reinterpret_cast<CSSStyleSheet *>(sheet.get());
    CHECK_FALSE(sheet->HasOneClient());
    sheet->RegisterClient(client);
    CHECK(sheet->HasOneClient());
    CHECK(sheet->Clients() == List<CSSStyleSheet *> {client});
    sheet->UnregisterClient(client);
    CHECK(sheet->Clients().empty());

    CHECK_FALSE(sheet->HasResolvedNesting());
    sheet->SetHasResolvedNesting(true);
    CHECK(sheet->HasResolvedNesting());
  }
}
