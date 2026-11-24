#include "Krystal.Booey/CSS/StyleSheets/StyleSheetContents.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Parser/Parser.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/Rules/ImportRule.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Loader/Cache/CachedCSSStyleSheet.hpp"
#include "Krystal.Booey/Style/Rules/RuleData.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Move.hpp"
#include <algorithm>
#include <utility>

namespace krys::boo::css
{
  namespace
  {
    /// @brief The most selector components a style rule can have; `style::RuleData` stores the index of one.
    constexpr uint32 MaximumSelectorComponentCount = style::RuleData::MaximumSelectorComponentCount;

    /// @brief Whether @p rule is a nesting style rule whose selectors, as written, have more components than
    /// `style::RuleData` holds.
    KRYS_NODISCARD bool HasTooManyOriginalSelectorComponents(const Rule &rule) noexcept
    {
      const auto *styleRuleWithNesting = DynamicDowncast<StyleRuleWithNesting>(rule);
      return styleRuleWithNesting
             && styleRuleWithNesting->OriginalSelectorList().ComponentCount() > MaximumSelectorComponentCount;
    }

    /// @brief @p rule as a style rule, if it is one with more selector components than `style::RuleData`
    /// holds.
    KRYS_NODISCARD const StyleRule *StyleRuleWithTooManySelectorComponents(const Rule &rule) noexcept
    {
      const auto *styleRule = DynamicDowncast<StyleRule>(rule);
      return styleRule && styleRule->GetSelectorList().ComponentCount() > MaximumSelectorComponentCount
               ? styleRule
               : nullptr;
    }

    /// @brief Calls @p handler with each of @p rules and the rules nested in them, depth first, until it
    /// returns true.
    bool TraverseRulesInList(const List<Ref<Rule>> &rules,
                             KRYS_NOESCAPE const Func<bool(const Rule &)> &handler) noexcept
    {
      for (const auto &rule : rules)
      {
        if (handler(*rule))
        {
          return true;
        }
        if (const auto *styleRuleWithNesting = DynamicDowncast<StyleRuleWithNesting>(rule.get());
            styleRuleWithNesting && TraverseRulesInList(styleRuleWithNesting->NestedRules(), handler))
        {
          return true;
        }
        if (const auto *groupRule = DynamicDowncast<GroupRule>(rule.get());
            groupRule && TraverseRulesInList(groupRule->ChildRules(), handler))
        {
          return true;
        }
      }
      return false;
    }
  }

  StyleSheetContents::StyleSheetContents(ImportRule *ownerRule, CSSOMString originalUrl,
                                         ParserContext context) noexcept
      : _ownerRule(ownerRule), _originalUrl(krys::move(originalUrl)), _parserContext(krys::move(context)),
        _defaultNamespace(dom::StarAtom()),
        _isUserStyleSheet(ownerRule && ownerRule->ParentStyleSheet()
                          && ownerRule->ParentStyleSheet()->IsUserStyleSheet())
  {
  }

  // NOLINTNEXTLINE(bugprone-copy-constructor-init): a copy starts with its own reference and weak counts.
  StyleSheetContents::StyleSheetContents(const StyleSheetContents &other) noexcept
      : _originalUrl(other._originalUrl), _parserContext(other._parserContext),
        _namespaces(other._namespaces), _defaultNamespace(other._defaultNamespace),
        _isUserStyleSheet(other._isUserStyleSheet), _loadCompleted(true),
        _hasSyntacticallyValidCSSHeader(other._hasSyntacticallyValidCSSHeader),
        _hasNestingRulesCache(other._hasNestingRulesCache)
  {
    // Only cacheable sheets are copied, and those have no imports to copy.
    krys_debug_assert(other.IsCacheable());
    krys_debug_assert(other._importRules.empty());

    // Namespace rules can't change, so the copy shares them.
    _namespaceRules = other._namespaceRules;

    _layerRulesBeforeImportRules.reserve(other._layerRulesBeforeImportRules.size());
    for (const auto &rule : other._layerRulesBeforeImportRules)
    {
      _layerRulesBeforeImportRules.push_back(rule->Copy());
    }
    _childRules.reserve(other._childRules.size());
    for (const auto &rule : other._childRules)
    {
      _childRules.push_back(rule->Copy());
    }
  }

