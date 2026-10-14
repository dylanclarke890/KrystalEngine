#pragma once

#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Loader/LoadedFromOpaqueSource.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class CachedResource;
}

namespace krys::boo::dom
{
  class Document;
  class Node;
}

namespace krys::boo::css
{
  class CSSStyleSheet;
  class ImportRule;
  class LayerRule;
  class NamespaceRule;
  class Rule;

  /// @brief The parsed contents of a style sheet: its rules and namespaces, shared between the CSSStyleSheet
  /// objects that use it.
  // FIXME: Loading (ParseAuthorStyleSheet, NotifyLoadedSheet, SubresourcesAllowReuse, IsLoadingSubresources)
  // comes with the resource loader.
  class StyleSheetContents final : public RefCounted<StyleSheetContents>,
                                   public CanMakeWeakPtr<StyleSheetContents>
  {
    friend class CSSStyleSheet;
    using PrefixNamespaceURIMap = HashMap<CSSOMStringAtom, CSSOMStringAtom>;

  private:
    ImportRule *_ownerRule {nullptr};

    CSSOMString _originalURL;

    CSSOMString _encodingFromCharsetRule;
    List<Ref<LayerRule>> _layerRulesBeforeImportRules;
    List<Ref<ImportRule>> _importRules;
    List<Ref<NamespaceRule>> _namespaceRules;
    List<Ref<Rule>> _childRules;
    PrefixNamespaceURIMap _namespaces;
    CSSOMStringAtom _defaultNamespace;

    bool _isUserStyleSheet;
    bool _loadCompleted {false};
    bool _hasSyntacticallyValidCSSHeader {true};
    bool _didLoadErrorOccur {false};
    bool _usesStyleBasedEditability {false};
    bool _isMutable {false};
    mutable bool _hasResolvedNesting {false};
    mutable Maybe<bool> _hasNestingRulesCache;
    uint32 _inMemoryCacheCount {0};

    ParserContext _parserContext;

    List<CSSStyleSheet *> _clients;

    StyleSheetContents(ImportRule *ownerRule, const CSSOMString &originalURL,
                       const ParserContext &context) noexcept;
    StyleSheetContents(const StyleSheetContents &other) noexcept;

  public:
    ~StyleSheetContents() noexcept;

    KRYS_NODISCARD static Ref<StyleSheetContents> Create() noexcept;
    KRYS_NODISCARD static Ref<StyleSheetContents> Create(const ParserContext &context) noexcept;
    KRYS_NODISCARD static Ref<StyleSheetContents> Create(const CSSOMString &originalURL,
                                                         const ParserContext &context) noexcept;
    KRYS_NODISCARD static Ref<StyleSheetContents>
      Create(ImportRule *ownerRule, const CSSOMString &originalURL, const ParserContext &context) noexcept;

    KRYS_NODISCARD const ParserContext &GetParserContext() const noexcept
    {
      return _parserContext;
    }

    KRYS_NODISCARD const CSSOMStringAtom &DefaultNamespace() const noexcept
    {
      return _defaultNamespace;
    }

    KRYS_NODISCARD const CSSOMStringAtom &
      NamespaceURIFromPrefix(const CSSOMStringAtom &prefix) const noexcept;

    /// @brief Parses @p sheetText into this sheet.
    bool ParseString(const CSSOMString &sheetText) noexcept;

    KRYS_NODISCARD bool IsCacheable() const noexcept;
    KRYS_NODISCARD bool IsCacheableWithNoBaseURLDependency() const noexcept;

    KRYS_NODISCARD bool IsLoading() const noexcept;

    void CheckLoaded() noexcept;
    void StartLoadingDynamicSheet() noexcept;

    KRYS_NODISCARD StyleSheetContents *RootStyleSheet() const noexcept;
    KRYS_NODISCARD dom::Node *SingleOwnerNode() const noexcept;
    KRYS_NODISCARD dom::Document *SingleOwnerDocument() const noexcept;

    KRYS_NODISCARD utf8_stringview Charset() const noexcept
    {
      return _parserContext.Charset;
    }

    KRYS_NODISCARD bool LoadCompleted() const noexcept
    {
      return _loadCompleted;
    }

    KRYS_NODISCARD bool MayDependOnBaseURL() const noexcept;

    /// @brief Calls @p handler on every rule (the imported sheets' too, after each import rule) until it
    /// returns true.
    /// @return Whether @p handler returned true.
    bool TraverseRules(KRYS_NOESCAPE const Func<bool(const Rule &)> &handler) const noexcept;

    /// @brief Calls @p handler on every resource the rules load until it returns true.
    /// @return Whether @p handler returned true.
    bool TraverseSubresources(KRYS_NOESCAPE const Func<bool(const CachedResource &)> &handler) const noexcept;

    void SetIsUserStyleSheet(bool value) noexcept
    {
      _isUserStyleSheet = value;
    }

    KRYS_NODISCARD bool IsUserStyleSheet() const noexcept
    {
      return _isUserStyleSheet;
    }

    void SetHasSyntacticallyValidCSSHeader(bool value) noexcept
    {
      _hasSyntacticallyValidCSSHeader = value;
    }

    KRYS_NODISCARD bool HasSyntacticallyValidCSSHeader() const noexcept
    {
      return _hasSyntacticallyValidCSSHeader;
    }

    void ParserAddNamespace(const CSSOMStringAtom &prefix, const CSSOMStringAtom &uri) noexcept;
    void ParserAppendRule(Ref<Rule> &&rule) noexcept;
    void ParserSetEncodingFromCharsetRule(const CSSOMString &encoding) noexcept;

    void ParserSetUsesStyleBasedEditability() noexcept
    {
      _usesStyleBasedEditability = true;
    }

    void ClearRules() noexcept;

    KRYS_NODISCARD CSSOMString EncodingFromCharsetRule() const noexcept
    {
      return _encodingFromCharsetRule;
    }

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

    KRYS_NODISCARD const List<Ref<Rule>> &ChildRules() const noexcept
    {
      return _childRules;
    }

    KRYS_NODISCARD StyleSheetContents *ParentStyleSheet() const noexcept;

    KRYS_NODISCARD ImportRule *OwnerRule() const noexcept
    {
      return _ownerRule;
    }

    void ClearOwnerRule() noexcept
    {
      _ownerRule = nullptr;
    }

    /// @note This is the URL that started the redirect chain that led to this style sheet. This property
    /// probably isn't useful for much except the JavaScript binding (which needs to use this value for
    /// security).
    KRYS_NODISCARD CSSOMString OriginalURL() const noexcept
    {
      return _originalURL;
    }

    KRYS_NODISCARD const url::URL &BaseURL() const noexcept
    {
      return _parserContext.BaseURL;
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return !RuleCount();
    }

    KRYS_NODISCARD uint32 RuleCount() const noexcept;

    /// @brief The rule at @p index, counting the early layer statements, then the import rules, then the
    /// namespace rules, then the other rules.
    KRYS_NODISCARD Rule *RuleAt(uint32 index) const noexcept;

    KRYS_NODISCARD bool UsesStyleBasedEditability() const noexcept
    {
      return _usesStyleBasedEditability;
    }

    /// @brief A rough size estimate for the memory cache.
    KRYS_NODISCARD uint32 EstimatedSizeInBytes() const noexcept;

    /// @return False, inserting nothing, if @p rule isn't allowed at @p index.
    bool WrapperInsertRule(Ref<Rule> &&rule, uint32 index) noexcept;

    /// @return False, deleting nothing, if the rule at @p index can't be deleted.
    bool WrapperDeleteRule(uint32 index) noexcept;

    KRYS_NODISCARD Ref<StyleSheetContents> Copy() const noexcept;

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

    KRYS_NODISCARD bool IsMutable() const noexcept
    {
      return _isMutable;
    }

    void SetMutable() noexcept
    {
      _isMutable = true;
    }

    KRYS_NODISCARD bool HasNestingRules() const noexcept;

    void ClearHasNestingRulesCache() noexcept
    {
      _hasNestingRulesCache = {};
    }

    KRYS_NODISCARD bool IsInMemoryCache() const noexcept
    {
      return _inMemoryCacheCount;
    }

    void AddedToMemoryCache() noexcept;
    void RemovedFromMemoryCache() noexcept;

    void ShrinkToFit() noexcept;

    void SetAsLoadedFromOpaqueSource() noexcept
    {
      _parserContext.LoadedFromOpaqueSource = boo::LoadedFromOpaqueSource::Yes;
    }

    KRYS_NODISCARD boo::LoadedFromOpaqueSource LoadedFromOpaqueSource() const noexcept
    {
      return _parserContext.LoadedFromOpaqueSource;
    }

    void SetLoadErrorOccured() noexcept
    {
      _didLoadErrorOccur = true;
    }

    KRYS_NODISCARD bool HasResolvedNesting() const noexcept
    {
      return _hasResolvedNesting;
    }

    void SetHasResolvedNesting(bool value) const noexcept
    {
      _hasResolvedNesting = value;
    }

  private:
    void ClearCharsetRule() noexcept;
  };
}
