#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSImportRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSKeyframesRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSRuleList.hpp"
#include "Krystal.Booey/CSS/Media/MediaList.hpp"
#include "Krystal.Booey/CSS/Parser/Parser.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/CSS/Rules/KeyframesRule.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Node.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/StyleSheetContentsCache.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo::css
{
  namespace
  {
    KRYS_NODISCARD style::Scope &StyleScopeFor(dom::ContainerNode &treeScope) noexcept
    {
      krys_debug_assert(Is<dom::Document>(treeScope) || Is<dom::ShadowRoot>(treeScope));
      if (auto *shadowRoot = DynamicDowncast<dom::ShadowRoot>(treeScope))
      {
        return shadowRoot->StyleScope();
      }
      return Downcast<dom::Document>(treeScope).StyleScope();
    }

    class StyleSheetCSSRuleList final : public CSSRuleList
    {
    private:
      WeakPtr<CSSStyleSheet> _styleSheet;

    public:
      explicit StyleSheetCSSRuleList(CSSStyleSheet *sheet) noexcept
          : _styleSheet(WeakPtr<CSSStyleSheet>::WithRef(sheet))
      {
      }

    private:
      void AddRef() const noexcept final
      {
        _styleSheet->AddRef();
      }

      void SubRef() const noexcept final
      {
        _styleSheet->SubRef();
      }

      KRYS_NODISCARD uint32 GetRefCount() const noexcept final
      {
        return _styleSheet->GetRefCount();
      }

      KRYS_NODISCARD uint32 Length() const noexcept final
      {
        return _styleSheet->Length();
      }

      KRYS_NODISCARD CSSRule *Item(uint32 index) const noexcept final
      {
        return _styleSheet->Item(index);
      }

      KRYS_NODISCARD CSSStyleSheet *GetStyleSheet() const noexcept final
      {
        return _styleSheet.get();
      }
    };
  }

  Ref<CSSStyleSheet> CSSStyleSheet::Create(Ref<StyleSheetContents> &&contents, CSSImportRule *ownerRule,
                                           Maybe<bool> isOriginClean) noexcept
  {
    return AdoptRef(*new CSSStyleSheet(krys::move(contents), ownerRule, isOriginClean));
  }

  Ref<CSSStyleSheet> CSSStyleSheet::Create(Ref<StyleSheetContents> &&contents, dom::Node &ownerNode,
                                           Maybe<bool> isOriginClean) noexcept
  {
    return AdoptRef(*new CSSStyleSheet(krys::move(contents), ownerNode, isOriginClean));
  }

  Ref<CSSStyleSheet> CSSStyleSheet::CreateInline(Ref<StyleSheetContents> &&contents,
                                                 dom::Element &owner) noexcept
  {
    return AdoptRef(*new CSSStyleSheet(krys::move(contents), owner, true));
  }

  dom::ExceptionOr<Ref<CSSStyleSheet>> CSSStyleSheet::Create(dom::Document &document, Init &&init) noexcept
  {
    url::URL baseURL;
    if (!init.BaseURL)
    {
      baseURL = document.BaseURL();
    }
    else
    {
      baseURL = document.CompleteURL(*init.BaseURL);
      if (!baseURL.IsValid())
      {
        return dom::Exception(dom::ExceptionCode::NotAllowedError, u8"Base URL is invalid");
      }
    }

    auto parserContext = ParserContext(document, baseURL, {});
    parserContext.shouldIgnoreImportRules = true;
    return AdoptRef(
      *new CSSStyleSheet(StyleSheetContents::Create(parserContext), document, krys::move(init)));
  }

  CSSStyleSheet::CSSStyleSheet(Ref<StyleSheetContents> &&contents, CSSImportRule *ownerRule,
                               Maybe<bool> isOriginClean) noexcept
      : _contents(krys::move(contents)), _isOriginClean(isOriginClean),
        _ownerRule(WeakPtr<CSSImportRule>::WithRef(ownerRule))
  {
    if (auto *parent = ParentStyleSheet())
    {
      _styleScope = WeakPtr<style::Scope>::WithRef(parent->StyleScope());
    }
    _contents->RegisterClient(this);
  }

  CSSStyleSheet::CSSStyleSheet(Ref<StyleSheetContents> &&contents, dom::Node &ownerNode,
                               Maybe<bool> isOriginClean) noexcept
      : _contents(krys::move(contents)), _isOriginClean(isOriginClean),
        _styleScope(WeakPtr<style::Scope>::WithRef(style::Scope::ForNode(ownerNode))),
        _ownerNode(WeakPtr<dom::Node>::WithRef(ownerNode))
  {
    _contents->RegisterClient(this);
  }

  // https://w3c.github.io/csswg-drafts/cssom-1/#dom-cssstylesheet-cssstylesheet
  CSSStyleSheet::CSSStyleSheet(Ref<StyleSheetContents> &&contents, dom::Document &document,
                               Init &&options) noexcept
      : _contents(krys::move(contents)), _isDisabled(options.Disabled), _wasConstructedByJS(true),
        _isOriginClean(true), _constructorDocument(WeakPtr<dom::Document>::WithRef(document))
  {
    _contents->RegisterClient(this);
    _contents->CheckLoaded();

    SwitchOn(
      krys::move(options.Media),
      [this](RefPtr<MediaList> &&mediaList)
      {
        if (auto queries = mediaList->MediaQueries(); !queries.empty())
        {
          SetMediaQueries(krys::move(queries));
        }
      },
      [this](CSSOMString &&mediaString)
      { SetMediaQueries(mq::MediaQueryParser::Parse(mediaString, StrictParserContext())); });
  }

  CSSStyleSheet::~CSSStyleSheet() noexcept
  {
    // For style rules outside the document, .parentStyleSheet can become null even if the style rule
    // is still observable from JavaScript. This matches the behavior of .parentNode for nodes, but
    // it's not ideal because it makes the CSSOM's behavior depend on the timing of garbage collection.
    for (auto &wrapper : _childRuleCSSOMWrappers)
    {
      if (wrapper)
      {
        wrapper->SetParentStyleSheet(nullptr);
      }
    }

    if (_mediaCSSOMWrapper)
    {
      _mediaCSSOMWrapper->DetachFromParent();
    }

    _contents->UnregisterClient(this);
  }

  dom::Node *CSSStyleSheet::OwnerNode() const noexcept
  {
    return _ownerNode.get();
  }

  RefPtr<StyleRuleWithNesting> CSSStyleSheet::PrepareChildStyleRuleForNesting(StyleRule &styleRule) noexcept
  {
    RuleMutationScope scope(this);
    auto &rules = _contents->_childRules;
    for (auto &rule : rules)
    {
      if (rule.get() == &styleRule)
      {
        auto styleRuleWithNesting = StyleRuleWithNesting::Create(krys::move(styleRule));
        rule = styleRuleWithNesting;
        return styleRuleWithNesting;
      }
    }
    return nullptr;
  }

  auto CSSStyleSheet::WillMutateRules() noexcept -> ContentsClonedForMutation
  {
    // If we are the only client it is safe to mutate.
    if (_contents->HasOneClient() && !_contents->IsInMemoryCache())
    {
      _contents->SetMutable();
      return ContentsClonedForMutation::No;
    }

    // Only cacheable stylesheets should have multiple clients.
    krys_debug_assert(_contents->IsCacheable());

    // Copy-on-write.
    _contents->UnregisterClient(this);
    _contents = _contents->Copy();
    _contents->RegisterClient(this);

    _contents->SetMutable();

    // Any existing CSSOM wrappers need to be connected to the copied child rules.
    ReattachChildRuleCSSOMWrappers();

    return ContentsClonedForMutation::Yes;
  }

  void CSSStyleSheet::DidMutateRuleFromCSSStyleDeclaration() noexcept
  {
    krys_debug_assert(_contents->IsMutable());
    krys_debug_assert(_contents->HasOneClient());
    DidMutate();
  }

  // FIXME: counter-style: we might need something similar for counter-style (rdar://103018993).
  void CSSStyleSheet::DidMutateRules(RuleMutationType mutationType,
                                     ContentsClonedForMutation contentsClonedForMutation,
                                     KeyframesRule *insertedKeyframesRule,
                                     const CSSOMString &modifiedKeyframesRuleName) noexcept
  {
    krys_debug_assert(_contents->IsMutable());
    krys_debug_assert(_contents->HasOneClient());

    _contents->SetHasResolvedNesting(false);

    ForEachStyleScope(
      [&](style::Scope &scope)
      {
        if ((mutationType == RuleMutationType::RuleInsertion || mutationType == RuleMutationType::RuleReplace)
            && contentsClonedForMutation == ContentsClonedForMutation::No
            && !scope.ActiveStyleSheetsContains(*this))
        {
          if (insertedKeyframesRule)
          {
            if (auto *resolver = scope.ResolverIfExists())
            {
              resolver->AddKeyframeStyle(ShareRef(*insertedKeyframesRule));
            }
            return;
          }
          scope.DidChangeActiveStyleSheetCandidates();
          return;
        }

        if (mutationType == RuleMutationType::KeyframesRuleMutation)
        {
          if (auto *ownerDocument = OwnerDocument())
          {
            ownerDocument->KeyframesRuleDidChange(modifiedKeyframesRuleName);
          }
        }

        scope.DidChangeStyleSheetContents();

        _wasMutated = true;
      });
  }

  void CSSStyleSheet::DidMutate() noexcept
  {
    ForEachStyleScope(
      [&](style::Scope &scope)
      {
        scope.DidChangeStyleSheetContents();
        _wasMutated = true;
      });
  }

  void CSSStyleSheet::ForEachStyleScope(KRYS_NOESCAPE const Func<void(style::Scope &)> &apply) noexcept
  {
    if (auto *scope = StyleScope())
    {
      apply(*scope);
      return;
    }
    for (auto &treeScope : _adoptingTreeScopes)
    {
      if (treeScope)
      {
        apply(StyleScopeFor(*treeScope));
      }
    }
  }

  void CSSStyleSheet::ClearOwnerNode() noexcept
  {
    _ownerNode = nullptr;
  }

  CSSImportRule *CSSStyleSheet::OwnerRule() const noexcept
  {
    return _ownerRule.get();
  }

  void CSSStyleSheet::ReattachChildRuleCSSOMWrappers() noexcept
  {
    for (uint32 i = 0; i < _childRuleCSSOMWrappers.size(); ++i)
    {
      if (!_childRuleCSSOMWrappers[i])
      {
        continue;
      }
      _childRuleCSSOMWrappers[i]->Reattach(*_contents->RuleAt(i));
    }
  }

  void CSSStyleSheet::SetDisabled(bool disabled) noexcept
  {
    if (disabled == _isDisabled)
    {
      return;
    }
    _isDisabled = disabled;

    ForEachStyleScope([](style::Scope &scope) { scope.DidChangeActiveStyleSheetCandidates(); });
  }

  void CSSStyleSheet::SetMediaQueries(mq::MediaQueryList &&mediaQueries) noexcept
  {
    _mediaQueries = krys::move(mediaQueries);
  }

  uint32 CSSStyleSheet::Length() const noexcept
  {
    return _contents->RuleCount();
  }

  CSSRule *CSSStyleSheet::Item(uint32 index) noexcept
  {
    auto ruleCount = Length();
    if (index >= ruleCount)
    {
      return nullptr;
    }

    krys_debug_assert(_childRuleCSSOMWrappers.empty() || _childRuleCSSOMWrappers.size() == ruleCount);
    if (_childRuleCSSOMWrappers.size() < ruleCount)
    {
      _childRuleCSSOMWrappers.resize(ruleCount);
    }

    auto &cssRule = _childRuleCSSOMWrappers[index];
    if (!cssRule)
    {
      cssRule = _contents->RuleAt(index)->CreateCSSOMWrapper(*this);
    }
    return cssRule.get();
  }

  bool CSSStyleSheet::CanAccessRules() const noexcept
  {
    if (_isOriginClean)
    {
      return *_isOriginClean;
    }

    const auto &baseURL = _contents->BaseURL();
    if (baseURL.IsEmpty())
    {
      return true;
    }

    auto *document = OwnerDocument();
    if (!document)
    {
      return false;
    }
    return DocumentCanRequest(*document, baseURL);
  }

  dom::ExceptionOr<uint32> CSSStyleSheet::InsertRule(const CSSOMString &ruleString, uint32 index) noexcept
  {
    krys_debug_assert(_childRuleCSSOMWrappers.empty()
                      || _childRuleCSSOMWrappers.size() == _contents->RuleCount());

    if (!CanAccessRules())
    {
      return dom::Exception(dom::ExceptionCode::SecurityError,
                            u8"Not allowed to insert rule into cross-origin stylesheet");
    }

    if (index > Length())
    {
      return dom::ExceptionCode::IndexSizeError;
    }

    auto rule = Parser::ParseRule(ruleString, _contents->GetParserContext(), _contents.get(),
                                  Parser::AllowedRules::ImportRules);
    if (!rule)
    {
      return dom::ExceptionCode::SyntaxError;
    }

    if (_wasConstructedByJS && rule->IsImportRule())
    {
      return dom::Exception(dom::ExceptionCode::SyntaxError,
                            u8"Cannot inserted an @import rule in a constructed CSSStyleSheet object");
    }

    RuleMutationScope mutationScope(this, RuleMutationType::RuleInsertion,
                                    DynamicDowncast<KeyframesRule>(rule.get()));

    bool isNamespace = rule->IsNamespaceRule();
    bool success = _contents->WrapperInsertRule(Ref<Rule>(krys::move(rule)), index);
    if (!success)
    {
      if (isNamespace)
      {
        return dom::ExceptionCode::InvalidStateError;
      }
      return dom::ExceptionCode::HierarchyRequestError;
    }

    if (!_childRuleCSSOMWrappers.empty())
    {
      _childRuleCSSOMWrappers.insert(_childRuleCSSOMWrappers.begin() + index, RefPtr<CSSRule>());
    }

    return index;
  }

  dom::ExceptionOr<void> CSSStyleSheet::DeleteRule(uint32 index) noexcept
  {
    krys_debug_assert(_childRuleCSSOMWrappers.empty()
                      || _childRuleCSSOMWrappers.size() == _contents->RuleCount());

    if (!CanAccessRules())
    {
      return dom::Exception(dom::ExceptionCode::SecurityError,
                            u8"Not allowed to delete rule from cross-origin stylesheet");
    }

    if (index >= Length())
    {
      return dom::ExceptionCode::IndexSizeError;
    }

    RuleMutationScope mutationScope(this);

    bool success = _contents->WrapperDeleteRule(index);
    if (!success)
    {
      return dom::ExceptionCode::InvalidStateError;
    }

    if (!_childRuleCSSOMWrappers.empty())
    {
      if (_childRuleCSSOMWrappers[index])
      {
        _childRuleCSSOMWrappers[index]->SetParentStyleSheet(nullptr);
      }
      _childRuleCSSOMWrappers.erase(_childRuleCSSOMWrappers.begin() + index);
    }

    return {};
  }

  dom::ExceptionOr<int32> CSSStyleSheet::AddRule(const CSSOMString &selector, const CSSOMString &style,
                                                 Maybe<uint32> index) noexcept
  {
    CSSOMString text;
    text.append(selector).append(u8" { ").append(style);
    if (!style.empty())
    {
      text.push_back(u8' ');
    }
    text.push_back(u8'}');

    auto insertRuleResult = InsertRule(text, index.value_or(Length()));
    if (insertRuleResult.HasException())
    {
      return insertRuleResult.ReleaseException();
    }

    // As per Microsoft documentation, always return -1.
    return -1;
  }

  dom::ExceptionOr<void> CSSStyleSheet::RemoveRule(uint32 index) noexcept
  {
    return DeleteRule(index);
  }

  dom::ExceptionOr<Ref<CSSRuleList>> CSSStyleSheet::CssRulesForBindings() noexcept
  {
    auto cssRules = CssRules();
    if (!cssRules)
    {
      return dom::Exception(dom::ExceptionCode::SecurityError,
                            u8"Not allowed to access cross-origin stylesheet");
    }
    return Ref<CSSRuleList>(krys::move(cssRules));
  }

  dom::ExceptionOr<Ref<CSSRuleList>> CSSStyleSheet::Rules() noexcept
  {
    return CssRulesForBindings();
  }

  RefPtr<CSSRuleList> CSSStyleSheet::CssRules() noexcept
  {
    if (!CanAccessRules())
    {
      return nullptr;
    }
    return CssRulesSkippingAccessCheck();
  }

  RefPtr<CSSRuleList> CSSStyleSheet::CssRulesSkippingAccessCheck() noexcept
  {
    if (!_ruleListCSSOMWrapper)
    {
      _ruleListCSSOMWrapper = CreateUnique<StyleSheetCSSRuleList>(this);
    }
    return ShareRefPtr(_ruleListCSSOMWrapper.get());
  }

  CSSOMString CSSStyleSheet::Href() const noexcept
  {
    return _contents->OriginalURL();
  }

  url::URL CSSStyleSheet::BaseURL() const noexcept
  {
    return _contents->BaseURL();
  }

  bool CSSStyleSheet::IsLoading() const noexcept
  {
    return _contents->IsLoading();
  }

  MediaList *CSSStyleSheet::Media() const noexcept
  {
    if (!_mediaCSSOMWrapper)
    {
      _mediaCSSOMWrapper = MediaList::Create(const_cast<CSSStyleSheet *>(this));
    }
    return _mediaCSSOMWrapper.get();
  }

  CSSStyleSheet *CSSStyleSheet::ParentStyleSheet() const noexcept
  {
    auto *ownerRule = _ownerRule.get();
    return ownerRule ? ownerRule->ParentStyleSheet() : nullptr;
  }

  CSSStyleSheet &CSSStyleSheet::RootStyleSheet() noexcept
  {
    auto *root = this;
    while (root->ParentStyleSheet())
    {
      root = root->ParentStyleSheet();
    }
    return *root;
  }

  const CSSStyleSheet &CSSStyleSheet::RootStyleSheet() const noexcept
  {
    return const_cast<CSSStyleSheet &>(*this).RootStyleSheet();
  }

  dom::Document *CSSStyleSheet::OwnerDocument() const noexcept
  {
    const auto &root = RootStyleSheet();
    return root.OwnerNode() ? &root.OwnerNode()->NodeDocument() : nullptr;
  }

  style::Scope *CSSStyleSheet::StyleScope() noexcept
  {
    return _styleScope.get();
  }

  void CSSStyleSheet::ClearChildRuleCSSOMWrappers() noexcept
  {
    _childRuleCSSOMWrappers.clear();
  }

  CSSOMString CSSStyleSheet::Type() const noexcept
  {
    return u8"text/css";
  }

  CSSOMString CSSStyleSheet::CssText(const SerialisationContext &context) noexcept
  {
    auto ruleList = CssRulesSkippingAccessCheck();
    if (!ruleList)
    {
      return {};
    }

    CSSOMString result;
    for (uint32 index = 0; index < ruleList->Length(); ++index)
    {
      auto *rule = ruleList->Item(index);
      if (!rule)
      {
        continue;
      }

      auto ruleText = rule->CssText(context);
      if (!result.empty() && !ruleText.empty())
      {
        result.push_back(u8' ');
      }
      result.append(ruleText);
    }
    return result;
  }

  // https://w3c.github.io/csswg-drafts/cssom-1/#dom-cssstylesheet-replacesync
  dom::ExceptionOr<void> CSSStyleSheet::ReplaceSync(CSSOMString &&text) noexcept
  {
    if (!_wasConstructedByJS)
    {
      return dom::Exception(dom::ExceptionCode::NotAllowedError,
                            u8"This CSSStyleSheet object was not constructed by JavaScript");
    }

    // Try to use the cache in the case where contents is replaced before the stylesheet is attached to the
    // document.
    if (IsDetached() && _childRuleCSSOMWrappers.empty())
    {
      auto key = style::StyleSheetContentsCache::Key {text, _contents->GetParserContext()};
      auto cachedContents = style::StyleSheetContentsCache::Singleton().Get(key);
      if (cachedContents)
      {
        _contents->UnregisterClient(this);
        _contents = Ref<StyleSheetContents>(krys::move(cachedContents));
        _contents->RegisterClient(this);
      }
      else
      {
        _contents->ParseString(text);
        if (_contents->IsCacheable())
        {
          style::StyleSheetContentsCache::Singleton().Add(krys::move(key), _contents);
        }
      }
      return {};
    }

    RuleMutationScope mutationScope(this, RuleMutationType::RuleReplace);
    _contents->ClearRules();
    for (auto &childRuleWrapper : _childRuleCSSOMWrappers)
    {
      if (childRuleWrapper)
      {
        childRuleWrapper->SetParentStyleSheet(nullptr);
      }
    }
    _childRuleCSSOMWrappers.clear();
    _contents->ParseString(text);
    return {};
  }

  bool CSSStyleSheet::IsDetached() const noexcept
  {
    return !_ownerNode && !_ownerRule
           && std::ranges::none_of(_adoptingTreeScopes,
                                   [](const WeakPtr<dom::ContainerNode> &treeScope) { return !!treeScope; });
  }

  dom::Document *CSSStyleSheet::ConstructorDocument() const noexcept
  {
    return _constructorDocument.get();
  }

  void CSSStyleSheet::AddAdoptingTreeScope(dom::ContainerNode &treeScope) noexcept
  {
    krys_debug_assert(Is<dom::Document>(treeScope) || Is<dom::ShadowRoot>(treeScope));
    std::erase_if(_adoptingTreeScopes, [](const WeakPtr<dom::ContainerNode> &scope) { return !scope; });
    if (std::ranges::none_of(_adoptingTreeScopes, [&](const WeakPtr<dom::ContainerNode> &scope)
                             { return scope.get() == &treeScope; }))
    {
      _adoptingTreeScopes.push_back(WeakPtr<dom::ContainerNode>::WithRef(treeScope));
    }
    StyleScopeFor(treeScope).DidChangeActiveStyleSheetCandidates();
  }

  void CSSStyleSheet::RemoveAdoptingTreeScope(dom::ContainerNode &treeScope) noexcept
  {
    krys_debug_assert(Is<dom::Document>(treeScope) || Is<dom::ShadowRoot>(treeScope));
    std::erase_if(_adoptingTreeScopes, [&](const WeakPtr<dom::ContainerNode> &scope)
                  { return !scope || scope.get() == &treeScope; });
    StyleScopeFor(treeScope).DidChangeStyleSheetContents();
  }

  Ref<StyleSheetContents> CSSStyleSheet::ProtectedContents() noexcept
  {
    return _contents;
  }

  void CSSStyleSheet::GetChildStyleSheets(HashSet<RefPtr<CSSStyleSheet>> &childStyleSheets) noexcept
  {
    auto ruleList = CssRules();
    if (!ruleList)
    {
      return;
    }

    for (uint32 index = 0; index < ruleList->Length(); ++index)
    {
      if (auto *rule = ruleList->Item(index))
      {
        rule->GetChildStyleSheets(childStyleSheets);
      }
    }
  }

  CSSStyleSheet::RuleMutationScope::RuleMutationScope(CSSStyleSheet *sheet, RuleMutationType mutationType,
                                                      KeyframesRule *insertedKeyframesRule) noexcept
      : _styleSheet(ShareRefPtr(sheet)), _mutationType(mutationType),
        _contentsClonedForMutation(ContentsClonedForMutation::No),
        _insertedKeyframesRule(ShareRefPtr(insertedKeyframesRule))
  {
    krys_debug_assert(_styleSheet);
    _contentsClonedForMutation = _styleSheet->WillMutateRules();
  }

  CSSStyleSheet::RuleMutationScope::RuleMutationScope(CSSRule *rule) noexcept
      : _styleSheet(ShareRefPtr(rule ? rule->ParentStyleSheet() : nullptr)),
        _mutationType(Is<CSSKeyframesRule>(rule) ? RuleMutationType::KeyframesRuleMutation
                                                 : RuleMutationType::OtherMutation),
        _contentsClonedForMutation(ContentsClonedForMutation::No), _insertedKeyframesRule(nullptr),
        _modifiedKeyframesRuleName(
          [rule]
          {
            auto *cssKeyframeRule = DynamicDowncast<CSSKeyframesRule>(rule);
            return cssKeyframeRule ? CSSOMString(cssKeyframeRule->Name().View()) : CSSOMString();
          }())
  {
    if (_styleSheet)
    {
      _contentsClonedForMutation = _styleSheet->WillMutateRules();
    }
  }

  CSSStyleSheet::RuleMutationScope::~RuleMutationScope() noexcept
  {
    if (_styleSheet)
    {
      _styleSheet->DidMutateRules(_mutationType, _contentsClonedForMutation, _insertedKeyframesRule.get(),
                                  _modifiedKeyframesRuleName);
      _styleSheet->Contents().ClearHasNestingRulesCache();
    }
  }
}