  StyleSheetContents::~StyleSheetContents() noexcept
  {
    ClearRules();
  }

  Ref<StyleSheetContents> StyleSheetContents::Create() noexcept
  {
    return Create(ParserContext(ParserMode::HTMLStandard));
  }

  Ref<StyleSheetContents> StyleSheetContents::Create(ParserContext context) noexcept
  {
    return AdoptRef(*new StyleSheetContents(nullptr, CSSOMString(), krys::move(context)));
  }

  Ref<StyleSheetContents> StyleSheetContents::Create(CSSOMString originalUrl, ParserContext context) noexcept
  {
    return AdoptRef(*new StyleSheetContents(nullptr, krys::move(originalUrl), krys::move(context)));
  }

  Ref<StyleSheetContents> StyleSheetContents::Create(ImportRule *ownerRule, CSSOMString originalUrl,
                                                     ParserContext context) noexcept
  {
    return AdoptRef(*new StyleSheetContents(ownerRule, krys::move(originalUrl), krys::move(context)));
  }

  Ref<StyleSheetContents> StyleSheetContents::Copy() const noexcept
  {
    return AdoptRef(*new StyleSheetContents(*this));
  }

#pragma region Parsing

  bool StyleSheetContents::ParseString(const CSSOMString &sheetText) noexcept
  {
    Parser::ParseStyleSheet(sheetText, GetParserContext(), *this);
    return true;
  }

  bool StyleSheetContents::ParseAuthorStyleSheet(const CachedCSSStyleSheet *cachedStyleSheet) noexcept
  {
    // The type is checked strictly in quirks mode too (see DIFFERENCES.md).
    auto sheetText = cachedStyleSheet->SheetText();
    if (!sheetText)
    {
      return false;
    }
    Parser::ParseStyleSheet(*sheetText, GetParserContext(), *this);
    return true;
  }

  void StyleSheetContents::ParserAppendRule(Ref<Rule> rule) noexcept
  {
    krys_debug_assert(!rule->IsCharsetRule());

    // Layer statements before any other rule may stay before the imports.
    if (auto *layerRule = DynamicDowncast<LayerRule>(rule.get());
        layerRule && layerRule->IsStatement() && _importRules.empty() && _namespaceRules.empty()
        && _childRules.empty())
    {
      _layerRulesBeforeImportRules.push_back(ShareRef(*layerRule));
      return;
    }

    // The parser only accepts imports and namespaces before the other rules.
    if (auto *importRule = DynamicDowncast<ImportRule>(rule.get()))
    {
      krys_debug_assert(_childRules.empty());
      _importRules.push_back(ShareRef(*importRule));
      importRule->SetParentStyleSheet(this);
      importRule->RequestStyleSheet();
      return;
    }
    if (auto *namespaceRule = DynamicDowncast<NamespaceRule>(rule.get()))
    {
      krys_debug_assert(_childRules.empty());
      ParserAddNamespace(namespaceRule->Prefix(), namespaceRule->Uri());
      _namespaceRules.push_back(ShareRef(*namespaceRule));
      return;
    }

    // A style rule with too many selector components is split into several, unless its nested rules' would
    // be too; then it's dropped.
    if (HasTooManyOriginalSelectorComponents(*rule))
    {
      return;
    }
    if (const auto *styleRule = StyleRuleWithTooManySelectorComponents(*rule))
    {
      for (auto &splitRule :
           styleRule->SplitIntoMultipleRulesWithMaximumSelectorComponentCount(MaximumSelectorComponentCount))
      {
        _childRules.emplace_back(krys::move(splitRule));
      }
      return;
    }
    _childRules.push_back(krys::move(rule));
  }

