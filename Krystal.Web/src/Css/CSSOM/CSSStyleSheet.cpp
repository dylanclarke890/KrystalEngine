#include "Krystal.Web/Css/CSSOM/CSSStyleSheet.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Visitor.hpp"
#include "Krystal.Web/Css/CSSOM/CSSImportRule.hpp"
#include "Krystal.Web/Css/CSSOM/CSSKeyframesRule.hpp"
#include "Krystal.Web/Css/CSSOM/CSSRuleList.hpp"
#include "Krystal.Web/Css/CSSOM/MediaList.hpp"
#include "Krystal.Web/Css/Parser/Parser.hpp"
#include "Krystal.Web/Css/Parser/ParserContext.hpp"
#include "Krystal.Web/Css/Queries/MediaQueryParser.hpp"
#include "Krystal.Web/Css/Rules/KeyframesRule.hpp"
#include "Krystal.Web/Css/Rules/Rule.hpp"
#include "Krystal.Web/Css/StyleSheets/StyleSheetContents.hpp"
#include "Krystal.Web/Css/Values/SerialisationContext.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Dom/Nodes/Node.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Dom/Types/ExceptionOr.hpp"
#include "Krystal.Web/Style/Resolver.hpp"
#include "Krystal.Web/Style/Rules/StyleSheetContentsCache.hpp"
#include "Krystal.Web/Style/Scope/Scope.hpp"

namespace krys::web
{
  namespace
  {
    /// @brief The style scope of a document or shadow root that adopted a sheet.
    KRYS_NODISCARD style::Scope &AdoptingStyleScope(ContainerNode &treeScope) noexcept
    {
      if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(treeScope))
      {
        return shadowRoot->StyleScope();
      }
      return Downcast<Document>(treeScope).StyleScope();
    }

    /// @brief A style sheet's live list of rules, which keeps the sheet alive while it's referenced.
    class StyleSheetRuleList final : public CSSRuleList
    {
    private:
      WeakPtr<CSSStyleSheet> _styleSheet;

    public:
      explicit StyleSheetRuleList(CSSStyleSheet &styleSheet) noexcept
          : _styleSheet(CreateWeakPtr(&styleSheet))
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

  CSSStyleSheet::CSSStyleSheet(Ref<css::StyleSheetContents> &&contents, CSSImportRule *ownerRule,
                               bool isOriginClean) noexcept
      : _contents(krys::move(contents)), _isOriginClean(isOriginClean), _ownerRule(CreateWeakPtr(ownerRule))
  {
    if (auto *parent = ParentStyleSheet())
    {
      _styleScope = CreateWeakPtr(parent->StyleScope());
    }
    _contents->RegisterClient(this);
  }

  CSSStyleSheet::CSSStyleSheet(Ref<css::StyleSheetContents> &&contents, Node &ownerNode,
                               bool isOriginClean) noexcept
      : _contents(krys::move(contents)), _isOriginClean(isOriginClean),
        _styleScope(CreateWeakPtr(&style::Scope::ForNode(ownerNode))), _ownerNode(CreateWeakPtr(&ownerNode))
  {
    _contents->RegisterClient(this);
  }

  CSSStyleSheet::CSSStyleSheet(Ref<css::StyleSheetContents> &&contents, Document &document,
                               Init options) noexcept
      : _contents(krys::move(contents)), _isDisabled(options.Disabled), _wasConstructedByJS(true),
        _constructorDocument(CreateWeakPtr(&document))
  {
    _contents->RegisterClient(this);
    _contents->CheckLoaded();
    // The media are the given media list's, or the given string's.
    SwitchOn(
      krys::move(options.Media),
      [this](const RefPtr<MediaList> &media)
      {
        if (auto queries = media->MediaQueries(); !queries.empty())
        {
          SetMediaQueries(krys::move(queries));
        }
      },
      [this](const CSSOMString &media)
      { SetMediaQueries(css::mq::MediaQueryParser::Parse(media, css::StrictParserContext())); });
  }

