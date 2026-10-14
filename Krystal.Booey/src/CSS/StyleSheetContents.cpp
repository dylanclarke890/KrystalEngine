#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/Rules/ImportRule.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Core/Assert.hpp"
#include <algorithm>
#include <utility>

namespace krys::boo::css
{
  namespace
  {
    // FIXME: Use Style::RuleData::MaximumSelectorComponentCount once the rule set is ported.
    // The selector list has to fit into RuleData.
    constexpr uint32 MaximumSelectorComponentCount = 8'192;

    bool TraverseRulesInList(const List<Ref<Rule>> &rules,
                             KRYS_NOESCAPE const Func<bool(const Rule &)> &handler) noexcept
    {
      for (const auto &rule : rules)
      {
        if (handler(*rule))
        {
          return true;
        }
        if (auto *styleRuleWithNesting = DynamicDowncast<StyleRuleWithNesting>(rule.get()))
        {
          if (TraverseRulesInList(styleRuleWithNesting->NestedRules(), handler))
          {
            return true;
          }
        }
        auto *groupRule = DynamicDowncast<GroupRule>(rule.get());
        if (!groupRule)
        {
          continue;
        }
        if (TraverseRulesInList(groupRule->ChildRules(), handler))
        {
          return true;
        }
      }
      return false;
    }
  }

  uint32 StyleSheetContents::EstimatedSizeInBytes() const noexcept
  {
    // Note that this does not take into account size of the strings hanging from various objects.
    // The assumption is that nearly all of of them are atoms that would exist anyway.
    uint32 size = sizeof(*this);

    // FIXME: This ignores the children of media and region rules.
    // Most rules are StyleRules.
    size += RuleCount() * StyleRule::AverageSizeInBytes();

    for (const auto &importRule : _importRules)
    {
      if (auto *sheet = importRule->StyleSheet())
      {
        size += sheet->EstimatedSizeInBytes();
      }
    }
    return size;
  }

  StyleSheetContents::StyleSheetContents(ImportRule *ownerRule, const CSSOMString &originalURL,
                                         const ParserContext &context) noexcept
      : _ownerRule(ownerRule), _originalURL(originalURL), _defaultNamespace(dom::StarAtom()),
        _isUserStyleSheet(ownerRule && ownerRule->ParentStyleSheet()
                          && ownerRule->ParentStyleSheet()->IsUserStyleSheet()),
        _parserContext(context)
  {
  }