  void StyleSheetContents::ParserAddNamespace(const CSSOMStringAtom &prefix,
                                              const CSSOMStringAtom &uri) noexcept
  {
    krys_debug_assert(uri != null);
    if (prefix == null)
    {
      _defaultNamespace = uri;
      return;
    }
    _namespaces.insert_or_assign(prefix, uri);
  }

  void StyleSheetContents::ShrinkToFit() noexcept
  {
    _importRules.shrink_to_fit();
    _childRules.shrink_to_fit();
  }

#pragma endregion

#pragma region Rules

  uint32 StyleSheetContents::RuleCount() const noexcept
  {
    return static_cast<uint32>(_layerRulesBeforeImportRules.size() + _importRules.size()
                               + _namespaceRules.size() + _childRules.size());
  }

  Rule *StyleSheetContents::RuleAt(uint32 index) const noexcept
  {
    krys_assert(index < RuleCount());

    size_t i = index;
    if (i < _layerRulesBeforeImportRules.size())
    {
      return _layerRulesBeforeImportRules[i].get();
    }
    i -= _layerRulesBeforeImportRules.size();
    if (i < _importRules.size())
    {
      return _importRules[i].get();
    }
    i -= _importRules.size();
    if (i < _namespaceRules.size())
    {
      return _namespaceRules[i].get();
    }
    i -= _namespaceRules.size();
    return _childRules[i].get();
  }

  void StyleSheetContents::ClearRules() noexcept
  {
    for (auto &importRule : _importRules)
    {
      krys_debug_assert(importRule->ParentStyleSheet() == this);
      importRule->ClearParentStyleSheet();
    }
    _layerRulesBeforeImportRules.clear();
    _importRules.clear();
    _namespaceRules.clear();
    _childRules.clear();
    RebuildNamespaces();
  }

  bool StyleSheetContents::WrapperInsertRule(Ref<Rule> rule, uint32 index) noexcept
  {
    krys_debug_assert(_isMutable);
    krys_assert(index <= RuleCount());
    // The CSSOM parses inserted rules without allowing @charset.
    krys_debug_assert(!rule->IsCharsetRule());

    auto *layerRule = DynamicDowncast<LayerRule>(rule.get());
    const bool isLayerStatement = layerRule && layerRule->IsStatement();

    // Any other rule going among the early layer statements, while there are no imports or namespaces for
    // them to precede, makes them ordinary rules.
    if (index < _layerRulesBeforeImportRules.size() && _importRules.empty() && _namespaceRules.empty()
        && !rule->IsImportRule() && !rule->IsNamespaceRule() && !isLayerStatement)
    {
      auto layerRules = std::exchange(_layerRulesBeforeImportRules, {});
      _childRules.insert(_childRules.begin(), layerRules.begin(), layerRules.end());
    }

    size_t i = index;
    if (i < _layerRulesBeforeImportRules.size() || (i == _layerRulesBeforeImportRules.size() && layerRule))
    {
      if (isLayerStatement)
      {
        _layerRulesBeforeImportRules.insert(_layerRulesBeforeImportRules.begin() + i, ShareRef(*layerRule));
        return true;
      }
      // Only a layer statement can go among them; a layer block after them is an ordinary rule.
      if (i < _layerRulesBeforeImportRules.size())
      {
        return false;
      }
    }
    i -= _layerRulesBeforeImportRules.size();

    // Imports go only among the imports, and only imports can go there.
    if (i < _importRules.size() || (i == _importRules.size() && rule->IsImportRule()))
    {
      auto *importRule = DynamicDowncast<ImportRule>(rule.get());
      if (!importRule)
      {
        return false;
      }
      _importRules.insert(_importRules.begin() + i, ShareRef(*importRule));
      importRule->SetParentStyleSheet(this);
      importRule->RequestStyleSheet();
      // Reported as a change, though nothing applies until the import loads, which reports another.
      return true;
    }
    if (rule->IsImportRule())
    {
      return false;
    }
    i -= _importRules.size();

    // Likewise namespaces, which can only be added while there are no rules but imports and namespaces.
    if (i < _namespaceRules.size() || (i == _namespaceRules.size() && rule->IsNamespaceRule()))
    {
      auto *namespaceRule = DynamicDowncast<NamespaceRule>(rule.get());
      if (!namespaceRule || !_childRules.empty() || !_layerRulesBeforeImportRules.empty())
      {
        return false;
      }
      _namespaceRules.insert(_namespaceRules.begin() + i, ShareRef(*namespaceRule));
      // The rule may go before another declaring the same prefix, which still wins.
      RebuildNamespaces();
      return true;
    }
    if (rule->IsNamespaceRule())
    {
      return false;
    }
    i -= _namespaceRules.size();

    // Unlike a parsed one, an inserted rule with too many selector components isn't split.
    if (HasTooManyOriginalSelectorComponents(*rule) || StyleRuleWithTooManySelectorComponents(*rule))
    {
      return false;
    }
    _childRules.insert(_childRules.begin() + i, krys::move(rule));
    return true;
  }