  CSSStyleSheet::~CSSStyleSheet() noexcept
  {
    // The rules' wrappers and the media list can outlive the sheet, which is no longer their parent.
    for (auto &rule : _childRules)
    {
      if (rule)
      {
        rule->SetParentStyleSheet(nullptr);
      }
    }
    if (_media)
    {
      _media->DetachFromParent();
    }
    _contents->UnregisterClient(this);
  }

  Ref<CSSStyleSheet> CSSStyleSheet::Create(Ref<css::StyleSheetContents> &&contents, CSSImportRule *ownerRule,
                                           bool isOriginClean) noexcept
  {
    return AdoptRef(*new CSSStyleSheet(krys::move(contents), ownerRule, isOriginClean));
  }

  Ref<CSSStyleSheet> CSSStyleSheet::Create(Ref<css::StyleSheetContents> &&contents, Node &ownerNode,
                                           bool isOriginClean) noexcept
  {
    return AdoptRef(*new CSSStyleSheet(krys::move(contents), ownerNode, isOriginClean));
  }

  Ref<CSSStyleSheet> CSSStyleSheet::CreateInline(Ref<css::StyleSheetContents> &&contents,
                                                 Element &owner) noexcept
  {
    return AdoptRef(*new CSSStyleSheet(krys::move(contents), owner, true));
  }

  ExceptionOr<Ref<CSSStyleSheet>> CSSStyleSheet::Create(Document &document, Init init) noexcept
  {
    // The sheet's base URL is the given one, resolved against the document's base URL, or the document's.
    net::Url baseUrl = document.BaseUrl();
    if (init.BaseUrl)
    {
      baseUrl = document.CompleteUrl(*init.BaseUrl);
      if (!baseUrl.IsValid())
      {
        return Exception(ExceptionCode::NotAllowedError, u8"The base URL is invalid");
      }
    }
    // A constructed sheet has no @import rules.
    auto parserContext = css::ParserContext(document, baseUrl, {});
    parserContext.shouldIgnoreImportRules = true;
    return AdoptRef(
      *new CSSStyleSheet(css::StyleSheetContents::Create(parserContext), document, krys::move(init)));
  }

  CSSOMString CSSStyleSheet::Href() const noexcept
  {
    return _contents->OriginalUrl();
  }

  Node *CSSStyleSheet::OwnerNode() const noexcept
  {
    return _ownerNode.get();
  }

  CSSStyleSheet *CSSStyleSheet::ParentStyleSheet() const noexcept
  {
    auto *ownerRule = _ownerRule.get();
    return ownerRule ? ownerRule->ParentStyleSheet() : nullptr;
  }