  StyleSheetContents::StyleSheetContents(const StyleSheetContents &other) noexcept
      : RefCounted(), CanMakeWeakPtr(), _originalURL(other._originalURL),
        _encodingFromCharsetRule(other._encodingFromCharsetRule), _namespaces(other._namespaces),
        _defaultNamespace(other._defaultNamespace), _isUserStyleSheet(other._isUserStyleSheet),
        _loadCompleted(true), _hasSyntacticallyValidCSSHeader(other._hasSyntacticallyValidCSSHeader),
        _usesStyleBasedEditability(other._usesStyleBasedEditability),
        _hasNestingRulesCache(other._hasNestingRulesCache), _parserContext(other._parserContext)
  {
    krys_debug_assert(other.IsCacheable());

    // FIXME: Copy import rules.
    krys_debug_assert(other._importRules.empty());

    // FIXME: Copy namespace rules.
    krys_debug_assert(other._namespaceRules.empty());

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

  Ref<StyleSheetContents> StyleSheetContents::Create(const ParserContext &context) noexcept
  {
    return AdoptRef(*new StyleSheetContents(nullptr, CSSOMString(), context));
  }

  Ref<StyleSheetContents> StyleSheetContents::Create(const CSSOMString &originalURL,
                                                     const ParserContext &context) noexcept
  {
    return AdoptRef(*new StyleSheetContents(nullptr, originalURL, context));
  }

  Ref<StyleSheetContents> StyleSheetContents::Create(ImportRule *ownerRule, const CSSOMString &originalURL,
                                                     const ParserContext &context) noexcept
  {
    return AdoptRef(*new StyleSheetContents(ownerRule, originalURL, context));
  }

  Ref<StyleSheetContents> StyleSheetContents::Copy() const noexcept
  {
    return AdoptRef(*new StyleSheetContents(*this));
  }

  bool StyleSheetContents::IsCacheable() const noexcept
  {
    // FIXME: Support copying import rules.
    if (!_importRules.empty())
    {
      return false;
    }
    // FIXME: Support copying namespace rules.
    if (!_namespaceRules.empty())
    {
      return false;
    }
    // FIXME: Support cached stylesheets in import rules.
    if (_ownerRule)
    {
      return false;
    }
    // This would require dealing with multiple clients for load callbacks.
    if (!_loadCompleted)
    {
      return false;
    }
    if (_didLoadErrorOccur)
    {
      return false;
    }
    // It is not the original sheet anymore.
    if (_isMutable)
    {
      return false;
    }
    // If the header is valid we are not going to need to check the SecurityOrigin.
    // FIXME: Valid mime type avoids the check too.
    if (!_hasSyntacticallyValidCSSHeader)
    {
      return false;
    }
    if (HasNestingRules())
    {
      return false;
    }
    return true;
  }

  bool StyleSheetContents::IsCacheableWithNoBaseURLDependency() const noexcept
  {
    if (!IsCacheable())
    {
      return false;
    }
    if (MayDependOnBaseURL())
    {
      return false;
    }
    return true;
  }

  void StyleSheetContents::ParserAppendRule(Ref<Rule> &&rule) noexcept
  {
    krys_debug_assert(!rule->IsCharsetRule());

    if (auto *layerRule = DynamicDowncast<LayerRule>(rule.get());
        layerRule && _importRules.empty() && _childRules.empty() && _namespaceRules.empty())
    {
      if (layerRule->IsStatement())
      {
        _layerRulesBeforeImportRules.push_back(Ref<LayerRule>::WithRef(*layerRule));
        return;
      }
    }

    if (auto *importRule = DynamicDowncast<ImportRule>(rule.get()))
    {
      // Parser enforces that @import rules come before anything else except @charset.
      krys_debug_assert(_childRules.empty());
      _importRules.push_back(Ref<ImportRule>::WithRef(*importRule));
      _importRules.back()->SetParentStyleSheet(this);
      _importRules.back()->RequestStyleSheet();
      return;
    }

    if (auto *namespaceRule = DynamicDowncast<NamespaceRule>(rule.get()))
    {
      // Parser enforces that @namespace rules come before all rules other than
      // import/charset rules
      krys_debug_assert(_childRules.empty());
      ParserAddNamespace(namespaceRule->Prefix(), namespaceRule->Uri());
      _namespaceRules.push_back(Ref<NamespaceRule>::WithRef(*namespaceRule));
      return;
    }

    // NOTE: The selector list has to fit into RuleData.
    // We don't support nested rules with too many selectors
    if (auto *styleRuleWithNesting = DynamicDowncast<StyleRuleWithNesting>(rule.get()))
    {
      if (styleRuleWithNesting->OriginalSelectorList().ComponentCount() > MaximumSelectorComponentCount)
      {
        return;
      }
    }

    if (auto *styleRule = DynamicDowncast<StyleRule>(rule.get());
        styleRule && styleRule->GetSelectorList().ComponentCount() > MaximumSelectorComponentCount)
    {
      // If we're adding a rule with a huge number of selectors, split it up into multiple rules
      for (auto &splitRule :
           styleRule->SplitIntoMultipleRulesWithMaximumSelectorComponentCount(MaximumSelectorComponentCount))
      {
        _childRules.push_back(krys::move(splitRule));
      }
      return;
    }

    _childRules.push_back(krys::move(rule));
  }

  Rule *StyleSheetContents::RuleAt(uint32 index) const noexcept
  {
    krys_assert(index < RuleCount());

    size_t childVectorIndex = index;
    if (childVectorIndex < _layerRulesBeforeImportRules.size())
    {
      return _layerRulesBeforeImportRules[childVectorIndex].get();
    }

    childVectorIndex -= _layerRulesBeforeImportRules.size();

    if (childVectorIndex < _importRules.size())
    {
      return _importRules[childVectorIndex].get();
    }

    childVectorIndex -= _importRules.size();

    if (childVectorIndex < _namespaceRules.size())
    {
      return _namespaceRules[childVectorIndex].get();
    }

    childVectorIndex -= _namespaceRules.size();

    return _childRules[childVectorIndex].get();
  }

  uint32 StyleSheetContents::RuleCount() const noexcept
  {
    size_t result = 0;
    result += _layerRulesBeforeImportRules.size();
    result += _importRules.size();
    result += _namespaceRules.size();
    result += _childRules.size();
    return static_cast<uint32>(result);
  }

  void StyleSheetContents::ClearCharsetRule() noexcept
  {
    _encodingFromCharsetRule.clear();
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
    ClearCharsetRule();
  }

  void StyleSheetContents::ParserSetEncodingFromCharsetRule(const CSSOMString &encoding) noexcept
  {
    // Parser enforces that there is ever only one @charset.
    krys_debug_assert(_encodingFromCharsetRule.empty());
    _encodingFromCharsetRule = encoding;
  }

  bool StyleSheetContents::WrapperInsertRule(Ref<Rule> &&rule, uint32 index) noexcept
  {
    krys_debug_assert(_isMutable);
    krys_assert(index <= RuleCount());
    // Parser::parseRule doesn't currently allow @charset so we don't need to deal with it.
    krys_debug_assert(!rule->IsCharsetRule());

    // Maybe the insert will be legal if we treat early layer statement rules as normal child rules?
    auto shouldMoveLayerRulesBeforeImportToNormalChildRules = [&]
    {
      if (index >= _layerRulesBeforeImportRules.size())
      {
        return false;
      }
      if (!_importRules.empty() || !_namespaceRules.empty())
      {
        return false;
      }
      auto *layerRule = DynamicDowncast<LayerRule>(rule.get());
      bool isLayerStatement = layerRule && layerRule->IsStatement();
      return !rule->IsImportRule() && !rule->IsNamespaceRule() && !isLayerStatement;
    };

    if (shouldMoveLayerRulesBeforeImportToNormalChildRules())
    {
      auto layerRules = std::exchange(_layerRulesBeforeImportRules, {});
      _childRules.insert(_childRules.begin(), layerRules.begin(), layerRules.end());
    }

    size_t childVectorIndex = index;
    if (childVectorIndex < _layerRulesBeforeImportRules.size()
        || (childVectorIndex == _layerRulesBeforeImportRules.size() && Is<LayerRule>(*rule)))
    {
      auto *layerRule = DynamicDowncast<LayerRule>(rule.get());
      if (!layerRule)
      {
        return false;
      }
      if (layerRule->IsStatement())
      {
        _layerRulesBeforeImportRules.insert(_layerRulesBeforeImportRules.begin() + childVectorIndex,
                                            Ref<LayerRule>::WithRef(*layerRule));
        return true;
      }
      if (childVectorIndex < _layerRulesBeforeImportRules.size())
      {
        return false;
      }
    }
    childVectorIndex -= _layerRulesBeforeImportRules.size();

    if (childVectorIndex < _importRules.size()
        || (childVectorIndex == _importRules.size() && rule->IsImportRule()))
    {
      // Inserting non-import rule before @import is not allowed.
      auto *importRule = DynamicDowncast<ImportRule>(rule.get());
      if (!importRule)
      {
        return false;
      }
      _importRules.insert(_importRules.begin() + childVectorIndex, Ref<ImportRule>::WithRef(*importRule));
      _importRules[childVectorIndex]->SetParentStyleSheet(this);
      _importRules[childVectorIndex]->RequestStyleSheet();
      // FIXME: Stylesheet doesn't actually change meaningfully before the imported sheets are loaded.
      return true;
    }
    // Inserting @import rule after a non-import rule is not allowed.
    if (Is<ImportRule>(*rule))
    {
      return false;
    }
    childVectorIndex -= _importRules.size();

    if (childVectorIndex < _namespaceRules.size()
        || (childVectorIndex == _namespaceRules.size() && rule->IsNamespaceRule()))
    {
      // Inserting non-namespace rules other than import and layer statement rules before @namespace is
      // not allowed.
      auto *namespaceRule = DynamicDowncast<NamespaceRule>(rule.get());
      if (!namespaceRule)
      {
        return false;
      }
      // Inserting @namespace rule when rules other than import/namespace/charset
      // are present is not allowed.
      if (!_childRules.empty() || !_layerRulesBeforeImportRules.empty())
      {
        return false;
      }

      _namespaceRules.insert(_namespaceRules.begin() + childVectorIndex,
                             Ref<NamespaceRule>::WithRef(*namespaceRule));

      // For now to be compatible with IE and Firefox if a namespace rule with the same
      // prefix is added, it overwrites previous ones.
      // FIXME: The eventual correct behavior would be to ensure that the last value in
      // the list wins.
      ParserAddNamespace(namespaceRule->Prefix(), namespaceRule->Uri());
      return true;
    }
    if (Is<NamespaceRule>(*rule))
    {
      return false;
    }
    childVectorIndex -= _namespaceRules.size();

    // If the number of selectors would overflow RuleData, we drop the operation.
    if (auto *styleRuleWithNesting = DynamicDowncast<StyleRuleWithNesting>(rule.get()))
    {
      if (styleRuleWithNesting->OriginalSelectorList().ComponentCount() > MaximumSelectorComponentCount)
      {
        return false;
      }
    }
    if (auto *styleRule = DynamicDowncast<StyleRule>(rule.get()))
    {
      if (styleRule->GetSelectorList().ComponentCount() > MaximumSelectorComponentCount)
      {
        return false;
      }
    }

    _childRules.insert(_childRules.begin() + childVectorIndex, krys::move(rule));
    return true;
  }

  bool StyleSheetContents::WrapperDeleteRule(uint32 index) noexcept
  {
    krys_debug_assert(_isMutable);
    krys_assert(index < RuleCount());

    size_t childVectorIndex = index;
    if (childVectorIndex < _layerRulesBeforeImportRules.size())
    {
      _layerRulesBeforeImportRules.erase(_layerRulesBeforeImportRules.begin() + childVectorIndex);
      return true;
    }
    childVectorIndex -= _layerRulesBeforeImportRules.size();

    if (childVectorIndex < _importRules.size())
    {
      _importRules[childVectorIndex]->CancelLoad();
      _importRules[childVectorIndex]->ClearParentStyleSheet();
      _importRules.erase(_importRules.begin() + childVectorIndex);
      return true;
    }
    childVectorIndex -= _importRules.size();

    if (childVectorIndex < _namespaceRules.size())
    {
      // Deleting @namespace rule when list contains anything other than @import or @namespace rules is not
      // allowed.
      if (!_childRules.empty())
      {
        return false;
      }
      _namespaceRules.erase(_namespaceRules.begin() + childVectorIndex);
      return true;
    }
    childVectorIndex -= _namespaceRules.size();

    _childRules.erase(_childRules.begin() + childVectorIndex);
    return true;
  }

  void StyleSheetContents::ParserAddNamespace(const CSSOMStringAtom &prefix,
                                              const CSSOMStringAtom &uri) noexcept
  {
    krys_debug_assert(uri != CSSOMStringAtom::Null());
    if (prefix == CSSOMStringAtom::Null())
    {
      _defaultNamespace = uri;
      return;
    }
    _namespaces.insert_or_assign(prefix, uri);
  }

  const CSSOMStringAtom &
    StyleSheetContents::NamespaceURIFromPrefix(const CSSOMStringAtom &prefix) const noexcept
  {
    auto it = _namespaces.find(prefix);
    if (it == _namespaces.end())
    {
      return CSSOMStringAtom::Null();
    }
    return it->second;
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

    auto protectedThis = Ref<StyleSheetContents>::WithRef(*this);
    if (auto *parentSheet = ParentStyleSheet())
    {
      parentSheet->CheckLoaded();
      _loadCompleted = true;
      return;
    }
    // FIXME: Ask the single owner node (sheetLoaded, notifyLoadedSheetAndAllCriticalSubresources) once
    // CSSStyleSheet and the DOM's style sheet owners exist; until then there never is one.
    _loadCompleted = true;
  }

  StyleSheetContents *StyleSheetContents::RootStyleSheet() const noexcept
  {
    const auto *root = this;
    while (root->ParentStyleSheet())
    {
      root = root->ParentStyleSheet();
    }
    return const_cast<StyleSheetContents *>(root);
  }

  bool StyleSheetContents::TraverseRules(KRYS_NOESCAPE const Func<bool(const Rule &)> &handler) const noexcept
  {
    for (const auto &importRule : _importRules)
    {
      if (handler(*importRule))
      {
        return true;
      }
      auto importedStyleSheet = RefPtr<StyleSheetContents>::WithRef(importRule->StyleSheet());
      if (importedStyleSheet && importedStyleSheet->TraverseRules(handler))
      {
        return true;
      }
    }
    return TraverseRulesInList(_childRules, handler);
  }

  bool StyleSheetContents::HasNestingRules() const noexcept
  {
    if (_hasNestingRulesCache)
    {
      return *_hasNestingRulesCache;
    }

    _hasNestingRulesCache = TraverseRulesInList(_childRules,
                                                [&](const Rule &rule)
                                                {
                                                  if (rule.IsStyleRuleWithNesting())
                                                  {
                                                    return true;
                                                  }
                                                  if (rule.IsNestedDeclarationsRule())
                                                  {
                                                    return true;
                                                  }
                                                  return false;
                                                });

    return *_hasNestingRulesCache;
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
          {
            return Downcast<StyleRule>(rule).Properties().TraverseSubresources(handler);
          }
          case RuleType::FontFace:
          {
            return Downcast<FontFaceRule>(rule).Properties().TraverseSubresources(handler);
          }
          case RuleType::Import:
          {
            // FIXME: Visit the import's cached style sheet once the resource loader exists.
            return false;
          }
          case RuleType::CounterStyle:
          {
            return static_cast<bool>(_parserContext.counterStyleAtRuleImageSymbolsEnabled);
          }
          case RuleType::Media:
          case RuleType::Page:
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
          case RuleType::ViewTransition:
          case RuleType::PositionTry:
          case RuleType::Function:
          case RuleType::FunctionDeclarations:
          {
            return false;
          }
        }
        krys_debug_assert_msg(false, "Unknown rule type.");
        return false;
      });
  }

  bool StyleSheetContents::MayDependOnBaseURL() const noexcept
  {
    return TraverseRules(
      [&](const Rule &rule) -> bool
      {
        switch (rule.Type())
        {
          case RuleType::Style:
          case RuleType::StyleWithNesting:
          case RuleType::NestedDeclarations:
          {
            return Downcast<StyleRule>(rule).Properties().MayDependOnBaseURL();
          }
          case RuleType::FontFace:
          {
            return Downcast<FontFaceRule>(rule).Properties().MayDependOnBaseURL();
          }
          case RuleType::Import:
          case RuleType::CounterStyle:
          case RuleType::Media:
          case RuleType::Page:
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
          case RuleType::ViewTransition:
          case RuleType::PositionTry:
          case RuleType::Function:
          case RuleType::FunctionDeclarations:
          {
            return false;
          }
        }
        krys_debug_assert_msg(false, "Unknown rule type.");
        return false;
      });
  }

  StyleSheetContents *StyleSheetContents::ParentStyleSheet() const noexcept
  {
    return _ownerRule ? _ownerRule->ParentStyleSheet() : nullptr;
  }

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

  void StyleSheetContents::ShrinkToFit() noexcept
  {
    _importRules.shrink_to_fit();
    _childRules.shrink_to_fit();
  }
}