  bool StyleSheetContents::WrapperDeleteRule(uint32 index) noexcept
  {
    krys_debug_assert(_isMutable);
    krys_assert(index < RuleCount());

    size_t i = index;
    if (i < _layerRulesBeforeImportRules.size())
    {
      _layerRulesBeforeImportRules.erase(_layerRulesBeforeImportRules.begin() + i);
      return true;
    }
    i -= _layerRulesBeforeImportRules.size();
    if (i < _importRules.size())
    {
      _importRules[i]->CancelLoad();
      _importRules[i]->ClearParentStyleSheet();
      _importRules.erase(_importRules.begin() + i);
      return true;
    }
    i -= _importRules.size();
    if (i < _namespaceRules.size())
    {
      // A namespace can't be removed once rules that may use it follow.
      if (!_childRules.empty())
      {
        return false;
      }
      _namespaceRules.erase(_namespaceRules.begin() + i);
      RebuildNamespaces();
      return true;
    }
    i -= _namespaceRules.size();
    _childRules.erase(_childRules.begin() + i);
    return true;
  }

  bool StyleSheetContents::TraverseRules(KRYS_NOESCAPE const Func<bool(const Rule &)> &handler) const noexcept
  {
    for (const auto &importRule : _importRules)
    {
      if (handler(*importRule))
      {
        return true;
      }
      if (auto importedStyleSheet = ShareRefPtr(importRule->StyleSheet());
          importedStyleSheet && importedStyleSheet->TraverseRules(handler))
      {
        return true;
      }
    }
    return TraverseRulesInList(_childRules, handler);
  }

  bool StyleSheetContents::HasNestingRules() const noexcept
  {
    if (!_hasNestingRulesCache)
    {
      _hasNestingRulesCache =
        TraverseRulesInList(_childRules, [](const Rule &rule)
                            { return rule.IsStyleRuleWithNesting() || rule.IsNestedDeclarationsRule(); });
    }
    return *_hasNestingRulesCache;
  }

  bool StyleSheetContents::MayDependOnBaseUrl() const noexcept
  {
    return TraverseRules(
      [](const Rule &rule)
      {
        switch (rule.Type())
        {
          case RuleType::Style:
          case RuleType::StyleWithNesting:
          case RuleType::NestedDeclarations:
            return Downcast<StyleRule>(rule).Properties().MayDependOnBaseUrl();
          case RuleType::FontFace:               return Downcast<FontFaceRule>(rule).Properties().MayDependOnBaseUrl();
          case RuleType::Import:
          case RuleType::CounterStyle:
          case RuleType::Media:
          case RuleType::Keyframes:
          case RuleType::Namespace:
          case RuleType::Charset:
          case RuleType::Keyframe:
          case RuleType::Supports:
          case RuleType::LayerBlock:
          case RuleType::LayerStatement:
          case RuleType::Container:
          case RuleType::FontFeatureValues:
          case RuleType::FontFeatureValuesBlock:
          case RuleType::FontPaletteValues:
          case RuleType::Margin:
          case RuleType::Property:
          case RuleType::Scope:
          case RuleType::StartingStyle:
          case RuleType::PositionTry:
          case RuleType::Function:
          case RuleType::FunctionDeclarations:   return false;
        }
        krys_debug_assert_msg(false, "Unknown rule type.");
        return false;
      });
  }