  MediaList *CSSStyleSheet::Media() const noexcept
  {
    // An imported sheet's media are its `@import` rule's.
    if (auto *ownerRule = _ownerRule.get())
    {
      return &ownerRule->Media();
    }
    if (!_media)
    {
      _media = MediaList::Create(const_cast<CSSStyleSheet *>(this));
    }
    return _media.get();
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

  CSSImportRule *CSSStyleSheet::OwnerRule() const noexcept
  {
    return _ownerRule.get();
  }

  RefPtr<CSSRuleList> CSSStyleSheet::CssRules() noexcept
  {
    return CanAccessRules() ? CssRulesSkippingAccessCheck() : nullptr;
  }

  ExceptionOr<Ref<CSSRuleList>> CSSStyleSheet::CssRulesForBindings() noexcept
  {
    // 1. If the origin-clean flag is unset, throw a SecurityError exception.
    if (!CanAccessRules())
    {
      return Exception(ExceptionCode::SecurityError, u8"The style sheet's rules can't be accessed");
    }

    // 2. Return a read-only, live CSSRuleList object representing the CSS rules.
    return Ref<CSSRuleList>(CssRulesSkippingAccessCheck());
  }

  ExceptionOr<uint32> CSSStyleSheet::InsertRule(const CSSOMString &rule, uint32 index) noexcept
  {
    krys_debug_assert(_childRules.empty() || _childRules.size() == Length());

    // 1. If the origin-clean flag is unset, throw a SecurityError exception.
    if (!CanAccessRules())
    {
      return Exception(ExceptionCode::SecurityError, u8"The style sheet's rules can't be changed");
    }

    // 2. (Skipped: without replace(), the disallow modification flag is never set.)

    // 3. Let parsed rule be the return value of invoking parse a rule with rule.
    auto parsedRule = css::Parser::ParseRule(rule, _contents->GetParserContext(), _contents.get(),
                                             css::Parser::AllowedRules::ImportRules);

    // 4. If parsed rule is a syntax error, throw a SyntaxError DOMException.
    if (!parsedRule)
    {
      return ExceptionCode::SyntaxError;
    }

    // 5. If parsed rule is an @import rule, and the constructed flag is set, throw a SyntaxError
    //    DOMException.
    if (_wasConstructedByJS && parsedRule->IsImportRule())
    {
      return Exception(ExceptionCode::SyntaxError, u8"A constructed style sheet can't have @import rules");
    }

    // 6. Return the result of invoking insert a CSS rule rule in the CSS rules at index.
    // To insert a CSS rule rule in a CSS rule list list at index index:
    // 1. Set length to the number of items in list.
    // 2. If index is greater than length, then throw an IndexSizeError exception.
    if (index > Length())
    {
      return ExceptionCode::IndexSizeError;
    }

    // 3.-5. (The rule was parsed above.)
    // 6. If new rule cannot be inserted into list at the zero-indexed position index due to constraints
    //    specified by CSS, then throw a HierarchyRequestError exception.
    // 7. If new rule is an @namespace at-rule, and list contains anything other than @import at-rules, and
    //    @namespace at-rules, throw an InvalidStateError exception.
    // 8. Insert new rule into list at the zero-indexed position index.
    // (The contents refuse a rule in the wrong place: an @namespace rule there is an InvalidStateError, as
    // in Chrome.)
    RuleMutationScope mutationScope(this, RuleMutationType::RuleInsertion,
                                    DynamicDowncast<css::KeyframesRule>(parsedRule.get()));
    bool isNamespaceRule = parsedRule->IsNamespaceRule();
    if (!_contents->WrapperInsertRule(krys::move(parsedRule), index))
    {
      return isNamespaceRule ? ExceptionCode::InvalidStateError : ExceptionCode::HierarchyRequestError;
    }
    if (!_childRules.empty())
    {
      _childRules.insert(_childRules.begin() + index, nullptr);
    }

    // 9. Return index.
    return index;
  }

  ExceptionOr<void> CSSStyleSheet::DeleteRule(uint32 index) noexcept
  {
    krys_debug_assert(_childRules.empty() || _childRules.size() == Length());

    // 1. If the origin-clean flag is unset, throw a SecurityError exception.
    if (!CanAccessRules())
    {
      return Exception(ExceptionCode::SecurityError, u8"The style sheet's rules can't be changed");
    }

    // 2. (Skipped: without replace(), the disallow modification flag is never set.)

    // 3. Remove a CSS rule in the CSS rules at index.
    // To remove a CSS rule from a CSS rule list list at index index:
    // 1. Set length to the number of items in list.
    // 2. If index is greater than or equal to length, then throw an IndexSizeError exception.
    if (index >= Length())
    {
      return ExceptionCode::IndexSizeError;
    }

    // 3. Set old rule to the indexth item in list.
    // 4. If old rule is an @namespace at-rule, and list contains anything other than @import at-rules, and
    //    @namespace at-rules, throw an InvalidStateError exception.
    // 5. Remove rule old rule from list at the zero-indexed position index.
    RuleMutationScope mutationScope(this);
    if (!_contents->WrapperDeleteRule(index))
    {
      return ExceptionCode::InvalidStateError;
    }

    // 6. Set old rule's parent CSS rule and parent CSS style sheet to null.
    if (!_childRules.empty())
    {
      if (auto &oldRule = _childRules[index])
      {
        oldRule->SetParentStyleSheet(nullptr);
      }
      _childRules.erase(_childRules.begin() + index);
    }
    return {};
  }

  ExceptionOr<void> CSSStyleSheet::ReplaceSync(const CSSOMString &text) noexcept
  {
    // To synchronously replace the rules of a CSSStyleSheet on sheet given text:
    // 1. If the constructed flag is not set, or the disallow modification flag is set, throw a
    //    NotAllowedError DOMException.
    if (!_wasConstructedByJS)
    {
      return Exception(ExceptionCode::NotAllowedError,
                       u8"Only a constructed style sheet's rules can be replaced");
    }

    // 2. Let rules be the result of running parse a stylesheet's contents from text.
    // 3. If rules contains one or more @import rules, remove those rules from rules.
    // 4. Set sheet's CSS rules to rules.
    // (A constructed sheet's parser ignores @import rules.)

    // A sheet nothing uses, whose rules no script holds, can share contents parsed from the same text.
    if (IsDetached() && _childRules.empty())
    {
      auto key = style::StyleSheetContentsCache::Key {text, _contents->GetParserContext()};
      if (auto cached = style::StyleSheetContentsCache::Singleton().Get(key))
      {
        _contents->UnregisterClient(this);
        _contents = krys::move(cached);
        _contents->RegisterClient(this);
        return {};
      }
      _contents->ParseString(text);
      if (_contents->IsCacheable())
      {
        style::StyleSheetContentsCache::Singleton().Add(krys::move(key), _contents);
      }
      return {};
    }

    RuleMutationScope mutationScope(this, RuleMutationType::RuleReplace);
    _contents->ClearRules();
    for (auto &rule : _childRules)
    {
      if (rule)
      {
        rule->SetParentStyleSheet(nullptr);
      }
    }
    _childRules.clear();
    _contents->ParseString(text);
    return {};
  }

  ExceptionOr<Ref<CSSRuleList>> CSSStyleSheet::Rules() noexcept
  {
    return CssRulesForBindings();
  }

  ExceptionOr<int32> CSSStyleSheet::AddRule(const CSSOMString &selector, const CSSOMString &block,
                                            Maybe<uint32> optionalIndex) noexcept
  {
    // 1. Let rule be an empty string.
    // 2. Append selector to rule.
    // 3. Append " { " to rule.
    CSSOMString rule = selector;
    rule.append(u8" { ");

    // 4. If block is not empty, append block, followed by a space, to rule.
    if (!block.empty())
    {
      rule.append(block).push_back(u8' ');
    }

    // 5. Append "}" to rule
    rule.push_back(u8'}');

    // 6. Let index be optionalIndex if provided, or the number of CSS rules in the stylesheet otherwise.
    auto index = optionalIndex.value_or(Length());

    // 7. Call insertRule(), with rule and index as arguments.
    if (auto result = InsertRule(rule, index); result.HasException())
    {
      return result.ReleaseException();
    }

    // 8. Return -1.
    return -1;
  }

  ExceptionOr<void> CSSStyleSheet::RemoveRule(uint32 index) noexcept
  {
    return DeleteRule(index);
  }

  Document *CSSStyleSheet::ConstructorDocument() const noexcept
  {
    return _constructorDocument.get();
  }

  void CSSStyleSheet::AddAdoptingTreeScope(ContainerNode &treeScope) noexcept
  {
    krys_debug_assert(Is<Document>(treeScope) || Is<ShadowRoot>(treeScope));
    std::erase_if(_adoptingTreeScopes, [](const WeakPtr<ContainerNode> &scope) { return !scope; });
    if (std::ranges::none_of(_adoptingTreeScopes,
                             [&](const WeakPtr<ContainerNode> &scope) { return scope.get() == &treeScope; }))
    {
      _adoptingTreeScopes.push_back(CreateWeakPtr(&treeScope));
    }
    AdoptingStyleScope(treeScope).DidChangeActiveStyleSheetCandidates();
  }

  void CSSStyleSheet::RemoveAdoptingTreeScope(ContainerNode &treeScope) noexcept
  {
    krys_debug_assert(Is<Document>(treeScope) || Is<ShadowRoot>(treeScope));
    std::erase_if(_adoptingTreeScopes,
                  [&](const WeakPtr<ContainerNode> &scope) { return !scope || scope.get() == &treeScope; });
    AdoptingStyleScope(treeScope).DidChangeStyleSheetContents();
  }

  uint32 CSSStyleSheet::Length() const noexcept
  {
    return _contents->RuleCount();
  }

  CSSRule *CSSStyleSheet::Item(uint32 index) noexcept
  {
    auto length = Length();
    if (index >= length)
    {
      return nullptr;
    }
    // The wrappers' list is made, at the rules' length, when one is first asked for.
    krys_debug_assert(_childRules.empty() || _childRules.size() == length);
    _childRules.resize(length);
    auto &rule = _childRules[index];
    if (!rule)
    {
      rule = _contents->RuleAt(index)->CreateCSSOMWrapper(*this);
    }
    return rule.get();
  }

  void CSSStyleSheet::ClearOwnerNode() noexcept
  {
    _ownerNode = nullptr;
  }

  net::Url CSSStyleSheet::BaseUrl() const noexcept
  {
    return _contents->BaseUrl();
  }

  bool CSSStyleSheet::IsLoading() const noexcept
  {
    return _contents->IsLoading();
  }

  Document *CSSStyleSheet::OwnerDocument() const noexcept
  {
    auto *ownerNode = RootStyleSheet().OwnerNode();
    return ownerNode ? &ownerNode->NodeDocument() : nullptr;
  }

  CSSStyleSheet &CSSStyleSheet::RootStyleSheet() noexcept
  {
    auto *root = this;
    while (auto *parent = root->ParentStyleSheet())
    {
      root = parent;
    }
    return *root;
  }

  const CSSStyleSheet &CSSStyleSheet::RootStyleSheet() const noexcept
  {
    return const_cast<CSSStyleSheet &>(*this).RootStyleSheet();
  }

  style::Scope *CSSStyleSheet::StyleScope() noexcept
  {
    return _styleScope.get();
  }

  void CSSStyleSheet::SetMediaQueries(css::mq::MediaQueryList &&mediaQueries) noexcept
  {
    _mediaQueries = krys::move(mediaQueries);
  }

  RefPtr<css::StyleRuleWithNesting>
    CSSStyleSheet::PrepareChildStyleRuleForNesting(css::StyleRule &styleRule) noexcept
  {
    RuleMutationScope mutationScope(this);
    for (auto &rule : _contents->_childRules)
    {
      if (rule.get() == &styleRule)
      {
        auto ruleWithNesting = css::StyleRuleWithNesting::Create(krys::move(styleRule));
        rule = ruleWithNesting;
        return ruleWithNesting;
      }
    }
    return nullptr;
  }

  auto CSSStyleSheet::WillMutateRules() noexcept -> ContentsClonedForMutation
  {
    // Contents only this sheet uses, and that aren't in the memory cache, can be changed in place.
    if (_contents->HasOneClient() && !_contents->IsInMemoryCache())
    {
      _contents->SetMutable();
      return ContentsClonedForMutation::No;
    }

    // Shared contents are copied, and the wrappers moved to the copy's rules.
    krys_debug_assert(_contents->IsCacheable());
    _contents->UnregisterClient(this);
    _contents = _contents->Copy();
    _contents->RegisterClient(this);
    _contents->SetMutable();
    ReattachChildRuleCSSOMWrappers();
    return ContentsClonedForMutation::Yes;
  }

  void CSSStyleSheet::DidMutateRules(RuleMutationType mutationType,
                                     ContentsClonedForMutation contentsClonedForMutation,
                                     css::KeyframesRule *insertedKeyframesRule,
                                     const CSSOMString &modifiedKeyframesRuleName) noexcept
  {
    krys_debug_assert(_contents->IsMutable());
    krys_debug_assert(_contents->HasOneClient());
    ForEachStyleScope(
      [&](style::Scope &scope)
      {
        // Rules added to a sheet that isn't active yet only make it a candidate again; inserted keyframes go
        // straight to the resolver.
        bool isInsertion =
          mutationType == RuleMutationType::RuleInsertion || mutationType == RuleMutationType::RuleReplace;
        if (isInsertion && contentsClonedForMutation == ContentsClonedForMutation::No
            && !scope.ActiveStyleSheetsContains(*this))
        {
          if (!insertedKeyframesRule)
          {
            scope.DidChangeActiveStyleSheetCandidates();
          }
          else if (auto *resolver = scope.ResolverIfExists())
          {
            resolver->AddKeyframeStyle(ShareRef(*insertedKeyframesRule));
          }
          return;
        }

        if (mutationType == RuleMutationType::KeyframesRuleMutation)
        {
          if (auto *document = OwnerDocument())
          {
            document->KeyframesRuleDidChange(modifiedKeyframesRuleName);
          }
        }
        scope.DidChangeStyleSheetContents();
        _wasMutated = true;
      });
  }

  void CSSStyleSheet::DidMutateRuleFromCSSStyleDeclaration() noexcept
  {
    krys_debug_assert(_contents->IsMutable());
    krys_debug_assert(_contents->HasOneClient());
    DidMutate();
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

  void CSSStyleSheet::ClearChildRuleCSSOMWrappers() noexcept
  {
    _childRules.clear();
  }

  void CSSStyleSheet::ReattachChildRuleCSSOMWrappers() noexcept
  {
    for (uint32 index = 0; index < _childRules.size(); ++index)
    {
      if (auto &rule = _childRules[index])
      {
        rule->Reattach(*_contents->RuleAt(index));
      }
    }
  }

  CSSOMString CSSStyleSheet::CssText(const css::SerialisationContext &context) noexcept
  {
    // The rules' serialisations, separated by spaces.
    CSSOMString text;
    for (uint32 index = 0; index < Length(); ++index)
    {
      auto ruleText = Item(index)->CssText(context);
      if (!text.empty() && !ruleText.empty())
      {
        text.push_back(u8' ');
      }
      text.append(ruleText);
    }
    return text;
  }

  void CSSStyleSheet::GetChildStyleSheets(HashSet<RefPtr<CSSStyleSheet>> &childStyleSheets) noexcept
  {
    if (!CanAccessRules())
    {
      return;
    }
    for (uint32 index = 0; index < Length(); ++index)
    {
      Item(index)->GetChildStyleSheets(childStyleSheets);
    }
  }

  bool CSSStyleSheet::IsDetached() const noexcept
  {
    return !_ownerNode && !_ownerRule
           && std::ranges::none_of(_adoptingTreeScopes,
                                   [](const WeakPtr<ContainerNode> &treeScope) { return !!treeScope; });
  }

  void CSSStyleSheet::ForEachStyleScope(KRYS_NOESCAPE const Func<void(style::Scope &)> &apply) noexcept
  {
    // A sheet in a document or shadow root updates its scope; an adopted sheet, every scope that adopted it.
    if (auto *scope = StyleScope())
    {
      apply(*scope);
      return;
    }
    for (auto &treeScope : _adoptingTreeScopes)
    {
      if (treeScope)
      {
        apply(AdoptingStyleScope(*treeScope));
      }
    }
  }

  CSSOMString CSSStyleSheet::Type() const noexcept
  {
    return u8"text/css";
  }

  RefPtr<CSSRuleList> CSSStyleSheet::CssRulesSkippingAccessCheck() noexcept
  {
    if (!_cssRules)
    {
      _cssRules = CreateUnique<StyleSheetRuleList>(*this);
    }
    return ShareRefPtr(_cssRules.get());
  }

  CSSStyleSheet::RuleMutationScope::RuleMutationScope(CSSStyleSheet *styleSheet,
                                                      RuleMutationType mutationType,
                                                      css::KeyframesRule *insertedKeyframesRule) noexcept
      : _styleSheet(ShareRefPtr(styleSheet)), _mutationType(mutationType),
        _insertedKeyframesRule(ShareRefPtr(insertedKeyframesRule))
  {
    krys_debug_assert(_styleSheet);
    _contentsClonedForMutation = _styleSheet->WillMutateRules();
  }

  CSSStyleSheet::RuleMutationScope::RuleMutationScope(CSSRule *rule) noexcept
      : _styleSheet(ShareRefPtr(rule ? rule->ParentStyleSheet() : nullptr)),
        _mutationType(Is<CSSKeyframesRule>(rule) ? RuleMutationType::KeyframesRuleMutation
                                                 : RuleMutationType::OtherMutation)
  {
    if (auto *keyframesRule = DynamicDowncast<CSSKeyframesRule>(rule))
    {
      _modifiedKeyframesRuleName = keyframesRule->Name().ToString();
    }
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
