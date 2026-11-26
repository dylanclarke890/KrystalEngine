#pragma once

#include "Krystal.Booey/Css/Parser/ParserContext.hpp"
#include "Krystal.Booey/Css/Types/CSSOMString.hpp"
#include "Krystal.Booey/Dom/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/HashMap.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Net/Url.hpp"

namespace krys::boo
{
  class CSSStyleSheet;
  class CachedCSSStyleSheet;
  class CachedResource;
  class Document;
  class Node;
}

namespace krys::boo::css
{
  class ImportRule;
  class LayerRule;
  class NamespaceRule;
  class Rule;

  /// @brief A parsed style sheet: its rules, the namespaces they declare, and the state of its imports. The
  /// CSSOM [`CSSStyleSheet`](https://drafts.csswg.org/cssom/#cssstylesheet) objects using it are its
  /// clients; an unmodified sheet can be shared between several, and kept in the memory cache.
  /// @note The rules are held in four lists, in the order a style sheet allows them: layer statements before
  /// any import, then imports, then namespaces, then the rest. `RuleAt` indexes them as one list.
  class StyleSheetContents final : public RefCounted<StyleSheetContents>,
                                   public CanMakeWeakPtr<StyleSheetContents>
  {
    friend class boo::CSSStyleSheet;
    using PrefixNamespaceUriMap = HashMap<CSSOMStringAtom, CSSOMStringAtom>;

  private:
    ImportRule *_ownerRule {nullptr};
    CSSOMString _originalUrl;
    ParserContext _parserContext;

    List<Ref<LayerRule>> _layerRulesBeforeImportRules;
    List<Ref<ImportRule>> _importRules;
    List<Ref<NamespaceRule>> _namespaceRules;
    List<Ref<Rule>> _childRules;

    PrefixNamespaceUriMap _namespaces;
    CSSOMStringAtom _defaultNamespace;

    bool _isUserStyleSheet {false};
    bool _loadCompleted {false};
    bool _hasSyntacticallyValidCSSHeader {true};
    bool _didLoadErrorOccur {false};
    bool _isMutable {false};
    mutable Maybe<bool> _hasNestingRulesCache;
    uint32 _inMemoryCacheCount {0};

    List<CSSStyleSheet *> _clients;

    StyleSheetContents(ImportRule *ownerRule, CSSOMString originalUrl, ParserContext context) noexcept;

    /// @brief A copy of a cacheable sheet, with copies of its rules, for a client that will modify it.
    StyleSheetContents(const StyleSheetContents &other) noexcept;

  public:
    ~StyleSheetContents() noexcept;

    /// @brief An empty sheet with a standards-mode parser context.
    KRYS_NODISCARD static Ref<StyleSheetContents> Create() noexcept;

    KRYS_NODISCARD static Ref<StyleSheetContents> Create(ParserContext context) noexcept;

    KRYS_NODISCARD static Ref<StyleSheetContents> Create(CSSOMString originalUrl,
                                                         ParserContext context) noexcept;

    /// @brief The sheet an `@import` rule loads.
    KRYS_NODISCARD static Ref<StyleSheetContents> Create(ImportRule *ownerRule, CSSOMString originalUrl,
                                                         ParserContext context) noexcept;

    /// @brief A copy for a client to modify; only a cacheable sheet can be copied.
    KRYS_NODISCARD Ref<StyleSheetContents> Copy() const noexcept;

#pragma region Parsing

    KRYS_NODISCARD const ParserContext &GetParserContext() const noexcept
    {
      return _parserContext;
    }

    /// @brief The URL relative URLs in the sheet resolve against.
    KRYS_NODISCARD const net::Url &BaseUrl() const noexcept
    {
      return _parserContext.BaseUrl;
    }

    /// @brief The encoding the sheet was decoded with, if one was given.
    KRYS_NODISCARD utf8_stringview Charset() const noexcept
    {
      return _parserContext.Charset;
    }

    /// @brief The URL the sheet was first requested from, before any redirects.
    KRYS_NODISCARD CSSOMString OriginalUrl() const noexcept
    {
      return _originalUrl;
    }

    /// @brief Parses @p sheetText, appending its rules.
    bool ParseString(const CSSOMString &sheetText) noexcept;

    /// @brief Parses the text of a fetched sheet, appending its rules.
    /// @return False, parsing nothing, if the response can't be used: it failed, or its type isn't CSS.
    bool ParseAuthorStyleSheet(const CachedCSSStyleSheet *cachedStyleSheet) noexcept;

    /// @brief Appends a rule the parser produced. A style rule with more selector components than a
    /// `style::RuleData` holds is split into several.
    void ParserAppendRule(Ref<Rule> rule) noexcept;

    /// @brief Declares @p prefix (or, if it's null, the default namespace) as @p uri.
    void ParserAddNamespace(const CSSOMStringAtom &prefix, const CSSOMStringAtom &uri) noexcept;

    /// @brief Records whether the first rule parsed; a sheet without a valid one may not be CSS at all.
    void SetHasSyntacticallyValidCSSHeader(bool value) noexcept
    {
      _hasSyntacticallyValidCSSHeader = value;
    }

    /// @brief Releases unused capacity once parsing is done.
    void ShrinkToFit() noexcept;

#pragma endregion

#pragma region Rules

    KRYS_NODISCARD const List<Ref<LayerRule>> &LayerRulesBeforeImportRules() const noexcept
    {
      return _layerRulesBeforeImportRules;
    }

    KRYS_NODISCARD const List<Ref<ImportRule>> &ImportRules() const noexcept
    {
      return _importRules;
    }

    KRYS_NODISCARD const List<Ref<NamespaceRule>> &NamespaceRules() const noexcept
    {
      return _namespaceRules;
    }

    /// @brief The rules after the namespace rules.
    KRYS_NODISCARD const List<Ref<Rule>> &ChildRules() const noexcept
    {
      return _childRules;
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return !RuleCount();
    }

    KRYS_NODISCARD uint32 RuleCount() const noexcept;

    /// @brief The rule at @p index, counting the early layer statements, then the import rules, then the
    /// namespace rules, then the other rules.
    KRYS_NODISCARD Rule *RuleAt(uint32 index) const noexcept;

    /// @brief Removes every rule, detaching the import rules from the sheet.
    void ClearRules() noexcept;

    /// @brief Inserts @p rule at @p index for the CSSOM's
    /// [insert a CSS rule](https://drafts.csswg.org/cssom/#insert-a-css-rule).
    /// @return False, inserting nothing, if @p rule isn't allowed at @p index.
    bool WrapperInsertRule(Ref<Rule> rule, uint32 index) noexcept;

    /// @brief Removes the rule at @p index for the CSSOM's
    /// [remove a CSS rule](https://drafts.csswg.org/cssom/#remove-a-css-rule).
    /// @return False, removing nothing, if the rule at @p index can't be removed.
    bool WrapperDeleteRule(uint32 index) noexcept;

    /// @brief Whether a rule's declarations may hold a URL, which resolves against the base URL.
    KRYS_NODISCARD bool MayDependOnBaseUrl() const noexcept;

    /// @brief Calls @p handler on every resource the rules load until it returns true.
    /// @return Whether @p handler returned true.
    bool TraverseSubresources(KRYS_NOESCAPE const Func<bool(const CachedResource &)> &handler) const noexcept;

    /// @brief Forgets whether the sheet has nesting rules, after a client changed a nested rule list.
    void ClearHasNestingRulesCache() noexcept
    {
      _hasNestingRulesCache = {};
    }

#pragma endregion

#pragma region Namespaces

    /// @brief The [default namespace](https://drafts.csswg.org/css-namespaces/#default-namespace), or `*`
    /// if none is declared.
    KRYS_NODISCARD const CSSOMStringAtom &DefaultNamespace() const noexcept
    {
      return _defaultNamespace;
    }

    /// @brief The namespace @p prefix is declared as, or null if it isn't.
    KRYS_NODISCARD CSSOMStringAtom NamespaceURIFromPrefix(const CSSOMStringAtom &prefix) const noexcept;

#pragma endregion

#pragma region Imports and loading

    /// @brief The `@import` rule that loaded the sheet, if any.
    KRYS_NODISCARD ImportRule *OwnerRule() const noexcept
    {
      return _ownerRule;
    }

    void ClearOwnerRule() noexcept
    {
      _ownerRule = nullptr;
    }

    /// @brief The sheet whose `@import` rule loaded this one, if any.
    KRYS_NODISCARD StyleSheetContents *ParentStyleSheet() const noexcept;

    /// @brief The sheet at the top of the import chain.
    KRYS_NODISCARD StyleSheetContents *RootStyleSheet() const noexcept;

    /// @brief The document of the node owning the root sheet, if it has one client with an owner node.
    KRYS_NODISCARD Document *SingleOwnerDocument() const noexcept;

    /// @brief Whether a user, rather than the page, supplied the sheet. Imported sheets inherit it.
    KRYS_NODISCARD bool IsUserStyleSheet() const noexcept
    {
      return _isUserStyleSheet;
    }

    void SetIsUserStyleSheet(bool value) noexcept
    {
      _isUserStyleSheet = value;
    }

    /// @brief Whether an imported sheet is still loading.
    KRYS_NODISCARD bool IsLoading() const noexcept;

    KRYS_NODISCARD bool LoadCompleted() const noexcept
    {
      return _loadCompleted;
    }

    /// @brief Once no import is loading, marks the sheet loaded and tells its parent sheet or owner node.
    void CheckLoaded() noexcept;

    /// @brief Tells the owner node that an import has started loading after the sheet was parsed.
    void StartLoadingDynamicSheet() noexcept;

    /// @brief Records that an imported sheet arrived, noting whether it failed or was blocked by nosniff.
    void NotifyLoadedSheet(const CachedCSSStyleSheet *sheet) noexcept;

    void SetLoadErrorOccured() noexcept
    {
      _didLoadErrorOccur = true;
    }

#pragma endregion

#pragma region Caching

    /// @brief Whether the sheet can be shared through the memory cache: it's loaded, unmodified, error-free,
    /// with a valid first rule, and has no imports and no nesting rules.
    KRYS_NODISCARD bool IsCacheable() const noexcept;

    /// @brief Whether the sheet is cacheable and its rules don't depend on its base URL, so it can be shared
    /// between sheets with different base URLs.
    KRYS_NODISCARD bool IsCacheableWithNoBaseUrlDependency() const noexcept;

    /// @brief Whether a cached sheet can be reused: none of the resources it loads failed.
    KRYS_NODISCARD bool SubresourcesAllowReuse() const noexcept;

    /// @brief A rough size for the memory cache's budget.
    KRYS_NODISCARD uint32 EstimatedSizeInBytes() const noexcept;

    KRYS_NODISCARD bool IsInMemoryCache() const noexcept
    {
      return _inMemoryCacheCount;
    }

    void AddedToMemoryCache() noexcept;

    void RemovedFromMemoryCache() noexcept;

#pragma endregion

#pragma region Clients

    void RegisterClient(CSSStyleSheet *sheet) noexcept;

    void UnregisterClient(CSSStyleSheet *sheet) noexcept;

    KRYS_NODISCARD bool HasOneClient() const noexcept
    {
      return _clients.size() == 1;
    }

    KRYS_NODISCARD List<CSSStyleSheet *> Clients() const noexcept
    {
      return _clients;
    }

    /// @brief Whether a client has modified, or is about to modify, the sheet, which then isn't shared.
    KRYS_NODISCARD bool IsMutable() const noexcept
    {
      return _isMutable;
    }

    void SetMutable() noexcept
    {
      _isMutable = true;
    }

#pragma endregion

  private:
    /// @brief Derives the namespaces from the `@namespace` rules, the last declaration of a prefix winning.
    void RebuildNamespaces() noexcept;

    /// @brief The owner node of the root sheet's only client, if it has one.
    KRYS_NODISCARD Node *SingleOwnerNode() const noexcept;

    /// @brief Calls @p handler with each rule, imported sheets' and nested ones included, until it returns
    /// true.
    /// @return Whether @p handler returned true.
    bool TraverseRules(KRYS_NOESCAPE const Func<bool(const Rule &)> &handler) const noexcept;

    /// @brief Whether a style rule has nested rules, which make the sheet uncacheable. Cached.
    KRYS_NODISCARD bool HasNestingRules() const noexcept;
  };
}