  bool StyleSheetContents::TraverseSubresources(
    KRYS_NOESCAPE const Func<bool(const CachedResource &)> &handler) const noexcept
  {
    return TraverseRules(
      [&](const Rule &rule)
      {
        switch (rule.Type())
        {
          case RuleType::Style:
          case RuleType::StyleWithNesting:
          case RuleType::NestedDeclarations:
            return Downcast<StyleRule>(rule).Properties().TraverseSubresources(handler);
          case RuleType::FontFace:
            return Downcast<FontFaceRule>(rule).Properties().TraverseSubresources(handler);
          case RuleType::Import:
          {
            const auto *cachedStyleSheet = Downcast<ImportRule>(rule).CachedStyleSheet();
            return cachedStyleSheet && handler(*cachedStyleSheet);
          }
          // Its image symbols aren't traversed, so it stops the traversal: a sheet with one isn't reused.
          case RuleType::CounterStyle:           return true;
          case RuleType::Media:
          case RuleType::Keyframes:
          case RuleType::Namespace:
          case RuleType::Charset:
          case RuleType::Keyframe:
          case RuleType::Supports:
          case RuleType::LayerBlock:
          case RuleType::LayerStatement:
          case RuleType::Container:
          case RuleType::FontFeatureValues:
          case RuleType::FontFeatureValuesBlock:
          case RuleType::FontPaletteValues:
          case RuleType::Margin:
          case RuleType::Property:
          case RuleType::Scope:
          case RuleType::StartingStyle:
          case RuleType::PositionTry:
          case RuleType::Function:
          case RuleType::FunctionDeclarations:   return false;
        }
        krys_debug_assert_msg(false, "Unknown rule type.");
        return false;
      });
  }

#pragma endregion

#pragma region Namespaces

  CSSOMStringAtom StyleSheetContents::NamespaceURIFromPrefix(const CSSOMStringAtom &prefix) const noexcept
  {
    auto it = _namespaces.find(prefix);
    return it != _namespaces.end() ? it->second : CSSOMStringAtom(null);
  }

  void StyleSheetContents::RebuildNamespaces() noexcept
  {
    _namespaces.clear();
    _defaultNamespace = dom::StarAtom();
    for (const auto &rule : _namespaceRules)
    {
      ParserAddNamespace(rule->Prefix(), rule->Uri());
    }
  }

#pragma endregion

#pragma region Imports and loading

  StyleSheetContents *StyleSheetContents::ParentStyleSheet() const noexcept
  {
    return _ownerRule ? _ownerRule->ParentStyleSheet() : nullptr;
  }

  StyleSheetContents *StyleSheetContents::RootStyleSheet() const noexcept
  {
    const auto *root = this;
    while (const auto *parent = root->ParentStyleSheet())
    {
      root = parent;
    }
    return const_cast<StyleSheetContents *>(root);
  }

  Node *StyleSheetContents::SingleOwnerNode() const noexcept
  {
    const auto *root = RootStyleSheet();
    if (root->_clients.empty())
    {
      return nullptr;
    }
    krys_debug_assert(root->_clients.size() == 1);
    return root->_clients[0]->OwnerNode();
  }

  Document *StyleSheetContents::SingleOwnerDocument() const noexcept
  {
    auto *ownerNode = SingleOwnerNode();
    return ownerNode ? &ownerNode->NodeDocument() : nullptr;
  }

  bool StyleSheetContents::IsLoading() const noexcept
  {
    return std::ranges::any_of(_importRules, [](const auto &importRule) { return importRule->IsLoading(); });
  }

  void StyleSheetContents::CheckLoaded() noexcept
  {
    if (IsLoading())
    {
      return;
    }

    // The owner node may release the sheet when told it's loaded.
    auto protectedThis = ShareRef(*this);
    if (auto *parentSheet = ParentStyleSheet())
    {
      parentSheet->CheckLoaded();
      _loadCompleted = true;
      return;
    }
    auto ownerNode = ShareRefPtr(SingleOwnerNode());
    if (!ownerNode)
    {
      _loadCompleted = true;
      return;
    }
    _loadCompleted = ownerNode->SheetLoaded();
    if (_loadCompleted)
    {
      ownerNode->NotifyLoadedSheetAndAllCriticalSubresources(_didLoadErrorOccur);
    }
  }

  void StyleSheetContents::StartLoadingDynamicSheet() noexcept
  {
    if (auto *ownerNode = SingleOwnerNode())
    {
      ownerNode->StartLoadingDynamicSheet();
    }
  }

  void StyleSheetContents::NotifyLoadedSheet(const CachedCSSStyleSheet *sheet) noexcept
  {
    krys_debug_assert(sheet);
    _didLoadErrorOccur |= sheet->ErrorOccurred() || !sheet->MimeTypeAllowedByNosniff();
  }

#pragma endregion

#pragma region Caching

  bool StyleSheetContents::IsCacheable() const noexcept
  {
    // Sheets with imports, and imported sheets, aren't cached: copying one would mean copying its import
    // rules' loads and clients.
    // FIXME: Work out how to cache them.
    if (!_importRules.empty() || _ownerRule)
    {
      return false;
    }
    // A sheet still loading would have to tell every client sharing it when it's done; a modified one is
    // no longer what was fetched.
    return _loadCompleted && !_didLoadErrorOccur && !_isMutable && _hasSyntacticallyValidCSSHeader
           && !HasNestingRules();
  }

  bool StyleSheetContents::IsCacheableWithNoBaseUrlDependency() const noexcept
  {
    return IsCacheable() && !MayDependOnBaseUrl();
  }

  bool StyleSheetContents::SubresourcesAllowReuse() const noexcept
  {
    return !TraverseSubresources([](const CachedResource &resource)
                                 { return resource.LoadFailedOrCanceled(); });
  }

  uint32 StyleSheetContents::EstimatedSizeInBytes() const noexcept
  {
    // Strings are left out, as most are atoms shared with the rest of the document. Every rule is counted
    // as a style rule, as most are, nested ones (in @media, say) included.
    auto ruleCount = RuleCount() - static_cast<uint32>(_childRules.size());
    std::ignore = TraverseRulesInList(_childRules,
                                      [&](const Rule &)
                                      {
                                        ++ruleCount;
                                        return false;
                                      });
    uint32 size = sizeof(*this) + ruleCount * StyleRule::AverageSizeInBytes();
    for (const auto &importRule : _importRules)
    {
      if (const auto *sheet = importRule->StyleSheet())
      {
        size += sheet->EstimatedSizeInBytes();
      }
    }
    return size;
  }

  void StyleSheetContents::AddedToMemoryCache() noexcept
  {
    krys_debug_assert(IsCacheable());
    ++_inMemoryCacheCount;
  }

  void StyleSheetContents::RemovedFromMemoryCache() noexcept
  {
    krys_debug_assert(_inMemoryCacheCount);
    krys_debug_assert(IsCacheable());
    --_inMemoryCacheCount;
  }

#pragma endregion

#pragma region Clients

  void StyleSheetContents::RegisterClient(CSSStyleSheet *sheet) noexcept
  {
    krys_debug_assert(std::ranges::find(_clients, sheet) == _clients.end());
    _clients.push_back(sheet);
  }

  void StyleSheetContents::UnregisterClient(CSSStyleSheet *sheet) noexcept
  {
    auto it = std::ranges::find(_clients, sheet);
    krys_debug_assert(it != _clients.end());
    if (it != _clients.end())
    {
      _clients.erase(it);
    }
  }

#pragma endregion
}
